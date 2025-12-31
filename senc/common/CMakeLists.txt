add_library(senc_common
	"aliases.hpp"
	"packets.hpp"
	"PacketSender.hpp"
	"PacketReceiver.hpp"
	"InlinePacketSender.hpp"
	"InlinePacketSender.cpp"
	"InlinePacketReceiver.hpp"
	"InlinePacketReceiver.cpp"
)

target_link_libraries(senc_common PRIVATE senc_utils)

set_property(TARGET senc_common PROPERTY CXX_STANDARD 20)
