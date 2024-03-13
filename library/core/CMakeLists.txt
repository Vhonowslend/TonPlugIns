# Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

################################################################################
# Bootstrap
################################################################################
cmake_minimum_required(VERSION 3.26)
list(APPEND CMAKE_MESSAGE_INDENT "[VoiceFX] ")

project(Core)
define_library(${PROJECT_NAME})

################################################################################
# Finish
################################################################################
setup_target(${PROJECT_NAME})
