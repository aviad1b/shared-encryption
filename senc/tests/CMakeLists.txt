add_executable(senc_tests
    "test_main.cpp"
    "test_ranges.cpp"
    "test_bin_file.cpp"
    "test_socket.cpp"
    "test_poly.cpp"
    "test_modint.cpp"
    "test_fraction.cpp"
    "test_enc.cpp"
    "test_shamir.cpp"
    "test_threshold_enc.cpp"
    "test_common.cpp"
    "test_strs.cpp"
    "test_bytes.cpp"
    "test_sql.cpp"
    "tests_utils.hpp"
    "test_server_storage.cpp"
    "test_server.cpp"
    "../server/handlers/ClientHandlerFactory.cpp"
    "../server/handlers/ConnectedClientHandler.cpp"
    "../server/handlers/ConnectingClientHandler.cpp"
    "../server/managers/DecryptionsManager.cpp"
    "../server/Server_impl.hpp"
    "../server/storage/ShortTermServerStorage.cpp"
    "../server/storage/SqliteServerStorage.cpp"
    "../server/managers/UpdateManager.cpp"
)

set_property(TARGET senc_common PROPERTY CXX_STANDARD 20)

target_link_libraries(senc_tests PRIVATE
    GTest::gtest_main
    senc_utils
    senc_common
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(senc_tests)
