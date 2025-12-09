#include <iostream>
#include "../common/InlinePacketReceiver.hpp"
#include "../common/InlinePacketSender.hpp"
#include "../utils/Socket.hpp"
#include <functional>
#include <map>

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
using std::cin;

constexpr Port DEFAULT_LISTEN_PORT = 4435;

enum class LoginMenuOption
{
	Signup,
	Login,
	Exit
};

enum class ConnStatus { Error, Connected, Disconnected };

using SockFunc = std::function<ConnStatus(Socket&)>;

struct OptionRecord
{
	std::string description;
	SockFunc func;
};

void run_client(Socket& sock);
bool login_menu(Socket& sock);
void main_menu(Socket& sock);
ConnStatus signup(Socket& sock);
ConnStatus login(Socket& sock);
ConnStatus logout(Socket& sock);

// maps login menu option to description and function
const std::map<LoginMenuOption, OptionRecord> LOGIN_OPTS {
	{ LoginMenuOption::Signup, { "Signup", signup } },
	{ LoginMenuOption::Login, { "Login", login } },
	{ LoginMenuOption::Exit, { "Exit", logout } }
};

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

/**
 * @brief Runs client on given socket.
 * @param sock Client socket.
 */
void run_client(Socket& sock)
{
	bool connected = login_menu(sock);
	if (!connected)
		return;

	main_menu(sock);;
}

bool login_menu(Socket& sock)
{
	std::string choiceStr;
	ConnStatus status{};
	SockFunc func{};

	do
	{
		cout << "Login Menu" << endl;
		cout << "==========" << endl;

		for (const auto& [opt, record] : LOGIN_OPTS)
			cout << (int)opt << ".\t" << record.description << endl;
		cout << endl;

		cout << "Enter your choice: ";
		std::getline(cin, choiceStr);
		try { func = LOGIN_OPTS.at((LoginMenuOption)std::stoi(choiceStr)).func; }
		catch (const std::exception&) { cout << "Bad choice, try again." << endl; }

		try { status = func(sock); }
		catch (const std::exception& e) { cout << "Error: " << e.what() << endl; }

		cout << endl;
	} while (ConnStatus::Error == status);

	return ConnStatus::Connected == status;
}
