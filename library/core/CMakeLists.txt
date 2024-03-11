# Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

project(Core
	VERSION ${PROJECT_VERSION}
)

define_library(${PROJECT_NAME})

configure_file(
	"templates/include/version.hpp.in"
	"include/version.hpp"
)
configure_file(
	"templates/source/version.cpp.in"
	"source/version.cpp"
)

configure_file(
	"templates/include/version.rc.in"
	"include/version.rc"
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
	TONPLUGINS_VERSION="${_VERSION}"
	TONPLUGINS_VERSION_MAJOR=${_VERSION_MAJOR}
	TONPLUGINS_VERSION_MINOR=${_VERSION_MINOR}
	TONPLUGINS_VERSION_PATCH=${_VERSION_PATCH}
	TONPLUGINS_VERSION_BUILD="${_VERSION_BUILD}"
	TONPLUGINS_VERSION_PRERELEASE="${_VERSION_PRERELEASE}"
	TONPLUGINS_VERSION_CMAKE="${PROJECT_VERSION}"
	TONPLUGINS_VERSION="${_VERSION}"
)
if(NOT ENABLE_FULL_VERSION)
	target_compile_definitions(${PROJECT_NAME} PUBLIC
		TONPLUGINS_DEMO
	)
endif()

setup_target(${PROJECT_NAME})
