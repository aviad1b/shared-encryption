#include <functional>
#include <iostream>
#include <map>
#include "../client_api/client_api.h"

namespace senc::cli_client
{
	using std::string;
	using std::endl;
	using std::cout;
	using std::cin;

	constexpr uint16_t DEFAULT_LISTEN_PORT = 4435;

	enum class LoginMenuOption
	{
		Signup = 1,
		Login,
		Exit
	};

	enum class MainMenuOption
	{
		MakeUserSet = 1,
		GetUserSets,
		GetMembers,
		Encrypt,
		Decrypt,
		ShowUpdates,
		UserSearch,
		RequestEvolve,
		Exit
	};

	enum class ConnStatus { NoChange, Connected, Disconnected };

	using PacketHandlerFunc = std::function<ConnStatus(const SENC_Handle&)>;

	struct OptionRecord
	{
		string description;
		PacketHandlerFunc func;
	};

	string input();
	string input(const string& msg);
	template <std::integral T> T input_num();
	template <std::integral T> T input_num(const string& msg);
	int start_client(const std::string& ip, uint16_t port);
	void run_client(const SENC_Handle& hClient);
	bool login_menu(const SENC_Handle& hClient);
	void main_menu(const SENC_Handle& hClient);
	ConnStatus signup(const SENC_Handle& hClient);
	ConnStatus login(const SENC_Handle& hClient);
	ConnStatus logout(const SENC_Handle& hClient);
	ConnStatus make_userset(const SENC_Handle& hClient);
	ConnStatus get_usersets(const SENC_Handle& hClient);
	ConnStatus get_members(const SENC_Handle& hClient);
	ConnStatus encrypt(const SENC_Handle& hClient);
	ConnStatus decrypt(const SENC_Handle& hClient);
	ConnStatus show_updates(const SENC_Handle& hClient);
	ConnStatus user_search(const SENC_Handle& hClient);
	ConnStatus request_evolve(const SENC_Handle& hClient);

	// maps login menu option to description and function
	const std::map<LoginMenuOption, OptionRecord> LOGIN_OPTS{
		{ LoginMenuOption::Signup, { "Signup", signup } },
		{ LoginMenuOption::Login , { "Login" , login  } },
		{ LoginMenuOption::Exit  , { "Exit"  , logout } },
	};

	// maps main menu option to description and function
	const std::map<MainMenuOption, OptionRecord> MAIN_OPTS{
		{ MainMenuOption::MakeUserSet  , { "Create a new userset"           , make_userset   } },
		{ MainMenuOption::GetUserSets  , { "Show my usersets"               , get_usersets   } },
		{ MainMenuOption::GetMembers   , { "Show userset's members"         , get_members    } },
		{ MainMenuOption::Encrypt      , { "Encrypt a message"              , encrypt        } },
		{ MainMenuOption::Decrypt      , { "Decrypt a message"              , decrypt        } },
		{ MainMenuOption::ShowUpdates  , { "Show new updates"               , show_updates   } },
		{ MainMenuOption::UserSearch   , { "User search"                    , user_search    } },
		{ MainMenuOption::RequestEvolve, { "Request key evolution"          , request_evolve } },
		{ MainMenuOption::Exit         , { "Exit"                           , logout         } },
	};

	int main(int argc, char** argv)
	{
		if (argc > 3 || argc < 2)
		{
			cout << "Usage: " << argv[0] << " <IP> [port]" << endl;
			return 1;
		}

		std::string ip = argv[1];

		uint16_t port = DEFAULT_LISTEN_PORT;
		if (argc >= 3)
		{
			try { port = static_cast<uint16_t>(std::stoi(argv[2])); }
			catch (const std::exception&)
			{
				cout << "Bad port: " << argv[2] << endl;
				return 1;
			}
		}

		return start_client(ip, port);
	}

	/**
	 * @brief Gets user input.
	 * @return User input.
	 */
	string input()
	{
		string res;
		std::getline(cin, res);
		return res;
	}

	/**
	 * @brief Gets user input.
	 * @param msg Message to show before input.
	 * @return User input.
	 */
	string input(const string& msg)
	{
		cout << msg;
		return input();
	}

	/**
	 * @brief Starts client, connecting to server at given IP and port.
	 * @param ip Server IP
	 * @param port Server port.
	 * @return Program exit code.
	 */
	int start_client(const std::string& ip, uint16_t port)
	{
		SENC_Handle hClient = SENC_Connect(ip.c_str(), port, nullptr, 0);
		run_client(hClient);

		return 0;
	}

	/**
	 * @brief Runs client on given socket.
	 * @param sock Client socket.
	 */
	void run_client(const SENC_Handle& hClient)
	{
		if (SENC_HasError(hClient))
		{
			cout << "Failed to connect to server: " << SENC_GetError(hClient) << endl;
			return;
		}

		bool connected = login_menu(hClient);
		if (!connected)
			return;
		cout << endl;

		main_menu(hClient);
	}

	/**
	 * @brief Runs login menu untill client is connected (or disconnected).
	 * @param sock Client socket.
	 * @return `true` if client ended up connecting, `false` if ended up disconnecting.
	 */
	bool login_menu(const SENC_Handle& hClient)
	{
		ConnStatus status{};
		PacketHandlerFunc func{};

		do
		{
			cout << "Login Menu" << endl;
			cout << "==========" << endl;

			for (const auto& [opt, record] : LOGIN_OPTS)
				cout << (int)opt << ".\t" << record.description << endl;
			cout << endl;

			string choiceStr = input("Enter your choice: ");
			try { func = LOGIN_OPTS.at((LoginMenuOption)std::stoi(choiceStr)).func; }
			catch (const std::exception&)
			{
				cout << "Bad choice, try again." << endl << endl;
				continue;
			}
			cout << endl;

			try { status = func(hClient); }
			catch (const std::exception& e)
			{
				cout << "Error: " << e.what() << endl << endl;
			}

			cout << endl;
		} while (ConnStatus::NoChange == status);

		return ConnStatus::Connected == status;
	}

	/**
	 * @brief Runs login menu untill client is disconnects.
	 * @param sock Client socket.
	 */
	void main_menu(const SENC_Handle& hClient)
	{
		ConnStatus status{};
		PacketHandlerFunc func{};

		do
		{
			cout << "Main Menu" << endl;
			cout << "==========" << endl;

			for (const auto& [opt, record] : MAIN_OPTS)
				cout << (int)opt << ".\t" << record.description << endl;
			cout << endl;

			string choiceStr = input("Enter your choice: ");
			try { func = MAIN_OPTS.at((MainMenuOption)std::stoi(choiceStr)).func; }
			catch (const std::exception&)
			{
				cout << "Bad choice, try again." << endl << endl;
				continue;
			}
			cout << endl;

			try { status = func(hClient); }
			catch (const std::exception& e)
			{
				cout << "Error: " << e.what() << endl << endl;
			}

			cout << endl;
		} while (ConnStatus::Disconnected != status);
	}

	ConnStatus signup(const SENC_Handle& hClient)
	{
		(void)hClient;
		return ConnStatus::NoChange; // TODO: Implement
	}

	ConnStatus login(const SENC_Handle& hClient)
	{
		(void)hClient;
		return ConnStatus::NoChange; // TODO: Implement
	}

	ConnStatus logout(const SENC_Handle& hClient)
	{
		(void)hClient;
		return ConnStatus::NoChange; // TODO: Implement
	}

	ConnStatus make_userset(const SENC_Handle& hClient)
	{
		(void)hClient;
		return ConnStatus::NoChange; // TODO: Implement
	}

	ConnStatus get_usersets(const SENC_Handle& hClient)
	{
		(void)hClient;
		return ConnStatus::NoChange; // TODO: Implement
	}

	ConnStatus get_members(const SENC_Handle& hClient)
	{
		(void)hClient;
		return ConnStatus::NoChange; // TODO: Implement
	}

	ConnStatus encrypt(const SENC_Handle& hClient)
	{
		(void)hClient;
		return ConnStatus::NoChange; // TODO: Implement
	}

	ConnStatus decrypt(const SENC_Handle& hClient)
	{
		(void)hClient;
		return ConnStatus::NoChange; // TODO: Implement
	}

	ConnStatus show_updates(const SENC_Handle& hClient)
	{
		(void)hClient;
		return ConnStatus::NoChange; // TODO: Implement
	}

	ConnStatus user_search(const SENC_Handle& hClient)
	{
		(void)hClient;
		return ConnStatus::NoChange; // TODO: Implement
	}

	ConnStatus request_evolve(const SENC_Handle& hClient)
	{
		(void)hClient;
		return ConnStatus::NoChange; // TODO: Implement
	}
}

int main(int argc, char** argv)
{
	return senc::cli_client::main(argc, argv);
}
