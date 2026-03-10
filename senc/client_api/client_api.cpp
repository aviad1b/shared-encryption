/*********************************************************************
 * \file   main.cpp
 * \brief  Main file of the client API (public function implementation).
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "client_api.h"

#include "../common/EncryptedPacketHandler.hpp"
#include "storage/ProfileStorage.hpp"
#include "../utils/bytes.hpp"
#include "Client.hpp"
#include "Value.hpp"

namespace api = senc::clientapi;
namespace utils = senc::utils;

void SENC_FreeHandle(uintptr_t handle) noexcept
{
	auto* pHandle = api::Handle::from_nint(handle);
	if (pHandle && pHandle->allocated())
	{
		try { delete pHandle; }
		catch(...) { }
	}
}

bool SENC_HasError(uintptr_t handle) noexcept
{
	auto* pHandle = api::Handle::from_nint(handle);
	return (pHandle && pHandle->has_error());
}

const char* SENC_GetError(uintptr_t handle) noexcept
{
	auto* pError = api::Error::from_nint(handle);
	return pError->what();
}

const char* SENC_GetStringValue(uintptr_t handle) noexcept
{
	auto* pValue = api::Value<std::string>::from_nint(handle);
	return pValue->get().c_str();
}

uint64_t SENC_GetBytesLen(uintptr_t handle) noexcept
{
	auto& buff = api::Value<utils::Buffer>::from_nint(handle)->get();
	return buff.size();
}

const uint8_t* SENC_GetBytesValue(uintptr_t handle) noexcept
{
	auto& buff = api::Value<utils::Buffer>::from_nint(handle)->get();
	return buff.data();
}

uintptr_t SENC_GetBytesBase64(uintptr_t handle) noexcept
{
	auto& buff = api::Value<utils::Buffer>::from_nint(handle)->get();
	return api::Value<std::string>::ret_new([&buff]()
	{
		return utils::bytes_to_base64(buff);
	})->as_nint();
}

uintptr_t SENC_NewCiphertext(const uint8_t* c1Bytes, uint64_t c1Len,
							 const uint8_t* c2Bytes, uint64_t c2Len,
							 const uint8_t* c3aBytes, uint64_t c3aLen,
							 const uint8_t* c3bBytes, uint64_t c3bLen) noexcept
{
	return api::Value<senc::Ciphertext>::ret_new(
		[c1Bytes, c1Len, c2Bytes, c2Len, c3aBytes, c3aLen, c3bBytes, c3bLen]()
		{
			senc::Ciphertext res{};
			auto& [c1, c2, c3] = res;
			auto& [c3a, c3b] = c3;

			c1 = senc::utils::from_bytes<std::tuple_element_t<0, senc::Ciphertext>>({ c1Bytes, c1Len });
			c2 = senc::utils::from_bytes<std::tuple_element_t<1, senc::Ciphertext>>({ c2Bytes, c2Len });
			c3a.Assign(c3aBytes, c3aLen);
			c3b.assign(c3bBytes, c3bBytes + c3bLen);

			return res;
		}
	)->as_nint();
}

uintptr_t SENC_GetCiphertextC1(uintptr_t hCiphertext) noexcept
{
	auto& ciphertext = api::Value<senc::Ciphertext>::from_nint(hCiphertext)->get();
	return api::Value<utils::Buffer>::ret_new([&ciphertext]()
	{
		return senc::utils::to_bytes(std::get<0>(ciphertext));
	})->as_nint();
}

uintptr_t SENC_GetCiphertextC2(uintptr_t hCiphertext) noexcept
{
	auto& ciphertext = api::Value<senc::Ciphertext>::from_nint(hCiphertext)->get();
	return api::Value<utils::Buffer>::ret_new([&ciphertext]()
	{
		return senc::utils::to_bytes(std::get<1>(ciphertext));
	})->as_nint();
}

uintptr_t SENC_GetCiphertextC3a(uintptr_t hCiphertext) noexcept
{
	auto& ciphertext = api::Value<senc::Ciphertext>::from_nint(hCiphertext)->get();
	return api::Value<utils::Buffer>::ret_new([&ciphertext]()
	{
		auto& c3a = std::get<0>(std::get<2>(ciphertext));
		return utils::Buffer(c3a.begin(), c3a.end());
	})->as_nint();
}

uintptr_t SENC_GetCiphertextC3b(uintptr_t hCiphertext) noexcept
{
	auto& ciphertext = api::Value<senc::Ciphertext>::from_nint(hCiphertext)->get();
	return api::Value<utils::Buffer>::ret_new([&ciphertext]()
	{
		return std::get<1>(std::get<2>(ciphertext));
	})->as_nint();
}

uintptr_t SENC_Connect(const char* serverIP, uint16_t serverPort,
					   void(*decryptFinishedCallback)(const char*, const uint8_t*, uint64_t, uintptr_t),
					   uintptr_t decryptFinishedContext) noexcept
{
	return api::Value<std::unique_ptr<api::IClient>>::ret_new(
		[serverIP, serverPort, decryptFinishedCallback, decryptFinishedContext]()
		{
			auto ip = utils::parse_ip(serverIP);
			if (!ip.has_value())
				throw api::ClientException("Failed to connect", "Bad IP: " + std::string(serverIP));
			return std::visit(
				[serverPort, decryptFinishedCallback, decryptFinishedContext]
				(const auto& ipInstance) -> std::unique_ptr<api::IClient>
				{
					using IP = std::remove_cvref_t<decltype(ipInstance)>;

					// if `decryptFinishedCallback` isn't null, wrap it for logic; otherwise, use empty lambda
					std::function<void(const senc::OperationID&, const utils::Buffer&)> outerCallback;
					if (decryptFinishedCallback)
						outerCallback =
							[decryptFinishedCallback, decryptFinishedContext]
							(const senc::OperationID& opid, const utils::Buffer& plaintext)
							{
								decryptFinishedCallback(
									opid.to_string().c_str(),
									plaintext.data(),
									static_cast<std::uint64_t>(plaintext.size()),
									decryptFinishedContext
								);
							};
					else
						outerCallback = [](const senc::OperationID&, const utils::Buffer&) { };

					return std::make_unique<api::Client<IP>>(
						ipInstance, serverPort,
						[]() { return senc::Schema{}; },
						senc::ClientPacketHandlerImplFactory<senc::EncryptedPacketHandler>{},
						outerCallback
					);
				},
				*ip
			);
		}
	)->as_nint();
}

void SENC_Disconnect(uintptr_t hClient) noexcept
{
	auto& spClient = api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get();
	spClient.reset();
}

uintptr_t SENC_SignUp(uintptr_t hClient, const char* profileBaseDir,
					  const char* username, const char* password) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client, profileBaseDir, username, password]()
	{
		client.signup(username, password, profileBaseDir);
	})->as_nint();
}

uintptr_t SENC_LogIn(uintptr_t hClient, const char* profileBaseDir,
					 const char* username, const char* password) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client, profileBaseDir, username, password]()
	{
		client.login(username, password, profileBaseDir);
	})->as_nint();
}

uintptr_t SENC_LogOut(uintptr_t hClient) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client]()
	{
		client.logout();
	})->as_nint();
}

uintptr_t SENC_IterUserProfile(uintptr_t hClient,
							   bool(*callback)(uintptr_t, uintptr_t),
							   uintptr_t context) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client, callback, context]()
	{
		// if `callback` isn't null, wrap it for logic; otherwise, use empty lambda
		std::function<bool(const api::storage::ProfileRecord&)> outerCallback;
		if (callback)
			outerCallback = [callback, context](const api::storage::ProfileRecord& record)
			{
				return callback(reinterpret_cast<uintptr_t>(&record), context);
			};
		else
			outerCallback = [](const api::storage::ProfileRecord&) { return true; };

		client.iter_profile(outerCallback);
	})->as_nint();
}

bool SENC_IsOwnerProfileRecord(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return rpRecord->is_owner();
}

uintptr_t SENC_GetProfileRecordRegPubKey(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::ret_new([rpRecord]()
	{
		return senc::pub_key_to_bytes(rpRecord->reg_pub_key());
	})->as_nint();
}

uintptr_t SENC_GetProfileRecordOwnerPubKey(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::ret_new([rpRecord]()
	{
		return senc::pub_key_to_bytes(rpRecord->owner_pub_key());
	})->as_nint();
}

uintptr_t SENC_GetProfileRecordRegExternalShard(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::ret_new([rpRecord]()
	{
		return senc::priv_key_shard_to_bytes(rpRecord->reg_external_priv_key_shard());
	})->as_nint();
}

uintptr_t SENC_GetProfileRecordRegInternalShard(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::ret_new([rpRecord]()
	{
		if (!rpRecord->is_owner())
			throw api::ClientException("Non-owner record has no external reg-layer shard");
		return senc::priv_key_shard_to_bytes(rpRecord->reg_internal_priv_key_shard());
	})->as_nint();
}

uintptr_t SENC_GetProfileRecordOwnerExternalShard(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::ret_new([rpRecord]()
	{
		if (!rpRecord->is_owner())
			throw api::ClientException("Non-owner record has no owner-layer shards");
		return senc::priv_key_shard_to_bytes(rpRecord->owner_external_priv_key_shard());
	})->as_nint();
}

uintptr_t SENC_GetProfileRecordOwnerInternalShard(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::ret_new([rpRecord]()
	{
		if (!rpRecord->is_owner())
			throw api::ClientException("Non-owner record has no owner-layer shards");
		return senc::priv_key_shard_to_bytes(rpRecord->owner_internal_priv_key_shard());
	})->as_nint();
}

uintptr_t SENC_MakeUserSet(uintptr_t hClient, uint64_t ownersCount, uint64_t regMembersCount,
						   const char** owners, const char** regMembers,
						   uint64_t ownersThreshold, uint64_t regMembersThreshold,
						   const char* name) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Value<std::string>::ret_new(
		[&client, ownersCount, regMembersCount, owners, regMembers,
		 ownersThreshold, regMembersThreshold, name]()
		{
			if (ownersCount + regMembersCount > senc::MAX_MEMBERS)
				throw api::ClientException(
					"Failed to create userset",
					"Invalid members count, maximum is " + std::to_string(senc::MAX_MEMBERS)
				);
			if (ownersThreshold > senc::MAX_MEMBERS)
				throw api::ClientException(
					"Failed to create userset",
					"Invalid owners threshold, maximum is " + std::to_string(senc::MAX_MEMBERS)
				);
			if (regMembersThreshold > senc::MAX_MEMBERS)
				throw api::ClientException(
					"Failed to create userset",
					"Invalid non-owners threshold, maximum is " + std::to_string(senc::MAX_MEMBERS)
				);
			std::span ownersSpan(owners, ownersCount);
			std::span regMembersSpan(regMembers, regMembersCount);
			return client.make_userset(
				utils::ranges::strings(ownersSpan),
				utils::ranges::strings(regMembersSpan),
				static_cast<senc::member_count_t>(ownersThreshold),
				static_cast<senc::member_count_t>(regMembersThreshold),
				name
			).to_string();
		}
	)->as_nint();
}

uintptr_t SENC_GetUserSets(uintptr_t hClient,
						   void(*callback)(const char*, const char*, uintptr_t),
						   uintptr_t context) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client, callback, context]()
	{
		// if `callback` isn't null, wrap it for logic; otherwise, use empty lambda
		std::function<void(const senc::UserSetID&, const std::string&)> outerCallback;
		if (callback)
			outerCallback = [callback, context](const senc::UserSetID& id, const std::string& name)
			{
				callback(id.to_string().c_str(), name.c_str(), context);
			};
		else
			outerCallback = [](const senc::UserSetID&, const std::string&) { };

		client.get_usersets(outerCallback);
	})->as_nint();
}

uintptr_t SENC_GetUserSetMembers(uintptr_t hClient,
								 const char* usersetID,
								 void(*ownersCallback)(const char*, uintptr_t),
								 void(*regsCallback)(const char*, uintptr_t),
								 uintptr_t context) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client, usersetID, ownersCallback, regsCallback, context]()
	{
		// for each callback, if isn't null, wrap it for logic; otherwise, use empty lambda
		std::function<void(const std::string&)> outerOwnersCallback, outerRegsCallback;

		if (ownersCallback)
			outerOwnersCallback = [ownersCallback, context](const std::string& username)
			{
				ownersCallback(username.c_str(), context);
			};
		else
			outerOwnersCallback = [](const std::string&) { };

		if (regsCallback)
			outerRegsCallback = [regsCallback, context](const std::string& username)
			{
				regsCallback(username.c_str(), context);
			};
		else
			outerRegsCallback = [](const std::string&) { };

		client.get_userset_members(usersetID, outerOwnersCallback, outerRegsCallback);
	})->as_nint();
}

uintptr_t SENC_Encrypt(uintptr_t hClient, const char* usersetID,
					   const uint8_t* msg, uint64_t msgLen) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Value<senc::Ciphertext>::ret_new([&client, usersetID, msg, msgLen]()
	{
		return client.encrypt(usersetID, utils::Buffer(msg, msg + msgLen));
	})->as_nint();
}

uintptr_t SENC_Decrypt(uintptr_t hClient, const char* usersetID, uintptr_t hCiphertext) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	auto& ciphertext = api::Value<senc::Ciphertext>::from_nint(hCiphertext)->get();
	return api::Value<std::string>::ret_new([&client, usersetID, &ciphertext]()
	{
		return client.decrypt(usersetID, ciphertext).to_string();
	})->as_nint();
}

uintptr_t SENC_ForceUpdate(uintptr_t hClient) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client]()
	{
		client.force_update();
	})->as_nint();
}

uintptr_t SENC_UserSearch(uintptr_t hClient,
						  const char* query,
						  void(*callback)(const char*, uintptr_t),
						  uintptr_t context) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client, query, callback, context]
	{
		std::function<void(const std::string&)> outerCallback;
		if (callback)
			outerCallback = [callback, context](const std::string& username)
			{
				callback(username.c_str(), context);
			};
		else
			outerCallback = [](const std::string&) { };
		return client.user_search(query, outerCallback);
	})->as_nint();
}
