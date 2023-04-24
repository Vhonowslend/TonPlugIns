// Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

#include "platform.hpp"

#include "warning-disable.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) // Windows
#define ST_WINDOWS
#else
#define ST_UNIX
#endif

#if defined(ST_WINDOWS)
#include <Windows.h>
#elif defined(ST_UNIX)
#include <dlfcn.h>
#endif
#include "warning-enable.hpp"

#ifdef ST_WINDOWS
std::u8string tonplugins::platform::wide_to_utf8(std::wstring const& v)
{
	std::vector<char8_t> buffer((v.length() + 1) * 4, 0);

	int res = WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(v.data()), static_cast<int>(v.length()), reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), nullptr, nullptr);
	if (res == 0) {
		throw std::runtime_error("Failed to convert Windows-native to UTF-8.");
	}

	return {buffer.data()};
}

std::filesystem::path tonplugins::platform::wide_to_utf8(std::filesystem::path const& v)
{
	auto wide   = v.wstring();
	auto narrow = wide_to_utf8(wide);
	return std::filesystem::path(narrow);
}

std::wstring tonplugins::platform::utf8_to_wide(std::u8string const& v)
{
	std::vector<wchar_t> buffer(v.length() + 1, 0);

	int res = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(v.data()), static_cast<int>(v.length()), reinterpret_cast<wchar_t*>(buffer.data()), static_cast<int>(buffer.size()));
	if (res == 0) {
		throw std::runtime_error("Failed to convert UTF-8 to Windows-native.");
	}

	return {buffer.data()};
}

std::filesystem::path tonplugins::platform::utf8_to_wide(std::filesystem::path const& v)
{
	auto narrow = v.string();
	auto wide   = utf8_to_wide(narrow);
	return std::filesystem::path(wide);
}
#endif

tonplugins::platform::library::library(std::filesystem::path file)
	: _library(nullptr)
{
#if defined(ST_WINDOWS)
	SetLastError(ERROR_SUCCESS);
	file        = ::tonplugins::platform::utf8_to_wide(file);
	DWORD flags = 0;
	if (file.is_absolute()) {
		flags |= LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;
	} else {
		flags |= LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;
	}
	_library = reinterpret_cast<void*>(LoadLibraryExW(file.wstring().data(), NULL, flags));
	if (!_library) {
		DWORD error = GetLastError();
		if (error != ERROR_PROC_NOT_FOUND) {
			PSTR        message = NULL;
			std::string ex      = "Failed to load library.";
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, error, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR)&message, 0, NULL);
			if (message) {
				ex = message;
				LocalFree(message);
				throw std::runtime_error(ex);
			}
		}
		throw std::runtime_error("Failed to load library.");
	}
#elif defined(ST_UNIX)
	_library = dlopen(file.u8string().c_str(), RTLD_LAZY);
	if (!_library) {
		if (char* error = dlerror(); error)
			throw std::runtime_error(error);
		else
			throw std::runtime_error("Failed to load library.");
	}
#endif
}

tonplugins::platform::library::~library()
{
#if defined(ST_WINDOWS)
	FreeLibrary(reinterpret_cast<HMODULE>(_library));
#elif defined(ST_UNIX)
	dlclose(_library);
#endif
}

void* tonplugins::platform::library::load_symbol(std::string_view name)
{
#if defined(ST_WINDOWS)
	return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(_library), name.data()));
#elif defined(ST_UNIX)
	return reinterpret_cast<void*>(dlsym(_library, name.data()));
#endif
}
std::shared_ptr<::tonplugins::platform::library> tonplugins::platform::library::load(std::filesystem::path file)
{
	static std::unordered_map<std::u8string, std::weak_ptr<::tonplugins::platform::library>> libraries;

	auto kv = libraries.find(file.u8string());
	if (kv != libraries.end()) {
		if (auto ptr = kv->second.lock(); ptr)
			return ptr;
		libraries.erase(kv);
	}

	auto ptr = std::make_shared<::tonplugins::platform::library>(file);
	libraries.emplace(file.u8string(), ptr);

	return ptr;
}

std::shared_ptr<::tonplugins::platform::library> tonplugins::platform::library::load(std::string_view name)
{
	return load(std::filesystem::path(name));
}
