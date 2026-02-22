/*********************************************************************
 * \file   client_api.hpp
 * \brief  Main header of the client API.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include <cstdint>

 // define SENC_CLIENT_API_WINDOWS if on windows
#if defined(WIN32) || defined(_WIN32) || (defined(__WIN32) && !defined(__CYGWIN__))
#define SENC_CLIENT_API_WINDOWS
#endif

/**
 * @def SENC_CLIENT_API_PUBLIC
 * @brief Macro for public function linkage.
 */

#ifdef SENC_CLIENT_API_WINDOWS

#ifdef I_SENC_CLIENT_API
#define SENC_CLIENT_API_PUBLIC extern "C" __declspec(dllexport)
#else // !I_SENC_CLIENT_API
#define SENC_CLIENT_API_PUBLIC extern "C" __declspec(dllimport)
#endif // end I_SENC_CLIENT_API

#else // !SENC_CLIENT_API_WINDOWS

#define SENC_CLIENT_API_PUBLIC extern "C" __attribute__((visibility("default")))

#endif // end SENC_CLIENT_API_WINDOWS

/**
 * @brief Deallocates previously returned handle.
 * @param handle Handle to deallocate.
 */
void FreeHandle(std::uintptr_t handle);

/**
 * @brief Checks if a handle contains an error (rather than a value).
 * @param handle Handle to check if contains error.
 * @return `true` if handle has error, `false` if has value.
 */
bool HasError(std::uintptr_t handle);

/**
 * @brief Gets error message from error handle.
 * @param handle Error handle.
 * @return Error message.
 * @note Accessing returned message after handle deallocation is undefined behaviour.
 */
const char* GetError(std::uintptr_t handle);

/**
 * @brief Gets string value from a string handle.
 * @param handle String handle.
 * @return String stored in handle.
 * @note Calling this function on a non-string handle is undefined behaviour.
 */
const char* GetString(std::uintptr_t handle);
