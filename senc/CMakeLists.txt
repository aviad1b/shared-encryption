cmake_minimum_required(VERSION 3.14)

project("senc")
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

# download crypto++
set(CRYPTOPP_BUILD_TESTING OFF CACHE BOOL "Disable Crypto++ tests" FORCE)
FetchContent_Declare(
    cryptopp-cmake
    GIT_REPOSITORY https://github.com/abdes/cryptopp-cmake.git
    GIT_TAG CRYPTOPP_8_9_0
)
FetchContent_MakeAvailable(cryptopp-cmake)

add_compile_options(-Werror -Wall -Wextra -Wvla)

add_subdirectory("utils")
add_subdirectory("server")
add_subdirectory("client")

option(BUILD_TESTING "Enable unit tests" ON)
if (BUILD_TESTING)
    add_subdirectory("tests")
endif()
