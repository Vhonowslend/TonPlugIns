# Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

# CMake Setup
cmake_minimum_required(VERSION 3.26)
list(APPEND CMAKE_MESSAGE_INDENT "[TonPlugIns] ")
project(TonPlugIns)

################################################################################
# Modules
################################################################################

# Search Paths
set(CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

# Include required Modules
# - Stock
include("CheckIPOSupported")
# - Custom
include("util")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/cmake-version/version.cmake")
	include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/cmake-version/version.cmake")
else()
	message(FATAL_ERROR "Please ensure you've cloned recursively and try again. Thank you!")
endif()

# Include optional Modules
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/cmake-clang/Clang.cmake")
	include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/cmake-clang/Clang.cmake")
endif()

################################################################################
# Options
################################################################################

# Allow overriding version
set(VERSION "" CACHE STRING "Override detected or pre-configured version with this string. Accepts strings in CMake and SemVer 2.0.0 format.")

set(ENABLE_FULL_VERSION ON CACHE BOOL "Enable the full feature set? (Do not enable for Demo/Free builds!)")

################################################################################
# Versioning
################################################################################

version(GENERATE _VERSION COMPRESSED MAJOR 0 MINOR 0 PATCH 0 TWEAK 0 REQUIRE "PATCH;")
version(PARSE _VERSION "${_VERSION}" REQUIRE "PATCH;TWEAK")

# If possible, automatically generate versions from git.
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
	find_program(GIT
		NAMES
			git
			git.exe
	)

	if(EXISTS "${GIT}")
		# Try and calculate the exist version using git.
		execute_process(COMMAND "${GIT}" describe --tags --long --abbrev=8 HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} RESULT_VARIABLE GIT_RESULT OUTPUT_VARIABLE GIT_OUTPUT ERROR_VARIABLE GIT_ERROR OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
		if((GIT_RESULT EQUAL 0) AND (NOT "${GIT_OUTPUT}" STREQUAL ""))
			# Result will be MAJOR.MINOR.PATCH-TWEAK-gHASH
			string(REPLACE "-" ";" GIT_OUTPUT "${GIT_OUTPUT}")
			string(REPLACE "." ";" GIT_OUTPUT "${GIT_OUTPUT}")

			# Split into components
			list(GET GIT_OUTPUT 0 GIT_OUTPUT_MAJOR)
			list(GET GIT_OUTPUT 1 GIT_OUTPUT_MINOR)
			list(GET GIT_OUTPUT 2 GIT_OUTPUT_PATCH)
			list(GET GIT_OUTPUT 3 GIT_OUTPUT_TWEAK)
			list(GET GIT_OUTPUT 4 GIT_OUTPUT_BUILD)

			# Special case: Tag contains prerelease
			if(GIT_OUTPUT_PATCH MATCHES "([0-9]+)([a-zA-Z]+)([0-9]*)")
				# Patch requires special parsing.
				set(GIT_OUTPUT_PATCH "${CMAKE_MATCH_1}")
				if(CMAKE_MATCH_3 GREATER 0)
					set(GIT_OUTPUT_PRERELEASE "${CMAKE_MATCH_2}")
				else()
					set(GIT_OUTPUT_PRERELEASE "a")
				endif()
				MATH(EXPR GIT_OUTPUT_TWEAK "${GIT_OUTPUT_TWEAK} + ${CMAKE_MATCH_3}")

				# Modify the global version
				version(MODIFY _VERSION "${_VERSION}" COMPRESS
					MAJOR "${GIT_OUTPUT_MAJOR}"
					MINOR "${GIT_OUTPUT_MINOR}"
					PATCH "${GIT_OUTPUT_PATCH}"
					TWEAK "${GIT_OUTPUT_TWEAK}"
					BUILD "${GIT_OUTPUT_BUILD}"
					PRERELEASE "${GIT_OUTPUT_PRERELEASE}"
					REQUIRE "PATCH;TWEAK"
				)
			else()
				# Modify the global version
				version(MODIFY _VERSION "${_VERSION}" COMPRESS
					MAJOR "${GIT_OUTPUT_MAJOR}"
					MINOR "${GIT_OUTPUT_MINOR}"
					PATCH "${GIT_OUTPUT_PATCH}"
					TWEAK "${GIT_OUTPUT_TWEAK}"
					BUILD "${GIT_OUTPUT_BUILD}"
					PRERELEASE "a"
					REQUIRE "PATCH;TWEAK"
				)
			endif()
		else()
			execute_process(COMMAND "${GIT}" rev-list --count HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} RESULT_VARIABLE GIT_RESULT OUTPUT_VARIABLE GIT_OUTPUT ERROR_VARIABLE GIT_ERROR OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
			if((GIT_RESULT EQUAL 0) AND (NOT "${GIT_OUTPUT}" STREQUAL ""))
				version(MODIFY _VERSION "${_VERSION}" COMPRESS
					TWEAK "${GIT_OUTPUT}"
					PRERELEASE "a"
					REQUIRE "PATCH;TWEAK"
				)

				# Determine the build using git.
				execute_process(COMMAND "${GIT}" log -1 "--pretty=format:g%h" --abbrev=8 HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} RESULT_VARIABLE GIT_RESULT OUTPUT_VARIABLE GIT_OUTPUT ERROR_VARIABLE GIT_ERROR OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
				if((GIT_RESULT EQUAL 0) AND (NOT "${GIT_OUTPUT}" STREQUAL ""))
					version(MODIFY _VERSION "${_VERSION}" COMPRESS
						BUILD "${GIT_OUTPUT}"
						REQUIRE "PATCH;TWEAK"
					)
				else()
					message(WARNING "Failed to detect build version with 'git'.")
				endif()
			else()
				message(WARNING "Failed to automatically detect version with 'git'.")
			endif()
		endif()
	else()
		message(WARNING "'git' not found, automatic version detection disabled.")
	endif()
else()
	message(STATUS "Not a git repository, automatic version detection disabled.")
endif()

# Allow manual overrides of the detected version.
if(${PREFIX}VERSION)
	version(PARSE _VERSION_CFG "${${PREFIX}VERSION}" REQUIRE "PATCH;TWEAK")
	if("${_VERSION_CFG_BUILD}" STREQUAL "")
		set(_VERSION_CFG_BUILD "${_VERSION_BUILD}")
	endif()
	version(GENERATE _VERSION COMPRESS
		MAJOR "${_VERSION_CFG_MAJOR}"
		MINOR "${_VERSION_CFG_MINOR}"
		PATCH "${_VERSION_CFG_PATCH}"
		TWEAK "${_VERSION_CFG_TWEAK}"
		PRERELEASE "${_VERSION_CFG_PRERELEASE}"
		BUILD "${_VERSION_CFG_BUILD}"
	)
endif()

# Fix up missing parts of the Version
version(PARSE _VERSION "${_VERSION}" REQUIRE "PATCH;TWEAK")

set(_VERSION_THIN "${_VERSION_MAJOR}.${_VERSION_MINOR}.${_VERSION_PATCH}")
if(NOT (_VERSION_PRERELEASE STREQUAL ""))
	set(_VERSION_THIN "${_VERSION_THIN}${_VERSION_PRERELEASE}${_VERSION_TWEAK}")
endif()
if(NOT (_VERSION_BUILD STREQUAL ""))
	set(_VERSION_THIN "${_VERSION_THIN}-${_VERSION_BUILD}")
endif()

# Parse & Log the detected version.
message(STATUS "Version ${_VERSION_THIN}")

################################################################################
# Project
################################################################################

# - Generate a version number compatible with CMake
version(GENERATE PROJECT_VERSION
	MAJOR "${_VERSION_MAJOR}"
	MINOR "${_VERSION_MINOR}"
	PATCH "${_VERSION_PATCH}"
	TWEAK "${_VERSION_TWEAK}"
	REQUIRE "PATCH;TWEAK"
)

# Project and Metadata
project(
	TonPlugIns
	LANGUAGES CXX;C
	VERSION "${PROJECT_VERSION}"
	DESCRIPTION "TonPlugIns is a collection of plug-ins for audio software."
	HOMEPAGE_URL "https://tonplugins.xaymar.com/"
)
set(PROJECT_TITLE "TonPlugIns")
set(PROJECT_AUTHORS "Michael Fabian 'Xaymar' Dirks <info@xaymar.com>")
set(PROJECT_COPYRIGHT "2020, Michael Fabian Dirks. All Rights Reserved")
set(PROJECT_TRADEMARKS "VST® is a registered trademark of Steinberg Media Technologies GmbH")
set(PROJECT_SOURCE "")
set(PROJECT_SOURCE_GENERATED "")
set(PROJECT_TEMPLATES "")
set(PROJECT_INCLUDES "")
set(PROJECT_LIBRARIES "")

# If this is not a full version, add the "(Demo)" suffix.
if(NOT ENABLE_FULL_VERSION)
	set(PROJECT_TITLE "${PROJECT_TITLE} (Demo)")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	# MacOS: We must target MacOS 10.15 or later.
	if(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS 10.15)
		set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "" FORCE)
	endif()

	# MacOS: We must target all architectures always.
	set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" FORCE)
endif()

# Detect the target architecture. (CMAKE_SYSTEM_ARCHITECTURE can't be trusted.)
try_compile(ARCH_RES
	SOURCES "${PROJECT_SOURCE_DIR}/cmake/architecture.c"
	NO_LOG
	OUTPUT_VARIABLE ARCH_OUT
)
string(REGEX MATCH "TARGET_SYSTEM_ARCHITECTURE=([a-zA-Z0-9_\-]+)" ARCH_OUT ${ARCH_OUT})
set(TARGET_SYSTEM_ARCHITECTURE "${CMAKE_MATCH_1}")
if(TARGET_SYSTEM_ARCHITECTURE STREQUAL "x86_64")
	set(TARGET_SYSTEM_IS_X86 ON)
	set(TARGET_SYSTEM_IS_64BIT ON)
elseif(TARGET_SYSTEM_ARCHITECTURE STREQUAL "x86_32")
	set(TARGET_SYSTEM_IS_X86 ON)
	set(TARGET_SYSTEM_IS_64BIT OFF)
elseif(TARGET_SYSTEM_ARCHITECTURE STREQUAL "arm64ec")
	set(TARGET_SYSTEM_IS_ARM ON)
	set(TARGET_SYSTEM_IS_X86 ON)
	set(TARGET_SYSTEM_IS_64BIT ON)
elseif(TARGET_SYSTEM_ARCHITECTURE STREQUAL "arm64")
	set(TARGET_SYSTEM_IS_ARM ON)
	set(TARGET_SYSTEM_IS_64BIT ON)
elseif(TARGET_SYSTEM_ARCHITECTURE MATCHES "^arm")
	set(TARGET_SYSTEM_IS_ARM ON)
	set(TARGET_SYSTEM_IS_64BIT OFF)
else()
endif()

# Set default Install prefix
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX "$ENV{COMMONPROGRAMFILES}/" CACHE PATH "Install location" FORCE)
	endif()
	set(CMAKE_INSTALL_PREFIX_VST3 "VST3")
	set(CMAKE_INSTALL_PREFIX_VST2 "VST2")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX "/Library/Audio/Plug-ins/" CACHE PATH "Install location" FORCE)
	endif()
	set(CMAKE_INSTALL_PREFIX_VST3 "VST3")
	set(CMAKE_INSTALL_PREFIX_VST2 "VST2")
else()
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX "/usr/lib/" CACHE PATH "Install location" FORCE)
	endif()
	set(CMAKE_INSTALL_PREFIX_VST3 "vst3")
	set(CMAKE_INSTALL_PREFIX_VST2 "vst2")
endif()

################################################################################
# APIs
################################################################################

# VST 2.x
#add_subdirectory("${PROJECT_SOURCE_DIR}/third-party/vst2sdk")
add_library(vst2sdk INTERFACE
	"${PROJECT_SOURCE_DIR}/third-party/vst2sdk/vst.h"
)
target_include_directories(vst2sdk INTERFACE
	"${PROJECT_SOURCE_DIR}/third-party/vst2sdk"
)
set_target_properties(vst2sdk PROPERTIES
	FOLDER "Steinberg"
)

# VST 3.x
set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES ON CACHE BOOL "" FORCE)
set(SMTG_ENABLE_VSTGUI_SUPPORT ON CACHE BOOL "" FORCE)
set(SMTG_CXX_STANDARD "23" CACHE STRING "" FORCE)
set(SMTG_ADD_VST3_UTILITIES OFF CACHE BOOL "" FORCE)
set(SMTG_USE_STATIC_CRT ON CACHE BOOL "" FORCE)
set(SMTG_RUN_VST_VALIDATOR OFF CACHE BOOL "" FORCE)
set(SMTG_CREATE_MODULE_INFO OFF CACHE BOOL "" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/third-party/vst3sdk")
set_target_properties(base cmake_modules cmake_VST_modules pluginterfaces sdk sdk_common sdk_hosting vstgui vstgui_standalone vstgui_support vstgui_uidescription validator VST3Inspector PROPERTIES
	FOLDER "Steinberg"
)

################################################################################
# Functions
################################################################################
function(define_target p_name p_type)
	add_library(${p_name} ${p_type})
	add_library(TonPlugIns::${p_name} ALIAS ${p_name})

	set_target_properties(${p_name} PROPERTIES
		# Remove platform specific prefixes.
		PREFIX ""
		IMPORT_PREFIX ""

		# Version
		MACHO_COMPATIBILITY_VERSION ${_VERSION_MAJOR}.${_VERSION_MINOR}
		MACHO_CURRENT_VERSION ${PROJECT_VERSION}
		SOVERSION ${_VERSION_MAJOR}.${_VERSION_MINOR}
		VERSION ${PROJECT_VERSION}
	)

	target_include_directories(${p_name}
		PRIVATE
			"${${p_name}_SOURCE_DIR}/source"
			"${${p_name}_BINARY_DIR}/source"
		PUBLIC
			"${${p_name}_SOURCE_DIR}/include"
			"${${p_name}_BINARY_DIR}/include"
	)

	# Adjust properties and compiler flags
	set_target_properties(${p_name} PROPERTIES
		# Always generate position independent code.
		POSITION_INDEPENDENT_CODE ON
		# Set C++ Standard and Extensions
		C_STANDARD 17
		C_STANDARD_REQUIRED ON
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
		# Always try to optimize at link-time
		INTERPROCEDURAL_OPTIMIZATION ON
	)
	target_compile_definitions(${p_name} PRIVATE
		__STDC_WANT_LIB_EXT1__=1
	)
	if(MSVC)
		target_compile_options(${p_name} PRIVATE
			# Allow multi-processor compiling.
			"/MP"
			# Enable C++ exceptions with SEH exceptions.
			"/EHa"
			# Enable updated __cplusplus macro
			"/Zc:__cplusplus"
			# Use static CRT
			$<$<CONFIG:>:/MT>
			$<$<CONFIG:Debug>:/MTd>
			$<$<CONFIG:Release>:/MT>
			$<$<CONFIG:RelWithDebInfo>:/MT>
			$<$<CONFIG:MinSizeRel>:/MT>
		)
		target_compile_definitions(${p_name} PRIVATE
			# We want C++, not MSVC++.
			_CRT_SECURE_NO_WARNINGS
			_ENABLE_EXTENDED_ALIGNED_STORAGE
			# Windows.h fixes
			WIN32_LEAN_AND_MEAN
			NOMINMAX
			NOINOUT
		)
	endif()

	# Architecture and Bitness
	if(TARGET_SYSTEM_IS_ARM)
		target_compile_definitions(${p_name} PUBLIC TONPLUGINS_ARCH_ARM)
	endif()
	if(TARGET_SYSTEM_IS_X86)
		target_compile_definitions(${p_name} PUBLIC TONPLUGINS_ARCH_X86)
	endif()
	if(TARGET_SYSTEM_IS_64BIT)
		target_compile_definitions(${p_name} PUBLIC TONPLUGINS_ARCH_64BIT)
	endif()

	# Always use fast math over precise math.
	if(MSVC)
		target_compile_options(${p_name} PRIVATE "/fp:fast")
	elseif(GNU)
		target_compile_options(${p_name} PRIVATE "-ffast-math")
	elseif(Clang)
		target_compile_options(${p_name} PRIVATE "-ffp-model=fast")
	elseif(AppleClang)
		# Appears to be the default.
	else()
		message(WARNING "Unknown compiler, unable to guarantee faster math processing.")
	endif()

	# Don't clutter the export table with useless exports.
	if(MSVC)
		# Appears to be the default.
	else()
		add_compile_options("-fvisibility=hidden")
	endif()

	# Enable a number of warnings
	if(MSVC)
		target_compile_options(${p_name} PRIVATE
			"/Wall"
			# Don't warn about unused variables, parameters, labels, functions, or typedefs.
			"/wd4100"
			"/wd4101"
			"/wd4102"
			"/wd4505"
			"/wd4514"
			"/wd5245"
			# Don't warn about unreferenced variables or parameters which are assigned/initialized.
			"/wd4189"
			# Don't warn about not-explicitly-handled enumeration identifiers
			"/wd4061"
			# Ignore automatic padding warnings.
			"/wd4820"
			# Ignore assignment/move/copy being implicit '= delete;'.
			"/wd4623"
			"/wd4625"
			"/wd4626"
			"/wd5026"
			"/wd5027"
			# Relative include paths are fine.
			"/wd4464"
			# Buggy warning: printf expects string literal
			"/wd4774"
			# Buggy warning: subobject initialization should be wrapped in braces
			"/wd5246"
			# Ignore undefined, unused or unreferenced pre-processor macros
			"/wd4688"
			# Ignore non-inlined functions
			"/wd4710"
			# Ignore Spectre mitigation insertions
			"/wd5045"
			# Ignore inserted padding.
			"/wd4324"
		)
	elseif(GNU OR Clang)
		target_compile_options(${p_name} PRIVATE
			"-Wall"
			"-Wextra"
			# Don't warn about unused variables, parameters, labels, functions, or typedefs.
			"-Wno-unused-function"
			"-Wno-unused-label"
			"-Wno-unused-local-typedefs"
			"-Wno-unused-parameter"
			"-Wno-unused-result"
			"-Wno-unused-const-variable"
			"-Wno-unused-variable"
			"-Wno-unused-value"
			# Don't warn about unreferenced variables or parameters which are assigned/initialized.
			"-Wno-unused-but-set-parameter"
			"-Wno-unused-but-set-variable"
			# Don't warn about not-explicitly-handled enumeration identifiers
			"-Wno-switch"
			# Ignore automatic padding warnings.
			"-Wno-padded"
			# Ignore implicit '= delete;'.
			# Ignore extra arguments for printf
			"-Wno-format-extra-args"
			# Ignore undefined, unused or unreferenced pre-processor macros
			"-Wno-unused-macros"
		)
	elseif(AppleClang)
		target_compile_options(${p_name} PRIVATE
			"-Wall"
			"-Wextra"
			# Don't warn about unused variables, parameters, labels, functions, or typedefs.
			"-Wno-unused-function"
			"-Wno-unused-label"
			"-Wno-unused-local-typedefs"
			"-Wno-unused-parameter"
			"-Wno-unused-result"
			"-Wno-unused-const-variable"
			"-Wno-unused-variable"
			"-Wno-unused-value"
			# Don't warn about unreferenced variables or parameters which are assigned/initialized.
			"-Wno-unused-but-set-parameter"
			"-Wno-unused-but-set-variable"
			# Don't warn about not-explicitly-handled enumeration identifiers
			"-Wno-switch"
			# Ignore automatic padding warnings.
			"-Wno-padded"
			# Ignore implicit '= delete;'.
			# Ignore extra arguments for printf
			"-Wno-format-extra-args"
			# Ignore undefined, unused or unreferenced pre-processor macros
			"-Wno-unused-macros"
		)
	endif()
endfunction()

function(setup_target p_name)
	# Common structure between all targets.
	file(GLOB_RECURSE TEMPLATES FOLLOW_SYMLINKS CONFIGURE_DEPENDS "templates/*")
	source_group(TREE "${${p_name}_SOURCE_DIR}/templates" PREFIX "Templates" FILES ${TEMPLATES})

	file(GLOB_RECURSE RESOURCES FOLLOW_SYMLINKS CONFIGURE_DEPENDS "resources/*")
	source_group(TREE "${${p_name}_SOURCE_DIR}/resources" PREFIX "Resources" FILES ${RESOURCES})
	file(GLOB_RECURSE RESOURCES FOLLOW_SYMLINKS CONFIGURE_DEPENDS "${${p_name}_BINARY_DIR}/resources/*")
	source_group(TREE "${${p_name}_BINARY_DIR}/resources" PREFIX "Resources/Generated" FILES ${GENERATED_RESOURCES})

	file(GLOB_RECURSE SOURCE_PRIVATE FOLLOW_SYMLINKS CONFIGURE_DEPENDS "source/*")
	source_group(TREE "${${p_name}_SOURCE_DIR}/source" PREFIX "Source Files" FILES ${SOURCE_PRIVATE})
	file(GLOB_RECURSE GENERATED_PRIVATE FOLLOW_SYMLINKS CONFIGURE_DEPENDS "${${p_name}_BINARY_DIR}/source/*")
	source_group(TREE "${${p_name}_BINARY_DIR}/source" PREFIX "Source Files/Generated" FILES ${GENERATED_PRIVATE})

	file(GLOB_RECURSE SOURCE_PUBLIC FOLLOW_SYMLINKS CONFIGURE_DEPENDS "include/*")
	source_group(TREE "${${p_name}_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${SOURCE_PUBLIC})
	file(GLOB_RECURSE GENERATED_PUBLIC FOLLOW_SYMLINKS CONFIGURE_DEPENDS "${${p_name}_BINARY_DIR}/include/*")
	source_group(TREE "${${p_name}_BINARY_DIR}/include" PREFIX "Header Files/Generated" FILES ${GENERATED_PUBLIC})

	target_sources(${p_name}
		PRIVATE
			${TEMPLATES}
			${RESOURCES}
			${GENERATED_RESOURCES}
			${SOURCE_PRIVATE}
			${GENERATED_PRIVATE}
			${SOURCE_PUBLIC}
			${GENERATED_PUBLIC}
	)

	set_source_files_properties(${TEMPLATES} ${RESOURCES} ${GENERATED_RESOURCES} PROPERTIES
		HEADER_FILE_ONLY ON
	)

	# Integrate with clang-format and clang-tidy
	if(COMMAND generate_compile_commands_json)
		generate_compile_commands_json(
			TARGETS ${p_name}
		)
#		if(COMMAND clang_tidy)
#			clang_tidy(
#				TARGETS ${p_name}
#				VERSION 9.0.0
#			)
#		endif()
		if(COMMAND clang_format)
			clang_format(
				TARGETS ${p_name}
				DEPENDENCY
				VERSION 9.0.0
			)
		endif()
	endif()
endfunction()

function(define_library p_name)
	message(STATUS "Defining library '${p_name}'...")
	define_target(${p_name} STATIC)
	set_target_properties(${p_name} PROPERTIES
		FOLDER "TonPlugins/Libraries"
		PROJECT_LABEL "${p_name}"
	)
endfunction()

function(define_plugin p_name)
	message(STATUS "Defining plugin '${p_name}'...")
	define_target(${p_name} STATIC)
	set_target_properties(${p_name} PROPERTIES
		FOLDER "TonPlugins/Plugins"
		PROJECT_LABEL "${p_name}"
	)

	# Add the common library to the whole mix.
	target_link_libraries(${p_name} PUBLIC
		# Steinberg VSTGUI
		vstgui_support
		# TonPlugins
		TonPlugIns::Core
		# Steinberg VST3 SDK
		sdk
	)
	target_compile_definitions(${p_name} PUBLIC
		$<$<CONFIG:Debug>:VSTGUI_LIVE_EDITING=1>
	)
endfunction()

function(define_interface_vst3 p_name)
	set(INTERFACE_NAME "${p_name}.vst3")
	set(INTERFACE_NAME "${INTERFACE_NAME}" PARENT_SCOPE)

	define_target(${INTERFACE_NAME} MODULE)
	set_target_properties(${INTERFACE_NAME} PROPERTIES
		FOLDER "TonPlugins/Plugins"
		PROJECT_LABEL "${INTERFACE_NAME}"
	)
	target_compile_definitions(${INTERFACE_NAME} PRIVATE
		TONPLUGINS_VST3
	)
	target_link_libraries(${INTERFACE_NAME} PUBLIC
		# Parent project
		$<LINK_LIBRARY:WHOLE_ARCHIVE,${p_name}>
	)

	# Add some special files
	if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
		target_sources(${INTERFACE_NAME} PRIVATE
			"${CMAKE_SOURCE_DIR}/third-party/vst3sdk/public.sdk/source/main/macexport.exp"
		)
	endif()

	# VST3 Specific requirements
	set_target_properties(${INTERFACE_NAME} PROPERTIES
		# We don't want the suffix twice.
		OUTPUT_NAME "${p_name}"
		# MacOS requires this to be a bundle.
		BUNDLE ON
		BUNDLE_EXTENSION ".vst3"
	)
	if(WIN32)
		set_target_properties(${INTERFACE_NAME} PROPERTIES
			# VST3 musts have ".vst3" at the end of their name.
			SUFFIX ".vst3"
		)
	elseif(APPLE)
		set_target_properties(${INTERFACE_NAME} PROPERTIES
			# VST3 musts have no suffix.
			SUFFIX ""
		)
	endif()

	# Install as a VST3.x plug-in
	set(_path "${CMAKE_INSTALL_PREFIX_VST3}/TonPlugins/${p_name}.vst3")
	if(NOT APPLE)
		set(_content_path "${_path}/Contents")
		set(_resource_path "${_content_path}/Resources")
		set(_binary_path "${_content_path}/")
		if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
			if(TARGET_SYSTEM_IS_X86 AND TARGET_SYSTEM_IS_ARM)
				set(_binary_path "${_binary_path}arm64ec")
			elseif(TARGET_SYSTEM_IS_X86 AND TARGET_SYSTEM_IS_64BIT)
				set(_binary_path "${_binary_path}x86_64")
			elseif(TARGET_SYSTEM_IS_ARM AND TARGET_SYSTEM_IS_64BIT)
				set(_binary_path "${_binary_path}arm64")
			elseif(TARGET_SYSTEM_IS_X86)
				set(_binary_path "${_binary_path}x86")
			elseif(TARGET_SYSTEM_IS_ARM)
				set(_binary_path "${_binary_path}arm")
			else()
				set(_binary_path "${_binary_path}unknown")
			endif()
			set(_binary_path "${_binary_path}-win")
		elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
			if(TARGET_SYSTEM_IS_X86 AND TARGET_SYSTEM_IS_64BIT)
				set(_binary_path "${_binary_path}x86_64")
			elseif(TARGET_SYSTEM_IS_ARM AND TARGET_SYSTEM_IS_64BIT)
				set(_binary_path "${_binary_path}aarch64")
			elseif(TARGET_SYSTEM_IS_X86)
				set(_binary_path "${_binary_path}i386")
			elseif(TARGET_SYSTEM_IS_ARM)
				set(_binary_path "${_binary_path}arm")
			else()
				set(_binary_path "${_binary_path}unknown")
			endif()
			set(_binary_path "${_binary_path}-linux")
		elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
			set(_binary_path "${_binary_path}MacOS")
		else()
			set(_binary_path "${_binary_path}Unknown")
		endif()
		# - Resources
		install(DIRECTORY "${${INTERFACE_NAME}_SOURCE_DIR}/resources/" DESTINATION "${_resource_path}" USE_SOURCE_PERMISSIONS OPTIONAL)
		install(DIRECTORY "${${INTERFACE_NAME}_BINARY_DIR}/resources/" DESTINATION "${_resource_path}" USE_SOURCE_PERMISSIONS OPTIONAL)
		# - Binaries
		install(TARGETS ${INTERFACE_NAME}
			LIBRARY DESTINATION "${_binary_path}"
			RUNTIME DESTINATION "${_binary_path}"
		)
		if(MSVC)
			install(FILES "$<TARGET_PDB_FILE:${INTERFACE_NAME}>" DESTINATION "${_binary_path}" OPTIONAL)
		endif()
		# - Extra Files
		if(D_PLATFORM_WINDOWS)
			# 'desktop.ini'
			#install(FILES "templates/desktop.ini" DESTINATION "${VST3_INSTALL_PREFIX}")
			#install(CODE "execute_process(COMMAND attrib +S +H \"${CMAKE_INSTALL_PREFIX}/${VST3_INSTALL_PREFIX}/desktop.ini\")")

			# 'icon.ico'
			#install(FILES "templates/icon.ico" DESTINATION "${VST3_INSTALL_PREFIX}")
			#install(CODE "execute_process(COMMAND attrib +H \"${CMAKE_INSTALL_PREFIX}/${VST3_INSTALL_PREFIX}/icon.ico\")")
		endif()
	else()
		install(TARGETS ${INTERFACE_NAME}
			BUNDLE DESTINATION "." COMPONENT ${INTERFACE_NAME}
		)
	endif()
endfunction()

function(define_interface_vst2 p_name)
	set(INTERFACE_NAME "${p_name}.vst2")
	set(INTERFACE_NAME "${INTERFACE_NAME}" PARENT_SCOPE)

	define_target(${INTERFACE_NAME} MODULE)
	set_target_properties(${INTERFACE_NAME} PROPERTIES
		FOLDER "TonPlugins/Plugins"
		PROJECT_LABEL "${INTERFACE_NAME}"
	)
	target_compile_definitions(${INTERFACE_NAME} PRIVATE
		TONPLUGINS_VST3
	)
	target_link_libraries(${INTERFACE_NAME} PUBLIC
		# Parent project
		$<LINK_LIBRARY:WHOLE_ARCHIVE,${p_name}>
		# Steinberg VST2 SDK
		vst2sdk
	)

	# VST2 Specific requirements
	set_target_properties(${INTERFACE_NAME} PROPERTIES
		# We don't want the suffix twice.
		OUTPUT_NAME "${p_name}"
		# MacOS requires this to be a bundle.
		BUNDLE ON
		BUNDLE_EXTENSION ".vst2"
	)
	if(WIN32)
		set_target_properties(${INTERFACE_NAME} PROPERTIES
			SUFFIX ".dll"
		)
	elseif(APPLE)
		set_target_properties(${INTERFACE_NAME} PROPERTIES
			SUFFIX ""
		)
	endif()

	# Install as a VST2.x plug-in
	set(_path "${CMAKE_INSTALL_PREFIX_VST2}/TonPlugins/${p_name}")
	if(NOT APPLE)
		set(_content_path "${_path}/Contents")
		set(_resource_path "${_content_path}/resources")
		set(_binary_path "${_content_path}/")
		# - Resources
		install(DIRECTORY "${${INTERFACE_NAME}_SOURCE_DIR}/resources/" DESTINATION "${_resource_path}" USE_SOURCE_PERMISSIONS OPTIONAL)
		install(DIRECTORY "${${INTERFACE_NAME}_BINARY_DIR}/resources/" DESTINATION "${_resource_path}" USE_SOURCE_PERMISSIONS OPTIONAL)
		# - Binaries
		install(TARGETS ${INTERFACE_NAME}
			LIBRARY DESTINATION "${_binary_path}"
			RUNTIME DESTINATION "${_binary_path}"
		)
		if(MSVC)
			install(FILES "$<TARGET_PDB_FILE:${INTERFACE_NAME}>" DESTINATION "${_binary_path}" OPTIONAL)
		endif()
		# - Extra Files
		if(D_PLATFORM_WINDOWS)
			# 'desktop.ini'
			#install(FILES "templates/desktop.ini" DESTINATION "${VST3_INSTALL_PREFIX}")
			#install(CODE "execute_process(COMMAND attrib +S +H \"${CMAKE_INSTALL_PREFIX}/${VST3_INSTALL_PREFIX}/desktop.ini\")")

			# 'icon.ico'
			#install(FILES "templates/icon.ico" DESTINATION "${VST3_INSTALL_PREFIX}")
			#install(CODE "execute_process(COMMAND attrib +H \"${CMAKE_INSTALL_PREFIX}/${VST3_INSTALL_PREFIX}/icon.ico\")")
		endif()
	else()
		install(TARGETS ${INTERFACE_NAME}
			BUNDLE DESTINATION "." COMPONENT ${INTERFACE_NAME}
		)
	endif()
endfunction()

################################################################################
# Common Library
################################################################################
file(GLOB library_list LIST_DIRECTORIES true "library/**")
foreach(library ${library_list})
	add_subdirectory(${library})
endforeach()

################################################################################
# Plugins
################################################################################

file(GLOB plugins_list LIST_DIRECTORIES true "plugins/**")
foreach(plugin ${plugins_list})
	add_subdirectory(${plugin})
endforeach()
