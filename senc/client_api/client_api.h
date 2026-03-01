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

#include <stdbool.h>
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
SENC_CLIENT_API_PUBLIC void SENC_FreeHandle(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Checks if a handle contains an error (rather than a value).
 * @param handle Handle to check if contains error.
 * @return `true` if handle has error, `false` if has value.
 */
SENC_CLIENT_API_PUBLIC bool SENC_HasError(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Gets error message from error handle.
 * @param handle Error handle.
 * @return Error message.
 * @note Accessing returned message after handle deallocation is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC const char* SENC_GetError(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Gets string value from a string handle.
 * @param handle String handle.
 * @return String stored in handle.
 * @note Calling this function on a non-string handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC const char* SENC_GetStringValue(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Gets bytes buffer length from bytes buffer handle.
 * @param handle Bytes buffer handle.
 * @return Length of bytes buffer in handle.
 * @note Calling this function on a non-bytes-buffer handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uint64_t SENC_GetBytesLen(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Gets bytes buffer bytes from bytes buffer handle.
 * @param handle Bytes buffer handle.
 * @return Pointer to byte buffer's bytes.
 * @note Calling this function on a non-bytes-buffer handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC const uint8_t* SENC_GetBytesValue(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Converts a bytes buffer to base64 string.
 * @param handle Bytes buffer handle.
 * @return String handle (of base64 string).
 * @note Calling this function on a non-bytes-buffer handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_GetBytesBase64(uintptr_t handle) SENC_NOTHROW;

/**
 * @brief Constructs a new ciphertext handle.
 * @param c1Bytes C1 bytes.
 * @param c1Len Length of C1 bytes.
 * @param c2Bytes C2 bytes.
 * @param c2Len Length of C2 bytes.
 * @param c3aBytes C3a bytes.
 * @param c3aLen Length of C3a bytes.
 * @param c3bBytes C3b bytes.
 * @param c3bLen Length of C3b bytes.
 * @return Ciphertext handle.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_NewCiphertext(const uint8_t* c1Bytes, uint64_t c1Len,
													const uint8_t* c2Bytes, uint64_t c2Len,
													const uint8_t* c3aBytes, uint64_t c3aLen,
													const uint8_t* c3bBytes, uint64_t c3bLen) SENC_NOTHROW;

/**
 * @brief Gets c1 from ciphertext handle.
 * @param hCiphertext Ciphertext handle.
 * @return C1 (bytes buffer handle).
 * @note Calling this function on a non-ciphertext handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_GetCiphertextC1(uintptr_t hCiphertext) SENC_NOTHROW;

/**
 * @brief Gets c2 from ciphertext handle.
 * @param hCiphertext Ciphertext handle.
 * @return C2 (bytes buffer handle).
 * @note Calling this function on a non-ciphertext handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_GetCiphertextC2(uintptr_t hCiphertext) SENC_NOTHROW;

/**
 * @brief Gets c3a from ciphertext handle.
 * @param hCiphertext Ciphertext handle.
 * @return C3a (bytes buffer handle).
 * @note Calling this function on a non-ciphertext handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_GetCiphertextC3a(uintptr_t hCiphertext) SENC_NOTHROW;

/**
 * @brief Gets c3b from ciphertext handle.
 * @param hCiphertext Ciphertext handle.
 * @return C3b (bytes buffer handle).
 * @note Calling this function on a non-ciphertext handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_GetCiphertextC3b(uintptr_t hCiphertext) SENC_NOTHROW;

/**
 * @brief Connects to server and returns client handle.
 * @param serverIP Server IP in string form (either IPv4 or IPv6).
 * @param serverPort Server's listen port.
 * @param decryptFinishedCallback Callback function to invoke on decrypt finish
 *								  (accepts opid chars, plaintext bytes, plaintext len, context).
 * @param decryptFinishedContext Context to pass `decryptFinishedCallback`.
 * @return New client handle.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_Connect(
	const char* serverIP, uint16_t serverPort,
	void(*decryptFinishedCallback)(const char*, const uint8_t*, uint64_t, uintptr_t),
	uintptr_t decryptFinishedContext
) SENC_NOTHROW;

/**
 * @brief Disconnects client and dellocates resources.
 * @param hClient Client handle.
 * @note After calling this function, `hClient` becomes unusable.
 */
SENC_CLIENT_API_PUBLIC void SENC_Disconnect(uintptr_t hClient) SENC_NOTHROW;

/**
 * @brief Signs up as a new username (and stays logged in).
 * @param hClient Client handle.
 * @param username Username to signup with.
 * @param password Password to signup with.
 * @return Null on success, error if failed.
 * @note Calling this function on a non-client handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_SignUp(uintptr_t hClient,
											 const char* username,
											 const char* password) SENC_NOTHROW;

/**
 * @brief Logs in to server (and stays logged in).
 * @param username Username to login with.
 * @param password Password to login with.
 * @return Null on success, error if failed.
 * @note Calling this function on a non-client handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_Login(uintptr_t hClient,
											const char* username,
											const char* password) SENC_NOTHROW;

/**
 * @brief Logs out of server.
 * @param hClient Client handle.
 * @return Null on success, error if failed.
 * @note This function does not deallocate resources - calling `Disconnect` is still required.
 * @note Calling this function on a non-client handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_Logout(uintptr_t hClient) SENC_NOTHROW;

/**
 * @brief Iterates over user profile data.
 * @param hClient Client handle.
 * @param callback Callback function accepting current profile data record pointer and context,
 *				   returning `false` to stop.
 * @param context Context to pass `callback`.
 * @return Null on success, error if failed.
 * @note Calling this function on a non-client handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_IterUserProfile(uintptr_t hClient,
													  bool(*callback)(uintptr_t, uintptr_t),
													  uintptr_t context) SENC_NOTHROW;

/**
 * @brief Checks if a profile record pointer is of an owner record.
 * @param pRecord Profile record pointer (as provided by `IterUserProfile`).
 * @return `true` if `pRecord` points to an owner record, otherwise `false` (or error if failed).
 * @note Calling this function on a non-profile-record pointer is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC bool SENC_IsOwnerProfileRecord(uintptr_t pRecord) SENC_NOTHROW;

/**
 * @brief Gets reg layer public key from profile record pointer.
 * @param pRecord Profile record pointer (as provided by `IterUserProfile`).
 * @return Handle to bytes buffer representing reg layer public key (or error if failed).
 * @note Calling this function on a non-profile-record pointer is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_GetProfileRecordRegPubKey(uintptr_t pRecord) SENC_NOTHROW;

/**
 * @brief Gets owner layer public key from profile record pointer.
 * @param pRecord Profile record pointer (as provided by `IterUserProfile`).
 * @return Handle to bytes buffer representing owner layer public key (or error if failed).
 * @note Calling this function on a non-profile-record pointer is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_GetProfileRecordOwnerPubKey(uintptr_t pRecord) SENC_NOTHROW;

/**
 * @brief Gets reg layer private key shard from profile record pointer.
 * @param pRecord Profile record pointer (as provided by `IterUserProfile`).
 * @return Handle to bytes buffer representing reg layer private key shard (or error if failed).
 * @note Calling this function on a non-profile-record pointer is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_GetProfileRecordRegShard(uintptr_t pRecord) SENC_NOTHROW;

/**
 * @brief Gets owner layer private key shard from profile record pointer.
 * @param pRecord Profile record pointer (as provided by `IterUserProfile`).
 * @return Handle to bytes buffer representing owner layer private key shard,
 *		   or error if not an owner record (or failed).
 * @note Calling this function on a non-profile-record pointer is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_GetProfileRecordOwnerShard(uintptr_t pRecord) SENC_NOTHROW;

/**
 * @brief Creates a new userset.
 * @note Requires user to be logged in.
 * @param hClient Client handle.
 * @param owners Usernames of userset's owner members.
 * @param regMembers Usernames of userset's non-owner members.
 * @param ownersThreshold Minimum amount of owners required for decryption.
 * @param regMembersThreshold Minimum amount of non-owners required for decryption.
 * @return Userset's ID (string handle).
 * @note Calling this function on a non-client handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_MakeUserSet(uintptr_t hClient,
												  uint64_t ownersCount,
												  uint64_t regMembersCount,
												  const char** owners,
												  const char** regMembers,
												  uint64_t ownersThreshold,
												  uint64_t regMembersThreshold) SENC_NOTHROW;

/**
 * @brief Gets all usersets owned by user.
 * @note Requires user to be logged in.
 * @param hClient Client handle.
 * @param callback Callback function accepting ID of each userset owned by user (string chars) and context.
 * @param context Context to pass `callback`.
 * @return Null on success, error if failed.
 * @note Calling this function on a non-client handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_GetUserSets(uintptr_t hClient,
												  void(*callback)(const char*, uintptr_t),
												  uintptr_t context) SENC_NOTHROW;

/**
 * @brief Gets all members of a specific userset.
 * @note Requires user to be logged in.
 * @param hClient Client handle.
 * @param usersetID ID of userset to get members of (string chars).
 * @param ownersCallback Callback func accepting username of each owner member (string chars) and context.
 * @param regsCallback Callback func accepting username of each non-owner member (string chars) and context.
 * @param context Context to pass `ownersCallback` and `regsCallback`.
 * @return Null on success, error if failed.
 * @note Calling this function on a non-client handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_GetUserSetMembers(uintptr_t hClient,
														const char* usersetID,
														void(*ownersCallback)(const char*, uintptr_t),
														void(*regsCallback)(const char*, uintptr_t),
														uintptr_t context) SENC_NOTHROW;

/**
 * @brief Encrypts a message under a userset.
 * @note Requires user to be logged in.
 * @param hClient Client handle.
 * @param usersetID ID of userset to encrypt under.
 * @param msg Message to encrypt.
 * @return Ciphertext handle of encrypted message.
 * @note Calling this function on a non-client handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_Encrypt(uintptr_t hClient,
											  const char* usersetID,
											  const uint8_t* msg,
											  uint64_t msgLen) SENC_NOTHROW;

/**
 * @brief Queues a message decryption under a userset.
 * @note Requires user to be logged in.
 * @param hClient Client handle.
 * @param usersetID ID of userset to decrypt under.
 * @param ciphertext Encrypted message to decrypt.
 * @return Decryption operation ID (string handle).
 * @note Calling this function on a non-client handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_Decrypt(uintptr_t hClient,
											  const char* usersetID,
											  uintptr_t hCiphertext) SENC_NOTHROW;

/**
 * @brief Forces client update.
 * @note Requires user to be logged in.
 * @param hClient Client handle.
 * @return Null on success, error if failed.
 * @note Calling this function on a non-client handle is undefined behaviour.
 */
SENC_CLIENT_API_PUBLIC uintptr_t SENC_ForceUpdate(uintptr_t hClient) SENC_NOTHROW;


#ifdef __cplusplus
#include <utility>

/**
 * @struct SENC_Handle
 * @brief Wraps `uintptr_t` handle and frees handle on descruction.
 */
struct SENC_Handle
{
	using Self = SENC_Handle;

	uintptr_t handle;

	SENC_Handle(): Self(0) { }

	SENC_Handle(uintptr_t handle) : handle(handle) { }

	SENC_Handle(SENC_Handle&& other) : handle(other.handle)
	{
		other.handle = 0;
	}

	SENC_Handle& operator=(SENC_Handle other)
	{
		this->swap(other);
		return *this;
	}

	~SENC_Handle()
	{
		SENC_FreeHandle(handle);
	}

	constexpr operator uintptr_t() const
	{
		return handle;
	}

	void swap(SENC_Handle& other)
	{
		std::swap(handle, other.handle);
	}
};

#endif // end __cplusplus
