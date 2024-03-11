// Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

#include "core.hpp"
#include "version.hpp"
#include "platform.hpp"

#include "warning-disable.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>

#ifdef _WIN32
#include <Windows.h>

#include <Shlobj.h>

#include <Knownfolders.h>
#include <WinNls.h>
#elif __APPLE__
#else
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include "warning-enable.hpp"

static std::string formatted_time(bool file_safe = false)
{
	// Get the current time.
	auto now = std::chrono::system_clock::now();

	// Preallocate a 32-byte buffer.
	std::vector<char> time_buffer(32, '\0');

	// Figure out if we want a file safe or log safe format.
	std::string_view local_format = "%04d-%02d-%02dT%02d:%02d:%02d.%06d";
	if (file_safe) {
		local_format = "%04d-%02d-%02dT%02d-%02d-%02d-%06d";
	}

	// Convert the current time into UTC.
	auto      nowt    = std::chrono::system_clock::to_time_t(now);
	struct tm tstruct = *gmtime(&nowt);
	auto      mis     = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());

	// Store the time according to the requested format.
	snprintf(time_buffer.data(), time_buffer.size(), local_format.data(), tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec, mis.count() % 1000000);

	return std::string(time_buffer.data());
};

tonplugins::core::core(std::string app_name) : _app_name(app_name)
{
	{ // Local Data Path
		std::filesystem::path result;

#ifdef _WIN32
		PWSTR             widebuffer;
		std::vector<char> buffer;

		const GUID paths[] = {
			FOLDERID_LocalAppData,
			FOLDERID_RoamingAppData,
			FOLDERID_UserProgramFilesCommon,
			FOLDERID_Documents,
		};

		for (const GUID path : paths) {
			if (SHGetKnownFolderPath(path, 0, NULL, &widebuffer) == S_OK) {
				size_t wsz = static_cast<size_t>(wcslen(widebuffer));
				size_t sz  = static_cast<size_t>(WideCharToMultiByte(CP_UTF8, 0, widebuffer, static_cast<int>(wsz), 0, 0, 0, nullptr));
				buffer.resize(sz + 1);
				WideCharToMultiByte(CP_UTF8, 0, widebuffer, static_cast<int>(wsz), buffer.data(), static_cast<int>(buffer.size()), 0, nullptr);
				CoTaskMemFree(widebuffer);

				result = std::filesystem::path(std::string_view(buffer.data(), buffer.size() - 1));
				break;
			}
		}
#elif __APPLE__
		result = "~" / "Library";
#else
		if (char* buffer = getenv("XDG_DATA_HOME"); buffer != nullptr) {
			result = std::filesystem::path(buffer);
		} else {
			if (char* buffer = getenv("HOME"); buffer != nullptr) {
				result = std::filesystem::path(buffer);
			} else {
				struct passwd* pw = getpwuid(getuid());
				result            = std::filesystem::path(pw->pw_dir);
			}
			result /= ".local";
			result /= "share";
		}
#endif

		// Always have somewhere to save files, even if it's temporary.
		if (result.empty())
			result = std::filesystem::temp_directory_path();

		_local_data = result / "Xaymar" / "TonPlugIns" / _app_name;
		std::filesystem::create_directories(_local_data);
	}
	{ // Roaming Data Path
		std::filesystem::path result;

#ifdef _WIN32
		PWSTR             widebuffer;
		std::vector<char> buffer;

		const GUID paths[] = {
			FOLDERID_RoamingAppData,
			FOLDERID_UserProgramFilesCommon,
			FOLDERID_Documents,
		};

		for (const GUID path : paths) {
			if (SHGetKnownFolderPath(path, 0, NULL, &widebuffer) == S_OK) {
				size_t wsz = static_cast<size_t>(wcslen(widebuffer));
				size_t sz  = static_cast<size_t>(WideCharToMultiByte(CP_UTF8, 0, widebuffer, static_cast<int>(wsz), 0, 0, 0, nullptr));
				buffer.resize(sz + 1);
				WideCharToMultiByte(CP_UTF8, 0, widebuffer, static_cast<int>(wsz), buffer.data(), static_cast<int>(buffer.size()), 0, nullptr);
				CoTaskMemFree(widebuffer);

				result = std::filesystem::path(std::string_view(buffer.data(), buffer.size() - 1));
				break;
			}
		}
#elif __APPLE__
		result = "~" / "Library" / "Preferences";
#else
		if (char* buffer = getenv("XDG_CONFIG_HOME"); buffer != nullptr) {
			result = std::filesystem::path(buffer);
		} else {
			if (char* buffer = getenv("HOME"); buffer != nullptr) {
				result = std::filesystem::path(buffer);
			} else {
				struct passwd* pw = getpwuid(getuid());
				result            = std::filesystem::path(pw->pw_dir);
			}
			result /= ".config";
		}
#endif

		// Always have somewhere to save files, even if it's temporary.
		if (result.empty())
			result = std::filesystem::temp_directory_path();

		_roaming_data = result / "Xaymar" / "TonPlugIns" / _app_name;
		std::filesystem::create_directories(_roaming_data);
	}
	{ // Cache Data Path
		std::filesystem::path result;

#ifdef _WIN32
		result = std::filesystem::temp_directory_path();
#elif __APPLE__
		result = "~" / "Library" / "Caches";
#else
		if (char* buffer = getenv("XDG_CACHE_HOME"); buffer != nullptr) {
			result = std::filesystem::path(buffer);
		} else {
			if (char* buffer = getenv("HOME"); buffer != nullptr) {
				result = std::filesystem::path(buffer);
			} else {
				struct passwd* pw = getpwuid(getuid());
				result            = std::filesystem::path(pw->pw_dir);
			}
			result /= ".cache";
		}
#endif

		// Always have somewhere to save files, even if it's temporary.
		if (result.empty())
			result = std::filesystem::temp_directory_path();

		_cache_data = result / "Xaymar" / "TonPlugIns" / _app_name;
		std::filesystem::create_directories(_cache_data);
	}

	{ // Try and open a log file
		// Create the directory for log files if it happens to be missing.
		std::filesystem::path log_path = std::filesystem::path(_local_data) / "logs";
		std::filesystem::create_directories(log_path);

		// Create the log file itself.
		std::filesystem::path log_file = std::filesystem::path(log_path).append(formatted_time(true) + ".log");
		_log_stream                    = std::ofstream(log_file, std::ios::trunc | std::ios::out);

		// Clean up old files.
		try { // Delete all log files older than 1 month.
			for (auto& entry : std::filesystem::directory_iterator(log_path)) {
				try {
					auto wt = entry.last_write_time();
					if ((decltype(wt)::clock::now() - wt) > std::chrono::hours(24 * 14)) {
						std::filesystem::remove(entry);
					}
				} catch (std::exception const& ex) {
					log("Failed to delete old log file '%s'.", entry.path().c_str());
				}
			}
		} catch (std::exception const& ex) {
			log("Failed to clean up log file(s): %s", ex.what());
		}
	}

	log("Loaded v%s.", tonplugins::get_version().to_string().c_str());

#ifdef WIN32
	// Log information about the Host process.
	{
		std::vector<wchar_t> file_name_w(256, 0);
		size_t               file_name_len = 0;
		do {
			file_name_len = static_cast<DWORD>(GetModuleFileNameW(NULL, file_name_w.data(), static_cast<DWORD>(file_name_w.size())));
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				file_name_w.resize(file_name_w.size() * 2);
			}
		} while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);

		auto file_name = tonplugins::platform::wide_to_utf8(std::wstring(file_name_w.data(), file_name_w.data() + file_name_len));
		log("Host Process: %s (0x%08" PRIx32 ")", file_name.c_str(), GetCurrentProcessId());
	}
#endif
}

tonplugins::core::~core()
{
	if (_log_stream.is_open()) {
		_log_stream.flush();
		_log_stream.close();
	}
}

std::filesystem::path tonplugins::core::local_data_path()
{
	return std::filesystem::path(_local_data);
}

std::filesystem::path tonplugins::core::roaming_data_path()
{
	return std::filesystem::path(_local_data);
}

std::filesystem::path tonplugins::core::cache_data_path()
{
	return std::filesystem::path(_local_data);
}

void tonplugins::core::log(std::string_view format, ...)
{
	// Build the string to write to the log file.
	std::string converted;
	{
		std::vector<char> format_buffer;
		std::vector<char> string_buffer;
		std::string       time = formatted_time();

		// Generate proper format string.
		{
			const char* local_format = "%s %s\n";

			size_t len = static_cast<size_t>(snprintf(nullptr, 0, local_format, time.c_str(), format.data())) + 1;
			format_buffer.resize(len);
			snprintf(format_buffer.data(), format_buffer.size(), local_format, time.c_str(), format.data());
		};

		{
			va_list args;
			va_list args2;

			va_start(args, format);
			va_copy(args2, args);
			size_t len = static_cast<size_t>(vsnprintf(nullptr, 0, format_buffer.data(), args)) + 1;
			va_end(args);

			string_buffer.resize(len);

			va_start(args2, format);
			vsnprintf(string_buffer.data(), string_buffer.size(), format_buffer.data(), args2);
			va_end(args2);
		}

		converted = std::string{string_buffer.data()};
	}

	// Write it to the log file.
	{ // This needs to be synchronous or bad things happen.
		std::lock_guard<std::mutex> lock(_log_stream_mutex);
		if (_log_stream.good() && !_log_stream.bad()) {
			_log_stream << converted;
			_log_stream.flush(); // Flush so no information is lost.
		}
	}

	// Send this to stdout.
	std::cout << converted;

#if defined(_WIN32) && defined(_MSC_VER)
	{ // Write to Debug console
		std::vector<char> string_buffer;

		{ // Need to prefix it as the debug console is noisy.
			const char* local_format = "[TonPlugins] %s";

			size_t rfsz = static_cast<size_t>(snprintf(nullptr, 0, local_format, converted.data())) + 1;
			string_buffer.resize(rfsz);
			snprintf(string_buffer.data(), string_buffer.size(), local_format, converted.data());
		}

		// MSVC: Print to debug console
		std::vector<wchar_t> wstring_buffer(converted.size(), 0);
		size_t               len = static_cast<size_t>(MultiByteToWideChar(CP_UTF8, 0, string_buffer.data(), static_cast<int>(string_buffer.size()), wstring_buffer.data(), 0)) + 1;
		wstring_buffer.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, string_buffer.data(), static_cast<int>(string_buffer.size()), wstring_buffer.data(), static_cast<int>(wstring_buffer.size()));
		OutputDebugStringW(wstring_buffer.data());
	}
#endif
}

std::shared_ptr<tonplugins::core> tonplugins::core::instance(std::string app_name)
{
	static std::mutex                      mtx;
	static std::weak_ptr<tonplugins::core> winst;
	std::shared_ptr<tonplugins::core>      inst;

	std::lock_guard<decltype(mtx)> lock(mtx);
	inst = winst.lock();
	if (!inst) {
		inst  = std::shared_ptr<tonplugins::core>(new tonplugins::core(std::move(app_name)));
		winst = inst;
	}
	return inst;
}
