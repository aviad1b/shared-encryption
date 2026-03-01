/*********************************************************************
 * \file   main.cpp
 * \brief  Main file of the client API (public function implementatoin).
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

void FreeHandle(uintptr_t handle) noexcept
{
	auto* pHandle = api::Handle::from_nint(handle);
	if (pHandle && pHandle->allocated())
	{
		try { delete pHandle; }
		catch(...) { }
	}
}

bool HasError(uintptr_t handle) noexcept
{
	auto* pHandle = api::Handle::from_nint(handle);
	return pHandle->has_error();
}

const char* GetError(uintptr_t handle) noexcept
{
	auto* pError = api::Error::from_nint(handle);
	return pError->what();
}

const char* GetStringValue(uintptr_t handle) noexcept
{
	auto* pValue = api::Value<std::string>::from_nint(handle);
	return pValue->get().c_str();
}

uint64_t GetBytesLen(uintptr_t handle) noexcept
{
	auto& buff = api::Value<utils::Buffer>::from_nint(handle)->get();
	return buff.size();
}

const uint8_t* GetBytesValue(uintptr_t handle) noexcept
{
	auto& buff = api::Value<utils::Buffer>::from_nint(handle)->get();
	return buff.data();
}

uintptr_t GetBytesBase64(uintptr_t handle) noexcept
{
	auto& buff = api::Value<utils::Buffer>::from_nint(handle)->get();
	return api::Value<std::string>::ret_new([&buff]()
	{
		return utils::bytes_to_base64(buff);
	})->as_nint();
}

uintptr_t NewCiphertext(const uint8_t* c1Bytes, uint64_t c1Len,
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

			c1 = utils::ECGroup::decode({ c1Bytes, c1Len });
			c2 = utils::ECGroup::decode({ c2Bytes, c2Len });
			c3a.Assign(c3aBytes, c3aLen);
			c3b.assign(c3bBytes, c3bBytes + c3bLen);

			return res;
		}
	)->as_nint();
}

uintptr_t GetCiphertextC1(uintptr_t hCiphertext) noexcept
{
	auto& ciphertext = api::Value<senc::Ciphertext>::from_nint(hCiphertext)->get();
	return api::Value<utils::Buffer>::ret_new([&ciphertext]()
	{
		return std::get<0>(ciphertext).encode();
	})->as_nint();
}

uintptr_t GetCiphertextC2(uintptr_t hCiphertext) noexcept
{
	auto& ciphertext = api::Value<senc::Ciphertext>::from_nint(hCiphertext)->get();
	return api::Value<utils::Buffer>::ret_new([&ciphertext]()
	{
		return std::get<1>(ciphertext).encode();
	})->as_nint();
}

uintptr_t GetCiphertextC3a(uintptr_t hCiphertext) noexcept
{
	auto& ciphertext = api::Value<senc::Ciphertext>::from_nint(hCiphertext)->get();
	return api::Value<utils::Buffer>::ret_new([&ciphertext]()
	{
		auto& c3a = std::get<0>(std::get<2>(ciphertext));
		return utils::Buffer(c3a.begin(), c3a.end());
	})->as_nint();
}

uintptr_t GetCiphertextC3b(uintptr_t hCiphertext) noexcept
{
	auto& ciphertext = api::Value<senc::Ciphertext>::from_nint(hCiphertext)->get();
	return api::Value<utils::Buffer>::ret_new([&ciphertext]()
	{
		return std::get<1>(std::get<2>(ciphertext));
	})->as_nint();
}

uintptr_t Connect(const char* serverIP, uint16_t serverPort,
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
					return std::make_unique<api::Client<IP>>(
						ipInstance, serverPort,
						[]() { return senc::Schema{}; },
						senc::ClientPacketHandlerImplFactory<senc::EncryptedPacketHandler>{},
						[decryptFinishedCallback, decryptFinishedContext]
						(const senc::OperationID& opid, const utils::Buffer& plaintext)
						{
							decryptFinishedCallback(
								opid.to_string().c_str(),
								plaintext.data(),
								static_cast<std::uint64_t>(plaintext.size()),
								decryptFinishedContext
							);
						}
					);
				},
				*ip
			);
		}
	)->as_nint();
}

void Disconnect(uintptr_t hClient) noexcept
{
	auto& spClient = api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get();
	spClient.reset();
}

uintptr_t SignUp(uintptr_t hClient, const char* username, const char* password) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client, username, password]()
	{
		client.signup(username, password);
	})->as_nint();
}

uintptr_t Login(uintptr_t hClient, const char* username, const char* password) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client, username, password]()
	{
		client.login(username, password);
	})->as_nint();
}

uintptr_t Logout(uintptr_t hClient) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client]()
	{
		client.logout();
	})->as_nint();
}

uintptr_t IterUserProfile(uintptr_t hClient, bool(*callback)(uintptr_t, uintptr_t), uintptr_t context) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client, callback, context]()
	{
		client.iter_profile([callback, context](const api::storage::ProfileRecord& record)
		{
			return callback(reinterpret_cast<uintptr_t>(&record), context);
		});
	})->as_nint();
}

bool IsOwnerProfileRecord(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return rpRecord->is_owner();
}

uintptr_t GetProfileRecordRegPubKey(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::ret_new([rpRecord]()
	{
		return senc::pub_key_to_bytes(rpRecord->reg_layer_pub_key());
	})->as_nint();
}

uintptr_t GetProfileRecordOwnerPubKey(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::ret_new([rpRecord]()
	{
		return senc::pub_key_to_bytes(rpRecord->owner_layer_pub_key());
	})->as_nint();
}

uintptr_t GetProfileRecordRegShard(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::ret_new([rpRecord]()
	{
			return senc::priv_key_shard_to_bytes(rpRecord->reg_layer_priv_key_shard());
	})->as_nint();
}

uintptr_t GetProfileRecordOwnerShard(uintptr_t pRecord) noexcept
{
	auto* rpRecord = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::ret_new([rpRecord]()
	{
		if (!rpRecord->is_owner())
			throw api::ClientException("Non-owner record has no owner-layer shard");
		return senc::priv_key_shard_to_bytes(rpRecord->reg_layer_priv_key_shard());
	})->as_nint();
}

uintptr_t MakeUserSet(uintptr_t hClient, uint8_t ownersCount, uint8_t regMembersCount,
					  const char** owners, const char** regMembers,
					  uint8_t ownersThreshold, uint8_t regMembersThreshold) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Value<std::string>::ret_new(
		[&client, ownersCount, regMembersCount, owners, regMembers, ownersThreshold, regMembersThreshold]()
		{
			std::span ownersSpan(owners, ownersCount);
			std::span regMembersSpan(regMembers, regMembersCount);
			return client.make_userset(
				utils::ranges::strings(ownersSpan),
				utils::ranges::strings(regMembersSpan),
				ownersThreshold, regMembersThreshold
			).to_string();
		}
	)->as_nint();
}

uintptr_t GetUserSets(uintptr_t hClient, void(*callback)(const char*, uintptr_t), uintptr_t context) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client, callback, context]()
	{
		client.get_usersets([callback, context](const senc::UserSetID& usersetID)
		{
			callback(usersetID.to_string().c_str(), context);
		});
	})->as_nint();
}

uintptr_t GetUserSetMembers(uintptr_t hClient,
							const char* usersetID,
							void(*ownersCallback)(const char*, uintptr_t),
							void(*regsCallback)(const char*, uintptr_t),
							uintptr_t context) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client, usersetID, ownersCallback, regsCallback, context]()
	{
		client.get_userset_members(
			usersetID,
			[ownersCallback, context](const std::string& username)
			{
				ownersCallback(username.c_str(), context);
			},
			[regsCallback, context](const std::string& username)
			{
				regsCallback(username.c_str(), context);
			}
		);
	})->as_nint();
}

uintptr_t Encrypt(uintptr_t hClient, const char* usersetID,
				  const uint8_t* msg, uint64_t msgLen) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Value<senc::Ciphertext>::ret_new([&client, usersetID, msg, msgLen]()
	{
		return client.encrypt(usersetID, utils::Buffer(msg, msg + msgLen));
	})->as_nint();
}

uintptr_t Decrypt(uintptr_t hClient, const char* usersetID, uintptr_t hCiphertext) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	auto& ciphertext = api::Value<senc::Ciphertext>::from_nint(hCiphertext)->get();
	return api::Value<std::string>::ret_new([&client, usersetID, &ciphertext]()
	{
		return client.decrypt(usersetID, ciphertext).to_string();
	})->as_nint();
}

uintptr_t ForceUpdate(uintptr_t hClient) noexcept
{
	auto& client = *(api::Value<std::unique_ptr<api::IClient>>::from_nint(hClient)->get());
	return api::Error::ret_null_or_err([&client]()
	{
		client.force_update();
	})->as_nint();
}
