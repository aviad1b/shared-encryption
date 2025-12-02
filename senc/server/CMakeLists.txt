add_executable (senc_server
	"ServerException.hpp"
	"IServerStorage.hpp"
	"ShortTermServerStorage.hpp"
	"ShortTermServerStorage.cpp"
	"Server.hpp"
	"Server.cpp"
	"main.cpp"
)

set_property(TARGET senc_server PROPERTY CXX_STANDARD 20)

target_link_libraries(senc_server PUBLIC
	senc_utils
	senc_common
)
