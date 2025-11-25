/*********************************************************************
 * \file   packets.hpp
 * \brief  Contains packet structs, as well as packet codes.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include <cstdint>
#include <vector>
#include <string>

#include "aliases.hpp"

namespace senc::pkt
{
	/**
	 * @enum senc::pkt::Code
	 * @brief Packet code (deintifier).
	 */
	enum class Code : std::uint8_t
	{
		ErrorResponse,
		SignupRequest,
		SignupResponse,
		LoginRequest,
		LoginResponse,
		LogoutRequest,
		LogoutResponse,
		MakeUserSetRequest,
		MakeUserSetResponse,
		GetUserSetsRequest,
		GetUserSetsResponse,
		GetMembersRequest,
		GetMembersResponse,
		DecryptRequest,
		DecryptResponse,
		UpdateRequest,
		UpdateResponse,
		DecryptParticipateRequest,
		DecryptParticipateResponse,
		SendDecryptionPartRequest,
		SendDecryptionPartResponse
	};


	// General control packets

	struct ErrorResponse
	{
		std::string msg;
	};


	// Signup cycle packets
	// Client requests to signup with a given username.
	// Server response with signup status.

	struct SignupRequest
	{
		std::string username;
	};

	struct SignupResponse
	{
		enum class Status : std::uint8_t
		{
			Success, UsernameTaken
		} status;
	};


	// Login cycle packets
	// Client requests to login with a given username.
	// Server responds with login status.

	struct LoginRequest
	{
		std::string username;
	};

	struct LoginResponse
	{
		enum class Status : std::uint8_t
		{
			Success, BadUsername
		} status;
	};


	// Logout cycle packets
	// Client requests to logout.
	// Server responds with logout response.
	// Both client and server close the connection.

	struct LogoutRequest { };

	struct LogoutResponse { };


	// MakeUserSet cycle packets
	// Client requests to make a new user set with given parameters.
	// Server responds with userset ID, public keys and private key shards.

	struct MakeUserSetRequest
	{
		// usernames to include as non-owner members
		std::vector<std::string> reg_members;

		// usernames to include as owners (in addition to requested)
		std::vector<std::string> owners;

		// threshold for how many non-owners are required for decryption
		std::uint8_t reg_members_threshold;

		// threshold for how many owners are required for decryption
		std::uint8_t owners_threshold;
	};

	struct MakeUserSetResponse
	{
		// userset ID
		UserSetID user_set_id;

		// public keys for encryption
		PubKey pub_key1, pub_key2;

		// private key shards for threshold decryption
		PrivKeyShard owner_priv_key1_shard;
		PrivKeyShard owner_priv_key2_shard;
	};


	// GetUserSets cycle packets
	// Client requests to get all usersets owned by requester.
	// Server responds with IDs of all usersets in which requester is an owner.

	struct GetUserSetsRequest {	};

	struct GetUserSetsResponse
	{
		std::vector<UserSetID> user_sets_ids;
	};


	// GetMembers cycle packets
	// Client requests to get all members of a userset with a given ID.
	// Server responds with IDs of all non-owners in userset, and IDs of all owners in userset.

	struct GetMembersRequest
	{
		UserSetID user_set_id;
	};

	struct GetMembersResponse
	{
		std::vector<std::string> reg_members;
		std::vector<std::string> owners;
	};


	// Decrypt cycle packets
	// Client requests to decrypt a given ciphertext under a userset with a given ID.
	// Server responds with operation ID which can be used to retrieve decryption result later.

	struct DecryptRequest
	{
		UserSetID user_set_id;
		Ciphertext ciphertext;
	};

	struct DecryptResponse
	{
		OperationID op_id;
	};


	// Update cycle packets
	// Client requests to run an update iteration.
	// Server responds with update information (see details in doc of `struct UpdateResponse`).

	struct UpdateRequest { };

	struct UpdateResponse
	{
		struct AddedAsMemberRecord
		{
			UserSetID user_set_id;
			PubKey pub_key1, pub_key2;
			PrivKeyShard priv_key1_shard;
		}; // new user sets that the user was added to as non-owner
		std::vector<AddedAsMemberRecord> added_as_reg_member;

		struct AddedAsOwnerRecord : AddedAsMemberRecord
		{
			PrivKeyShard priv_key2_shard;
		}; // // new user sets that the user was added to as owner
		std::vector<AddedAsOwnerRecord> added_as_owner;

		// new pending decryption operations for the user to participate in
		std::vector<OperationID> on_lookup;

		struct ToDecryptRecord
		{
			OperationID op_id;
			Ciphertext ciphertext;
			std::vector<PrivKeyShardID> shards_ids;
		}; // pending decryptions that require part from requester
		std::vector<ToDecryptRecord> to_decrypt;

		struct FinishedDecryptionsRecord
		{
			OperationID op_id;
			UserSetID user_set_id;
			std::vector<DecryptionPart> parts1;
			std::vector<DecryptionPart> parts2;
		}; // decryptions requested by requester that have finished
		std::vector<FinishedDecryptionsRecord> finished_decryptions;
	};


	// DecryptParticipate cycle packets
	// Client requests to participate in a decryption operation for which requester was under lookup.
	// Server responds with status ("send part" if wants client to participate, "not required" if no 
	//                              longer needs client for this operation).

	struct DecryptParticipateRequest
	{
		OperationID op_id;
	};

	struct DecryptParticipateResponse
	{
		enum class Status : std::uint8_t
		{
			SendPart, NotRequired
		} status;
	};


	// SendDecryption cycle packets
	// Client sends decryption part previouslt requested by server (in an update iteration).
	// Server responds.

	struct SendDecryptionPartRequest
	{
		OperationID op_id;
		DecryptionPart decryption_part;
	};

	struct SendDecryptionPartResponse {	};
}
