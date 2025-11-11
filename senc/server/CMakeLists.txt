add_executable (senc_server
	"main.cpp"
)

set_property(TARGET senc_server PROPERTY CXX_STANDARD 20)

target_link_libraries(senc_server PUBLIC
	senc_utils
)

target_include_directories(
	senc_server PUBLIC
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_BINARY_DIR}/utils"
)
