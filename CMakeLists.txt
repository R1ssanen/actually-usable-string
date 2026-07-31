cmake_minimum_required(VERSION 3.10)
add_library(xstr INTERFACE)
target_sources(xstr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/xstr.c)
target_include_directories(xstr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})