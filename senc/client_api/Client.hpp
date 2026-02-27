/*********************************************************************
 * \file   Client.hpp
 * \brief  Header of `Client` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "../common/ClientPacketHandlerFactory.hpp"
#include "../common/QueuedPacketHandler.hpp"
#include "storage/ProfileStorage.hpp"
#include "../utils/Socket.hpp"
#include "IClient.hpp"
#include <optional>

namespace senc::clientapi
{
	/**
	 * @class senc::clientapi::Client
	 * @brief Manages internal client functionality.
	 * @tparam IP IP type used for communication.
	 */
	template <utils::IPType IP>
	class Client : public IClient
	{
	public:
		using Self = Client;
		using Socket = utils::TcpSocket<IP>;

		/**
		 * @brief Constructs a new client and connects it to server.
		 * @param serverIP Server's IP address.
		 * @param serverPort Server's listening port.
		 * @param schemaFactory Function constructing instance of decryption schema used by server.
		 * @param packetHandlerFactory Factory constructing implementation of `PacketHandler`.
		 * @param decryptFinishedCallback Callback function to call on decryption finish.
		 */
		Client(const IP& serverIP, utils::Port& serverPort,
			   std::function<Schema()> schemaFactory,
			   ClientPacketHandlerFactory packetHandlerFactory,
			   std::function<void(const OperationID&, const utils::Buffer&)> decryptFinishedCallback);

		/**
		 * @brief Client destructor, logs out of server.
		 */
		~Client();

		void signup(const std::string& username, const std::string& password) override;

		void login(const std::string& username, const std::string& password) override;

		void logout() override;

		UserSetID make_userset(utils::ranges::StringViewRange&& owners,
							   utils::ranges::StringViewRange&& regMembers,
							   member_count_t ownersThreshold,
							   member_count_t regMembersThreshold) override;

		void get_usersets(std::function<void(const UserSetID&)> callback) override;

		void get_userset_members(const UserSetID& usersetID,
								 std::function<void(const std::string&)> ownersCallback,
								 std::function<void(const std::string&)> regsCallback) override;

		Ciphertext encrypt(const UserSetID& usersetID, const utils::Buffer& msg) override;

		OperationID decrypt(const UserSetID& usersetID, const Ciphertext& ciphertext) override;

	private:
		IP _serverIP;
		utils::Port _serverPort;
		std::function<void(const OperationID&, const utils::Buffer&)> _decryptFinishedCallback;
		ClientPacketHandlerFactory _packetHandlerFactory;
		std::optional<storage::ProfileStorage> _storage;
		std::optional<QueuedPacketHandler> _packetHandler;
		Schema _schema;
		Socket _sock;

		/**
		 * @brief Makes sure client is connected to server.
		 */
		void ensure_connected();

		/**
		 * @brief Emplaces backet handler to be ready to handle packets.
		 */
		void emplace_packet_handler();

		/**
		 * @brief Loads user's profile from memory.
		 * @param username Username of user to load its profile.
		 * @param password Password of user to load its profile.
		 */
		void load_profile(const std::string& username, const std::string& password);

		/**
		 * @brief Unloads loaded user's profile.
		 */
		void unload_profile();

		/**
		 * @brief Locates a profile record from userset ID.
		 * @param usersetID Userset ID.
		 * @return Located profile record.
		 * @throw ClientException If not found or user not logged in.
		 */
		storage::ProfileRecord find_profile_record_by_userset_id(const UserSetID& usersetID);

		/**
		 * @brief Sends request and returns retrieved response.
		 * @tparam Resp Response type.
		 * @tparam Req Request type.
		 * @param request Request to send.
		 * @return Received response.
		 * @throw ClientException If error occured.
		 */
		template <typename Resp, typename Req>
		Resp post(const Req& request);

		/**
		 * @brief Sends request and returns retrieved response.
		 * @tparam Resp Response type.
		 * @tparam Req Request type.
		 * @param packetHandler Packet handler instance to use for sending and receiving.
		 * @param request Request to send.
		 * @return Received response.
		 * @throw ClientException If error occured.
		 */
		template <typename Resp, typename Req>
		static Resp post(PacketHandler& packetHandler, const Req& request);
	};
}

#include "Client_impl.hpp"
