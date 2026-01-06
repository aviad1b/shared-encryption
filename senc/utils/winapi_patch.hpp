#pragma once

#include "env.hpp"

#ifdef SENC_WINDOWS

#include <WinSock2.h> // has to be before <Windows.h>
#include <Windows.h>

// undef min and max to be able to use std functions with same names

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#endif
