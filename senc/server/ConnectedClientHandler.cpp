/*********************************************************************
 * \file   ConnectedClientHandler.cpp
 * \brief  Implementation of ConnectedClientHandler class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "ConnectedClientHandler.hpp"

namespace senc::server
{
	ConnectedClientHandler::ConnectedClientHandler(utils::Socket& sock,
												   const std::string& username,
												   Schema& schema,
												   IServerStorage& storage,
												   PacketReceiver& receiver,
												   PacketSender& sender,
												   UpdateManager& updateManager,
												   DecryptionsManager& decryptionsManager)
		: _distShardID(utils::Random<PrivKeyShardID>::get_dist()),
		  _sock(sock), _username(username),
		  _schema(schema), _storage(storage),
		  _receiver(receiver), _sender(sender),
		  _updateManager(updateManager), _decryptionsManager(decryptionsManager) { }

	void ConnectedClientHandler::loop()
	{
		Status status = Status::Connected;
		while (Status::Connected == status)
		{
			status = iteration();
		}
	}

	pkt::MakeUserSetResponse ConnectedClientHandler::make_userset(
		const std::string& creator,
		const std::vector<std::string>& owners,
		const std::vector<std::string>& regMembers,
		member_count_t ownersThreshold,
		member_count_t regMembersThreshold)
	{
		pkt::MakeUserSetResponse res{};

		utils::HashSet<std::string> setOwners(owners.begin(), owners.end());
		utils::HashSet<std::string> setRegMembers(regMembers.begin(), regMembers.end());

		res.user_set_id = _storage.new_userset(
			setOwners, setRegMembers, ownersThreshold, regMembersThreshold
		);

		// generate keys, and shards for each member
		PrivKey privKey1{}, privKey2{};
		std::tie(res.pub_key1, privKey1) = _schema.keygen();
		std::tie(res.pub_key2, privKey2) = _schema.keygen();

		auto poly1 = Shamir::sample_poly(privKey1, regMembersThreshold);
		auto poly2 = Shamir::sample_poly(privKey2, ownersThreshold);

		// generate unique shard IDs
		utils::HashSet<PrivKeyShardID> regMembersShardsIDs;
		utils::HashSet<PrivKeyShardID> ownersShardsIDs;
		PrivKeyShardID creatorShardID{};
		auto idExists = [creatorShardID, regMembersShardsIDs, ownersShardsIDs](const auto& x)
		{
			return creatorShardID == x ||
				regMembersShardsIDs.contains(x) ||
				ownersShardsIDs.contains(x);
		};
		creatorShardID = _distShardID(idExists);
		for (member_count_t i = 0; i < regMembers.size(); ++i)
			regMembersShardsIDs.insert(_distShardID(idExists));
		for (member_count_t i = 0; i < owners.size(); ++i)
			ownersShardsIDs.insert(_distShardID(idExists));

		// make private key shards for all members
		res.priv_key1_shard = Shamir::make_shard(poly1, creatorShardID);
		res.priv_key2_shard = Shamir::make_shard(poly2, creatorShardID);
		auto ownersShards1 = Shamir::make_shards(poly1, ownersShardsIDs);
		auto ownersShards2 = Shamir::make_shards(poly2, ownersShardsIDs);
		auto regMembersShards = Shamir::make_shards(poly1, regMembersShardsIDs);

		// for all non-creator members, register update for userset
		// TODO: Should shards be moved here?
		for (const auto& [owner, shard1, shard2] : utils::views::zip(setOwners, ownersShards1, ownersShards2))
			_updateManager.register_owner(
				owner, res.user_set_id,
				res.pub_key1, res.pub_key2,
				shard1, shard2
			);
		for (const auto& [regMember, shard] : utils::views::zip(setRegMembers, regMembersShards))
			_updateManager.register_reg_member(
				regMember, res.user_set_id,
				res.pub_key1, res.pub_key2,
				shard
			);

		return res;
	}

	OperationID ConnectedClientHandler::initiate_decryption(const UserSetID& usersetID, Ciphertext&& ciphertext)
	{
		auto info = _storage.get_userset_info(usersetID);		

		// register oepration in decryptions manager
		auto opid = _decryptionsManager.register_new_operation(
			_username, usersetID,
			std::move(ciphertext),
			info.owners_threshold,
			info.reg_members_threshold
		);

		// inform all relevant members of lookup
		for (const auto& member : utils::views::join(info.owners, info.reg_members))
			_updateManager.register_lookup(member, opid);

		return opid;
	}

	void ConnectedClientHandler::continue_operation(const OperationID& opid,
													const DecryptionsManager::PrepareRecord& opPrepRecord)
	{
		auto members = utils::views::join(opPrepRecord.owners_found, opPrepRecord.reg_members_found);

		// get shards IDs of all members
		std::vector<PrivKeyShardID> shardsIDs;
		for (const auto& member : members)
			shardsIDs.push_back(_storage.get_shard_id(member, opPrepRecord.userset_id));

		// for each member, make an update of ciphertext to decrypt
		for (const auto& member : members)
			_updateManager.register_decryption_participated(
				member, opid,
				opPrepRecord.ciphertext,
				shardsIDs
			);
	}

	ConnectedClientHandler::Status ConnectedClientHandler::iteration()
	{
		auto req = _receiver.recv_request<
			pkt::LogoutRequest,
			pkt::MakeUserSetRequest,
			pkt::GetUserSetsRequest,
			pkt::GetMembersRequest,
			pkt::DecryptRequest,
			pkt::UpdateRequest,
			pkt::DecryptParticipateRequest,
			pkt::SendDecryptionPartRequest
		>(_sock);

		if (req.has_value())
			return std::visit(
				[this](auto& r) { return handle_request(r); },
				*req
			);

		// if reached here, bad request
		_sender.send_response(_sock, pkt::ErrorResponse{ "Bad request" });
		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::LogoutRequest& request)
	{
		(void)request;
		_sender.send_response(_sock, pkt::LogoutResponse{});
		return Status::Disconnected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::MakeUserSetRequest& request)
	{
		pkt::MakeUserSetResponse response{};
		try
		{
			response = make_userset(
				_username,
				request.owners, request.reg_members,
				request.owners_threshold, request.reg_members_threshold
			);
		}
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{
				std::string("Failed to create userset: ") + e.what()
			});
			return Status::Connected;
		}

		_sender.send_response(_sock, response);
		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::GetUserSetsRequest& request)
	{
		(void)request;

		utils::HashSet<UserSetID> usersets;
		try { usersets = _storage.get_usersets(_username); }
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{
				std::string("Failed to fetch usersets: ") + e.what()
			});
			return Status::Connected;
		}

		pkt::GetUserSetsResponse response{};
		response.user_sets_ids.insert(
			response.user_sets_ids.end(),
			usersets.begin(), usersets.end()
		);
		// TODO: Sort?

		_sender.send_response(_sock, response);

		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::GetMembersRequest& request)
	{
		UserSetInfo info{};
		try { info = _storage.get_userset_info(request.user_set_id); }
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{
				std::string("Failed to fetch userset members: ") + e.what()
			});
			return Status::Connected;
		}

		pkt::GetMembersResponse response{};
		response.owners.insert(
			response.owners.end(),
			info.owners.begin(), info.owners.end()
		);
		response.reg_members.insert(
			response.reg_members.end(),
			info.reg_members.begin(),
			info.reg_members.end()
		);
		// TODO: Sort?

		_sender.send_response(_sock, response);

		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::DecryptRequest& request)
	{
		OperationID opid{};

		try { opid = initiate_decryption(request.user_set_id, std::move(request.ciphertext)); }
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{
				std::string("Failed to initiate decryption operation: ") + e.what()
			});
			return Status::Connected;
		}

		_sender.send_response(_sock, pkt::DecryptResponse{ opid });
		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::UpdateRequest& request)
	{
		pkt::UpdateResponse response{};

		try { response = _updateManager.retrieve_updates(_username); }
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{
				std::string("Failed to fetch updates: ") + e.what()
			});
			return Status::Connected;
		}

		_sender.send_response(_sock, response);
		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::DecryptParticipateRequest& request)
	{
		std::optional<DecryptionsManager::PrepareRecord> opPrepRecord;

		try
		{
			// TODO: Requirement check should be done here, 
			// currently not supported by manager for simplicity
			// (see pkt::DecryptParticipateResponse::Status)
			opPrepRecord = _decryptionsManager.register_participant(
				request.op_id,
				_username,
				_storage.get_usersets(_username).contains(
					_decryptionsManager.get_operation_userset(request.op_id)
				) // TODO: storage should probably have a separate method for this purpose...
			);
		}
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{
				std::string("Failed to fetch operation: ") + e.what()
			});
			return Status::Connected;
		}

		// if decryptions manager returned a prep record, continue to collection stage
		if (opPrepRecord.has_value())
		{
			try { continue_operation(request.op_id, *opPrepRecord); }
			catch (const ServerException& e) { /* TODO: Should probably inform operation initiator? */ }
		}

		// finally, send ack (ask client to send decryption part next (in fitting update))
		_sender.send_response(_sock, pkt::DecryptParticipateResponse{
			pkt::DecryptParticipateResponse::Status::SendPart
		});

		return Status::Connected;
	}

	ConnectedClientHandler::Status ConnectedClientHandler::handle_request(pkt::SendDecryptionPartRequest& request)
	{
		std::optional<DecryptionsManager::CollectedRecord> opCollRecord;

		try
		{
			auto userset = _decryptionsManager.get_operation_userset(request.op_id);
			auto shardID = _storage.get_shard_id(_username, userset);

			opCollRecord = _decryptionsManager.register_part(
				request.op_id,
				std::move(request.decryption_part),
				std::move(shardID),
				_storage.get_usersets(_username).contains(
					userset
				) // TODO: once again, storage should probably have a separate method for this purpose...
			);
		}
		catch (const ServerException& e)
		{
			_sender.send_response(_sock, pkt::ErrorResponse{
				std::string("Failed to fetch operation: ") + e.what()
			});
			return Status::Connected;
		}

		// if decryptions manager returned collection record, finalize operation
		if (opCollRecord.has_value())
		{
			try { finish_operation(*opCollRecord); }
			catch (const ServerException& e) { /* TODO: Should probably inform operation initiator? */ }
		}

		// finally, send ack
		_sender.send_response(_sock, pkt::SendDecryptionPartResponse{});

		return Status::Connected;
	}
}
