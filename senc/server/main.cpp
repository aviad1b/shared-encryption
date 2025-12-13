#include <iostream>
#include "../common/InlinePacketReceiver.hpp"
#include "../common/InlinePacketSender.hpp"
#include "ShortTermServerStorage.hpp"
#include "Server.hpp"

using senc::server::ShortTermServerStorage;
using senc::server::DecryptionsManager;
using senc::server::UpdateManager;
using senc::InlinePacketReceiver;
using senc::InlinePacketSender;
using senc::server::Server;
using senc::utils::Port;
using senc::Schema;

constexpr Port DEFAULT_LISTEN_PORT = 4435;

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
			std::cout << "Bad port: " << argv[1] << std::endl;
			return 1;
		}
	}

	Schema schema;
	InlinePacketReceiver receiver;
	InlinePacketSender sender;
	ShortTermServerStorage storage;
	UpdateManager updateManager;
	DecryptionsManager decryptionsManager;
	Server server(
		port,
		schema,
		storage,
		receiver,
		sender,
		updateManager,
		decryptionsManager
	);

	server.start();
	std::cout << "[info] Server listening at port " << port << "." << std::endl;
	std::cout << "[info] Use \"stop\" to stop server." << std::endl;

	std::string cmd;
	while (cmd != "stop")
	{
		std::cout << "> ";
		std::getline(std::cin, cmd);
	}

	server.stop();
	server.wait();

	return 0;
}
