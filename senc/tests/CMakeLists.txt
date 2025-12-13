add_executable(senc_tests
    "test_main.cpp"
    "test_ranges.cpp"
    "test_socket.cpp"
    "test_poly.cpp"
    "test_modint.cpp"
    "test_fraction.cpp"
    "test_enc.cpp"
    "test_shamir.cpp"
    "test_threshold_enc.cpp"
    "test_common.cpp"
    "test_strs.cpp"
    "tests_utils.hpp"
    "tests_utils.cpp"
    "test_storage.cpp"
    "test_server.cpp"
    "../server/ClientHandlerFactory.cpp"
    "../server/ConnectedClientHandler.cpp"
    "../server/ConnectingClientHandler.cpp"
    "../server/DecryptionsManager.cpp"
    "../server/Server.cpp"
    "../server/ShortTermServerStorage.cpp"
    "../server/UpdateManager.cpp"
)

target_link_libraries(senc_tests PRIVATE
    GTest::gtest_main
    senc_utils
    senc_common
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(senc_tests)
