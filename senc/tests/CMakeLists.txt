include(FetchContent)

# download GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/v1.17.x.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(senc_tests
    "test_main.cpp"
    "test_socket.cpp"
    "test_poly.cpp"
    "test_modint.cpp"
    "test_fraction.cpp"
)

target_link_libraries(senc_tests PRIVATE
    GTest::gtest_main
    senc_utils
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(senc_tests)
