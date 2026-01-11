/*********************************************************************
 * \file   EncryptedPacketHandler.cpp
 * \brief  Implementation of EncryptedPacketHandler class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "EncryptedPacketHandler.hpp"

#include "SockUtils.hpp"

namespace senc
{
	EncryptedPacketHandler::EncryptedPacketHandler()
		: _powDist(utils::Random<utils::BigInt>::get_dist_below(Group::order())) { }
	
	std::pair<bool, std::string> EncryptedPacketHandler::establish_connection_client_side(utils::Socket& sock)
	{
		// send protocol version
		sock.send_connected_primitive(pkt::PROTOCOL_VERSION);

		// receive flag indicating whether protocol version is OK
		const bool isProtocolVersoinOK = sock.recv_connected_primitive<bool>();
		if (!isProtocolVersoinOK)
			return { false, "Bad protocol version" };

		try
		{
			// sample x and send g^x for key exchange
			const utils::BigInt x = _powDist();
			const Group gx = Group::generator().pow(x);
			SockUtils::send_ecgroup_elem(sock, gx);

			// receive gy for key exchange
			Group gy{};
			SockUtils::recv_ecgroup_elem(sock, gy);

			// compute g^xy and dereive key
			gy *= gx;
			_key = _kdf(gy);
		}
		catch (const std::exception& e)
		{
			return { false, std::string("Failed to exchange key: ") + e.what() };
		}

		return { true, "" }; // success
	}

	std::pair<bool, std::string> EncryptedPacketHandler::establish_connection_server_side(utils::Socket& sock)
	{
		// receive & check protocol version
		auto protocolVersion = sock.recv_connected_primitive<std::uint8_t>();
		if (protocolVersion != pkt::PROTOCOL_VERSION)
		{
			sock.send_connected_primitive(false); // bad protocol version
			return { false, "Bad protocol version" };
		}
		sock.send_connected_primitive(true); // protocol version OK

		try
		{
			// receive gx for key exchange
			Group gx{};
			SockUtils::recv_ecgroup_elem(sock, gx);

			// sample y and send gy for key exchange
			const utils::BigInt y = _powDist();
			const Group gy = Group::generator().pow(y);
			SockUtils::send_ecgroup_elem(sock, gy);

			// compute g^xy and dereive key
			gx *= gy;
			_key = _kdf(gx);
		}
		catch (const std::exception& e)
		{
			return { false, std::string("Failed to exchange key: ") + e.what() };
		}

		return { true, "" }; // success
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::ErrorResponse& packet)
	{
		utils::Buffer data{};
		utils::write_bytes(data, packet.msg);
		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::ErrorResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.msg, it, end);
	}

	void EncryptedPacketHandler::send_request_data(utils::Socket& sock, const pkt::SignupRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.username);
		utils::write_bytes(data, packet.password);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_request_data(utils::Socket& sock, pkt::SignupRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.username, it, end);
		it = utils::read_bytes(out.password, it, end);
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::SignupResponse& packet)
	{
		utils::Buffer data{};
		utils::write_bytes(data, packet.status);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::SignupResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.status, it, end);
	}

	void EncryptedPacketHandler::send_request_data(utils::Socket& sock, const pkt::LoginRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.username);
		utils::write_bytes(data, packet.password);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_request_data(utils::Socket& sock, pkt::LoginRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.username, it, end);
		it = utils::read_bytes(out.password, it, end);
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::LoginResponse& packet)
	{
		utils::Buffer data{};
		utils::write_bytes(data, packet.status);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::LoginResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.status, it, end);
	}

	void EncryptedPacketHandler::send_request_data(utils::Socket& sock, const pkt::LogoutRequest& packet)
	{
		(void)sock;
		(void)packet;
	}

	void EncryptedPacketHandler::recv_request_data(utils::Socket& sock, pkt::LogoutRequest& out)
	{
		(void)sock;
		(void)out;
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::LogoutResponse& packet)
	{
		(void)sock;
		(void)packet;
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::LogoutResponse& out)
	{
		(void)sock;
		(void)out;
	}

	void EncryptedPacketHandler::send_request_data(utils::Socket& sock, const pkt::MakeUserSetRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.owners_threshold);
		utils::write_bytes(data, packet.reg_members_threshold);
		utils::write_bytes(data, static_cast<member_count_t>(packet.owners.size()));
		utils::write_bytes(data, static_cast<member_count_t>(packet.reg_members.size()));
		for (const auto& owner : packet.owners)
			utils::write_bytes(data, owner);
		for (const auto& regMember : packet.reg_members)
			utils::write_bytes(data, regMember);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_request_data(utils::Socket& sock, pkt::MakeUserSetRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.owners_threshold, it, end);
		it = utils::read_bytes(out.reg_members_threshold, it, end);

		member_count_t ownersCount{};
		it = utils::read_bytes(ownersCount, it, end);
		out.owners.resize(ownersCount);

		member_count_t regMembersCount{};
		it = utils::read_bytes(regMembersCount, it, end);
		out.reg_members.resize(regMembersCount);

		for (auto& owner : out.owners)
			it = utils::read_bytes(owner, it, end);

		for (auto& regMember : out.reg_members)
			it = utils::read_bytes(regMember, it, end);
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::MakeUserSetResponse& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.user_set_id);
		write_pub_key(data, packet.reg_layer_pub_key);
		write_pub_key(data, packet.owner_layer_pub_key);
		write_priv_key_shard(data, packet.reg_layer_priv_key_shard);
		write_priv_key_shard(data, packet.owner_layer_priv_key_shard);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::MakeUserSetResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.user_set_id, it, end);
		it = read_pub_key(out.reg_layer_pub_key, it, end);
		it = read_pub_key(out.owner_layer_pub_key, it, end);
		it = read_priv_key_shard(out.reg_layer_priv_key_shard, it, end);
		it = read_priv_key_shard(out.owner_layer_priv_key_shard, it, end);
	}

	void EncryptedPacketHandler::send_request_data(utils::Socket& sock, const pkt::GetUserSetsRequest& packet)
	{
		(void)sock;
		(void)packet;
	}

	void EncryptedPacketHandler::recv_request_data(utils::Socket& sock, pkt::GetUserSetsRequest& out)
	{
		(void)sock;
		(void)out;
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::GetUserSetsResponse& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, static_cast<userset_count_t>(packet.user_sets_ids.size()));
		for (const auto& userSetID : packet.user_sets_ids)
			utils::write_bytes(data, userSetID);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::GetUserSetsResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		userset_count_t usersetsCount{};
		it = utils::read_bytes(usersetsCount, it, end);
		out.user_sets_ids.resize(usersetsCount);
		for (auto& userSetID : out.user_sets_ids)
			it = utils::read_bytes(userSetID, it, end);
	}

	void EncryptedPacketHandler::send_request_data(utils::Socket& sock, const pkt::GetMembersRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.user_set_id);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_request_data(utils::Socket& sock, pkt::GetMembersRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.user_set_id, it, end);
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::GetMembersResponse& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, static_cast<member_count_t>(packet.owners.size()));
		utils::write_bytes(data, static_cast<member_count_t>(packet.reg_members.size()));
		for (const auto& owner : packet.owners)
			utils::write_bytes(data, owner);
		for (const auto& reg_member : packet.reg_members)
			utils::write_bytes(data, reg_member);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::GetMembersResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		member_count_t ownersCount{};
		it = utils::read_bytes(ownersCount, it, end);
		out.owners.resize(ownersCount);

		member_count_t regMembersCount{};
		it = utils::read_bytes(regMembersCount, it, end);
		out.reg_members.resize(regMembersCount);

		for (auto& owner : out.owners)
			it = utils::read_bytes(owner, it, end);

		for (auto& regMember : out.reg_members)
			it = utils::read_bytes(regMember, it, end);
	}

	void EncryptedPacketHandler::send_request_data(utils::Socket& sock, const pkt::DecryptRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.user_set_id);
		write_ciphertext(data, packet.ciphertext);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_request_data(utils::Socket& sock, pkt::DecryptRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.user_set_id, it, end);
		it = read_ciphertext(out.ciphertext, it, end);
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::DecryptResponse& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.op_id);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::DecryptResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.op_id, it, end);
	}

	void EncryptedPacketHandler::send_request_data(utils::Socket& sock, const pkt::UpdateRequest& packet)
	{
		(void)sock;
		(void)packet;
	}

	void EncryptedPacketHandler::recv_request_data(utils::Socket& sock, pkt::UpdateRequest& out)
	{
		(void)sock;
		(void)out;
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::UpdateResponse& packet)
	{
		utils::Buffer data{};

		// write vector lengths
		utils::write_bytes(data, static_cast<userset_count_t>(packet.added_as_owner.size()));
		utils::write_bytes(data, static_cast<userset_count_t>(packet.added_as_reg_member.size()));
		utils::write_bytes(data, static_cast<lookup_count_t>(packet.on_lookup.size()));
		utils::write_bytes(data, static_cast<pending_count_t>(packet.to_decrypt.size()));
		utils::write_bytes(data, static_cast<res_count_t>(packet.finished_decryptions.size()));

		// write added_as_owner records
		for (const auto& record : packet.added_as_owner)
			write_update_record(data, record);

		// write added_as_reg_member records
		for (const auto& record : packet.added_as_reg_member)
			write_update_record(data, record);
		
		// write on_lookup records
		for (const auto& record : packet.on_lookup)
			utils::write_bytes(data, record);

		// send to_decrypt records
		for (const auto& record : packet.to_decrypt)
			write_update_record(data, record);

		// send finished_decryptions records
		for (const auto& record : packet.finished_decryptions)
			write_update_record(data, record);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::UpdateResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		// read vector lengths

		userset_count_t addedAsOwnerCount{};
		it = utils::read_bytes(addedAsOwnerCount, it, end);

		userset_count_t addedAsRegMemberCount{};
		it = utils::read_bytes(addedAsRegMemberCount, it, end);

		lookup_count_t onLookupCount{};
		it = utils::read_bytes(onLookupCount, it, end);

		pending_count_t toDecryptCount{};
		it = utils::read_bytes(toDecryptCount, it, end);

		res_count_t finishedDecryptionsCount{};
		it = utils::read_bytes(finishedDecryptionsCount, it, end);

		// end read vector lengths

		// read added_as_owner records
		out.added_as_owner.resize(addedAsOwnerCount);
		for (auto& record : out.added_as_owner)
			it = read_update_record(record, it, end);

		// read added_as_reg_member records
		out.added_as_reg_member.resize(addedAsRegMemberCount);
		for (auto& record : out.added_as_reg_member)
			it = read_update_record(record, it, end);

		// read on_lookup records
		out.on_lookup.resize(onLookupCount);
		for (auto& record : out.on_lookup)
			it = utils::read_bytes(record, it, end);

		// read to_decrypt records
		out.to_decrypt.resize(toDecryptCount);
		for (auto& record : out.to_decrypt)
			it = read_update_record(record, it, end);

		// read finished_decryptions records
		out.finished_decryptions.resize(finishedDecryptionsCount);
		for (auto& record : out.finished_decryptions)
			it = read_update_record(record, it, end);
	}

	void EncryptedPacketHandler::send_request_data(utils::Socket& sock, const pkt::DecryptParticipateRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.op_id);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_request_data(utils::Socket& sock, pkt::DecryptParticipateRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.op_id, it, end);
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::DecryptParticipateResponse& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.status);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::DecryptParticipateResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.status, it, end);
	}

	void EncryptedPacketHandler::send_request_data(utils::Socket& sock, const pkt::SendDecryptionPartRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.op_id);
		write_decryption_part(data, packet.decryption_part);

		send_encrypted_data(sock, data);
	}

	void EncryptedPacketHandler::recv_request_data(utils::Socket& sock, pkt::SendDecryptionPartRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(sock, data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.op_id, it, end);
		it = read_decryption_part(out.decryption_part, it, end);
	}

	void EncryptedPacketHandler::send_response_data(utils::Socket& sock, const pkt::SendDecryptionPartResponse& packet)
	{
		(void)sock;
		(void)packet;
	}

	void EncryptedPacketHandler::recv_response_data(utils::Socket& sock, pkt::SendDecryptionPartResponse& out)
	{
		(void)sock;
		(void)out;
	}

	void EncryptedPacketHandler::send_encrypted_data(utils::Socket& sock, const utils::Buffer& data)
	{
		utils::enc::Ciphertext<Schema> encryptedData = _schema.encrypt(data, _key);
		const auto& [c1, c2] = encryptedData;
		sock.send_connected_primitive(static_cast<std::uint64_t>(c1.size()));
		sock.send_connected_primitive(static_cast<std::uint64_t>(c2.size()));
		sock.send_connected(c1);
		sock.send_connected(c2);
	}

	void EncryptedPacketHandler::recv_encrypted_data(utils::Socket& sock, utils::Buffer& out)
	{
		utils::enc::Ciphertext<Schema> encryptedData{};
		auto& [c1, c2] = encryptedData;

		const auto c1Size = sock.recv_connected_primitive<std::uint64_t>();
		c1.resize(c1Size);

		const auto c2Size = sock.recv_connected_primitive<std::uint64_t>();
		c2.resize(c2Size);

		sock.recv_connected_exact_into(c1);
		sock.recv_connected_exact_into(c2);

		out = _schema.decrypt(encryptedData, _key);
	}

	void EncryptedPacketHandler::write_big_int(utils::Buffer& out, const std::optional<utils::BigInt>& value)
	{
		if (!value.has_value())
		{
			utils::write_bytes(out, static_cast<utils::bigint_size_t>(0));
			return;
		}

		utils::write_bytes(out, static_cast<utils::bigint_size_t>(value->MinEncodedSize()));

		byte* outValue = out.data() + out.size();
		out.resize(out.size() + value->MinEncodedSize());
		value->Encode(outValue, value->MinEncodedSize());
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_big_int(std::optional<utils::BigInt>& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		const utils::bigint_size_t size{};
		it = utils::read_bytes(size, it, end);
		if (!size)
		{
			out = std::nullopt;
			return;
		}

		out = utils::BigInt{};
		out->Decode(std::to_address(it), size);
	}

	void EncryptedPacketHandler::write_pub_key(utils::Buffer& out, const PubKey& elem)
	{
		return write_ecgroup_elem(out, elem);
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_pub_key(PubKey& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		return read_ecgroup_elem(out, it, end);
	}
}
