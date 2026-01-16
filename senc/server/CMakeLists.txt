add_executable (senc_server
	"aliases.hpp"
	"ServerException.hpp"
	"storage/IServerStorage.hpp"
	"storage/ShortTermServerStorage.hpp"
	"storage/ShortTermServerStorage.cpp"
	"handlers/ConnectingClientHandler.hpp"
	"handlers/ConnectingClientHandler.cpp"
	"handlers/ConnectedClientHandler.hpp"
	"handlers/ConnectedClientHandler.cpp"
	"handlers/ClientHandlerFactory.hpp"
	"handlers/ClientHandlerFactory.cpp"
	"managers/UpdateManager.hpp"
	"managers/UpdateManager.cpp"
	"managers/DecryptionsManager.hpp"
	"managers/DecryptionsManager.cpp"
	"io/InteractiveConsole.hpp"
	"io/InteractiveConsole_windows.cpp"
	"io/InteractiveConsole_linux.cpp"
	"loggers/ILogger.hpp"
	"loggers/DummyLogger.hpp"
	"loggers/ConsoleLogger.hpp"
	"loggers/ConsoleLogger.cpp"
	"loggers/ConnectingClientLogger.hpp"
	"loggers/ConnectedClientLogger.hpp"
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
