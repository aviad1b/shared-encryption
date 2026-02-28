/*********************************************************************
 * \file   main.cpp
 * \brief  Main file of the client API (public function implementatoin).
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "client_api.h"

#include "storage/ProfileStorage.hpp"
#include "../utils/bytes.hpp"
#include "Value.hpp"
#include "client_api.h"

namespace api = senc::clientapi;
namespace utils = senc::utils;

void FreeHandle(uintptr_t handle) noexcept
{
	auto* pHandle = api::Handle::from_nint(handle);
	if (pHandle->allocated())
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
	auto* pBuff = api::Value<utils::Buffer>::from_nint(handle);
	return pBuff->get().size();
}

const uint8_t* GetBytesValue(uintptr_t handle) noexcept
{
	auto* pBuff = api::Value<utils::Buffer>::from_nint(handle);
	return pBuff->get().data();
}

uintptr_t GetBytesBase64(uintptr_t handle) noexcept
{
	auto* pBuff = api::Value<utils::Buffer>::from_nint(handle);
	return api::Value<std::string>::ret_new([pBuff]()
	{
		return utils::bytes_to_base64(pBuff->get());
	})->as_nint();
}

bool IsOwnerProfileRecord(uintptr_t pRecord) noexcept
{
	auto* record = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return record->is_owner();
}

uintptr_t GetProfileRecordRegPubKey(uintptr_t pRecord) noexcept
{
	auto* record = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::new_instance(
		senc::pub_key_to_bytes(record->reg_layer_pub_key())
	)->as_nint();
}

uintptr_t GetProfileRecordOwnerPubKey(uintptr_t pRecord) noexcept
{
	auto* record = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::new_instance(
		senc::pub_key_to_bytes(record->owner_layer_pub_key())
	)->as_nint();
}

uintptr_t GetProfileRecordRegShard(uintptr_t pRecord) noexcept
{
	auto* record = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	return api::Value<utils::Buffer>::new_instance(
		senc::priv_key_shard_to_bytes(record->reg_layer_priv_key_shard())
	)->as_nint();
}

uintptr_t GetProfileRecordOwnerShard(uintptr_t pRecord) noexcept
{
	auto* record = reinterpret_cast<api::storage::ProfileRecord*>(pRecord);
	if (!record->is_owner())
		return api::Error::new_instance("Non-owner record has no owner-layer shard")->as_nint();
	return api::Value<utils::Buffer>::new_instance(
		senc::priv_key_shard_to_bytes(record->reg_layer_priv_key_shard())
	)->as_nint();
}
