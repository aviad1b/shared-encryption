add_library (senc_utils
	"concepts.hpp"
	"Exception.hpp"
	"Exception.cpp"
	"Socket.hpp"
	"Socket.cpp"
	"Socket_impl.hpp"
	"poly.hpp"
	"poly_impl.hpp"
)

target_link_libraries(senc_utils PRIVATE ws2_32)

set_property(TARGET senc_utils PROPERTY CXX_STANDARD 20)
