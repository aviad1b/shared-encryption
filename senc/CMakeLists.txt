cmake_minimum_required(VERSION 3.12)

project("senc")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Werror -Wall -Wextra -Wvla)

add_subdirectory("senc")
