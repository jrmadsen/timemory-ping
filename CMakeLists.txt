cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(timemory-ping LANGUAGES CXX)

find_package(timemory REQUIRED COMPONENTS headers compile-options analysis-tools)

add_executable(ting ting.cpp ting.hpp)
target_link_libraries(ting timemory::timemory)
install(TARGETS ting DESTINATION bin)
