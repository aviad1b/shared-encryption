/*********************************************************************
 * \file   main.cpp
 * \brief  Main file of the client API (used for DLL initialization).
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "client_api.h"

#include "storage/ProfileStorage.hpp"
#include "../utils/bytes.hpp"
#include "Value.hpp"

namespace api = senc::clientapi;
namespace utils = senc::utils;

void FreeHandle(std::uintptr_t handle) noexcept
{
	auto* pHandle = api::Handle::from_nint(handle);
	if (pHandle->allocated())
	{
		try { delete pHandle; }
		catch(...) { }
	}
}

bool HasError(std::uintptr_t handle) noexcept
{
	auto* pHandle = api::Handle::from_nint(handle);
	return pHandle->has_error();
}

const char* GetError(std::uintptr_t handle) noexcept
{
	auto* pError = api::Error::from_nint(handle);
	return pError->what();
}

const char* GetStringValue(std::uintptr_t handle) noexcept
{
	auto* pValue = api::Value<std::string>::from_nint(handle);
	return pValue->get().c_str();
}

std::uint64_t GetBytesLen(std::uintptr_t handle) noexcept
{
	auto* pBuff = api::Value<utils::Buffer>::from_nint(handle);
	return pBuff->get().size();
}

const std::uint8_t* GetBytesValue(std::uintptr_t handle) noexcept
{
	auto* pBuff = api::Value<utils::Buffer>::from_nint(handle);
	return pBuff->get().data();
}

std::uintptr_t GetBytesBase64(std::uintptr_t handle) noexcept
{
	auto* pBuff = api::Value<utils::Buffer>::from_nint(handle);
	return api::Value<std::string>::ret_new([pBuff]()
	{
		return utils::bytes_to_base64(pBuff->get());
	})->as_nint();
}

std::uintptr_t LocateUserProfileFile(const char* username) noexcept
{
	return api::Value<std::string>::ret_new([username]()
	{
		return std::string(username) + ".sencp";
	})->as_nint();
}

std::uintptr_t LoadUserProfile(const char* path, const char* username, const char* password) noexcept
{
	return api::Value<api::storage::ProfileStorage>::new_instance(
		path, username, password
	)->as_nint();
}
