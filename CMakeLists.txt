cmake_minimum_required(VERSION 3.12)
project(aybabtu)

find_package(Python COMPONENTS Interpreter)

# Use waf to resolve dependencies
if(NOT DEFINED STEINWURF_RESOLVE)
    message(STATUS "Resolving dependencies...")
    execute_process(
        COMMAND ${Python_EXECUTABLE} waf resolve ${STEINWURF_RESOLVE_OPTIONS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE STATUS)

    if(STATUS AND NOT STATUS EQUAL 0)
        message(FATAL_ERROR "Failed: ${STATUS}")
    endif()

    set(STEINWURF_RESOLVE "${CMAKE_CURRENT_SOURCE_DIR}/resolve_symlinks")
    set(STEINWURF_TOP_NAME aybabtu)
endif()

# platform dependency
if(NOT TARGET steinwurf::platform)
    add_subdirectory("${STEINWURF_RESOLVE}/platform" platform EXCLUDE_FROM_ALL)
endif()

# cpuid dependency
if(NOT TARGET steinwurf::cpuid)
    add_subdirectory("${STEINWURF_RESOLVE}/cpuid" cpuid EXCLUDE_FROM_ALL)
endif()

# Define library
file(GLOB_RECURSE aybabtu_sources ./src/*.cpp)

# Is this the top-level steinwurf project?
if(${PROJECT_NAME} STREQUAL ${STEINWURF_TOP_NAME})
    # Create static library
    add_library(aybabtu STATIC ${aybabtu_sources})

    # Get all steinwurf object libraries and link directly with them.
    get_property(steinwurf_object_libraries GLOBAL
        PROPERTY steinwurf::object_libraries)
    target_link_libraries(aybabtu PRIVATE ${steinwurf_object_libraries})

    # Install library
    install(FILES $<TARGET_FILE:aybabtu> DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
else()
    # Create object library
    add_library(aybabtu OBJECT ${aybabtu_sources})

    # Add this library to a global list of steinwurf object libraries
    set_property(GLOBAL APPEND PROPERTY steinwurf::object_libraries
        steinwurf::aybabtu)

    # Link object dependencies
    target_link_libraries(aybabtu PRIVATE steinwurf::cpuid)
endif()

# Link header only dependencies
target_link_libraries(aybabtu PRIVATE steinwurf::platform)

# Check Accelerations
include(CheckCXXCompilerFlag)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|AppleClang|Clang")
    # # SSE2
    # check_cxx_compiler_flag("-Werror -msse2" HAS_SSE2)

    # if(HAS_SSE2)
    # file(GLOB_RECURSE sse2 ./src/*sse2.cpp)
    # set_source_files_properties(${sse2} PROPERTIES COMPILE_FLAGS -msse2)
    # endif()

    # SSSE3
    check_cxx_compiler_flag("-Werror -mssse3" HAS_SSSE3)

    if(HAS_SSSE3)
        file(GLOB_RECURSE ssse3 ./src/*ssse3.cpp)
        set_source_files_properties(${ssse3} PROPERTIES COMPILE_FLAGS -mssse3)
    endif()

    # # SSE4.2
    # check_cxx_compiler_flag("-Werror -msse4.2" HAS_SSE42)

    # if(HAS_SSE42)
    # file(GLOB_RECURSE sse42 ./src/*sse42.cpp)
    # set_source_files_properties(${sse42} PROPERTIES COMPILE_FLAGS -msse4.2)
    # endif()

    # AVX2
    check_cxx_compiler_flag("-Werror -mavx2" HAS_AVX2)

    if(HAS_AVX2)
        file(GLOB_RECURSE avx2 ./src/*avx2.cpp)
        set_source_files_properties(${avx2} PROPERTIES COMPILE_FLAGS -mavx2)
    endif()

    # NEON
    check_cxx_compiler_flag("-Werror -mfpu=neon" HAS_NEON)

    if(HAS_NEON)
        file(GLOB_RECURSE neon ./src/*neon.cpp)
        set_source_files_properties(${neon} PROPERTIES COMPILE_FLAGS -mfpu=neon)
    endif()
elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
    # AVX2
    check_cxx_compiler_flag(/arch:AVX2 HAS_AVX2)

    if(HAS_AVX2)
        file(GLOB_RECURSE avx2 ./src/*avx2.cpp)
        set_source_files_properties(${avx2} PROPERTIES COMPILE_FLAGS /arch:AVX2)
    endif()
endif()

target_include_directories(aybabtu INTERFACE src)
target_compile_features(aybabtu PUBLIC cxx_std_11)
add_library(steinwurf::aybabtu ALIAS aybabtu)

# Install headers excluding "detail" as these are internal to the library.
install(
    DIRECTORY ./src/aybabtu
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
    FILES_MATCHING
    PATTERN *.hpp
    PATTERN ./src/aybabtu/detail EXCLUDE)

# Is top level project?
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    add_executable(example examples/example.cpp)
    target_link_libraries(example steinwurf::aybabtu)
endif()
