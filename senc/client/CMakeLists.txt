add_executable(senc_client
	"main.cpp"
	"io/input.hpp"
	"io/input.cpp"
	"io/input_impl.hpp"
	"io/output.hpp"
	"io/output.cpp"
)

set_property(TARGET senc_client PROPERTY CXX_STANDARD 20)

target_link_libraries(senc_client PUBLIC
	senc_utils
	senc_common
)
