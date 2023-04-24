// Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

#include "ringbuffer.hpp"

#include "warning-disable.hpp"
#include <algorithm>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
// Must be after Windows.h
#include <VersionHelpers.h>
#else
#include <unistd.h>
#endif

#include "warning-enable.hpp"

size_t get_page_size()
{
#ifdef _WIN32
	SYSTEM_INFO info = {0};
	GetSystemInfo(&info);
	return static_cast<size_t>(std::max(info.dwPageSize, info.dwAllocationGranularity));
#else
	return static_cast<size_t>(sysconf(_SC_PAGESIZE));
#endif
}

struct internal_data {
#ifdef _WIN32
	std::shared_ptr<void> area  = nullptr;
	std::shared_ptr<void> left  = nullptr;
	std::shared_ptr<void> right = nullptr;
#else
#endif
};

#ifdef _WIN32
// This only exists because std::unique_ptr needs it, while std::shared_ptr does not need it. Fuck C++'s inconsistencies...
struct virtualfree {
	void operator()(void* ptr)
	{
		VirtualFree(ptr, 0, MEM_RELEASE);
	}
};

#endif

template<typename T>
tonplugins::memory::ring<T>::ring(size_t minimum_size)
	: _write_pos(0), _read_pos(0)
{
	// Allocate the internal data structure.
	auto id        = std::make_shared<internal_data>();
	_internal_data = id;

	// Align the size with the page size to allow memory mapping hacks.
	size_t page = get_page_size();
	_size       = (minimum_size + (page - 1)) / page;

#ifdef _WIN32
	constexpr size_t max_attempts = 255;

	if (IsWindows10OrGreater()) {
		size_t real_size = this->_size * sizeof(T);

		// Create a pagefile backed section for the buffer.
		for (size_t attempt = 0; (attempt < max_attempts) && (!id->area); attempt++) {
			void* ptr = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, static_cast<DWORD>(std::clamp<size_t>(real_size, 0, std::numeric_limits<DWORD>::max())), nullptr);
			if (ptr) {
				id->area = std::shared_ptr<void>{ptr, [](void* ptr) { CloseHandle(ptr); }};
			}
		}
		if (!id->area) {
			throw std::runtime_error("Failed to create a memory mapped buffer.");
		}

		// Try a few times to allocate a continuous memory region.
		std::unique_ptr<void, virtualfree> placeholder = nullptr;
		for (size_t attempt = 0; (attempt < max_attempts) && (!placeholder); attempt++) {
			void* ptr = VirtualAlloc2(nullptr, nullptr, real_size * 2, MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, nullptr, 0);
			if (ptr) {
				placeholder = std::unique_ptr<void, virtualfree>(ptr);
			}
		}
		if (!placeholder) {
			throw std::runtime_error("Failed to allocate virtually continuous memory.");
		}

		// Split the region in half.
		bool split = false;
		for (size_t attempt = 0; (attempt < max_attempts) && (split == false); attempt++) {
			split = (VirtualFree(placeholder.get(), real_size, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER) == TRUE);
		}
		if (!split) {
			throw std::runtime_error("Failed to split virtually continuous memory in half.");
		}

		// Try and map the left half
		for (size_t attempt = 0; (attempt < max_attempts) && (!id->left); attempt++) {
			void* ptr = MapViewOfFile3(id->area.get(), nullptr, reinterpret_cast<uint8_t*>(placeholder.get()), 0, real_size, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, nullptr, 0);
			if (ptr) {
				id->left = std::shared_ptr<void>(ptr, [](void* ptr) { UnmapViewOfFile(ptr); });
			}
		}
		if (!id->left) {
			throw std::runtime_error("Failed to map buffer into left half of virtually continuous memory.");
		} else {
			// From now on, we're only allowed to free the second half.
			/* placeholder = std::unique_ptr<void>(reinterpret_cast<uint8_t*>(placeholder.release()) + real_size, [](void* ptr) {
				VirtualFree(ptr, 0, MEM_RELEASE);
			});*/
		}

		for (size_t attempt = 0; (attempt < max_attempts) && (!id->right); attempt++) {
			void* ptr = MapViewOfFile3(id->area.get(), nullptr, reinterpret_cast<uint8_t*>(placeholder.get()) + real_size, 0, real_size, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, nullptr, 0);
			if (ptr) {
				id->right = std::shared_ptr<void>(ptr, [](void* ptr) { UnmapViewOfFile(ptr); });
			}
		}
		if (!id->right) {
			throw std::runtime_error("Failed to map buffer into right half of virtually continuous memory.");
		}

		// Release the placeholder to prevent undefined behavior.
		placeholder.release();

		// Assign the left half to the buffer pointer.
		_buffer = reinterpret_cast<T*>(id->left.get());
	} else { // Fall back to legacy method.
		throw std::runtime_error("Not yet implemented.");
	}
#else
	throw std::runtime_error("Not yet implemented.");
#pragma error("Not yet implemented.");
#endif
}

template<typename T>
tonplugins::memory::ring<T>::~ring()
{
	// Literally need to do nothing!
}

template<typename T>
size_t tonplugins::memory::ring<T>::write(size_t size, T const* buffer)
{
	size_t length = std::min(size, this->size() - this->used());
	memcpy(this->_buffer + this->_write_pos, buffer, sizeof(T) * length);
	this->_write_pos += length;
	return length;
}

template<typename T>
size_t tonplugins::memory::ring<T>::write(std::vector<T> const& buffer)
{
	return this->write(buffer.size(), buffer.data());
}

template<typename T>
size_t tonplugins::memory::ring<T>::read(size_t size, T* buffer)
{
	size_t length = std::min(this->used(), size);
	memcpy(buffer, this->_buffer + this->_read_pos, sizeof(T) * length);
	this->_read_pos += length;
	return length;
}

template<typename T>
size_t tonplugins::memory::ring<T>::read(std::vector<T>& buffer)
{
	return this->read(buffer.size(), buffer.data());
}

template<typename T>
size_t tonplugins::memory::ring<T>::free()
{
	return this->size() - this->used();
}

template<typename T>
size_t tonplugins::memory::ring<T>::used()
{
	// Is the write pointer in front of the read pointer?
	if (this->_write_pos > this->_read_pos) {
		// If yes, just subtract the read position from the write position.
		return this->_write_pos - this->_read_pos;
	} else {
		// Otherwise, treat the write position as a number of elements, and the read position needs to be subtracted from the size.
		return this->_write_pos + (this->_size - this->_read_pos);
	}
}

template<typename T>
size_t tonplugins::memory::ring<T>::size()
{
	return this->_size;
}

template class tonplugins::memory::ring<float>;
template class tonplugins::memory::ring<double>;
template class tonplugins::memory::ring<int8_t>;
template class tonplugins::memory::ring<uint8_t>;
template class tonplugins::memory::ring<int16_t>;
template class tonplugins::memory::ring<uint16_t>;
template class tonplugins::memory::ring<int32_t>;
template class tonplugins::memory::ring<uint32_t>;
template class tonplugins::memory::ring<int64_t>;
template class tonplugins::memory::ring<uint64_t>;
