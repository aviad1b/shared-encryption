add_library(senc_common
	"SockUtils.hpp"
	"SockUtils.cpp"
	"aliases.hpp"
	"aliases.cpp"
	"packets.hpp"
	"ConnEstablishException.hpp"
	"PacketHandler.hpp"
	"PacketHandlerFactory.hpp"
	"InlinePacketHandler.hpp"
	"InlinePacketHandler.cpp"
	"EncryptedPacketHandler.hpp"
	"EncryptedPacketHandler.cpp"
)

set_property(TARGET senc_common PROPERTY CXX_STANDARD 20)

set_target_properties(senc_common PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(senc_common PRIVATE senc_utils)
