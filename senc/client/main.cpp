#include <iostream>
#include "../common/InlinePacketReceiver.hpp"
#include "../common/InlinePacketSender.hpp"
#include "../utils/Socket.hpp"
#include <functional>
#include <map>

namespace senc
{
	using utils::bytes_from_base64;
	using utils::bytes_to_base64;
	using utils::UUIDException;
	using utils::Exception;
	using utils::TcpSocket;
	using utils::Socket;
	using utils::Buffer;
	using utils::IPv4;
	using utils::Port;
	using utils::UUID;
	using std::vector;
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

	template <typename Self>
	concept NumInputable = std::integral<Self> &&
		(std::numeric_limits<Self>::min() >= std::numeric_limits<int>::min()) &&
		(std::numeric_limits<Self>::max() <= std::numeric_limits<int>::max());

	string input();
	string input(const string& msg);
	vector<string> input_vec(const string& msg);
	template <NumInputable T> T input_num();
	template <NumInputable T> T input_num(const string& msg);
	UUID input_uuid();
	UUID input_uuid(const string& msg);
	PrivKeyShard input_priv_key_shard();
	PrivKeyShard input_priv_key_shard(const string& msg);
	Ciphertext input_ciphertext();
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

	string input()
	{
		string res;
		std::getline(cin, res);
		return res;
	}

	string input(const string& msg)
	{
		cout << msg;
		return input();
	}

	vector<string> input_vec(const string& msg)
	{
		vector<string> res;
		string curr;

		cout << msg;
		while (!(curr = input()).empty())
			res.push_back(curr);

		return res;
	}

	UUID input_uuid()
	{
		while (true)
		{
			try { return UUID(input()); }
			catch (const UUIDException&) { cout << "Bad input, try again: "; }
		}
	}

	UUID input_uuid(const string& msg)
	{
		cout << msg;
		return input_uuid();
	}

	PrivKeyShard input_priv_key_shard()
	{
		bool valid = false;
		PrivKeyShard res{};
		do
		{
			string str = input();
			auto commaIndex = str.find(',');
			if (str.empty() || str[0] != '(' || str[str.length() - 1] != ')' || commaIndex == string::npos)
			{
				cout << "Invalid input, try again: ";
				continue;
			}

			string idStr = str.substr(0, commaIndex);
			string valStr = str.substr(commaIndex + 1);

			try
			{
				res.first = PrivKeyShardID(std::stoi(idStr));
				res.second = PrivKeyShardValue(std::stoi(valStr));
			}
			catch (const std::exception&)
			{
				cout << "Invalid input, try again: ";
				continue;
			}

			valid = true; // if reached here, input is valid
		} while (!valid);

		return res;
	}

	PrivKeyShard input_priv_key_shard(const string& msg)
	{
		cout << msg;
		return input_priv_key_shard();
	}

	Ciphertext input_ciphertext()
	{
		auto c1 = utils::ECGroup::from_bytes(bytes_from_base64(input("Enter ciphertext c1 (base64):\n")));
		cout << endl;

		auto c2 = utils::ECGroup::from_bytes(bytes_from_base64(input("Enter ciphertext c2 (base64):\n")));
		cout << endl;

		auto c3aBuffer = bytes_from_base64(input("Enter ciphertext c3a (base64):\n"));
		cout << endl;

		auto c3b = bytes_from_base64(input("Enter ciphertext c3b (base64):\n"));
		cout << endl;

		CryptoPP::SecByteBlock c3a(c3aBuffer.data(), c3aBuffer.size());
		utils::enc::AES1L::Ciphertext c3{ c3a, c3b };

		return { std::move(c1), std::move(c2), std::move(c3) };
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
		SockFunc func{};

		do
		{
			cout << "Login Menu" << endl;
			cout << "==========" << endl;

			for (const auto& [opt, record] : LOGIN_OPTS)
				cout << (int)opt << ".\t" << record.description << endl;
			cout << endl;

			string choiceStr = input("Enter your choice: ");
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
		SockFunc func{};

		do
		{
			cout << "Main Menu" << endl;
			cout << "==========" << endl;

			for (const auto& [opt, record] : MAIN_OPTS)
				cout << (int)opt << ".\t" << record.description << endl;
			cout << endl;

			string choiceStr = input("Enter your choice: ");
			try { func = MAIN_OPTS.at((MainMenuOption)std::stoi(choiceStr)).func; }
			catch (const std::exception&) { cout << "Bad choice, try again." << endl; }

			try { status = func(sock); }
			catch (const std::exception& e) { cout << "Error: " << e.what() << endl; }

			cout << endl;
		} while (ConnStatus::Disconnected != status);
	}

	template <NumInputable T>
	inline T input_num()
	{
		constexpr T MIN = std::numeric_limits<T>::min();
		constexpr T MAX = std::numeric_limits<T>::max();
		bool invalid = false;
		int num = 0;

		do
		{
			invalid = false;
			string str = input();
			try { num = std::stoi(str); }
			catch (const std::exception&)
			{
				cout << "Bad input (should be number in range " << MIN << ".." << MAX << ")." << endl;
				cout << "Try again: ";
				invalid = true;
			}
		} while (invalid);

		return static_cast<T>(num);
	}

	template <NumInputable T>
	inline T input_num(const string& msg)
	{
		cout << msg;
		return input_num<T>();
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
		string username = input("Enter username: ");

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
		string username = input("Enter username: ");

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

	ConnStatus logout(Socket& sock)
	{
		string confirm = input("Are you sure you want to leave? (y/n): ");
		if (confirm != "y" && confirm != "Y")
			return ConnStatus::Connected;

		post<pkt::LogoutResponse>(sock, pkt::LoginRequest{});

		cout << "Goodbye!" << endl;
		return ConnStatus::Disconnected;
	}

	ConnStatus make_userset(Socket& sock)
	{
		vector<string> owners = input_vec(
			"Enter owners (usernames, each in new line, ending with empty line):\n"
		);
		vector<string> regMembers = input_vec(
			"Enter non-owner members (usernames, each in new line, ending with empty line):\n"
		);
		auto ownersThreshold = input_num<member_count_t>("Enter owners threshold for decryption: ");
		auto regMembersThreshold = input_num<member_count_t>("Enter non-owner members threshold for decryption: ");

		auto resp = post<pkt::MakeUserSetResponse>(sock, pkt::MakeUserSetRequest{
			.reg_members = std::move(regMembers),
			.owners = std::move(owners),
			.reg_members_threshold = regMembersThreshold,
			.owners_threshold = ownersThreshold
		});

		cout << "Userset created successfully:" << endl << endl;

		cout << "\tID: " << resp.user_set_id << endl << endl;

		cout << "First public key:" << endl << bytes_to_base64(resp.pub_key1.to_bytes()) << endl << endl;

		cout << "Second public key:" << endl << bytes_to_base64(resp.pub_key2.to_bytes()) << endl << endl;

		cout << "First private key shard: (" << resp.priv_key1_shard.first
			 << "," << resp.priv_key1_shard.second << ")" << endl << endl;

		cout << "Second private key shard: (" << resp.priv_key2_shard.first
			<< "," << resp.priv_key2_shard.second << ")" << endl << endl;

		return ConnStatus::Connected;
	}

	ConnStatus get_usersets(Socket& sock)
	{
		auto resp = post<pkt::GetUserSetsResponse>(sock, pkt::GetUserSetsRequest{});

		if (resp.user_sets_ids.empty())
			cout << "You do not own any usersets." << endl;
		else
		{
			cout << "IDs of owned usersets:" << endl;
			for (const auto& id : resp.user_sets_ids)
				cout << id << endl;
		}

		return ConnStatus::Connected;
	}

	ConnStatus get_members(Socket& sock)
	{
		auto id = input_uuid("Enter userset ID: ");

		auto resp = post<pkt::GetMembersResponse>(sock, pkt::GetMembersRequest{ id });

		cout << "Owners:" << endl;
		for (const auto& owner : resp.owners)
			cout << owner << endl;
		cout << endl;

		cout << "Non-owners:" << endl;
		for (const auto& regMember : resp.reg_members)
			cout << regMember << endl;
		cout << endl;

		return ConnStatus::Connected;
	}

	ConnStatus encrypt(Socket& sock)
	{
		enum class PlaintextOption { Text = 1, Binary };
		static Schema schema;

		// let user choose text message or binary message
		cout << (int)PlaintextOption::Text << ". Encrypt text message" << endl;
		cout << (int)PlaintextOption::Binary << ". Encrypt binary message" << endl;
		cout << endl;
		PlaintextOption choice = (PlaintextOption)input_num<int>("Enter your choice: ");
		while (PlaintextOption::Text != choice && PlaintextOption::Binary != choice)
			choice = (PlaintextOption)input_num<int>("Invalid input, try again: ");

		Buffer plaintext;
		if (PlaintextOption::Text == choice)
		{
			string msg = input("Enter message to encrypt (text):\n");
			plaintext = Buffer(msg.begin(), msg.end());
		}
		else plaintext = bytes_from_base64(input("Enter message to encrypt (base64):\n"));
		cout << endl;

		auto pubKey1 = PubKey::from_bytes(bytes_from_base64(input("Enter first public key (base64):\n")));
		cout << endl;
		auto pubKey2 = PubKey::from_bytes(bytes_from_base64(input("Enter second public key (base64):\n")));
		cout << endl;

		auto [c1, c2, c3] = schema.encrypt(plaintext, pubKey1, pubKey2);
		const auto& [c3a, c3b] = c3;
		Buffer c3aBuffer(c3a.begin(), c3a.end()); // TODO: Remove once utils support all byte ranges

		cout << "Encrypted message:" << endl;
		cout << "c1:\t" << bytes_to_base64(c1.to_bytes()) << endl;
		cout << "c1:\t" << bytes_to_base64(c2.to_bytes()) << endl;
		cout << "c3a:\t" << bytes_to_base64(c3aBuffer) << endl;
		cout << "c3b:\t" << bytes_to_base64(c3b) << endl;
		cout << endl;

		return ConnStatus::Connected;
	}

	ConnStatus decrypt(Socket& sock)
	{
		auto usersetID = input_uuid("Enter ID of userset to decrypt under: ");
		cout << endl;

		Ciphertext ciphertext = input_ciphertext();

		auto resp = post<pkt::DecryptResponse>(sock, pkt::DecryptRequest{
			usersetID, std::move(ciphertext)
		});

		cout << "Decryption request submitted successfully." << endl;
		cout << "Operation ID: " << resp.op_id << endl;

		return ConnStatus::Connected;
	}

	ConnStatus participate(Socket& sock)
	{
		auto opid = input_uuid("Enter operation ID: ");

		auto resp = post<pkt::DecryptParticipateResponse>(sock, pkt::DecryptParticipateRequest{ opid });

		if (resp.status == pkt::DecryptParticipateResponse::Status::SendPart)
			cout << "Participance registered, be ready to send part in a future update." << endl;
		else
			cout << "Your participance is not needed for this operation." << endl;

		return ConnStatus::Connected;
	}
}

int main(int argc, char** argv)
{
	return senc::main(argc, argv);
}
