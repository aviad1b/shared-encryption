add_executable(senc_client
	"main.cpp"
	"input.hpp"
	"input.cpp"
	"input_impl.hpp"
	"output.hpp"
	"output.cpp"
)

set_property(TARGET senc_client PROPERTY CXX_STANDARD 20)

target_link_libraries(senc_client PUBLIC
	senc_utils
	senc_common
)
