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

	template <utils::IPType IP>
	void start_server(Port port, InteractiveConsole& console, Schema& schema,
				  IServerStorage& storage, PacketReceiver& receiver, PacketSender& sender,
				  UpdateManager& updateManager, DecryptionsManager& decryptionsManager);

	void run_server(IServer& server, InteractiveConsole& console);

	int main(int argc, char** argv)
	{
		if (argc > 3)
			std::cout << "Usage: " << argv[0] << " [IPv4|IPv6] [port]" << std::endl;

		std::vector<std::string> args(argv + 1, argv + argc);
		bool isIPv6 = false;

		// pop if has either "IPv4" or "IPv6", for "IPv6" set isIPv6 to true:
		auto itIPv4 = std::find(args.begin(), args.end(), "IPv4");
		auto itIPv6 = std::find(args.begin(), args.end(), "IPv6");
		if (itIPv4 != args.end() || itIPv6 != args.end()) // if has either IPv4 or IPv6
		{
			if (itIPv6 == args.end()) // if has IPv4 and not IPv6
				args.erase(itIPv4);
			else if (itIPv4 == args.end()) // if has IPv6 and not IPv4
			{
				args.erase(itIPv6);
				isIPv6 = true;
			}
			else
			{
				std::cout << "Usage: " << argv[0] << " [IPv4|IPv6] [port]" << std::endl;
				return 1;
			}
		}

		Port port = DEFAULT_LISTEN_PORT;
		if (args.size() >= 1)
		{
			try { port = std::stoi(args[0]); }
			catch (const std::exception&)
			{
				std::cerr << "Bad port: " << args[0] << std::endl;
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
		
		if (isIPv6)
		{
			start_server<utils::IPv6>(
				port, *console, schema, storage,
				receiver, sender,
				updateManager, decryptionsManager
			);
		}
		else
		{
			start_server<utils::IPv4>(
				port, *console, schema, storage,
				receiver, sender,
				updateManager, decryptionsManager
			);
		}

		return 0;
	}

	bool handle_cmd(InteractiveConsole& console, const std::string& cmd)
	{
		(void)console;
		return cmd == "stop"; // stop if command is "stop"
	}

	template <utils::IPType IP>
	void start_server(Port port, InteractiveConsole& console, Schema& schema,
					  IServerStorage& storage, PacketReceiver& receiver, PacketSender& sender,
					  UpdateManager& updateManager, DecryptionsManager& decryptionsManager)
	{
		Server<IP> server(
			port,
			[&console](const std::string& msg) { console.print("[info] " + msg); },
			schema,
			storage,
			receiver,
			sender,
			updateManager,
			decryptionsManager
		);

		run_server(server, console);
	}

	void run_server(IServer& server, InteractiveConsole& console)
	{
		server.start();

		console.print("[info] Server listening at port " + std::to_string(server.port()) + ".");
		console.print("[info] Use \"stop\" to stop server.");

		console.start_inputs(); // start input loop

		server.stop();
		server.wait();
	}
}

int main(int argc, char** argv)
{
	return senc::server::main(argc, argv);
}
