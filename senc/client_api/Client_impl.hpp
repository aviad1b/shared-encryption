/*********************************************************************
 * \file   Client_impl.hpp
 * \brief  Implementation of `Client` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "Client.hpp"

#include "ClientException.hpp"
#include "ClientUtils.hpp"
#include <algorithm>

namespace senc::clientapi
{
	template <utils::IPType IP>
	inline Client<IP>::Client(const IP& serverIP, utils::Port& serverPort,
							  std::function<Schema()> schemaFactory,
							  ClientPacketHandlerFactory packetHandlerFactory,
							  std::function<void(const OperationID&, const utils::Buffer&)> decryptFinishedCallback)
		: _serverIP(serverIP), _serverPort(serverPort),
		  _decryptFinishedCallback(decryptFinishedCallback),
		  _packetHandlerFactory(packetHandlerFactory),
		  _schema(schemaFactory()),
		  _sock(serverIP, serverPort)
	{
		this->_packetHandler = _packetHandlerFactory(_sock);
	}

	template <utils::IPType IP>
	inline Client<IP>::~Client()
	{
		if (this->_packetHandler) // if still connected (packet handler not null)
			logout();
	}

	template <utils::IPType IP>
	inline void Client<IP>::signup(const std::string& username, const std::string& password)
	{
		ensure_connected();

		pkt::SignupResponse resp = this->post<pkt::SignupResponse>(pkt::SignupRequest{
			username, password
		});

		if (resp.status == pkt::SignupResponse::Status::UsernameTaken)
			throw ClientException("Signup failed", "Username already taken");
		if (resp.status != pkt::SignupResponse::Status::Success)
			throw ClientException("Signup failed", "Unknown error");

		this->load_profile(username, password);
	}

	template <utils::IPType IP>
	inline void Client<IP>::login(const std::string& username, const std::string& password)
	{
		ensure_connected();

		pkt::LoginResponse resp = this->post<pkt::LoginResponse>(pkt::LoginRequest{
			username, password
		});

		if (resp.status == pkt::LoginResponse::Status::BadLogin)
			throw ClientException("Login failed", "Bad username or password");
		if (resp.status != pkt::LoginResponse::Status::Success)
			throw ClientException("Login failed", "Unknown error");

		this->load_profile(username, password);
	}

	template <utils::IPType IP>
	inline void Client<IP>::logout()
	{
		this->post<pkt::LogoutResponse>(pkt::LogoutRequest{});
		this->_packetHandler.reset();
		this->_sock.close();
	}

	template <utils::IPType IP>
	inline UserSetID Client<IP>::make_userset(utils::ranges::StringViewRange&& owners,
											  utils::ranges::StringViewRange&& regMembers,
											  member_count_t ownersThreshold,
											  member_count_t regMembersThreshold)
	{
		pkt::MakeUserSetResponse resp = this->post<pkt::MakeUserSetResponse>(pkt::MakeUserSetRequest{
			.reg_members = utils::to_vector<std::string>(regMembers),
			.owners = utils::to_vector<std::string>(owners),
			.reg_members_threshold = regMembersThreshold,
			.owners_threshold = ownersThreshold
		});

		this->_storage->add_profile_data(storage::ProfileRecord::owner(
			UserSetID(resp.user_set_id),
			std::move(resp.reg_layer_pub_key),
			std::move(resp.owner_layer_pub_key),
			std::move(resp.reg_layer_priv_key_shard),
			std::move(resp.owner_layer_priv_key_shard)
		));

		return resp.user_set_id;
	}

	template <utils::IPType IP>
	inline void Client<IP>::get_usersets(std::function<void(const UserSetID&)> callback)
	{
		pkt::GetUserSetsResponse resp = this->post<pkt::GetUserSetsResponse>(pkt::GetUserSetsRequest{});
		for (const UserSetID& id : resp.user_sets_ids)
			callback(id);
	}

	template <utils::IPType IP>
	inline void Client<IP>::get_userset_members(const UserSetID& usersetID,
												std::function<void(const std::string&)> ownersCallback,
												std::function<void(const std::string&)> regsCallback)
	{
		pkt::GetMembersResponse resp = this->post<pkt::GetMembersResponse>(pkt::GetMembersRequest{
			usersetID
		});
		for (const std::string& owner : resp.owners)
			ownersCallback(owner);
		for (const std::string& reg : resp.reg_members)
			regsCallback(reg);
	}

	template <utils::IPType IP>
	inline Ciphertext Client<IP>::encrypt(const UserSetID& usersetID, const utils::Buffer& msg)
	{
		const storage::ProfileRecord record = find_profile_record_by_userset_id(usersetID);

		return _schema.encrypt(
			msg,
			record.reg_layer_pub_key(),
			record.owner_layer_pub_key()
		); 
	}

	template <utils::IPType IP>
	inline OperationID Client<IP>::decrypt(const UserSetID& usersetID, const Ciphertext& ciphertext)
	{
		pkt::DecryptResponse resp = this->post<pkt::DecryptResponse>(pkt::DecryptRequest{
			usersetID, ciphertext
		});
		return resp.op_id;
	}

	template <utils::IPType IP>
	inline void senc::clientapi::Client<IP>::ensure_connected()
	{
		if (this->_packetHandler) // if connected (packet handler not null)
			return; // nothing to do

		// if not connected (disconnected earlier) - reconnect
		this->_sock = Socket(_serverIP, _serverPort);
		this->_packetHandler = _packetHandlerFactory(_sock);
	}

	template <utils::IPType IP>
	inline void Client<IP>::load_profile(const std::string& username, const std::string& password)
	{
		_storage.emplace(
			ClientUtils::locate_user_profile_file(username, password),
			username, password
		);
	}

	template <utils::IPType IP>
	inline void Client<IP>::unload_profile()
	{
		_storage.reset();
	}

	template <utils::IPType IP>
	inline storage::ProfileRecord Client<IP>::find_profile_record_by_userset_id(const UserSetID& usersetID)
	{
		if (!_storage)
			throw ClientException("Failed to get user data", "Not logged in");
		auto profileData = _storage->iter_profile_data();
		const auto it = std::find_if(
			profileData.begin(), profileData.end(),
			[usersetID](const storage::ProfileRecord& record)
			{
				return record.userset_id() == usersetID;
			}
		);
		if (it == profileData.end())
			throw ClientException(
				"Local storage error",
				"Failed to locate userset " + usersetID.to_string()
			);
		return *it;
	}

	template <utils::IPType IP>
	template <typename Resp, typename Req>
	inline Resp senc::clientapi::Client<IP>::post(const Req& request)
	{
		if (!_packetHandler)
			throw ClientException("Failed to send request", "Not logged in");
		_packetHandler->send_request(request);
		auto resp = _packetHandler->recv_response<Resp, pkt::ErrorResponse>();
		if (!resp)
			throw ClientException("Unexpected response received");
		if (std::holds_alternative<pkt::ErrorResponse>(*resp))
			throw ClientException(std::get<pkt::ErrorResponse>(*resp).msg);
		return std::get<Resp>(*resp);
	}
}
