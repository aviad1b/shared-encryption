/*********************************************************************
 * \file   env.hpp
 * \brief  Defines environment-related macros.
 * 
 * \author aviad1b
 * \date   December 2025, Teveth 5786
 *********************************************************************/

#pragma once

// define SENC_WINDOWS if on windows
#if defined(WIN32) || defined(_WIN32) || (defined(__WIN32) && !defined(__CYGWIN__))
#define SENC_WINDOWS
#endif
