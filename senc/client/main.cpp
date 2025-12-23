#include <functional>
#include <iostream>
#include <map>
#include "../common/InlinePacketReceiver.hpp"
#include "../common/InlinePacketSender.hpp"
#include "../utils/Socket.hpp"
#include "input.hpp"

namespace senc
{
	using AddedAsMemberRecord = pkt::UpdateResponse::AddedAsMemberRecord;
	using AddedAsOwnerRecord = pkt::UpdateResponse::AddedAsOwnerRecord;
	using ToDecryptRecord = pkt::UpdateResponse::ToDecryptRecord;
	using FinishedDecryptionsRecord = pkt::UpdateResponse::FinishedDecryptionsRecord;
	using utils::bytes_from_base64;
	using utils::bytes_to_base64;
	using utils::Exception;
	using utils::TcpSocket;
	using utils::Socket;
	using utils::Buffer;
	using utils::IPv4;
	using utils::Port;
	using std::vector;
	using std::string;
	using std::endl;
	using std::cout;

	constexpr Port DEFAULT_LISTEN_PORT = 4435;

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
		Update,
		Participate,
		CompPart,
		SendPart,
		JoinParts,
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
	ConnStatus join_parts(Socket& sock);
	void print_userset_data(size_t idx,
							const utils::OneOf<AddedAsOwnerRecord, AddedAsMemberRecord> auto& data);
	void print_to_decrypt_data(size_t idx, const ToDecryptRecord& data);
	void print_finished_data(size_t idx, const FinishedDecryptionsRecord& data);

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
		{ MainMenuOption::CompPart, { "Compute part for decryption", send_part } },
		{ MainMenuOption::SendPart, { "Send part for decryption", send_part } },
		{ MainMenuOption::JoinParts, { "Join decryption parts", join_parts } },
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
		// first, send protocol version to server
		sock.send_connected_primitive(pkt::PROTOCOL_VERSION);

		const bool isProtocolVersionSupported = sock.recv_connected_primitive<bool>();
		if (!isProtocolVersionSupported)
		{
			cout << "Protocol version not supported by server, exiting." << endl;
			return;
		}

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
		string username = input_username("Enter username: ");

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

		return ConnStatus::Error;
	}

	ConnStatus login(Socket& sock)
	{
		string username = input_username("Enter username: ");

		auto resp = post<pkt::LoginResponse>(sock, pkt::LoginRequest{ username });
		if (resp.status == pkt::LoginResponse::Status::Success)
		{
			cout << "Logged in successfully as \"" << username << "\"." << endl;
			return ConnStatus::Connected;
		}

		if (resp.status == pkt::LoginResponse::Status::BadUsername)
			cout << "Login failed: Bad username." << endl;
		else
			cout << "Login failed: Unknown error." << endl;

		return ConnStatus::Error;
	}

	ConnStatus logout(Socket& sock)
	{
		if (!input_yesno("Are you sure you want to leave? (y/n): "))
			return ConnStatus::Connected;

		post<pkt::LogoutResponse>(sock, pkt::LogoutRequest{});

		cout << "Goodbye!" << endl;
		return ConnStatus::Disconnected;
	}

	ConnStatus make_userset(Socket& sock)
	{
		vector<string> owners = input_usernames(
			"Enter owners (usernames, each in new line, ending with empty line):\n"
		);
		vector<string> regMembers = input_usernames(
			"Enter non-owner members (usernames, each in new line, ending with empty line):\n"
		);
		auto ownersThreshold = input_threshold("Enter owners threshold for decryption: ");
		auto regMembersThreshold = input_threshold("Enter non-owner members threshold for decryption: ");

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
		auto id = input_userset_id("Enter userset ID: ");

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
		(void)sock;

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
		auto pubKey1 = PubKey::from_bytes(bytes_from_base64(input("Enter first public key (base64):\n")));
		auto pubKey2 = PubKey::from_bytes(bytes_from_base64(input("Enter second public key (base64):\n")));

		auto [c1, c2, c3] = schema.encrypt(plaintext, pubKey1, pubKey2);
		const auto& [c3a, c3b] = c3;

		cout << "Encrypted message:" << endl;
		cout << "c1:\t" << bytes_to_base64(c1.to_bytes()) << endl;
		cout << "c1:\t" << bytes_to_base64(c2.to_bytes()) << endl;
		cout << "c3a:\t" << bytes_to_base64(c3a) << endl;
		cout << "c3b:\t" << bytes_to_base64(c3b) << endl;
		cout << endl;

		return ConnStatus::Connected;
	}

	ConnStatus decrypt(Socket& sock)
	{
		auto usersetID = input_userset_id("Enter ID of userset to decrypt under: ");
		Ciphertext ciphertext = input_ciphertext();

		auto resp = post<pkt::DecryptResponse>(sock, pkt::DecryptRequest{
			usersetID, std::move(ciphertext)
		});

		cout << "Decryption request submitted successfully." << endl;
		cout << "Operation ID: " << resp.op_id << endl;

		return ConnStatus::Connected;
	}

	ConnStatus update(Socket& sock)
	{
		auto resp = post<pkt::UpdateResponse>(sock, pkt::UpdateRequest{});

		if (!resp.added_as_owner.empty())
		{
			cout << "Added to " << resp.added_as_owner.size() << " new usersets as owner:" << endl;
			for (const auto& [i, data] : resp.added_as_owner | utils::views::enumerate)
				print_userset_data(i, data);
			cout << endl;
		}

		if (!resp.added_as_reg_member.empty())
		{
			cout << "Added to " << resp.added_as_reg_member.size() << " new usersets as non-owner:" << endl;
			for (const auto& [i, data] : resp.added_as_reg_member | utils::views::enumerate)
				print_userset_data(i, data);
			cout << endl;
		}

		if (!resp.on_lookup.empty())
		{
			cout << "IDs of operations looking for you:" << endl;
			for (const auto& opid : resp.on_lookup)
				cout << opid << endl;
			cout << endl;
		}

		if (!resp.to_decrypt.empty())
		{
			cout << "Pending decryption operations:" << endl;
			for (const auto& [i, data] : resp.to_decrypt | utils::views::enumerate)
				print_to_decrypt_data(i, data);
			cout << endl;
		}

		if (!resp.finished_decryptions.empty())
		{
			cout << "Finished decryption operations:" << endl;
			for (const auto& [i, data] : resp.finished_decryptions | utils::views::enumerate)
				print_finished_data(i, data);
			cout << endl;
		}

		return ConnStatus::Connected;
	}

	ConnStatus participate(Socket& sock)
	{
		auto opid = input_operation_id("Enter operation ID: ");

		auto resp = post<pkt::DecryptParticipateResponse>(sock, pkt::DecryptParticipateRequest{ opid });

		if (resp.status == pkt::DecryptParticipateResponse::Status::SendPart)
			cout << "Participance registered, be ready to send part in a future update." << endl;
		else
			cout << "Your participance is not needed for this operation." << endl;

		return ConnStatus::Connected;
	}

	ConnStatus comp_part(Socket& sock)
	{
		(void)sock;

		bool isOwner = input_yesno("Is this an owner layer part? (y/n): ");
		Ciphertext ciphertext = input_ciphertext();
		PrivKeyShard privKeyShard = input_priv_key_shard("Enter your private key shard: ");
		auto privKeyShardsIDs = input_priv_key_shard_ids("Enter envolved private key shard IDs (each in new line): ");
		cout << endl;
		DecryptionPart part{};
		if (isOwner)
			part = Shamir::decrypt_get_2l<2>(ciphertext, privKeyShard, privKeyShardsIDs);
		else
			part = Shamir::decrypt_get_2l<1>(ciphertext, privKeyShard, privKeyShardsIDs);

		cout << "Result decryption part: " << utils::bytes_to_base64(part.to_bytes()) << endl;

		return ConnStatus::Connected;
	}

	ConnStatus send_part(Socket& sock)
	{
		auto opid = input_operation_id("Enter operation ID: ");
		DecryptionPart part = input_decryption_part("Enter decryption part to send: ");

		post<pkt::SendDecryptionPartResponse>(sock, pkt::SendDecryptionPartRequest{
			opid,
			std::move(part)
		});

		cout << "Part submitted successfully." << endl;

		return ConnStatus::Connected;
	}

	ConnStatus join_parts(Socket& sock)
	{
		(void)sock;

		auto ciphertext = input_ciphertext();

		auto parts1 = input_decryption_parts("Enter layer1 decryption parts: ");

		auto parts2 = input_decryption_parts("Enter layer2 decryption parts: ");

		cout << endl;

		auto decrypted = Shamir::decrypt_join_2l(ciphertext, parts1, parts2);

		auto isText = input_yesno("Is this a textual message? (y/n): ");

		std::string msg;
		if (isText)
			msg = std::string(decrypted.begin(), decrypted.end());
		else
			msg = utils::bytes_to_base64(decrypted);

		cout << "Decrypted message:" << endl << msg << endl;

		return ConnStatus::Connected;
	}

	inline void print_userset_data(size_t idx,
								   const utils::OneOf<AddedAsOwnerRecord, AddedAsMemberRecord> auto& data)
	{
		using Data = std::remove_cvref_t<decltype(data)>;
		cout << "==============================" << endl;
		cout << "Set #" << (idx + 1) << ":" << endl << endl;
		cout << "ID: " << data.user_set_id << endl << endl;
		cout << "First public key:" << endl << bytes_to_base64(data.pub_key1.to_bytes()) << endl << endl;
		cout << "Second public key:" << endl << bytes_to_base64(data.pub_key2.to_bytes()) << endl << endl;
		cout << "First private key shard: (" << data.priv_key1_shard.first
			 << "," << data.priv_key1_shard.second << ")" << endl << endl;
		cout << "First private key shard: (" << data.priv_key1_shard.first
			 << "," << data.priv_key1_shard.second << ")" << endl << endl;
		if constexpr (std::same_as<Data, AddedAsOwnerRecord>)
			cout << "Second private key shard: (" << data.priv_key2_shard.first
				 << "," << data.priv_key2_shard.second << ")" << endl << endl;
		cout << "==============================" << endl << endl << endl;
	}

	void print_to_decrypt_data(size_t idx, const ToDecryptRecord& data)
	{
		const auto& [c1, c2, c3] = data.ciphertext;
		const auto& [c3a, c3b] = c3;
		Buffer c3aBuffer(c3a.begin(), c3a.end());

		cout << "==============================" << endl;

		cout << "To-Decrypt Operation #" << (idx + 1) << ":" << endl << endl;

		cout << "Operation ID: " << data.op_id << endl << endl;

		cout << "Ciphertext:" << endl
			 << "c1:\t" << bytes_to_base64(c1.to_bytes()) << endl
			 << "c2:\t" << bytes_to_base64(c2.to_bytes()) << endl
			 << "c3a:\t" << bytes_to_base64(c3aBuffer) << endl
			 << "c3b:\t" << bytes_to_base64(c3b) << endl
			 << endl;

		cout << "Involved Shards IDs: ";
		if (!data.shards_ids.empty())
		{
			auto it = data.shards_ids.cbegin();
			cout << *it;
			for (++it; it != data.shards_ids.cend(); ++it)
				cout << ", " << *it;
		}
		cout << endl << endl;

		cout << "==============================" << endl;
	}

	void print_finished_data(size_t idx, const FinishedDecryptionsRecord& data)
	{
		cout << "==============================" << endl;

		cout << "Finished Operation #" << (idx + 1) << ":" << endl << endl;

		cout << "Operation ID: " << data.op_id << endl << endl;

		cout << "First layer decryption parts:" << endl;
		for (const auto& part : data.parts1)
			cout << utils::bytes_to_base64(part.to_bytes()) << endl;
		cout << endl;

		cout << "First layer involved shard IDs: ";
		if (!data.shardsIDs1.empty())
		{
			auto it = data.shardsIDs1.cbegin();
			cout << *it;
			for (++it; it != data.shardsIDs1.cend(); ++it)
				cout << ", " << *it;
		}
		cout << endl << endl;

		cout << "Second layer decryption parts:" << endl;
		for (const auto& part : data.parts1)
			cout << utils::bytes_to_base64(part.to_bytes()) << endl;
		cout << endl;

		cout << "Second layer involved shard IDs: ";
		if (!data.shardsIDs2.empty())
		{
			auto it = data.shardsIDs2.cbegin();
			cout << *it;
			for (++it; it != data.shardsIDs2.cend(); ++it)
				cout << ", " << *it;
		}
		cout << endl << endl;

		cout << "==============================" << endl;
	}
}

int main(int argc, char** argv)
{
	return senc::main(argc, argv);
}
