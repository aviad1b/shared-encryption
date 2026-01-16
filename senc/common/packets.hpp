/*********************************************************************
 * \file   packets.hpp
 * \brief  Contains packet structs and associated packet codes.
 *
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "aliases.hpp"
#include "sizes.hpp"

namespace senc::pkt
{
	// protocol versions:
	// 1 : v1.0.0-v1.0.1
	// 2 : v1.1.0+
	constexpr std::uint8_t PROTOCOL_VERSION = 2; // v1.1.0+

	/**
	 * @enum Code
	 * @brief Packet type identifier.
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


	// =================================================================
	// General control packets
	// =================================================================

	/**
	 * @struct ErrorResponse
	 * @brief Server error response packet.
	 */
	struct ErrorResponse
	{
		static constexpr auto CODE = Code::ErrorResponse;
		bool operator==(const ErrorResponse&) const = default;

		/// Error message from server.
		std::string msg;
	};


	// =================================================================
	// Signup cycle
	// Client requests to signup with a given username and password.
	// Server response with signup status.
	// =================================================================

	/**
	 * @struct SignupRequest
	 * @brief Request to register a new username.
	 */
	struct SignupRequest
	{
		static constexpr auto CODE = Code::SignupRequest;
		bool operator==(const SignupRequest&) const = default;

		/// Desired username.
		std::string username;

		/// Password for login.
		std::string password;
	};

	/**
	 * @struct SignupResponse
	 * @brief Result of a signup operation.
	 */
	struct SignupResponse
	{
		static constexpr auto CODE = Code::SignupResponse;
		bool operator==(const SignupResponse&) const = default;

		/**
		 * @enum Status
		 * @brief Signup result code.
		 */
		enum class Status : std::uint8_t
		{
			/// Signup succeeded.
			Success,
			/// Username already taken.
			UsernameTaken
		} status; ///< Signup status.
	};


	// =================================================================
	// Login cycle
	// Client requests to login with a given username and password.
	// Server responds with login status.
	// =================================================================

	/**
	 * @struct LoginRequest
	 * @brief Request to log in with a given username.
	 */
	struct LoginRequest
	{
		static constexpr auto CODE = Code::LoginRequest;
		bool operator==(const LoginRequest&) const = default;

		/// Username to log in as.
		std::string username;

		/// Login password.
		std::string password;
	};

	/**
	 * @struct LoginResponse
	 * @brief Result of a login operation.
	 */
	struct LoginResponse
	{
		static constexpr auto CODE = Code::LoginResponse;
		bool operator==(const LoginResponse&) const = default;

		/**
		 * @enum Status
		 * @brief Login result code.
		 */
		enum class Status : std::uint8_t
		{
			/// Login succeeded.
			Success,
			/// Username does not exist.
			BadLogin
		} status; ///< Login status.
	};


	// =================================================================
	// Logout cycle
	// Client requests to logout.
	// Server responds with logout response.
	// Both client and server close the connection.
	// =================================================================

	/**
	 * @struct LogoutRequest
	 * @brief Request to log out of the system.
	 */
	struct LogoutRequest
	{
		static constexpr auto CODE = Code::LogoutRequest;
		bool operator==(const LogoutRequest&) const = default;
	};

	/**
	 * @struct LogoutResponse
	 * @brief Acknowledgement of logout.
	 */
	struct LogoutResponse
	{
		static constexpr auto CODE = Code::LogoutResponse;
		bool operator==(const LogoutResponse&) const = default;
	};


	// =================================================================
	// MakeUserSet cycle
	// Client requests to make a new user set with given parameters.
	// Server responds with userset ID, public keys and private key shards.
	// =================================================================

	/**
	 * @struct MakeUserSetRequest
	 * @brief Request to create a new user set with thresholds and members.
	 */
	struct MakeUserSetRequest
	{
		static constexpr auto CODE = Code::MakeUserSetRequest;
		bool operator==(const MakeUserSetRequest&) const = default;

		/// Usernames to include as non-owner members.
		std::vector<std::string> reg_members;

		/// Usernames to include as owners (besides requester).
		std::vector<std::string> owners;

		/// Threshold for number of non-owners required for decryption.
		member_count_t reg_members_threshold;

		/// Threshold for number of owners required for decryption.
		member_count_t owners_threshold;
	};

	/**
	 * @struct MakeUserSetResponse
	 * @brief Response containing the new user set details.
	 */
	struct MakeUserSetResponse
	{
		static constexpr auto CODE = Code::MakeUserSetResponse;
		bool operator==(const MakeUserSetResponse&) const = default;

		/// ID of created user set.
		UserSetID user_set_id;

		/// Public key for encryption on non-owner layer.
		PubKey reg_layer_pub_key;

		/// Public key for encryption on owner layer.
		PubKey owner_layer_pub_key;

		/// Private key shard for non-owner layer.
		PrivKeyShard reg_layer_priv_key_shard;

		/// Private key shard for owner layer.
		PrivKeyShard owner_layer_priv_key_shard;
	};


	// =================================================================
	// GetUserSets cycle
	// Client requests to get all usersets owned by requester.
	// Server responds with IDs of all usersets in which requester is an owner.
	// =================================================================

	/**
	 * @struct GetUserSetsRequest
	 * @brief Request to retrieve user sets owned by requester.
	 */
	struct GetUserSetsRequest
	{
		static constexpr auto CODE = Code::GetUserSetsRequest;
		bool operator==(const GetUserSetsRequest&) const = default;
	};

	/**
	 * @struct GetUserSetsResponse
	 * @brief Response listing user sets owned by requester.
	 */
	struct GetUserSetsResponse
	{
		static constexpr auto CODE = Code::GetUserSetsResponse;
		bool operator==(const GetUserSetsResponse&) const = default;

		/// IDs of user sets the requester owns.
		std::vector<UserSetID> user_sets_ids;
	};


	// =================================================================
	// GetMembers cycle
	// Client requests to get all members of a userset with a given ID.
	// Server responds with IDs of all non-owners in userset, and IDs of
	// all owners in userset.
	// =================================================================

	/**
	 * @struct GetMembersRequest
	 * @brief Request to retrieve all members of a user set.
	 */
	struct GetMembersRequest
	{
		static constexpr auto CODE = Code::GetMembersRequest;
		bool operator==(const GetMembersRequest&) const = default;

		/// ID of the user set to get members of.
		UserSetID user_set_id;
	};

	/**
	 * @struct GetMembersResponse
	 * @brief List of members (owners and non-owners) in the requested user set.
	 */
	struct GetMembersResponse
	{
		static constexpr auto CODE = Code::GetMembersResponse;
		bool operator==(const GetMembersResponse&) const = default;

		/// Non-owner member usernames.
		std::vector<std::string> reg_members;

		/// Owner usernames.
		std::vector<std::string> owners;
	};


	// =================================================================
	// Decrypt cycle
	// Client requests to decrypt a given ciphertext under a userset 
	// with a given ID.
	// Server responds with operation ID which can be used to retrieve 
	// decryption result later.
	// =================================================================

	/**
	 * @struct DecryptRequest
	 * @brief Request to decrypt a ciphertext under a specific user set.
	 */
	struct DecryptRequest
	{
		static constexpr auto CODE = Code::DecryptRequest;
		bool operator==(const DecryptRequest&) const = default;

		/// ID of the user set to decrypt under.
		UserSetID user_set_id;

		/// Ciphertext to decrypt.
		Ciphertext ciphertext;

		DecryptRequest() : user_set_id(), ciphertext() { }
		DecryptRequest(const UserSetID& userSetID, const Ciphertext& ciphertext)
			: user_set_id(userSetID), ciphertext(ciphertext) { }
		DecryptRequest(const UserSetID& userSetID, Ciphertext&& ciphertext)
			: user_set_id(userSetID), ciphertext(std::move(ciphertext)) { }
		DecryptRequest(UserSetID&& userSetID, const Ciphertext& ciphertext)
			: user_set_id(std::move(userSetID)), ciphertext(ciphertext) { }
		DecryptRequest(UserSetID&& userSetID, Ciphertext&& ciphertext)
			: user_set_id(std::move(userSetID)),
			  ciphertext(std::move(ciphertext)) { }
	};

	/**
	 * @struct DecryptResponse
	 * @brief Response containing operation ID for later retrieval.
	 */
	struct DecryptResponse
	{
		static constexpr auto CODE = Code::DecryptResponse;
		bool operator==(const DecryptResponse&) const = default;

		/// Decryption operation ID assigned by server.
		OperationID op_id;
	};


	// =================================================================
	// Update cycle
	// Client requests to run an update iteration.
	// Server responds with update information (see details in doc of 
	// `struct UpdateResponse`).
	// =================================================================

	/**
	 * @struct UpdateRequest
	 * @brief Request server to run an update iteration.
	 */
	struct UpdateRequest
	{
		static constexpr auto CODE = Code::UpdateRequest;
		bool operator==(const UpdateRequest&) const = default;
	};

	/**
	 * @struct UpdateResponse
	 * @brief Contains server-side updates regarding membership and decryptions.
	 */
	struct UpdateResponse
	{
		static constexpr auto CODE = Code::UpdateResponse;
		bool operator==(const UpdateResponse&) const = default;

		/**
		 * @struct AddedAsMemberRecord
		 * @brief Record indicating user has been added as a member to a user set.
		 */
		struct AddedAsMemberRecord
		{
			bool operator==(const AddedAsMemberRecord&) const = default;

			/// User set ID.
			UserSetID user_set_id;

			/// Public key of the set for non-owner layer encryption.
			PubKey reg_layer_pub_key;

			/// Public key of the set for owner layer encryption.
			PubKey owner_layer_pub_key;

			/// Private key shard for non-owner layer decryption.
			PrivKeyShard reg_layer_priv_key_shard;
		};

		/// List of usersets the user was added to as non-owner.
		std::vector<AddedAsMemberRecord> added_as_reg_member;


		/**
		 * @struct AddedAsOwnerRecord
		 * @brief Record indicating user has been added as an owner to a user set.
		 */
		struct AddedAsOwnerRecord
		{
			bool operator==(const AddedAsOwnerRecord&) const = default;

			/// User set ID.
			UserSetID user_set_id;

			/// Public key of the set for non-owner layer encryption.
			PubKey reg_layer_pub_key;

			/// Public key of the set for owner layer encryption.
			PubKey owner_layer_pub_key;

			/// Private key shard for non-owner layer decryption.
			PrivKeyShard reg_layer_priv_key_shard;

			/// Private key shard for owner layer decryption.
			PrivKeyShard owner_layer_priv_key_shard;
		};

		/// List of usersets the user was added to as owner.
		std::vector<AddedAsOwnerRecord> added_as_owner;


		/// IDs of decryption operations under which server wants requester to participate.
		std::vector<OperationID> on_lookup;


		/**
		 * @struct ToDecryptRecord
		 * @brief Record for pending decryption requiring decryption parts from user.
		 */
		struct ToDecryptRecord
		{
			bool operator==(const ToDecryptRecord&) const = default;

			/// ID of decryption operation to participate in.
			OperationID op_id;

			/// Ciphertext being decrypted.
			Ciphertext ciphertext;

			/// IDs of key shards used in decryption.
			std::vector<PrivKeyShardID> shards_ids;
		};

		/// Pending decryptions requiring the requester's participation.
		std::vector<ToDecryptRecord> to_decrypt;


		/**
		 * @struct FinishedDecryptionsRecord
		 * @brief Completed decryptions requested by requester.
		 */
		struct FinishedDecryptionsRecord
		{
			bool operator==(const FinishedDecryptionsRecord&) const = default;

			/// Decryption operation ID.
			OperationID op_id;

			/// Decryption parts for non-owner layer.
			std::vector<DecryptionPart> reg_layer_parts;

			/// Decryption parts for owner layer.
			std::vector<DecryptionPart> owner_layer_parts;

			// Shards IDs used in parts of non-owner layer.
			std::vector<PrivKeyShardID> reg_layer_shards_ids;

			// Shards IDs used in parts of owner layer.
			std::vector<PrivKeyShardID> owner_layer_shards_ids;
		};

		/// Finished decryptions requested by this client.
		std::vector<FinishedDecryptionsRecord> finished_decryptions;
	};


	// =================================================================
	// DecryptParticipate cycle
	// Client requests to participate in a decryption operation for 
	// which requester was under lookup.
	// Server responds with status (layer to send part of if wants client to 
	// participate, "not required" if no longer needs client for this 
	// operation).
	// =================================================================

	/**
	 * @struct DecryptParticipateRequest
	 * @brief Request to participate in a decryption operation for which requester was under lookup.
	 */
	struct DecryptParticipateRequest
	{
		static constexpr auto CODE = Code::DecryptParticipateRequest;
		bool operator==(const DecryptParticipateRequest&) const = default;

		/// Operation ID.
		OperationID op_id;
	};

	/**
	 * @struct DecryptParticipateResponse
	 * @brief Server response indicating if requester's participation is required.
	 */
	struct DecryptParticipateResponse
	{
		static constexpr auto CODE = Code::DecryptParticipateResponse;
		bool operator==(const DecryptParticipateResponse&) const = default;

		/**
		 * @enum Status
		 * @brief Participation status code.
		 */
		enum class Status : std::uint8_t
		{
			/// User must submit a non-owner layer decryption part.
			SendRegLayerPart,

			/// User must submit an owner layer decryption part.
			SendOwnerLayerPart,

			/// No longer needed.
			NotRequired
		} status; ///< Participation requirement status.
	};


	// =================================================================
	// SendDecryption cycle
	// Client sends decryption part previouslt requested by server (in 
	// an update iteration).
	// Server responds.
	// =================================================================

	/**
	 * @struct SendDecryptionPartRequest
	 * @brief Request containing a decryption contribution from the client.
	 */
	struct SendDecryptionPartRequest
	{
		static constexpr auto CODE = Code::SendDecryptionPartRequest;
		bool operator==(const SendDecryptionPartRequest&) const = default;

		/// Operation ID for which the part is submitted.
		OperationID op_id;

		/// Decryption part.
		DecryptionPart decryption_part;
	};

	/**
	 * @struct SendDecryptionPartResponse
	 * @brief Acknowledgement of submitted decryption part.
	 */
	struct SendDecryptionPartResponse
	{
		static constexpr auto CODE = Code::SendDecryptionPartResponse;
		bool operator==(const SendDecryptionPartResponse&) const = default;
	};
}
