# Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

project(TonStream)
define_plugin(TonStream)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
	Boost::boost
)

setup_target(${PROJECT_NAME})
