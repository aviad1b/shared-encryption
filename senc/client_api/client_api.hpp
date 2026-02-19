/*********************************************************************
 * \file   client_api.hpp
 * \brief  Main header of the client API.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

 // define SENC_CLIENT_API_WINDOWS if on windows
#if defined(WIN32) || defined(_WIN32) || (defined(__WIN32) && !defined(__CYGWIN__))
#define SENC_CLIENT_API_WINDOWS
#endif

/**
 * @def SENC_API_PRIVATE
 * @brief Macro for private function linkage.
 */

/**
 * @def SENC_API_PUBLIC
 * @brief Macro for public function linkage.
 */

#define SENC_CLIENT_API_PRIVATE

#ifdef SENC_CLIENT_API_WINDOWS

#ifdef I_SENC_CLIENT_API
#define SENC_CLIENT_API_PUBLIC extern "C" __declspec(dllexport)
#else // !I_SENC_CLIENT_API
#define SENC_CLIENT_API_PUBLIC extern "C" __declspec(dllimport)
#endif // end I_SENC_CLIENT_API

#else // !SENC_CLIENT_API_WINDOWS

#define SENC_CLIENT_API_PUBLIC extern "C" __attribute__((visibility("default")))

#endif // end SENC_CLIENT_API_WINDOWS
