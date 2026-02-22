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
 * @typedef handle_predicate_t
 * @brief Pointer to function which accepts a handle and returns true/false.
 */
typedef bool(*handle_predicate_t)(std::uintptr_t);

/**
 * @brief Deallocates previously returned handle.
 * @param handle Handle to deallocate.
 */
SENC_CLIENT_API_PUBLIC void FreeHandle(std::uintptr_t handle) noexcept;

/**
 * @brief Checks if a handle contains an error (rather than a value).
 * @param handle Handle to check if contains error.
 * @return `true` if handle has error, `false` if has value.
 */
SENC_CLIENT_API_PUBLIC bool HasError(std::uintptr_t handle) noexcept;

/**
 * @brief Gets error message from error handle.
 * @param handle Error handle.
 * @return Error message.
 * @note Accessing returned message after handle deallocation is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC const char* GetError(std::uintptr_t handle) noexcept;

/**
 * @brief Gets string value from a string handle.
 * @param handle String handle.
 * @return String stored in handle.
 * @note Calling this function on a non-string handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC const char* GetStringValue(std::uintptr_t handle) noexcept;

/**
 * @brief Gets bytes buffer length from bytes buffer handle.
 * @param handle Bytes buffer handle.
 * @return Length of bytes buffer in handle.
 * @note Calling this function on a non-bytes-buffer handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC std::uint64_t GetBytesLen(std::uintptr_t handle) noexcept;

/**
 * @brief Gets bytes buffer bytes from bytes buffer handle.
 * @param handle Bytes buffer handle.
 * @return Pointer to byte buffer's bytes.
 * @note Calling this function on a non-bytes-buffer handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC const std::uint8_t* GetBytesValue(std::uintptr_t handle) noexcept;

/**
 * @brief Converts a bytes buffer to base64 string.
 * @param handle Bytes buffer handle.
 * @return String handle (of base64 string).
 * @note Calling this function on a non-bytes-buffer handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC std::uintptr_t GetBytesBase64(std::uintptr_t handle) noexcept;

/**
 * @brief Locates user's profile file path based on username.
 * @param username Username.
 * @return Handle to user profile file path (or error if failed).
 */
SENC_CLIENT_API_PUBLIC std::uintptr_t LocateUserProfileFile(const char* username) noexcept;

/**
 * @brief Loads user's profile from storage.
 * @param path Profile file path.
 * @param username User's username.
 * @param password User's password.
 * @return Handle to user profile (or error if failed).
 */
SENC_CLIENT_API_PUBLIC std::uintptr_t LoadUserProfile(const char* path, const char* username, const char* password) noexcept;
