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
    "tests_utils.cpp"
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
    "test_client_storage.cpp"
    "../client_api/storage/ProfileRecord.cpp"
    "../client_api/storage/ProfileStorage.cpp"
    "test_client_api.cpp"
)

set_property(TARGET senc_common PROPERTY CXX_STANDARD 20)

target_link_libraries(senc_tests PRIVATE
    GTest::gtest_main
    senc_utils
    senc_common
    senc_client_api
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(senc_tests)
