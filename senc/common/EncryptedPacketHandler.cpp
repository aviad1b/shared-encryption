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
	utils::Distribution<utils::BigInt> EncryptedPacketHandler::_powDist(
		utils::Random<utils::BigInt>::get_dist_below(Group::order())
	);

	EncryptedPacketHandler::Self EncryptedPacketHandler::server(utils::Socket& sock)
	{
		Self res(sock);

		// receive & check protocol version
		auto protocolVersion = res._sock.recv_connected_primitive<std::uint8_t>();
		if (protocolVersion != pkt::PROTOCOL_VERSION)
		{
			res._sock.send_connected_primitive(false); // bad protocol version
			throw ConnEstablishException("Bad protocol version");
		}
		res._sock.send_connected_primitive(true); // protocol version OK

		try
		{
			// receive gx for key exchange
			Group gx{};
			SockUtils::recv_ecgroup_elem(res._sock, gx);

			// sample y and send gy for key exchange
			const utils::BigInt y = _powDist();
			const Group gy = Group::generator().pow(y);
			SockUtils::send_ecgroup_elem(res._sock, gy);

			// compute g^xy and dereive key
			const Group sharedSecret = gx.pow(y); // gx^y = g^(xy)
			res._key = res._kdf(sharedSecret);
		}
		catch (const std::exception& e)
		{
			throw ConnEstablishException(std::string("Failed to exchange key: ") + e.what());
		}

		return res;
	}

	EncryptedPacketHandler::Self EncryptedPacketHandler::client(utils::Socket& sock)
	{
		Self res(sock);

		// send protocol version
		res._sock.send_connected_primitive(pkt::PROTOCOL_VERSION);

		// receive flag indicating whether protocol version is OK
		const bool isProtocolVersoinOK = res._sock.recv_connected_primitive<bool>();
		if (!isProtocolVersoinOK)
			throw ConnEstablishException("Bad protocol version");

		try
		{
			// sample x and send g^x for key exchange
			const utils::BigInt x = _powDist();
			const Group gx = Group::generator().pow(x);
			SockUtils::send_ecgroup_elem(res._sock, gx);

			// receive gy for key exchange
			Group gy{};
			SockUtils::recv_ecgroup_elem(res._sock, gy);

			// compute g^xy and dereive key
			const Group sharedSecret = gy.pow(x); // gy^x = g^(xy)
			res._key = res._kdf(sharedSecret);
		}
		catch (const std::exception& e)
		{
			throw ConnEstablishException(std::string("Failed to exchange key: ") + e.what());
		}

		return res;
	}

	bool EncryptedPacketHandler::validate_synchronization(const Base* other) const
	{
		// return false if other is not of same type as self
		const Self* other2 = dynamic_cast<const Self*>(other);
		if (!other2)
			return false;

		// check synchronized keys
		return (this->_key == other2->_key);
	}

	void EncryptedPacketHandler::send_response_data(const pkt::ErrorResponse& packet)
	{
		utils::Buffer data{};
		utils::write_bytes(data, packet.msg);
		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_response_data(pkt::ErrorResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.msg, it, end);
	}

	void EncryptedPacketHandler::send_request_data(const pkt::SignupRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.username);
		utils::write_bytes(data, packet.password);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_request_data(pkt::SignupRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.username, it, end);
		it = utils::read_bytes(out.password, it, end);
	}

	void EncryptedPacketHandler::send_response_data(const pkt::SignupResponse& packet)
	{
		utils::Buffer data{};
		utils::write_bytes(data, packet.status);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_response_data(pkt::SignupResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.status, it, end);
	}

	void EncryptedPacketHandler::send_request_data(const pkt::LoginRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.username);
		utils::write_bytes(data, packet.password);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_request_data(pkt::LoginRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.username, it, end);
		it = utils::read_bytes(out.password, it, end);
	}

	void EncryptedPacketHandler::send_response_data(const pkt::LoginResponse& packet)
	{
		utils::Buffer data{};
		utils::write_bytes(data, packet.status);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_response_data(pkt::LoginResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.status, it, end);
	}

	void EncryptedPacketHandler::send_request_data(const pkt::LogoutRequest& packet)
	{
		(void)packet;
	}

	void EncryptedPacketHandler::recv_request_data(pkt::LogoutRequest& out)
	{
		(void)out;
	}

	void EncryptedPacketHandler::send_response_data(const pkt::LogoutResponse& packet)
	{
		(void)packet;
	}

	void EncryptedPacketHandler::recv_response_data(pkt::LogoutResponse& out)
	{
		(void)out;
	}

	void EncryptedPacketHandler::send_request_data(const pkt::MakeUserSetRequest& packet)
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

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_request_data(pkt::MakeUserSetRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
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

	void EncryptedPacketHandler::send_response_data(const pkt::MakeUserSetResponse& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.user_set_id);
		write_pub_key(data, packet.reg_layer_pub_key);
		write_pub_key(data, packet.owner_layer_pub_key);
		write_priv_key_shard(data, packet.reg_layer_priv_key_shard);
		write_priv_key_shard(data, packet.owner_layer_priv_key_shard);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_response_data(pkt::MakeUserSetResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.user_set_id, it, end);
		it = read_pub_key(out.reg_layer_pub_key, it, end);
		it = read_pub_key(out.owner_layer_pub_key, it, end);
		it = read_priv_key_shard(out.reg_layer_priv_key_shard, it, end);
		it = read_priv_key_shard(out.owner_layer_priv_key_shard, it, end);
	}

	void EncryptedPacketHandler::send_request_data(const pkt::GetUserSetsRequest& packet)
	{
		(void)packet;
	}

	void EncryptedPacketHandler::recv_request_data(pkt::GetUserSetsRequest& out)
	{
		(void)out;
	}

	void EncryptedPacketHandler::send_response_data(const pkt::GetUserSetsResponse& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, static_cast<userset_count_t>(packet.user_sets_ids.size()));
		for (const auto& userSetID : packet.user_sets_ids)
			utils::write_bytes(data, userSetID);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_response_data(pkt::GetUserSetsResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		userset_count_t usersetsCount{};
		it = utils::read_bytes(usersetsCount, it, end);
		out.user_sets_ids.resize(usersetsCount);
		for (auto& userSetID : out.user_sets_ids)
			it = utils::read_bytes(userSetID, it, end);
	}

	void EncryptedPacketHandler::send_request_data(const pkt::GetMembersRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.user_set_id);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_request_data(pkt::GetMembersRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.user_set_id, it, end);
	}

	void EncryptedPacketHandler::send_response_data(const pkt::GetMembersResponse& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, static_cast<member_count_t>(packet.owners.size()));
		utils::write_bytes(data, static_cast<member_count_t>(packet.reg_members.size()));
		for (const auto& owner : packet.owners)
			utils::write_bytes(data, owner);
		for (const auto& reg_member : packet.reg_members)
			utils::write_bytes(data, reg_member);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_response_data(pkt::GetMembersResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
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

	void EncryptedPacketHandler::send_request_data(const pkt::DecryptRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.user_set_id);
		write_ciphertext(data, packet.ciphertext);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_request_data(pkt::DecryptRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.user_set_id, it, end);
		it = read_ciphertext(out.ciphertext, it, end);
	}

	void EncryptedPacketHandler::send_response_data(const pkt::DecryptResponse& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.op_id);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_response_data(pkt::DecryptResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.op_id, it, end);
	}

	void EncryptedPacketHandler::send_request_data(const pkt::UpdateRequest& packet)
	{
		(void)packet;
	}

	void EncryptedPacketHandler::recv_request_data(pkt::UpdateRequest& out)
	{
		(void)out;
	}

	void EncryptedPacketHandler::send_response_data(const pkt::UpdateResponse& packet)
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

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_response_data(pkt::UpdateResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
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

	void EncryptedPacketHandler::send_request_data(const pkt::DecryptParticipateRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.op_id);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_request_data(pkt::DecryptParticipateRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.op_id, it, end);
	}

	void EncryptedPacketHandler::send_response_data(const pkt::DecryptParticipateResponse& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.status);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_response_data(pkt::DecryptParticipateResponse& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.status, it, end);
	}

	void EncryptedPacketHandler::send_request_data(const pkt::SendDecryptionPartRequest& packet)
	{
		utils::Buffer data{};

		utils::write_bytes(data, packet.op_id);
		write_decryption_part(data, packet.decryption_part);

		send_encrypted_data(data);
	}

	void EncryptedPacketHandler::recv_request_data(pkt::SendDecryptionPartRequest& out)
	{
		utils::Buffer data{};
		recv_encrypted_data(data);
		const auto end = data.end();
		auto it = data.begin();

		it = utils::read_bytes(out.op_id, it, end);
		it = read_decryption_part(out.decryption_part, it, end);
	}

	void EncryptedPacketHandler::send_response_data(const pkt::SendDecryptionPartResponse& packet)
	{
		(void)packet;
	}

	void EncryptedPacketHandler::recv_response_data(pkt::SendDecryptionPartResponse& out)
	{
		(void)out;
	}

	EncryptedPacketHandler::EncryptedPacketHandler(utils::Socket& sock)
		: Base(sock) { }

	void EncryptedPacketHandler::send_encrypted_data(const utils::Buffer& data)
	{
		utils::enc::Ciphertext<Schema> encryptedData = _schema.encrypt(data, _key);
		const auto& [c1, c2] = encryptedData;
		_sock.send_connected_primitive(static_cast<encdata_size_t>(c1.size()));
		_sock.send_connected_primitive(static_cast<encdata_size_t>(c2.size()));
		_sock.send_connected(c1);
		_sock.send_connected(c2);
	}

	void EncryptedPacketHandler::recv_encrypted_data(utils::Buffer& out)
	{
		utils::enc::Ciphertext<Schema> encryptedData{};
		auto& [c1, c2] = encryptedData;

		const auto c1Size = _sock.recv_connected_primitive<encdata_size_t>();
		c1.resize(c1Size);

		const auto c2Size = _sock.recv_connected_primitive<encdata_size_t>();
		c2.resize(c2Size);

		_sock.recv_connected_exact_into(c1);
		_sock.recv_connected_exact_into(c2);

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

		const auto oldSize = out.size();
		out.resize(out.size() + value->MinEncodedSize());
		value->Encode(out.data() + oldSize, value->MinEncodedSize());
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_big_int(std::optional<utils::BigInt>& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		utils::bigint_size_t size{};
		it = utils::read_bytes(size, it, end);
		if (!size)
		{
			out = std::nullopt;
			return it;
		}

		out = utils::BigInt{};
		out->Decode(std::to_address(it), size);

		return it + size;
	}

	void EncryptedPacketHandler::write_ecgroup_elem(utils::Buffer& out, const utils::ECGroup& elem)
	{
		// if x is written as nullopt then elem is identity (and y isn't written)
		if (elem.is_identity())
		{
			write_big_int(out, std::nullopt);
			return;
		}
		write_big_int(out, elem.x());
		write_big_int(out, elem.y());
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_ecgroup_elem(utils::ECGroup& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		std::optional<utils::BigInt> x, y;

		// if x is written as nullopt then elem is identity (and y isn't written)
		it = read_big_int(x, it, end);
		if (!x.has_value())
		{
			out = utils::ECGroup::identity();
			return it;
		}
		it = read_big_int(y, it, end);

		out = utils::ECGroup(std::move(*x), std::move(*y));

		return it;
	}

	void EncryptedPacketHandler::write_pub_key(utils::Buffer& out, const PubKey& elem)
	{
		return write_ecgroup_elem(out, elem);
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_pub_key(PubKey& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		return read_ecgroup_elem(out, it, end);
	}

	void EncryptedPacketHandler::write_priv_key_shard_id(utils::Buffer& out, const PrivKeyShardID& shardID)
	{
		return write_big_int(out, shardID);
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_priv_key_shard_id(PrivKeyShardID& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		std::optional<utils::BigInt> value;
		it = read_big_int(value, it, end);

		out = *value;

		return it;
	}

	void EncryptedPacketHandler::write_priv_key_shard(utils::Buffer& out, const PrivKeyShard& shard)
	{
		write_priv_key_shard_id(out, shard.first);
		write_big_int(out, shard.second);
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_priv_key_shard(PrivKeyShard& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		it = read_priv_key_shard_id(out.first, it, end);

		std::optional<utils::BigInt> value;
		it = read_big_int(value, it, end);
		out.second = *value;

		return it;
	}

	void EncryptedPacketHandler::write_ciphertext(utils::Buffer& out, const Ciphertext& ciphertext)
	{
		const auto& [c1, c2, c3] = ciphertext;
		const auto& [c3a, c3b] = c3;

		write_ecgroup_elem(out, c1);
		write_ecgroup_elem(out, c2);

		utils::write_bytes(out, static_cast<buffer_size_t>(c3a.size()));
		utils::write_bytes(out, static_cast<buffer_size_t>(c3b.size()));
		utils::write_bytes(out, c3a);
		utils::write_bytes(out, c3b);
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_ciphertext(Ciphertext& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		auto& [c1, c2, c3] = out;
		auto& [c3a, c3b] = c3;

		it = read_ecgroup_elem(c1, it, end);
		it = read_ecgroup_elem(c2, it, end);

		// c3: reserve space then read directly from socket

		buffer_size_t c3aSize{};
		it = utils::read_bytes(c3aSize, it, end);

		buffer_size_t c3bSize{};
		it = utils::read_bytes(c3bSize, it, end);

		c3a.resize(c3aSize);
		it = utils::read_bytes(c3a, it, end);

		c3b.resize(c3bSize);
		it = utils::read_bytes(c3b, it, end);

		return it;
	}

	void EncryptedPacketHandler::write_decryption_part(utils::Buffer& out, const DecryptionPart& part)
	{
		write_ecgroup_elem(out, part);
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_decryption_part(DecryptionPart& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		return read_ecgroup_elem(out, it, end);
	}

	void EncryptedPacketHandler::write_update_record(utils::Buffer& out, const pkt::UpdateResponse::AddedAsOwnerRecord& record)
	{
		write_update_record(
			out,
			reinterpret_cast<const pkt::UpdateResponse::AddedAsMemberRecord&>(record)
		);
		write_priv_key_shard(out, record.owner_layer_priv_key_shard);
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_update_record(pkt::UpdateResponse::AddedAsOwnerRecord& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		it = read_update_record(
			reinterpret_cast<pkt::UpdateResponse::AddedAsMemberRecord&>(out),
			it, end
		);
		it = read_priv_key_shard(out.owner_layer_priv_key_shard, it, end);

		return it;
	}

	void EncryptedPacketHandler::write_update_record(utils::Buffer& out, const pkt::UpdateResponse::AddedAsMemberRecord& record)
	{
		utils::write_bytes(out, record.user_set_id);
		write_pub_key(out, record.reg_layer_pub_key);
		write_pub_key(out, record.owner_layer_pub_key);
		write_priv_key_shard(out, record.reg_layer_priv_key_shard);
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_update_record(pkt::UpdateResponse::AddedAsMemberRecord& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		it = utils::read_bytes(out.user_set_id, it, end);
		it = read_pub_key(out.reg_layer_pub_key, it, end);
		it = read_pub_key(out.owner_layer_pub_key, it, end);
		it = read_priv_key_shard(out.reg_layer_priv_key_shard, it, end);
		return it;
	}

	void EncryptedPacketHandler::write_update_record(utils::Buffer& out, const pkt::UpdateResponse::ToDecryptRecord& record)
	{
		utils::write_bytes(out, record.op_id);
		write_ciphertext(out, record.ciphertext);
		utils::write_bytes(out, static_cast<member_count_t>(record.shards_ids.size()));
		for (const auto& shardID : record.shards_ids)
			write_priv_key_shard_id(out, shardID);
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_update_record(pkt::UpdateResponse::ToDecryptRecord& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		it = utils::read_bytes(out.op_id, it, end);
		it = read_ciphertext(out.ciphertext, it, end);

		member_count_t shardsIDsCount{};
		it = utils::read_bytes(shardsIDsCount, it, end);
		
		out.shards_ids.resize(shardsIDsCount);
		for (auto& shardID : out.shards_ids)
			it = read_priv_key_shard_id(shardID, it, end);

		return it;
	}

	void EncryptedPacketHandler::write_update_record(utils::Buffer& out, const pkt::UpdateResponse::FinishedDecryptionsRecord& record)
	{
		// NOTE: Assuming each shards IDs vector has is exactly one more than its corresponding parts vector
		utils::write_bytes(out, static_cast<member_count_t>(record.reg_layer_parts.size()));
		utils::write_bytes(out, static_cast<member_count_t>(record.owner_layer_parts.size()));
		utils::write_bytes(out, record.op_id);
		for (const auto& part : record.reg_layer_parts)
			write_decryption_part(out, part);
		for (const auto& part : record.owner_layer_parts)
			write_decryption_part(out, part);
		for (const auto& shardID : record.reg_layer_shards_ids)
			write_priv_key_shard_id(out, shardID);
		for (const auto& shardID : record.owner_layer_shards_ids)
			write_priv_key_shard_id(out, shardID);
	}

	utils::Buffer::iterator EncryptedPacketHandler::read_update_record(pkt::UpdateResponse::FinishedDecryptionsRecord& out, utils::Buffer::iterator it, utils::Buffer::iterator end)
	{
		// NOTE: Assuming each shards IDs vector has is exactly one more than its corresponding parts vector

		// read sizes
		member_count_t regLayerPartsCount{}, ownerLayerPartsCount{};
		it = utils::read_bytes(regLayerPartsCount, it, end);
		it = utils::read_bytes(ownerLayerPartsCount, it, end);
		it = utils::read_bytes(out.op_id, it, end);

		// read parts
		out.reg_layer_parts.resize(regLayerPartsCount);
		for (auto& part : out.reg_layer_parts)
			it = read_decryption_part(part, it, end);
		out.owner_layer_parts.resize(ownerLayerPartsCount);
		for (auto& part : out.owner_layer_parts)
			it = read_decryption_part(part, it, end);

		// read shards IDs
		out.reg_layer_shards_ids.resize(regLayerPartsCount + 1);
		for (auto& shardID : out.reg_layer_shards_ids)
			it = read_priv_key_shard_id(shardID, it, end);
		out.owner_layer_shards_ids.resize(ownerLayerPartsCount + 1);
		for (auto& shardID : out.owner_layer_shards_ids)
			it = read_priv_key_shard_id(shardID, it, end);

		return it;
	}
}
