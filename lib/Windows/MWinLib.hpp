//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <stdint.h>

#undef INT8_MIN
#undef INT16_MIN
#undef INT32_MIN
#undef INT64_MIN
#undef INT8_MAX
#undef UINT8_MAX
#undef INT16_MAX
#undef UINT16_MAX
#undef INT32_MAX
#undef UINT32_MAX
#undef INT64_MAX
#undef UINT64_MAX

#include <intsafe.h>

#define NOMINMAX

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <CommCtrl.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <ShellAPI.h>
#include <Uxtheme.h>
#include <vssym32.h>
#include <comdef.h>
#include <Wincodec.h>

#undef GetNextWindow
#undef PlaySound
#undef GetWindow
#undef ClipRegion
#undef CreateDialog
#undef max
#undef min

#pragma warning (disable : 4355)	// this is used in Base Initializer list
#pragma warning (disable : 4996)	// unsafe function or variable
#pragma warning (disable : 4068)	// unknown pragma
#pragma warning (disable : 4996)	// stl copy()

#pragma comment (lib, "MLib")
#pragma comment (lib, "MWinLib")

#include <ciso646>

#include "MLib.hpp"

#include "MTypes.hpp"
