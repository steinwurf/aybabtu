cmake_minimum_required (VERSION 3.10)
project(aybabtu)

file(GLOB_RECURSE aybabtu_sources ./src/*.cpp)

add_library(aybabtu STATIC ${aybabtu_sources})
add_library(steinwurf::aybabtu ALIAS aybabtu)

target_include_directories(aybabtu INTERFACE src/)
