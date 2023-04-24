// Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

#pragma once
#include "warning-disable.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include "warning-enable.hpp"

namespace tonplugins::platform {
	class library {
		void* _library;

		public:
		library(std::filesystem::path file);
		~library();

		void* load_symbol(std::string_view name);

		static std::shared_ptr<::tonplugins::platform::library> load(std::filesystem::path file);

		static std::shared_ptr<::tonplugins::platform::library> load(std::string_view name);
	};

#ifdef _WIN32
	std::u8string         wide_to_utf8(std::wstring const& v);
	std::filesystem::path wide_to_utf8(std::filesystem::path const& v);

	std::wstring          utf8_to_wide(std::u8string const& v);
	std::filesystem::path utf8_to_wide(std::filesystem::path const& v);
#endif

} // namespace tonplugins::platform
