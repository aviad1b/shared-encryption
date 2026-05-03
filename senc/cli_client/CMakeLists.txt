add_executable(senc_cli_client
	"main.cpp"
)

set_property(TARGET senc_cli_client PROPERTY CXX_STANDARD 20)

target_link_libraries(senc_cli_client PUBLIC
	senc_client_api
)
