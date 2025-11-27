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


	struct RequestTag { };

	template <typename Self>
	concept Request = std::derived_from<Self, RequestTag>;

	struct ResponseTag { };

	template <typename Self>
	concept Response = std::derived_from<Self, ResponseTag>;


	// =================================================================
	// General control packets
	// =================================================================

	/**
	 * @struct ErrorResponse
	 * @brief Server error response packet.
	 */
	struct ErrorResponse : ResponseTag
	{
		bool operator==(const ErrorResponse&) const = default;

		/// Error message from server.
		std::string msg;
	};


	// =================================================================
	// Signup cycle
	// Client requests to signup with a given username.
	// Server response with signup status.
	// =================================================================

	/**
	 * @struct SignupRequest
	 * @brief Request to register a new username.
	 */
	struct SignupRequest : RequestTag
	{
		bool operator==(const SignupRequest&) const = default;

		/// Desired username.
		std::string username;
	};

	/**
	 * @struct SignupResponse
	 * @brief Result of a signup operation.
	 */
	struct SignupResponse : ResponseTag
	{
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
	// Client requests to login with a given username.
	// Server responds with login status.
	// =================================================================

	/**
	 * @struct LoginRequest
	 * @brief Request to log in with a given username.
	 */
	struct LoginRequest : RequestTag
	{
		bool operator==(const LoginRequest&) const = default;

		/// Username to log in as.
		std::string username;
	};

	/**
	 * @struct LoginResponse
	 * @brief Result of a login operation.
	 */
	struct LoginResponse : ResponseTag
	{
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
			BadUsername
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
	struct LogoutRequest : RequestTag
	{
		bool operator==(const LogoutRequest&) const = default;
	};

	/**
	 * @struct LogoutResponse
	 * @brief Acknowledgement of logout.
	 */
	struct LogoutResponse : ResponseTag
	{
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
	struct MakeUserSetRequest : RequestTag
	{
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
	struct MakeUserSetResponse : ResponseTag
	{
		bool operator==(const MakeUserSetResponse&) const = default;

		/// ID of created user set.
		UserSetID user_set_id;

		/// Public key for encryption on first layer (non-owner layer).
		PubKey pub_key1;

		/// Public key for encryption on second layer (owner layer).
		PubKey pub_key2;

		/// Private key shard for first layer (non-owner layer).
		PrivKeyShard owner_priv_key1_shard;

		/// Private key shard for second layer (non-owner layer).
		PrivKeyShard owner_priv_key2_shard;
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
	struct GetUserSetsRequest : RequestTag
	{
		bool operator==(const GetUserSetsRequest&) const = default;
	};

	/**
	 * @struct GetUserSetsResponse
	 * @brief Response listing user sets owned by requester.
	 */
	struct GetUserSetsResponse : ResponseTag
	{
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
	struct GetMembersRequest : RequestTag
	{
		bool operator==(const GetMembersRequest&) const = default;

		/// ID of the user set to get members of.
		UserSetID user_set_id;
	};

	/**
	 * @struct GetMembersResponse
	 * @brief List of members (owners and non-owners) in the requested user set.
	 */
	struct GetMembersResponse : ResponseTag
	{
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
	struct DecryptRequest : RequestTag
	{
		bool operator==(const DecryptRequest&) const = default;

		/// ID of the user set to decrypt under.
		UserSetID user_set_id;

		/// Ciphertext to decrypt.
		Ciphertext ciphertext;

		DecryptRequest() : user_set_id(), ciphertext() {}
	};

	/**
	 * @struct DecryptResponse
	 * @brief Response containing operation ID for later retrieval.
	 */
	struct DecryptResponse : ResponseTag
	{
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
	struct UpdateRequest : RequestTag
	{
		bool operator==(const UpdateRequest&) const = default;
	};

	struct UpdateRecordTag { };

	template <typename Self>
	concept UpdateRecord = std::derived_from<Self, UpdateRecordTag>;

	/**
	 * @struct UpdateResponse
	 * @brief Contains server-side updates regarding membership and decryptions.
	 */
	struct UpdateResponse : ResponseTag
	{
		/**
		 * @struct AddedAsMemberRecord
		 * @brief Record indicating user has been added as a member to a user set.
		 */
		struct AddedAsMemberRecord : UpdateRecordTag
		{
			bool operator==(const AddedAsMemberRecord&) const = default;

			/// User set ID.
			UserSetID user_set_id;

			/// Public key of the set for first layer (non-owner layer) encryption.
			PubKey pub_key1;

			/// Public key of the set for second layer (owner layer) encryption.
			PubKey pub_key2;

			/// Private key shard for first layer (non-owner layer) decryption.
			PrivKeyShard priv_key1_shard;
		};

		/// List of usersets the user was added to as non-owner.
		std::vector<AddedAsMemberRecord> added_as_reg_member;


		/**
		 * @struct AddedAsOwnerRecord
		 * @brief Record indicating user has been added as an owner to a user set.
		 */
		struct AddedAsOwnerRecord : AddedAsMemberRecord
		{
			bool operator==(const AddedAsOwnerRecord&) const = default;

			/// Private key shard for second layer (owner layer) decryption.
			PrivKeyShard priv_key2_shard;
		};

		/// List of usersets the user was added to as owner.
		std::vector<AddedAsOwnerRecord> added_as_owner;


		/// IDs of decryption operations under which server wants requester to participate.
		std::vector<OperationID> on_lookup;


		/**
		 * @struct ToDecryptRecord
		 * @brief Record for pending decryption requiring decryption parts from user.
		 */
		struct ToDecryptRecord : UpdateRecordTag
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
		struct FinishedDecryptionsRecord : UpdateRecordTag
		{
			bool operator==(const FinishedDecryptionsRecord&) const = default;

			/// Decryption operation ID.
			OperationID op_id;

			/// ID of userset under which decryption occurred.
			UserSetID user_set_id;

			/// Decryption parts for first layer (non-owner layer).
			std::vector<DecryptionPart> parts1;

			/// Decryption parts for second layer (owner layer).
			std::vector<DecryptionPart> parts2;
		};

		/// Finished decryptions requested by this client.
		std::vector<FinishedDecryptionsRecord> finished_decryptions;
	};


	// =================================================================
	// DecryptParticipate cycle
	// Client requests to participate in a decryption operation for 
	// which requester was under lookup.
	// Server responds with status ("send part" if wants client to 
	// participate, "not required" if no longer needs client for this 
	// operation).
	// =================================================================

	/**
	 * @struct DecryptParticipateRequest
	 * @brief Request to participate in a decryption operation for which requester was under lookup.
	 */
	struct DecryptParticipateRequest : RequestTag
	{
		bool operator==(const DecryptParticipateRequest&) const = default;

		/// Operation ID.
		OperationID op_id;
	};

	/**
	 * @struct DecryptParticipateResponse
	 * @brief Server response indicating if requester's participation is required.
	 */
	struct DecryptParticipateResponse : ResponseTag
	{
		bool operator==(const DecryptParticipateResponse&) const = default;

		/**
		 * @enum Status
		 * @brief Participation status code.
		 */
		enum class Status : std::uint8_t
		{
			/// User must submit a decryption part.
			SendPart,

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
	struct SendDecryptionPartRequest : RequestTag
	{
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
	struct SendDecryptionPartResponse : ResponseTag
	{
		bool operator==(const SendDecryptionPartResponse&) const = default;
	};
}
