cmake_minimum_required (VERSION 3.10)
project(aybabtu)

file(GLOB aybabtu_headers ./src/aybabtu/*.hpp)
file(GLOB_RECURSE aybabtu_sources ./src/*.cpp)

add_library(aybabtu STATIC ${aybabtu_sources})
add_library(steinwurf::aybabtu ALIAS aybabtu)

target_include_directories(aybabtu INTERFACE src/)

install(FILES ${aybabtu_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/aybabtu)
install(FILES $<TARGET_FILE:aybabtu> DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
