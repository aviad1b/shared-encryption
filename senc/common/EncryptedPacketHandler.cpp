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
}
