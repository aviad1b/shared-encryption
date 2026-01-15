#include <functional>
#include <iostream>
#include <map>
#include "../common/EncryptedPacketHandler.hpp"
#include "../utils/Socket.hpp"
#include "io/output.hpp"
#include "io/input.hpp"

namespace senc::client
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
	using utils::IPType;
	using utils::IPv4;
	using utils::IPv6;
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

	enum class ConnStatus { NoChange, Connected, Disconnected };

	using PacketHandlerFunc = std::function<ConnStatus(PacketHandler&)>;

	struct OptionRecord
	{
		string description;
		PacketHandlerFunc func;
	};

	std::optional<std::variant<IPv4, IPv6>> parse_ip(const char* str);
	int start_client(const IPType auto& ip, Port port);
	void run_client(Socket& sock);
	bool login_menu(PacketHandler& packetHandler);
	void main_menu(PacketHandler& packetHandler);
	template <typename Resp> Resp post(PacketHandler& packetHandler, const auto& request);
	ConnStatus signup(PacketHandler& packetHandler);
	ConnStatus login(PacketHandler& packetHandler);
	ConnStatus logout(PacketHandler& packetHandler);
	ConnStatus make_userset(PacketHandler& packetHandler);
	ConnStatus get_usersets(PacketHandler& packetHandler);
	ConnStatus get_members(PacketHandler& packetHandler);
	ConnStatus encrypt(PacketHandler& packetHandler);
	ConnStatus decrypt(PacketHandler& packetHandler);
	ConnStatus update(PacketHandler& packetHandler);
	ConnStatus participate(PacketHandler& packetHandler);
	ConnStatus comp_part(PacketHandler& packetHandler);
	ConnStatus send_part(PacketHandler& packetHandler);
	ConnStatus join_parts(PacketHandler& packetHandler);
	void print_userset_data(size_t idx,
							const utils::OneOf<AddedAsOwnerRecord, AddedAsMemberRecord> auto& data);
	void print_to_decrypt_data(size_t idx, const ToDecryptRecord& data);
	void print_finished_data(size_t idx, const FinishedDecryptionsRecord& data);

	// maps login menu option to description and function
	const std::map<LoginMenuOption, OptionRecord> LOGIN_OPTS {
		{ LoginMenuOption::Signup, { "Signup", signup } },
		{ LoginMenuOption::Login , { "Login" , login  } },
		{ LoginMenuOption::Exit  , { "Exit"  , logout } },
	};

	// maps main menu option to description and function
	const std::map<MainMenuOption, OptionRecord> MAIN_OPTS{
		{ MainMenuOption::MakeUserSet, { "Create a new userset"       , make_userset } },
		{ MainMenuOption::GetUserSets, { "Show my usersets"           , get_usersets } },
		{ MainMenuOption::GetMembers , { "Show userset's members"     , get_members  } },
		{ MainMenuOption::Encrypt    , { "Encrypt a message"          , encrypt      } },
		{ MainMenuOption::Decrypt    , { "Decrypt a message"          , decrypt      } },
		{ MainMenuOption::Update     , { "Run an update cycle"        , update       } },
		{ MainMenuOption::Participate, { "Participate in decryption"  , participate  } },
		{ MainMenuOption::CompPart   , { "Compute part for decryption", comp_part    } },
		{ MainMenuOption::SendPart   , { "Send part for decryption"   , send_part    } },
		{ MainMenuOption::JoinParts  , { "Join decryption parts"      , join_parts   } },
		{ MainMenuOption::Exit       , { "Exit"                       , logout       } },
	};

	int main(int argc, char** argv)
	{
		if (argc > 3 || argc < 2)
		{
			cout << "Usage: " << argv[0] << " <IP> [port]" << endl;
			return 1;
		}

		auto ip = parse_ip(argv[1]);
		if (!ip.has_value())
		{
			cout << "Bad IP: " << argv[1] << endl;
			return 1;
		}

		Port port = DEFAULT_LISTEN_PORT;
		if (argc >= 3)
		{
			try { port = utils::parse_port(argv[2]); }
			catch (const std::exception&)
			{
				cout << "Bad port: " << argv[2] << endl;
				return 1;
			}
		}

		return std::visit(
			[port](const auto& x) { return start_client(x, port); },
			*ip
		);
	}

	/**
	 * @brief Parses IP instance from string representation.
	 * @param str String representation of IP address.
	 * @return IP instance, or `std::nullopt` if invalid.
	 */
	std::optional<std::variant<IPv4, IPv6>> parse_ip(const char* str)
	{
		try { return IPv4(str); }
		catch (const Exception&) { }

		try { return IPv6(str); }
		catch (const Exception&) { }

		return std::nullopt;
	}

	/**
	 * @brief Starts client, connecting to server at given IP and port.
	 * @param ip Server IP
	 * @param port Server port.
	 * @return Program exit code.
	 */
	int start_client(const IPType auto& ip, Port port)
	{
		using IP = std::remove_cvref_t<decltype(ip)>;

		std::optional<TcpSocket<IP>> sock;
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
		std::optional<EncryptedPacketHandler> packetHandler;
		try { packetHandler.emplace(EncryptedPacketHandler::client(sock)); }
		catch (const ConnEstablishException& e)
		{
			cout << "Failed to connect to server: " << e.what() << endl;
			return;
		}

		bool connected = login_menu(*packetHandler);
		if (!connected)
			return;
		cout << endl;

		main_menu(*packetHandler);
	}

	/**
	 * @brief Runs login menu untill client is connected (or disconnected).
	 * @param sock Client socket.
	 * @return `true` if client ended up connecting, `false` if ended up disconnecting.
	 */
	bool login_menu(PacketHandler& packetHandler)
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

			string choiceStr = io::input("Enter your choice: ");
			try { func = LOGIN_OPTS.at((LoginMenuOption)std::stoi(choiceStr)).func; }
			catch (const std::exception&)
			{
				cout << "Bad choice, try again." << endl << endl;
				continue;
			}
			cout << endl;

			try { status = func(packetHandler); }
			catch (const utils::SocketException& e)
			{
				cout << "Lost connection to the server: " << e.what() << endl;
				status = ConnStatus::Disconnected;
			}
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
	void main_menu(PacketHandler& packetHandler)
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

			string choiceStr = io::input("Enter your choice: ");
			try { func = MAIN_OPTS.at((MainMenuOption)std::stoi(choiceStr)).func; }
			catch (const std::exception&)
			{
				cout << "Bad choice, try again." << endl << endl;
				continue;
			}
			cout << endl;

			try { status = func(packetHandler); }
			catch (const utils::SocketException& e)
			{
				cout << "Lost connection to the server: " << e.what() << endl;
				status = ConnStatus::Disconnected;
			}
			catch (const std::exception& e)
			{
				cout << "Error: " << e.what() << endl << endl;
			}

			cout << endl;
		} while (ConnStatus::Disconnected != status);
	}

	/**
	 * @brief Sends request and returns retrieved response.
	 * @tparam Resp Response type.
	 * @param packetHandler Packet handler to use for sending and receiving.
	 * @param request Request to send.
	 * @return Received response, or `std::nullopt` if error occured.
	 * @throw Exception If error occured.
	 */
	template <typename Resp>
	inline Resp post(PacketHandler& packetHandler, const auto& request)
	{
		packetHandler.send_request(request);
		auto resp = packetHandler.recv_response<Resp, pkt::ErrorResponse>();
		if (!resp.has_value())
			throw Exception("Unexpected response received");
		if (std::holds_alternative<pkt::ErrorResponse>(*resp))
			throw Exception(std::get<pkt::ErrorResponse>(*resp).msg);
		return std::get<Resp>(*resp);
	}

	ConnStatus signup(PacketHandler& packetHandler)
	{
		string username = io::input_username("Enter username: ");
		cout << endl;

		string password = io::input_password("Enter password: ");
		cout << endl << endl;

		auto resp = post<pkt::SignupResponse>(packetHandler, pkt::SignupRequest{
			username, password
		});
		if (resp.status == pkt::SignupResponse::Status::Success)
		{
			cout << "Signed up successfully as \"" << username << "\"." << endl;
			return ConnStatus::Connected;
		}
	
		if (resp.status == pkt::SignupResponse::Status::UsernameTaken)
			cout << "Signup failed: Username already taken." << endl;
		else
			cout << "Signup failed: Unknown error." << endl;

		return ConnStatus::NoChange;
	}

	ConnStatus login(PacketHandler& packetHandler)
	{
		string username = io::input_username("Enter username: ");
		cout << endl;

		string password = io::input_password("Enter password: ");
		cout << endl << endl;

		auto resp = post<pkt::LoginResponse>(packetHandler, pkt::LoginRequest{
			username, password
		});
		if (resp.status == pkt::LoginResponse::Status::Success)
		{
			cout << "Logged in successfully as \"" << username << "\"." << endl;
			return ConnStatus::Connected;
		}

		if (resp.status == pkt::LoginResponse::Status::BadLogin)
			cout << "Login failed: Bad username or password." << endl;
		else
			cout << "Login failed: Unknown error." << endl;

		return ConnStatus::NoChange;
	}

	ConnStatus logout(PacketHandler& packetHandler)
	{
		if (!io::input_yesno("Are you sure you want to leave? (y/n): "))
			return ConnStatus::NoChange;
		cout << endl << endl;

		post<pkt::LogoutResponse>(packetHandler, pkt::LogoutRequest{});

		cout << "Goodbye!" << endl;
		return ConnStatus::Disconnected;
	}

	ConnStatus make_userset(PacketHandler& packetHandler)
	{
		vector<string> owners = io::input_usernames(
			"Enter owners (usernames, each in new line, ending with empty line): "
		);
		vector<string> regMembers = io::input_usernames(
			"Enter non-owner members (usernames, each in new line, ending with empty line): "
		);

		auto ownersThreshold = io::input_threshold("Enter owners threshold for decryption: ");
		cout << endl;

		auto regMembersThreshold = io::input_threshold("Enter non-owner members threshold for decryption: ");
		cout << endl;

		auto resp = post<pkt::MakeUserSetResponse>(packetHandler, pkt::MakeUserSetRequest{
			.reg_members = std::move(regMembers),
			.owners = std::move(owners),
			.reg_members_threshold = regMembersThreshold,
			.owners_threshold = ownersThreshold
		});

		cout << "Userset created successfully:" << endl << endl;

		cout << "ID: " << resp.user_set_id << endl << endl;

		io::print_pub_keys(resp.reg_layer_pub_key, resp.owner_layer_pub_key);
		cout << endl;

		io::print_reg_layer_priv_key_shard(resp.reg_layer_priv_key_shard);
		cout << endl;
		
		io::print_owner_layer_priv_key_shard(resp.owner_layer_priv_key_shard);
		cout << endl;

		return ConnStatus::Connected;
	}

	ConnStatus get_usersets(PacketHandler& packetHandler)
	{
		auto resp = post<pkt::GetUserSetsResponse>(packetHandler, pkt::GetUserSetsRequest{});

		if (resp.user_sets_ids.empty())
			cout << "You do not own any usersets." << endl;
		else
		{
			cout << "IDs of owned usersets:" << endl;
			for (const auto& [i, id] : resp.user_sets_ids | utils::views::enumerate)
				cout << (i + 1) << ".\t" << id << endl;
		}
		cout << endl;

		return ConnStatus::Connected;
	}

	ConnStatus get_members(PacketHandler& packetHandler)
	{
		auto id = io::input_userset_id("Enter userset ID: ");
		cout << endl;

		auto resp = post<pkt::GetMembersResponse>(packetHandler, pkt::GetMembersRequest{ id });

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

	ConnStatus encrypt(PacketHandler& packetHandler)
	{
		(void)packetHandler;

		enum class PlaintextOption { Text = 1, Binary };
		static Schema schema;

		// let user choose text message or binary message
		cout << (int)PlaintextOption::Text << ". Encrypt text message" << endl;
		cout << (int)PlaintextOption::Binary << ". Encrypt binary message" << endl;
		cout << endl;
		PlaintextOption choice = (PlaintextOption)io::input_num<int>("Enter your choice: ");
		while (PlaintextOption::Text != choice && PlaintextOption::Binary != choice)
			choice = (PlaintextOption)io::input_num<int>("Invalid input, try again: ");
		cout << endl;

		Buffer plaintext;
		if (PlaintextOption::Text == choice)
		{
			string msg = io::input("Enter message to encrypt (text): ");
			plaintext = Buffer(msg.begin(), msg.end());
		}
		else plaintext = bytes_from_base64(io::input("Enter message to encrypt (base64): "));
		cout << endl;

		auto [regLayerPubKey, ownerLayerPubKey] = io::input_pub_keys("Enter encryption key: ");
		cout << endl;

		auto ciphertext = schema.encrypt(plaintext, regLayerPubKey, ownerLayerPubKey);

		cout << "Encrypted message (ciphertext): ";
		io::print_ciphertext(ciphertext);
		cout << endl;

		return ConnStatus::Connected;
	}

	ConnStatus decrypt(PacketHandler& packetHandler)
	{
		auto usersetID = io::input_userset_id("Enter ID of userset to decrypt under: ");
		cout << endl;

		Ciphertext ciphertext = io::input_ciphertext("Enter ciphertext: ");
		cout << endl << endl;

		auto resp = post<pkt::DecryptResponse>(packetHandler, pkt::DecryptRequest{
			usersetID, std::move(ciphertext)
		});

		cout << "Decryption request submitted successfully." << endl;
		cout << "Operation ID: " << resp.op_id << endl << endl;

		return ConnStatus::Connected;
	}

	ConnStatus update(PacketHandler& packetHandler)
	{
		auto resp = post<pkt::UpdateResponse>(packetHandler, pkt::UpdateRequest{});
		bool hadUpdates = false;

		if (!resp.added_as_owner.empty())
		{
			hadUpdates = true;
			cout << "Added to " << resp.added_as_owner.size() << " new usersets as owner:" << endl;
			for (const auto& [i, data] : resp.added_as_owner | utils::views::enumerate)
				print_userset_data(i, data);
		}

		if (!resp.added_as_reg_member.empty())
		{
			hadUpdates = true;
			cout << "Added to " << resp.added_as_reg_member.size() << " new usersets as non-owner:" << endl;
			for (const auto& [i, data] : resp.added_as_reg_member | utils::views::enumerate)
				print_userset_data(i, data);
		}

		if (!resp.on_lookup.empty())
		{
			hadUpdates = true;
			cout << "IDs of operations looking for you:" << endl;
			for (const auto& [i, opid] : resp.on_lookup | utils::views::enumerate)
				cout << (i + 1) << ".\t" << opid << endl;
		}

		if (!resp.to_decrypt.empty())
		{
			hadUpdates = true;
			cout << "Pending decryption operations:" << endl;
			for (const auto& [i, data] : resp.to_decrypt | utils::views::enumerate)
				print_to_decrypt_data(i, data);
		}

		if (!resp.finished_decryptions.empty())
		{
			hadUpdates = true;
			cout << "Finished decryption operations:" << endl;
			for (const auto& [i, data] : resp.finished_decryptions | utils::views::enumerate)
				print_finished_data(i, data);
		}

		if (!hadUpdates)
			cout << "No updates to show." << endl;
		cout << endl;

		return ConnStatus::Connected;
	}

	ConnStatus participate(PacketHandler& packetHandler)
	{
		auto opid = io::input_operation_id("Enter operation ID: ");
		cout << endl;

		auto resp = post<pkt::DecryptParticipateResponse>(packetHandler, pkt::DecryptParticipateRequest{ opid });

		if (resp.status == pkt::DecryptParticipateResponse::Status::SendRegLayerPart)
			cout << "Participance registered, be ready to send non-owner layer part in a future update." << endl;
		else if (resp.status == pkt::DecryptParticipateResponse::Status::SendOwnerLayerPart)
			cout << "Participance registered, be ready to send owner layer part in a future update." << endl;
		else
			cout << "Your participance is not needed for this operation." << endl;

		return ConnStatus::Connected;
	}

	ConnStatus comp_part(PacketHandler& packetHandler)
	{
		(void)packetHandler;

		bool isOwner = io::input_yesno("Is this an owner layer part? (y/n): ");
		cout << endl;

		Ciphertext ciphertext = io::input_ciphertext("Enter ciphertext: ");
		cout << endl;

		PrivKeyShard privKeyShard = io::input_priv_key_shard("Enter your decryption key shard: ");
		cout << endl;

		auto privKeyShardsIDs = io::input_priv_key_shard_ids("Enter involved decryption key shard IDs (each in new line): ");
		cout << endl;

		DecryptionPart part{};
		if (isOwner)
			part = Shamir::decrypt_get_2l<OWNER_LAYER>(ciphertext, privKeyShard, privKeyShardsIDs);
		else
			part = Shamir::decrypt_get_2l<REG_LAYER>(ciphertext, privKeyShard, privKeyShardsIDs);

		cout << "Result decryption part: " << utils::bytes_to_base64(part.to_bytes()) << endl;

		return ConnStatus::Connected;
	}

	ConnStatus send_part(PacketHandler& packetHandler)
	{
		auto opid = io::input_operation_id("Enter operation ID: ");
		cout << endl;

		DecryptionPart part = io::input_decryption_part("Enter decryption part to send: ");
		cout << endl;

		post<pkt::SendDecryptionPartResponse>(packetHandler, pkt::SendDecryptionPartRequest{
			opid,
			std::move(part)
		});

		cout << "Part submitted successfully." << endl << endl;

		return ConnStatus::Connected;
	}

	ConnStatus join_parts(PacketHandler& packetHandler)
	{
		(void)packetHandler;

		auto ciphertext = io::input_ciphertext("Enter ciphertext: ");
		cout << endl;

		auto regLayerParts = io::input_decryption_parts("Enter non-owner layer decryption parts: ");

		auto ownerLayerParts = io::input_decryption_parts("Enter owner layer decryption parts: ");

		cout << endl;

		auto decrypted = Shamir::decrypt_join_2l(ciphertext, regLayerParts, ownerLayerParts);

		auto isText = io::input_yesno("Is this a textual message? (y/n): ");
		cout << endl;

		std::string msg;
		if (isText)
			msg = std::string(decrypted.begin(), decrypted.end());
		else
			msg = utils::bytes_to_base64(decrypted);

		cout << "Decrypted message:" << endl << msg << endl << endl;

		return ConnStatus::Connected;
	}

	inline void print_userset_data(size_t idx,
								   const utils::OneOf<AddedAsOwnerRecord, AddedAsMemberRecord> auto& data)
	{
		using Data = std::remove_cvref_t<decltype(data)>;
		cout << "==============================" << endl;

		cout << "Set #" << (idx + 1) << ":" << endl << endl;

		cout << "ID: " << data.user_set_id << endl << endl;

		io::print_pub_keys(data.reg_layer_pub_key, data.owner_layer_pub_key);
		cout << endl;

		io::print_reg_layer_priv_key_shard(data.reg_layer_priv_key_shard);

		if constexpr (std::same_as<Data, AddedAsOwnerRecord>)
		{
			cout << endl;
			io::print_owner_layer_priv_key_shard(data.owner_layer_priv_key_shard);
		}

		cout << "==============================" << endl << endl << endl;
	}

	void print_to_decrypt_data(size_t idx, const ToDecryptRecord& data)
	{
		cout << "==============================" << endl;

		cout << "To-Decrypt Operation #" << (idx + 1) << ":" << endl << endl;

		cout << "Operation ID: " << data.op_id << endl << endl;

		cout << "Ciphertext: ";
		io::print_ciphertext(data.ciphertext);
		cout << endl;

		cout << "Involved Shards IDs: ";
		if (!data.shards_ids.empty())
		{
			auto it = data.shards_ids.cbegin();
			cout << *it;
			for (++it; it != data.shards_ids.cend(); ++it)
				cout << ", " << *it;
		}
		cout << endl;

		cout << "==============================" << endl;
	}

	void print_finished_data(size_t idx, const FinishedDecryptionsRecord& data)
	{
		cout << "==============================" << endl;

		cout << "Finished Operation #" << (idx + 1) << ":" << endl << endl;

		cout << "Operation ID: " << data.op_id << endl << endl;

		cout << "Non-owner layer decryption parts:" << endl;
		for (const auto& part : data.reg_layer_parts)
			cout << utils::bytes_to_base64(part.to_bytes()) << endl;
		cout << endl;

		cout << "Non-owner layer involved shard IDs: ";
		if (!data.reg_layer_shards_ids.empty())
		{
			auto it = data.reg_layer_shards_ids.cbegin();
			cout << *it;
			for (++it; it != data.reg_layer_shards_ids.cend(); ++it)
				cout << ", " << *it;
		}
		cout << endl << endl;

		cout << "Owner layer decryption parts:" << endl;
		for (const auto& part : data.owner_layer_parts)
			cout << utils::bytes_to_base64(part.to_bytes()) << endl;
		cout << endl;

		cout << "Owner layer involved shard IDs: ";
		if (!data.owner_layer_shards_ids.empty())
		{
			auto it = data.owner_layer_shards_ids.cbegin();
			cout << *it;
			for (++it; it != data.owner_layer_shards_ids.cend(); ++it)
				cout << ", " << *it;
		}
		cout << endl << endl;

		cout << "==============================" << endl;
	}
}

int main(int argc, char** argv)
{
	return senc::client::main(argc, argv);
}
