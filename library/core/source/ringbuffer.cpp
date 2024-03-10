// Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

#include "ringbuffer.hpp"

#include "warning-disable.hpp"
#include <algorithm>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
// Must be after Windows.h
#include <VersionHelpers.h>
// Fix missing VirtualAlloc2
#pragma comment(lib, "mincore")
#else
#include <unistd.h>
#endif

#include "warning-enable.hpp"

size_t get_minimum_page_size()
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
tonplugins::memory::ring<T>::ring(size_t size) : _write_pos(0), _read_pos(0)
{
	// Calculate the proper size.
	size_t page = get_minimum_page_size();
	_size       = size;
	_size *= sizeof(T); // Convert to Bytes
	_size += (page - 1); // Prepare for rounding up
	_size /= page; // Round towards zero and convert to Pages
	_size *= page; // Convert to Bytes
	_size /= sizeof(T); // Convert to Elements.

	// Allocate the internal data structure.
	auto id        = std::make_shared<internal_data>();
	_internal_data = id;

#ifdef _WIN32
	constexpr size_t max_attempts = 255;

	if (IsWindows10OrGreater()) {
		size_t real_size = _size * sizeof(T);

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
// This is legal, but MSVC will complain about it.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 28160 6333)
#endif
			split = (VirtualFree(placeholder.get(), real_size, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER) != FALSE);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
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
			placeholder = std::unique_ptr<void, virtualfree>(reinterpret_cast<uint8_t*>(placeholder.release()) + real_size);
		}

		for (size_t attempt = 0; (attempt < max_attempts) && (!id->right); attempt++) {
			void* ptr = MapViewOfFile3(id->area.get(), nullptr, placeholder.get(), 0, real_size, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, nullptr, 0);
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
	// Early-Exit if something is invalid.
	if ((!buffer) || (size == 0))
		return 0;

	// Limit the size of the write to the buffer size.
	size_t elements = std::min(size, _size);

	// Copy data from the buffer into the ring.
	memcpy(_buffer + _write_pos, buffer, sizeof(T) * elements);

	// Advance the write position by the number of elements, wrapped into the actual buffer size.
	size_t write_old = _write_pos;
	size_t write_new = write_old + elements;
	_write_pos       = write_new % _size;

	// Advance the read position if we just overwrite part of it.
	if ((write_old < _read_pos) && (write_new >= _read_pos)) {
		// (w0 < r) && (w1 >= r), c=10
		//Read caught up to write:
		// s=5, r=0, w=0: (0 < 0) && (5 >= 0) = false (should be false)
		//Write caught up to read:
		// s=5, r=3, w=0: (0 < 3) && (5 >= 3) = true (should be true)
		_read_pos = _write_pos + 1;
		// Adding 1 allows us to differentiate between read catching up to write, and write catching up to read.
		// If write catches up to read, then read will be 1 ahead of write. Thus (w0 < r) && (w1 >= r) can be true.
		// If read catches up to write, then read will be exactly on write. Thus (w0 < r) && (w1 >= r) can be false.
		// Otherwise, we would have ambiguity between the two cases.
	}

	// Return the length actually written.
	return elements;
}

template<typename T>
T* tonplugins::memory::ring<T>::peek(size_t size)
{
	// Early-Exit if something is invalid.
	if ((size == 0) || (size > used())) {
		return nullptr;
	}

	// Calculate the pointer to return.
	T* ptr = _buffer + _read_pos;

	// Advance the read position and wrap it back into the actual buffer size.
	_read_pos = (_read_pos + size) % _size;

	// Return the pointer.
	return ptr;
}

template<typename T>
size_t tonplugins::memory::ring<T>::read(size_t size, T* buffer)
{
	// Early-Exit if something is invalid.
	if ((!buffer) || (size == 0))
		return 0;

	// Limit the length of the read to the available used space.
	size = std::min(used(), size);

	// Copy data from the ring into the buffer.
	memcpy(buffer, _buffer + _read_pos, sizeof(T) * size);

	// Advance the read position and wrap it back into the actual buffer size.
	_read_pos = (_read_pos + size) % _size;

	// Return the length actually read.
	return size;
}

template<typename T>
size_t tonplugins::memory::ring<T>::used()
{
	// Is the write pointer in front of the read pointer?
	if (_write_pos > _read_pos) {
		// If yes, just subtract the read position from the write position.
		return _write_pos - _read_pos;
	} else {
		// Otherwise, treat the write position as a number of elements, and the read position needs to be subtracted from the size.
		return _write_pos + (_size - _read_pos);
	}
}

template<typename T>
size_t tonplugins::memory::ring<T>::size()
{
	return _size;
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
