add_library(senc_common
	"SockUtils.hpp"
	"SockUtils.cpp"
	"aliases.hpp"
	"packets.hpp"
	"PacketHandler.hpp"
	"PacketHandlerFactory.hpp"
	"InlinePacketHandler.hpp"
	"InlinePacketHandler.cpp"
	"EncryptedPacketHandler.hpp"
	"EncryptedPacketHandler.cpp"
)

target_link_libraries(senc_common PRIVATE senc_utils)

set_property(TARGET senc_common PROPERTY CXX_STANDARD 20)
