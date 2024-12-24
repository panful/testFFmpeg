cmake_minimum_required(VERSION 3.4...3.20 FATAL_ERROR)

project(GLFW_LIB_GENERATOR VERSION 3.4.0 LANGUAGES C)

include(CMakeDependentOption)
cmake_dependent_option(GLFW_BUILD_WIN32 "Build support for Win32" ON "WIN32" OFF)
cmake_dependent_option(GLFW_BUILD_X11 "Build support for X11" ON "UNIX;NOT APPLE" OFF)

set(GLFW_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(src)
