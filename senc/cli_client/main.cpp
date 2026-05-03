#include <functional>
#include <iostream>
#include <mutex>
#include <map>
#include "../client_api/client_api.h"
#include "../utils/ranges.hpp"
#include "../utils/bytes.hpp"

#include "../utils/env.hpp"
#ifdef SENC_WINDOWS
#include "../utils/winapi_patch.hpp"
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace senc::cli_client
{
	using utils::bytes_from_base64;
	using utils::bytes_to_base64;
	using utils::BytesView;
	using utils::Buffer;
	using std::vector;
	using std::string;
	using std::endl;
	using std::cout;
	using std::cin;

	constexpr uint16_t DEFAULT_LISTEN_PORT = 4435;

	constexpr const char* PROFILE_BASE_DIR = ".";

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

	struct FinishedDec
	{
		string opid;
		Buffer msg;
	};

	vector<FinishedDec> finishedDecs;
	std::mutex mtxFinishedDecs;

	string input();
	string input(const string& msg);
	template <std::integral T> T input_num();
	template <std::integral T> T input_num(const string& msg);
	string input_password();
	string input_password(const string& msg);
	int start_client(const std::string& ip, uint16_t port);
	void run_client(const SENC_Handle& hClient);
	bool login_menu(const SENC_Handle& hClient);
	void main_menu(const SENC_Handle& hClient);
	void finished_dec_callback(const char* opid, const uint8_t* msg, uint64_t msgLen, uintptr_t ctx);
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
	 * @brief Gets numeric user input.
	 * @tparam T Input type.
	 * @return User input.
	 */
	template<std::integral T>
	T input_num()
	{
		constexpr auto MIN = std::numeric_limits<T>::min();
		constexpr auto MAX = std::numeric_limits<T>::max();
		bool invalid = false;
		long long num = 0;

		do
		{
			invalid = false;
			string str = input();

			try { num = std::stoll(str); }
			catch (const std::exception&)
			{
				std::cout << "Bad input (should be number in range " << MIN << ".." << MAX << ")." << std::endl;
				std::cout << "Try again: ";
				invalid = true;
			}
		} while (invalid);

		return static_cast<T>(num);
	}

	/**
	 * @brief Gets numeric user input.
	 * @tparam T Input type.
	 * @param msg Message to show before input.
	 * @return User input.
	 */
	template<std::integral T>
	T input_num(const string& msg)
	{
		cout << msg;
		return input_num<T>();
	}

	#ifdef SENC_WINDOWS
	/**
	 * @brief Gets user password input.
	 * @return User password input.
	 */
	string input_password()
	{
		// set console input mode to no echo
		HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
		DWORD oldMode = 0; // to restore console mode after
		GetConsoleMode(hStdin, &oldMode);
		SetConsoleMode(hStdin, oldMode & (~ENABLE_ECHO_INPUT)); // disable echo input

		std::string res = input();

		// reset console mode
		SetConsoleMode(hStdin, oldMode);

		// manual newline since it was disabled with echoing
		std::cout << std::endl;

		return res;
	}
#else
	/**
	 * @brief Gets user password input.
	 * @return User password input.
	 */
	std::string input_password()
	{
		// set console input attributes to no echo
		termios oldAttrs{};
		tcgetattr(STDIN_FILENO, &oldAttrs); // to restore attributes later
		termios newAttrs = oldAttrs;
		newAttrs.c_lflag &= ~ECHO; // disable echo input
		tcsetattr(STDIN_FILENO, TCSANOW, &newAttrs);

		std::string res = input();

		// reset console mode
		tcsetattr(STDIN_FILENO, TCSANOW, &oldAttrs);

		// manual newline since it was disabled with echoing
		std::cout << std::endl;

		return res;
	}
#endif

	/**
	 * @brief Gets user password input.
	 * @param msg Message to show before input.
	 * @return User password input.
	 */
	std::string input_password(const std::string& msg)
	{
		std::cout << msg;
		return input_password();
	}

	/**
	 * @brief Starts client, connecting to server at given IP and port.
	 * @param ip Server IP
	 * @param port Server port.
	 * @return Program exit code.
	 */
	int start_client(const std::string& ip, uint16_t port)
	{
		SENC_Handle hClient = SENC_Connect(
			ip.c_str(), port,
			finished_dec_callback, 0
		);
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

	/**
	 * @brief Callback function for finished decryptions.
	 * @details Adds finished decryption to vector.
	 * @param opid Operation ID.
	 * @param msg Decrypted message.
	 * @param msgLen Length of decrypted message.
	 * @param cts Context arg (unused).
	 */
	void finished_dec_callback(const char* opid, const uint8_t* msg, uint64_t msgLen, uintptr_t ctx)
	{
		(void)ctx;
		const std::lock_guard lock(mtxFinishedDecs);
		finishedDecs.emplace_back(opid, Buffer(msg, msg + msgLen));
	}

	ConnStatus signup(const SENC_Handle& hClient)
	{
		string username = input("Enter username: ");
		cout << endl;

		string password = input_password("Enter password: ");
		cout << endl;

		string confirmPassword = input_password("Confirm password: ");
		cout << endl;

		if (password != confirmPassword)
		{
			cout << "Password confirm does not match password input." << endl;
			return ConnStatus::NoChange;
		}

		SENC_Handle hRes = SENC_SignUp(
			hClient, PROFILE_BASE_DIR,
			username.c_str(), password.c_str()
		);
		if (SENC_HasError(hRes))
			throw std::runtime_error(SENC_GetError(hRes));

		cout << "Signed up successfully as \"" << username << "\"." << endl;
		return ConnStatus::Connected;
	}

	ConnStatus login(const SENC_Handle& hClient)
	{
		string username = input("Enter username: ");
		cout << endl;

		string password = input_password("Enter password: ");
		cout << endl;

		SENC_Handle hRes = SENC_LogIn(
			hClient, PROFILE_BASE_DIR,
			username.c_str(), password.c_str()
		);
		if (SENC_HasError(hRes))
			throw std::runtime_error(SENC_GetError(hRes));

		cout << "Logged in successfully as \"" << username << "\"." << endl;
		return ConnStatus::Connected;
	}

	ConnStatus logout(const SENC_Handle& hClient)
	{
		SENC_Handle hRes = SENC_LogOut(hClient);
		if (SENC_HasError(hRes))
			throw std::runtime_error(SENC_GetError(hRes));

		cout << "Goodbye!" << endl;
		return ConnStatus::Disconnected;
	}

	ConnStatus make_userset(const SENC_Handle& hClient)
	{
		vector<string> owners, regMembers;
		string curInput;

		string name = input("Enter userset name: ");
		cout << endl;

		cout << "Enter owner usernames (end with newline): ";
		while (!(curInput = input()).empty())
			owners.emplace_back(std::move(curInput));
		cout << endl;

		cout << "Enter non-owner usernames (end with newline): ";
		while (!(curInput = input()).empty())
			regMembers.emplace_back(std::move(curInput));
		cout << endl;

		auto ownersThreshold = input_num<uint16_t>("Enter owners threshold: ");
		cout << endl;

		auto regMembersThreshold = input_num<uint16_t>("Enter non-owners threshold: ");
		cout << endl;

		auto ownersPtrs = utils::to_vector<const char*>(
			owners | std::views::transform(&std::string::c_str)
		);
		auto regMembersPtrs = utils::to_vector<const char*>(
			regMembers | std::views::transform(&std::string::c_str)
		);
		SENC_Handle hUSID = SENC_MakeUserSet(
			hClient,
			static_cast<uint64_t>(owners.size()),
			static_cast<uint64_t>(regMembers.size()),
			ownersPtrs.data(),
			regMembersPtrs.data(),
			ownersThreshold,
			regMembersThreshold,
			name.c_str()
		);

		if (SENC_HasError(hUSID))
			throw std::runtime_error(SENC_GetError(hUSID));

		cout << "Userset created successfully." << endl;
		cout << "ID: " << SENC_GetStringValue(hUSID) << endl << endl;

		return ConnStatus::Connected;
	}

	static void get_usersets_callback(const char* id, const char* name, uintptr_t ctx)
	{
		auto& i = *reinterpret_cast<std::size_t*>(ctx);
		cout << (i + 1) << ".\t" << id << "\t(" << name << ")" << endl;
		i++;
	}

	ConnStatus get_usersets(const SENC_Handle& hClient)
	{
		std::size_t i = 0;
		SENC_Handle hRes = SENC_GetUserSets(
			hClient,
			get_usersets_callback,
			reinterpret_cast<uintptr_t>(&i)
		);
		if (SENC_HasError(hRes))
			throw std::runtime_error(SENC_GetError(hRes));

		if (!i)
			cout << "You do not own any usersets." << endl;

		return ConnStatus::Connected;
	}

	static void get_members_owners_callback(const char* username, uintptr_t ctx)
	{
		auto& i = reinterpret_cast<std::pair<std::size_t, std::size_t>*>(ctx)->first;
		if (!i)
			cout << endl << "Owners:" << endl;
		cout << username << endl;
		i++;
	}

	static void get_members_reg_callback(const char* username, uintptr_t ctx)
	{
		auto& j = reinterpret_cast<std::pair<std::size_t, std::size_t>*>(ctx)->second;
		if (!j)
			cout << endl << "Non-owners:" << endl;
		cout << username << endl;
		j++;
	}

	ConnStatus get_members(const SENC_Handle& hClient)
	{
		string usid = input("Enter userset ID: ");

		std::pair<std::size_t, std::size_t > ij{ 0, 0 };
		SENC_Handle hRes = SENC_GetUserSetMembers(
			hClient,
			usid.c_str(),
			get_members_owners_callback,
			get_members_reg_callback,
			reinterpret_cast<uintptr_t>(&ij)
		);
		if (SENC_HasError(hRes))
			throw std::runtime_error(SENC_GetError(hRes));

		return ConnStatus::Connected;
	}

	ConnStatus encrypt(const SENC_Handle& hClient)
	{
		(void)hClient;

		string usersetID = input("Enter ID of userset to encrypt under: ");
		cout << endl;
		
		enum class PlaintextOption { Text = 1, Binary };

		// let user choose text message or binary message
		cout << (int)PlaintextOption::Text << ". Encrypt text message" << endl;
		cout << (int)PlaintextOption::Binary << ". Encrypt binary message" << endl;
		cout << endl;
		PlaintextOption choice = (PlaintextOption)input_num<int>("Enter your choice: ");
		while (PlaintextOption::Text != choice && PlaintextOption::Binary != choice)
			choice = (PlaintextOption)input_num<int>("Invalid input, try again: ");
		cout << endl;

		Buffer plaintext;
		if (PlaintextOption::Text == choice)
		{
			string msg = input("Enter message to encrypt (text): ");
			plaintext = Buffer(msg.begin(), msg.end());
		}
		else plaintext = bytes_from_base64(input("Enter message to encrypt (base64): "));
		cout << endl;

		SENC_Handle hCiphertext = SENC_Encrypt(
			hClient,
			usersetID.c_str(),
			plaintext.data(),
			static_cast<uint64_t>(plaintext.size())
		);
		if (SENC_HasError(hCiphertext))
			throw std::runtime_error(SENC_GetError(hCiphertext));

		SENC_Handle hC1 = SENC_GetCiphertextC1(hCiphertext);
		SENC_Handle hC2 = SENC_GetCiphertextC2(hCiphertext);
		SENC_Handle hC3a = SENC_GetCiphertextC3a(hCiphertext);
		SENC_Handle hC3b = SENC_GetCiphertextC3b(hCiphertext);
		cout << "Encrypted message (ciphertext): "
			 << bytes_to_base64(BytesView(SENC_GetBytesValue(hC1), SENC_GetBytesLen(hC1))) << endl
			 << bytes_to_base64(BytesView(SENC_GetBytesValue(hC2), SENC_GetBytesLen(hC2))) << endl
			 << bytes_to_base64(BytesView(SENC_GetBytesValue(hC3a), SENC_GetBytesLen(hC3a))) << endl
			 << bytes_to_base64(BytesView(SENC_GetBytesValue(hC3b), SENC_GetBytesLen(hC3b))) << endl
			 << endl;

		return ConnStatus::Connected;
	}

	ConnStatus decrypt(const SENC_Handle& hClient)
	{
		string usersetID = input("Enter ID of userset to decrypt under: ");
		cout << endl;

		cout << "Enter ciphertext: ";
		auto c1 = bytes_from_base64(input());
		auto c2 = bytes_from_base64(input());
		auto c3a = bytes_from_base64(input());
		auto c3b = bytes_from_base64(input());
		cout << endl;

		SENC_Handle hCiphertext = SENC_NewCiphertext(
			c1.data(), static_cast<uint64_t>(c1.size()),
			c2.data(), static_cast<uint64_t>(c2.size()),
			c3a.data(), static_cast<uint64_t>(c3a.size()),
			c3b.data(), static_cast<uint64_t>(c3b.size())
		);

		vector<string> usernames;
		string curInput;
		cout << "Enter usernames to send decryption to (including yourself, if so you wish): " << endl;
		while (!(curInput = input()).empty())
			usernames.emplace_back(std::move(curInput));
		cout << endl << endl;

		auto usernamesPtrs = utils::to_vector<const char*>(
			usernames | std::views::transform(&std::string::c_str)
		);
		SENC_Handle hOPID = SENC_DecryptSend(
			hClient,
			usersetID.c_str(),
			hCiphertext,
			static_cast<uint64_t>(usernames.size()),
			usernamesPtrs.data()
		);
		if (SENC_HasError(hOPID))
			throw std::runtime_error(SENC_GetError(hOPID));

		cout << "Decryption request submitted successfully." << endl;
		cout << "Operation ID: " << SENC_GetStringValue(hOPID) << endl << endl;

		return ConnStatus::Connected;
	}

	ConnStatus show_updates(const SENC_Handle& hClient)
	{
		(void)hClient;

		const std::lock_guard lock(mtxFinishedDecs);
		
		if (finishedDecs.empty())
		{
			cout << "No updates to show." << endl << endl;
			return ConnStatus::Connected;
		}

		for (const auto& finishedDec : finishedDecs)
		{
			cout << "Finished decryption " << finishedDec.opid << ":";
			string ynIn = input("Show as text? (y/n): ");
			if ("y" == ynIn || "Y" == ynIn)
				cout << std::string(finishedDec.msg.begin(), finishedDec.msg.end()) << endl;
			else
				cout << bytes_to_base64(finishedDec.msg) << endl;
			cout << endl;
		}
		cout << endl;

		finishedDecs.clear();

		return ConnStatus::Connected;
	}

	static void user_search_callback(const char* username, uintptr_t ctx)
	{
		auto& i = *reinterpret_cast<std::size_t*>(ctx);
		if (!i)
			cout << "Search results:" << endl;
		cout << username << endl;
		i++;
	}

	ConnStatus user_search(const SENC_Handle& hClient)
	{
		std::string query = input("Enter part of username: ");
		cout << endl;

		std::size_t i = 0;
		SENC_Handle hRes = SENC_UserSearch(
			hClient, query.c_str(),
			user_search_callback,
			reinterpret_cast<uintptr_t>(&i)
		);
		cout << endl;

		if (SENC_HasError(hRes))
			throw std::runtime_error(SENC_GetError(hRes));

		return ConnStatus::Connected;
	}

	ConnStatus request_evolve(const SENC_Handle& hClient)
	{
		string usid = input("Enter ID of userset to evolve: ");
		cout << endl;

		SENC_Handle hRes = SENC_EvolveUserSet(hClient, usid.c_str());
		if (SENC_HasError(hRes))
			throw std::runtime_error(SENC_GetError(hRes));

		cout << "Key evolution successfully requested for userset " << usid << endl << endl;
		return ConnStatus::Connected;
	}
}

int main(int argc, char** argv)
{
	return senc::cli_client::main(argc, argv);
}
