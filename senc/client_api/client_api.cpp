/*********************************************************************
 * \file   main.cpp
 * \brief  Main file of the client API (used for DLL initialization).
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "client_api.hpp"

#include "Value.hpp"

namespace api = senc::clientapi;

void FreeHandle(std::uintptr_t handle)
{
	api::Handle* pHandle = reinterpret_cast<api::Handle*>(handle);

	if (pHandle->allocated())
		delete pHandle;
}

bool HasError(std::uintptr_t handle)
{
	api::Handle* pHandle = reinterpret_cast<api::Handle*>(handle);

	return pHandle->has_error();
}

const char* GetError(std::uintptr_t handle)
{
	api::Error* pError = reinterpret_cast<api::Error*>(handle);

	return pError->what();
}

const char* GetString(std::uintptr_t handle)
{
	api::Value<std::string>* pValue = reinterpret_cast<api::Value<std::string>*>(handle);

	return pValue->get().c_str();
}
