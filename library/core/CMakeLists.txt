# Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

################################################################################
# Bootstrap
################################################################################
cmake_minimum_required(VERSION 3.26)
project(Core)
list(APPEND CMAKE_MESSAGE_INDENT "[${PROJECT_NAME}] ")
define_library(${PROJECT_NAME})

################################################################################
# Finish
################################################################################
setup_target(${PROJECT_NAME})
