/*********************************************************************
 * \file    client_api.hpp
 * \brief   Main header of the client API.
 * \details This API provides an abstraction of the client utilities.
 *          It provides (read-only) access to client profiles, and
 *          allows for high-level communication with the server.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include <stdint.h>

 // define SENC_CLIENT_API_WINDOWS if on windows
#if defined(WIN32) || defined(_WIN32) || (defined(__WIN32) && !defined(__CYGWIN__))
#define SENC_CLIENT_API_WINDOWS
#endif

#ifndef SENC_CLINK
#ifdef __cplusplus
#define SENC_CLINK extern "C"
#else // !__cplusplus
#define SENC_CLINK
#endif // end __cplusplus
#endif // end !CLINK

/**
 * @def SENC_NOTHROW
 * @brief Macro for function that doesn't throw.
 */

#ifndef SENC_NOTHROW
#ifdef __cplusplus
#define SENC_NOTHROW noexcept
#else // !__cplusplus
#define SENC_NOTHROW
#endif // end __cplusplus
#endif // end !CLINK

/**
 * @def SENC_CLIENT_API_PUBLIC
 * @brief Macro for public function linkage.
 */

#ifdef SENC_CLIENT_API_WINDOWS

#ifdef I_SENC_CLIENT_API
#define SENC_CLIENT_API_PUBLIC SENC_CLINK __declspec(dllexport)
#else // !I_SENC_CLIENT_API
#define SENC_CLIENT_API_PUBLIC SENC_CLINK __declspec(dllimport)
#endif // end I_SENC_CLIENT_API

#else // !SENC_CLIENT_API_WINDOWS

#define SENC_CLIENT_API_PUBLIC SENC_CLINK __attribute__((visibility("default")))

#endif // end SENC_CLIENT_API_WINDOWS

/**
 * @brief Deallocates previously returned handle.
 * @param handle Handle to deallocate.
 */
SENC_CLIENT_API_PUBLIC void FreeHandle(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Checks if a handle contains an error (rather than a value).
 * @param handle Handle to check if contains error.
 * @return `true` if handle has error, `false` if has value.
 */
SENC_CLIENT_API_PUBLIC bool HasError(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Gets error message from error handle.
 * @param handle Error handle.
 * @return Error message.
 * @note Accessing returned message after handle deallocation is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC const char* GetError(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Gets string value from a string handle.
 * @param handle String handle.
 * @return String stored in handle.
 * @note Calling this function on a non-string handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC const char* GetStringValue(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Gets bytes buffer length from bytes buffer handle.
 * @param handle Bytes buffer handle.
 * @return Length of bytes buffer in handle.
 * @note Calling this function on a non-bytes-buffer handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uint64_t GetBytesLen(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Gets bytes buffer bytes from bytes buffer handle.
 * @param handle Bytes buffer handle.
 * @return Pointer to byte buffer's bytes.
 * @note Calling this function on a non-bytes-buffer handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC const uint8_t* GetBytesValue(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Converts a bytes buffer to base64 string.
 * @param handle Bytes buffer handle.
 * @return String handle (of base64 string).
 * @note Calling this function on a non-bytes-buffer handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t GetBytesBase64(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Locates user's profile file path based on username.
 * @param username Username.
 * @return Handle to user profile file path (or error if failed).
 */
SENC_CLIENT_API_PUBLIC uintptr_t LocateUserProfileFile(const char* username) SENC_NOTHROW;

/**
 * @brief Loads user's profile from storage.
 * @param path Profile file path.
 * @param username User's username.
 * @param password User's password.
 * @return Handle to user profile (or error if failed).
 */
SENC_CLIENT_API_PUBLIC uintptr_t LoadUserProfile(const char* path, const char* username, const char* password) SENC_NOTHROW;

/**
 * @brief Iterates over user profile data.
 * @param hProfile Profile handle.
 * @param callback Callback function accepting current profile data record pointer, returning `false` to stop.
 * @return Nullopt value on success, error if failed.
 * @note Calling this function on a non-profile handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t IterUserProfile(uintptr_t hProfile, bool(*callback)(uintptr_t)) SENC_NOTHROW;

/**
 * @brief Checks if a profile record pointer is of an owner record.
 * @param pRecord Profile record pointer (as provided by `IterUserProfile`).
 * @return `true` if `pRecord` points to an owner record, otherwise `false` (or error if failed).
 * @note Calling this function on a non-profile-record pointer is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC bool IsOwnerProfileRecord(uintptr_t pRecord) SENC_NOTHROW;

/**
 * @brief Gets reg layer public key from profile record pointer.
 * @param pRecord Profile record pointer (as provided by `IterUserProfile`).
 * @return Handle to bytes buffer representing reg layer public key (or error if failed).
 * @note Calling this function on a non-profile-record pointer is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t GetProfileRecordRegPubKey(uintptr_t pRecord) SENC_NOTHROW;

/**
 * @brief Gets owner layer public key from profile record pointer.
 * @param pRecord Profile record pointer (as provided by `IterUserProfile`).
 * @return Handle to bytes buffer representing owner layer public key (or error if failed).
 * @note Calling this function on a non-profile-record pointer is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t GetProfileRecordOwnerPubKey(uintptr_t pRecord) SENC_NOTHROW;

/**
 * @brief Gets reg layer private key shard from profile record pointer.
 * @param pRecord Profile record pointer (as provided by `IterUserProfile`).
 * @return Handle to bytes buffer representing reg layer private key shard (or error if failed).
 * @note Calling this function on a non-profile-record pointer is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t GetProfileRecordRegShard(uintptr_t pRecord) SENC_NOTHROW;

/**
 * @brief Gets owner layer private key shard from profile record pointer.
 * @param pRecord Profile record pointer (as provided by `IterUserProfile`).
 * @return Handle to bytes buffer representing owner layer private key shard,
 *		   or error if not an owner record (or failed).
 * @note Calling this function on a non-profile-record pointer is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t GetProfileRecordOwnerShard(uintptr_t pRecord) SENC_NOTHROW;
