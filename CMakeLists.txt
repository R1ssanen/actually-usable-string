cmake_minimum_required(VERSION 3.19)
add_library(xstr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/xstr.c)
target_include_directories(xstr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})