cmake_minimum_required(VERSION 3.14)

project("senc" LANGUAGES CXX)

if(MSVC)
    # force GoogleTest to use dynamic runtime
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

include(FetchContent)
cmake_policy(SET CMP0135 NEW)

# download GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/v1.17.x.zip
)
FetchContent_MakeAvailable(googletest)

add_compile_definitions(CRYPTOPP_DISABLE_ASM)

# download cryptopp-modern
FetchContent_Declare(
    cryptopp-cmake
    GIT_REPOSITORY https://github.com/abdes/cryptopp-cmake.git
    GIT_TAG CRYPTOPP_8_9_0
)

# disable cryptopp tests & make cryptopp available
set(CRYPTOPP_BUILD_TESTING OFF CACHE BOOL "Disable cryptopp tests" FORCE)
set(CRYPTOPP_BUILD_DOCUMENTATION OFF CACHE BOOL "Disable cryptopp docs" FORCE)
FetchContent_MakeAvailable(cryptopp-cmake)

# remove VS project files from the fetched cryptopp source to prevent VS confusion
FetchContent_GetProperties(cryptopp-cmake)
if(cryptopp-cmake_POPULATED)
    set(CRYPTOPP_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/cryptopp-cmake-build/cryptopp")
    file(GLOB VS_FILES 
        "${CRYPTOPP_SOURCE_DIR}/*.sln"
        "${CRYPTOPP_SOURCE_DIR}/*.vcxproj"
        "${CRYPTOPP_SOURCE_DIR}/*.vcxproj.filters"
    )
    if(VS_FILES)
        file(REMOVE ${VS_FILES})
    endif()
endif()

# use libuuid in linux only
if (WIN32)
else()
    find_path(UUID_INCLUDE_DIR uuid/uuid.h)
    find_library(UUID_LIBRARY NAMES uuid)
    
    if(NOT UUID_INCLUDE_DIR OR NOT UUID_LIBRARY)
        message(FATAL_ERROR "libuuid not found. Install with: sudo apt-get install uuid-dev")
    endif()
    
    # create an interface library to link against
    add_library(libuuid INTERFACE)
    target_include_directories(libuuid INTERFACE ${UUID_INCLUDE_DIR})
    target_link_libraries(libuuid INTERFACE ${UUID_LIBRARY})
endif()

if(MSVC)
    # add_compile_options(/W4) # warning level 4
else()
    add_compile_options(-Werror -Wall -Wextra -Wvla)
endif()

add_subdirectory("utils")
add_subdirectory("common")
add_subdirectory("server")
add_subdirectory("client")

option(BUILD_TESTING "Enable unit tests" ON)
if (BUILD_TESTING)
    add_subdirectory("tests")
endif()
