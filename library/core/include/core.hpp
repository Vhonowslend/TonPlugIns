// Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

#pragma once
#include "warning-disable.hpp"
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include "warning-enable.hpp"

#ifdef _MSC_VER
#define TONPLUGINS_EXPORT __declspec(dllexport)
#define TONPLUGINS_HIDDEN
#else
#define TONPLUGINS_EXPORT __attribute__((visibility("default")))
#define TONPLUGINS_HIDDEN __attribute__((visibility("hidden")))
#endif

/// Currrent function name (as const char*)
#ifdef _MSC_VER
// Microsoft Visual Studio
#define __FUNCTION_SIG__ __FUNCSIG__
#define __FUNCTION_NAME__ __func__
#elif defined(__GNUC__) || defined(__MINGW32__)
// GCC and MinGW
#define __FUNCTION_SIG__ __PRETTY_FUNCTION__
#define __FUNCTION_NAME__ __func__
#else
// Any other compiler
#define __FUNCTION_SIG__ __func__
#define __FUNCTION_NAME__ __func__
#endif

/// Forceful inlining
#ifndef FORCE_INLINE
#ifdef _MSC_VER
#define FORCE_INLINE __force_inline
#elif defined(__GNUC__) || defined(__MINGW32__)
#define FORCE_INLINE __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif
#endif

namespace tonplugins {
	class core {
		std::string           _app_name;
		std::filesystem::path _local_data;
		std::filesystem::path _roaming_data;
		std::filesystem::path _cache_data;

		std::ofstream _log_stream;
		std::mutex    _log_stream_mutex;

		private:
		core(std::string app_name);

		public:
		~core();

		public /* Paths */:
		/** Path for non-roaming data, such as log files, crash dumps, etc.
		 */
		std::filesystem::path local_data();

		/** Path for roaming data, such as configuration, presets, etc.
		 */
		std::filesystem::path roaming_data();

		/** Path for temporary data, such as cache, etc.
		 */
		std::filesystem::path cache_data();

		public:
		void log(std::string_view format, ...);

		public:
		static std::shared_ptr<tonplugins::core> instance(std::string app_name = "");
	};
} // namespace tonplugins
