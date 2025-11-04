add_library (senc_utils
	"Exception.hpp"
	"Exception.cpp"
	"Socket.hpp"
	"Socket.cpp"
	"Socket_impl.hpp"
)

set_property(TARGET senc_utils PROPERTY CXX_STANDARD 20)
