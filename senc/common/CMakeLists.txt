add_library(senc_common
	"packets.hpp"
	"common.cpp"
)

target_link_libraries(senc_common PRIVATE senc_utils)

set_property(TARGET senc_common PROPERTY CXX_STANDARD 20)
