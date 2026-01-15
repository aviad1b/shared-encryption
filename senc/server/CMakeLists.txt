add_executable (senc_server
	"aliases.hpp"
	"ServerException.hpp"
	"IServerStorage.hpp"
	"ShortTermServerStorage.hpp"
	"ShortTermServerStorage.cpp"
	"ConnectingClientHandler.hpp"
	"ConnectingClientHandler.cpp"
	"ConnectedClientHandler.hpp"
	"ConnectedClientHandler.cpp"
	"ClientHandlerFactory.hpp"
	"ClientHandlerFactory.cpp"
	"UpdateManager.hpp"
	"UpdateManager.cpp"
	"DecryptionsManager.hpp"
	"DecryptionsManager.cpp"
	"InteractiveConsole.hpp"
	"InteractiveConsole_windows.cpp"
	"InteractiveConsole_linux.cpp"
	"Logger.hpp"
	"DummyLogger.hpp"
	"ConsoleLogger.hpp"
	"ConsoleLogger.cpp"
	"IServer.hpp"
	"Server.hpp"
	"Server_impl.hpp"
	"main.cpp"
)

set_property(TARGET senc_server PROPERTY CXX_STANDARD 20)

target_link_libraries(senc_server PUBLIC
	senc_utils
	senc_common
)
