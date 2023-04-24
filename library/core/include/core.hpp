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
