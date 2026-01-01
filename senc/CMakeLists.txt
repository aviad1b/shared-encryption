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
FetchContent_Declare(
    cryptopp
    GIT_REPOSITORY https://github.com/weidai11/cryptopp.git
    GIT_TAG CRYPTOPP_8_9_0
)

FetchContent_GetProperties(cryptopp)
if(NOT cryptopp_POPULATED)
    FetchContent_Populate(cryptopp)
    
    # remove any VS project files from CryptoPP source
    file(GLOB VS_FILES 
        "${cryptopp_SOURCE_DIR}/*.sln"
        "${cryptopp_SOURCE_DIR}/*.vcxproj"
        "${cryptopp_SOURCE_DIR}/*.vcxproj.filters"
    )
    if(VS_FILES)
        file(REMOVE ${VS_FILES})
    endif()
    
    # create a cryptopp subdirectory wrapper for proper includes
    set(CRYPTOPP_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/cryptopp_include")
    file(MAKE_DIRECTORY "${CRYPTOPP_INCLUDE_DIR}/cryptopp")
    
    # copy all headers to cryptopp/ subdirectory
    file(GLOB CRYPTOPP_HEADERS "${cryptopp_SOURCE_DIR}/*.h")
    foreach(header ${CRYPTOPP_HEADERS})
        get_filename_component(header_name ${header} NAME)
        configure_file(${header} "${CRYPTOPP_INCLUDE_DIR}/cryptopp/${header_name}" COPYONLY)
    endforeach()
    
    # get all cpp files
    file(GLOB CRYPTOPP_SOURCES "${cryptopp_SOURCE_DIR}/*.cpp")
    
    # filter out test and bench files
    foreach(source_file ${CRYPTOPP_SOURCES})
        get_filename_component(filename ${source_file} NAME)
        if(NOT filename MATCHES "^(test|bench|valid|dlltest|datatest)")
            list(APPEND CRYPTOPP_SOURCES_FILTERED ${source_file})
        endif()
    endforeach()
    
    add_library(cryptopp STATIC ${CRYPTOPP_SOURCES_FILTERED})
    
    # include both the source dir (for internal includes) and wrapper dir (for external includes)
    target_include_directories(cryptopp 
        PRIVATE ${cryptopp_SOURCE_DIR}
        PUBLIC ${CRYPTOPP_INCLUDE_DIR}
    )
    
    # add SIMD compiler flags for GCC/MinGW
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(cryptopp PRIVATE
            -march=native
            -maes -msse4.1 -msse4.2 -mssse3 -mpclmul
        )
    elseif(MSVC)
        target_compile_options(cryptopp PRIVATE /arch:AVX2)
    endif()
    
    set_target_properties(cryptopp PROPERTIES 
        FOLDER "ThirdParty"
        EXCLUDE_FROM_ALL TRUE
        EXCLUDE_FROM_DEFAULT_BUILD TRUE
    )
endif()

# download uuid-dev for linux only
if (WIN32)
else()
FetchContent_Declare(
    libuuid
    GIT_REPOSITORY https://git.kernel.org/pub/scm/utils/util-linux/util-linux.git
    GIT_TAG v2.39
)
FetchContent_MakeAvailable(libuuid)
endif()

add_compile_options(-Werror -Wall -Wextra -Wvla)

add_subdirectory("utils")
add_subdirectory("common")
add_subdirectory("server")
add_subdirectory("client")

option(BUILD_TESTING "Enable unit tests" ON)
if (BUILD_TESTING)
    add_subdirectory("tests")
endif()
