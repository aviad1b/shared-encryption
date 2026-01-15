#include <iostream>
#include "../common/EncryptedPacketHandler.hpp"
#include "ShortTermServerStorage.hpp"
#include "InteractiveConsole.hpp"
#include "Server.hpp"

namespace senc::server
{
	using utils::Port;

	constexpr Port DEFAULT_LISTEN_PORT = 4435;

	std::tuple<bool, Port> parse_args(int argc, char** argv);

	bool handle_cmd(InteractiveConsole& console, const std::string& cmd);

	template <utils::IPType IP>
	int start_server(Port port, InteractiveConsole& console, Schema& schema,
					 IServerStorage& storage, PacketHandlerFactory& packetHandlerFactory,
					 UpdateManager& updateManager, DecryptionsManager& decryptionsManager);

	void run_server(IServer& server, InteractiveConsole& console);

	int main(int argc, char** argv)
	{
		bool isIPv6 = false;
		Port port{};
		try { std::tie(isIPv6, port) = parse_args(argc, argv); }
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			return 1;
		}

		std::optional<InteractiveConsole> console;
		try { console.emplace(handle_cmd); }
		catch (const std::exception&)
		{
			std::cerr << "Failed to initialize console" << std::endl;
			return 1;
		}

		Schema schema;
		auto packetHandlerFactory = PacketHandlerImplFactory<EncryptedPacketHandler>{};
		ShortTermServerStorage storage;
		UpdateManager updateManager;
		DecryptionsManager decryptionsManager;
		
		if (isIPv6)
			return start_server<utils::IPv6>(
				port, *console, schema, storage,
				packetHandlerFactory,
				updateManager, decryptionsManager
			);
		else
			return start_server<utils::IPv4>(
				port, *console, schema, storage,
				packetHandlerFactory,
				updateManager, decryptionsManager
			);
	}

	/**
	 * @brief Parses program arguments.
	 * @return isIPv6, port
	 * @throw utils::Exception On error.
	 */
	std::tuple<bool, Port> parse_args(int argc, char** argv)
	{
		static const std::string USAGE = std::string("Usage: ") + argv[0] + " [IPv4|IPv6] [port]";
		if (argc > 3)
			throw utils::Exception(USAGE);

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
			else throw utils::Exception(USAGE);
		}

		Port port = DEFAULT_LISTEN_PORT;
		if (args.size() >= 1)
		{
			try { port = utils::parse_port(args[0]); }
			catch (const std::exception&)
			{
				throw utils::Exception("Bad port: " + args[0]);
			}
		}

		return { isIPv6, port };
	}

	/**
	 * @brief Handles a server command input.
	 * @param console Server console (by ref).
	 * @param cmd Inputed command.
	 * @return `true` if server should stop, otherwise `false`.
	 */
	bool handle_cmd(InteractiveConsole& console, const std::string& cmd)
	{
		(void)console;
		return cmd == "stop"; // stop if command is "stop"
	}

	/**
	 * @brief Starts up server (and waits for it to finish runnings).
	 * @param port Server's listen port.
	 * @param schema Server's encryption schema instance (by ref).
	 * @param storage Server's storage instance (by ref).
	 * @param packetHandlerFactory Factory used for constructing packet handlers (by ref).
	 * @param updateManager Server's update manager instance (by ref).
	 * @param decryptionsManager Server's decryptions manager instance (by ref).
	 * @return Server exit code.
	 */
	template <utils::IPType IP>
	int start_server(Port port, InteractiveConsole& console, Schema& schema,
					 IServerStorage& storage, PacketHandlerFactory& packetHandlerFactory,
					 UpdateManager& updateManager, DecryptionsManager& decryptionsManager)
	{
		std::optional<Server<IP>> server;
		try
		{
			server.emplace(
				port,
				[&console](const std::string& msg) { console.print("[info] " + msg); },
				schema,
				storage,
				packetHandlerFactory,
				updateManager,
				decryptionsManager
			);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error initializing server: " << e.what() << std::endl;
			return 1;
		}

		run_server(*server, console);

		return 0;
	}

	/**
	 * @brief Runs server (and waits for it to finish running).
	 * @param server Server to run (by ref).
	 * @param console Server console (by ref).
	 */
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
