#include <iostream>
#include "../common/InlinePacketReceiver.hpp"
#include "../common/InlinePacketSender.hpp"
#include "ShortTermServerStorage.hpp"
#include "InteractiveConsole.hpp"
#include "Server.hpp"

namespace senc::server
{
	using utils::Port;

	constexpr Port DEFAULT_LISTEN_PORT = 4435;

	bool handle_cmd(InteractiveConsole& console, const std::string& cmd);

	int main(int argc, char** argv)
	{
		if (argc > 2)
			std::cout << "Usage: " << argv[0] << " [port]" << std::endl;

		Port port = DEFAULT_LISTEN_PORT;
		if (argc >= 2)
		{
			try { port = std::stoi(argv[1]); }
			catch (const std::exception&)
			{
				std::cerr << "Bad port: " << argv[1] << std::endl;
				return 1;
			}
		}

		std::optional<InteractiveConsole> console;
		try { console.emplace(handle_cmd); }
		catch (const std::exception&)
		{
			std::cerr << "Failed to initialize console" << std::endl;
			return 1;
		}

		Schema schema;
		InlinePacketReceiver receiver;
		InlinePacketSender sender;
		ShortTermServerStorage storage;
		UpdateManager updateManager;
		DecryptionsManager decryptionsManager;
		Server server(
			port,
			[&console](const std::string& msg) { console->print(msg); },
			schema,
			storage,
			receiver,
			sender,
			updateManager,
			decryptionsManager
		);

		server.start();

		console->print("[info] Server listening at port " + std::to_string(port) + ".");
		console->print("[info] Use \"stop\" to stop server.");

		console->start_inputs(); // start input loop

		server.stop();
		server.wait();

		return 0;
	}

	bool handle_cmd(InteractiveConsole& console, const std::string& cmd)
	{
		(void)console;
		return cmd == "stop"; // stop if command is "stop"
	}
}

int main(int argc, char** argv)
{
	return senc::server::main(argc, argv);
}
