add_library(senc_packets
	"packets.hpp"
)

target_link_libraries(senc_packets PRIVATE senc_utils)

set_property(TARGET senc_packets PROPERTY CXX_STANDARD 20)
