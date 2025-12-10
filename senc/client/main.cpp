#include <iostream>
#include "../common/InlinePacketReceiver.hpp"
#include "../common/InlinePacketSender.hpp"
#include "../utils/Socket.hpp"
#include <functional>
#include <map>

namespace pkt = senc::pkt;
using senc::InlinePacketReceiver;
using senc::InlinePacketSender;
using senc::utils::Exception;
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

enum class MainMenuOption
{
	MakeUserSet,
	GetUserSets,
	GetMembers,
	Encrypt,
	Decrypt,
	Update,
	Participate,
	CompPart,
	SendPart,
	Exit
};

enum class ConnStatus { Error, Connected, Disconnected };

using SockFunc = std::function<ConnStatus(Socket&)>;

struct OptionRecord
{
	string description;
	SockFunc func;
};

void run_client(Socket& sock);
bool login_menu(Socket& sock);
void main_menu(Socket& sock);
template <typename Resp> Resp post(Socket& sock, const auto& request);
ConnStatus signup(Socket& sock);
ConnStatus login(Socket& sock);
ConnStatus logout(Socket& sock);
ConnStatus make_userset(Socket& sock);
ConnStatus get_usersets(Socket& sock);
ConnStatus get_members(Socket& sock);
ConnStatus encrypt(Socket& sock);
ConnStatus decrypt(Socket& sock);
ConnStatus update(Socket& sock);
ConnStatus participate(Socket& sock);
ConnStatus comp_part(Socket& sock);
ConnStatus send_part(Socket& sock);

// maps login menu option to description and function
const std::map<LoginMenuOption, OptionRecord> LOGIN_OPTS {
	{ LoginMenuOption::Signup, { "Signup", signup } },
	{ LoginMenuOption::Login, { "Login", login } },
	{ LoginMenuOption::Exit, { "Exit", logout } }
};

// maps main menu option to description and function
const std::map<MainMenuOption, OptionRecord> MAIN_OPTS{
	{ MainMenuOption::MakeUserSet, { "Create a new userset", make_userset } },
	{ MainMenuOption::GetUserSets, { "Show my usersets", get_usersets } },
	{ MainMenuOption::GetMembers, { "Show userset's memebrs", get_members } },
	{ MainMenuOption::Encrypt, { "Encrypt a message", encrypt } },
	{ MainMenuOption::Decrypt, { "Decrypt a message", decrypt } },
	{ MainMenuOption::Update, { "Run an update cycle", update } },
	{ MainMenuOption::Participate, { "Participate in decryption", participate } },
	{ MainMenuOption::CompPart, { "Compute part for decryption", comp_part } },
	{ MainMenuOption::SendPart, { "Send part for decryption", send_part } },
	{ MainMenuOption::Exit, { "Exit", logout } }
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

/**
 * @brief Runs login menu untill client is connected (or disconnected).
 * @param sock Client socket.
 * @return `true` if client ended up connecting, `false` if ended up disconnecting.
 */
bool login_menu(Socket& sock)
{
	ConnStatus status{};
	string choiceStr;
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

/**
 * @brief Runs login menu untill client is disconnects.
 * @param sock Client socket.
 */
void main_menu(Socket& sock)
{
	ConnStatus status{};
	string choiceStr;
	SockFunc func{};

	do
	{
		cout << "Main Menu" << endl;
		cout << "==========" << endl;

		for (const auto& [opt, record] : MAIN_OPTS)
			cout << (int)opt << ".\t" << record.description << endl;
		cout << endl;

		cout << "Enter your choice: ";
		std::getline(cin, choiceStr);
		try { func = MAIN_OPTS.at((MainMenuOption)std::stoi(choiceStr)).func; }
		catch (const std::exception&) { cout << "Bad choice, try again." << endl; }

		try { status = func(sock); }
		catch (const std::exception& e) { cout << "Error: " << e.what() << endl; }

		cout << endl;
	} while (ConnStatus::Disconnected != status);
}

/**
 * @brief Sends request and returns retrieved response.
 * @tparam Resp Response type.
 * @param sock Socket to send request and receive response through.
 * @param request Request to send.
 * @return Received response, or `std::nullopt` if error occured.
 * @throw Exception If error occured.
 */
template <typename Resp>
inline Resp post(Socket& sock, const auto& request)
{
	static InlinePacketReceiver receiver;
	static InlinePacketSender sender;

	sender.send_request(sock, request);
	auto resp = receiver.recv_response<Resp, pkt::ErrorResponse>(sock);
	if (!resp.has_value())
		throw Exception("Unexpected response received");
	if (std::holds_alternative<pkt::ErrorResponse>(*resp))
		throw Exception(std::get<pkt::ErrorResponse>(*resp).msg);
	return std::get<Resp>(*resp);
}

ConnStatus signup(Socket& sock)
{
	string username;

	cout << "Enter username: ";
	std::getline(cin, username);

	auto resp = post<pkt::SignupResponse>(sock, pkt::SignupRequest{ username });
	if (resp.status == pkt::SignupResponse::Status::Success)
	{
		cout << "Signed up successfully as \"" << username << "\"." << endl;
		return ConnStatus::Connected;
	}
	
	if (resp.status == pkt::SignupResponse::Status::UsernameTaken)
		cout << "Signup failed: Username already taken." << endl;
	else
		cout << "Signup failed: Unknown error." << endl;

	return ConnStatus::Disconnected;
}

ConnStatus login(Socket& sock)
{
	string username;

	cout << "Enter username: ";
	std::getline(cin, username);

	auto resp = post<pkt::LoginResponse>(sock, pkt::LoginRequest{ username });
	if (resp.status == pkt::LoginResponse::Status::Success)
	{
		cout << "Logged in successfully as \"" << username << "\"." << endl;
		return ConnStatus::Connected;
	}

	if (resp.status == pkt::LoginResponse::Status::BadUsername)
		cout << "Signup failed: Bad username." << endl;
	else
		cout << "Signup failed: Unknown error." << endl;

	return ConnStatus::Disconnected;
}
