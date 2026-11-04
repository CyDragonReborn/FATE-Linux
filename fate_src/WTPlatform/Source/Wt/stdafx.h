/***************************************************************************************
 *
 * stdafx.h
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 ***************************************************************************************/

#pragma once 

//Pre-compiled header for WTPlatform, used to reduce compile times and select
//standard pragmas and preprocessor definitions.

//Disable generally harmless errors
#pragma warning( disable : 4100 ) //'identifier' : unreferenced formal parameter
#pragma warning( disable : 4201 ) //nonstandard extension used : nameless struct/union
#pragma warning( disable : 4250 ) //'class1' : inherits 'class2::member' via dominance
#pragma warning( disable : 4511 ) //'class' : copy constructor could not be generated
#pragma warning( disable : 4702 ) //unreachable code

//Preprocessor definitions related to windows.h
//Enable strict type checking
#ifndef STRICT
#define STRICT 
#endif
//Disable non-standard min and max macros (they conflict with std::numeric_limits<>)
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <cstddef>
#include <cassert>

//STL support
#include <algorithm>
#include <limits>
#include <string>
#include <list>
#include <vector>
#include <map>

//Windows and DirectX support
#define _WIN32_WINDOWS 0x0410
#define _WIN32_WINNT 0x0500
#include <windows.h>
#ifndef USE_DIRECTX_7
#ifdef USE_DIRECTX_8
#include <d3d8.h>
#else
#include <d3d9.h>
#endif
#endif
