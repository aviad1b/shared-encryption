#include <iostream>
#include "../common/InlinePacketReceiver.hpp"
#include "../common/InlinePacketSender.hpp"
#include "../utils/Socket.hpp"

namespace pkt = senc::pkt;
using senc::InlinePacketReceiver;
using senc::InlinePacketSender;
using senc::utils::TcpSocket;
using senc::utils::Socket;
using senc::utils::IPv4;
using senc::utils::Port;
using std::string;
using std::endl;
using std::cout;

constexpr Port DEFAULT_LISTEN_PORT = 4435;

void run_client(Socket& sock);
bool login_menu(Socket& sock);
void main_menu(Socket& sock);

int main(int argc, char** argv)
{
	if (argc > 3)
	{
		cout << "Usage: " << argv[0] << " [IP] [port]" << endl;
		return 1;
	}

	IPv4 ip = "127.0.0.1";
	if (argc >= 2)
	{
		try { ip = argv[1]; }
		catch (const std::exception&)
		{
			cout << "Bad IP: " << argv[1] << endl;
			return 1;
		}
	}

	Port port = DEFAULT_LISTEN_PORT;
	if (argc >= 3)
	{
		try { port = std::stoi(argv[2]); }
		catch (const std::exception&)
		{
			cout << "Bad port: " << argv[2] << endl;
			return 1;
		}
	}

	std::optional<TcpSocket<IPv4>> sock;
	try { sock.emplace(ip, port); }
	catch (const std::exception& e)
	{
		cout << "Failed to connect to server: " << e.what() << endl;
		return 1;
	}

	run_client(*sock);

	return 0;
}

void run_client(Socket& sock)
{
	bool connected = login_menu(sock);
	if (!connect)
		return;

	main_menu(sock);;
}
