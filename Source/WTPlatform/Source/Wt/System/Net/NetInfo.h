/*
	NetInfo.h

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved
 */
#pragma once

#include "WT/System/Object.h"

namespace WT
{
namespace System
{
namespace Net
{
	class CNetInfo : public CObject
	{
	public:
		CNetInfo();
		virtual ~CNetInfo();

		// Returns TRUE if the user is currently connected
		// to the internet.
		// This is a suprisingly hard function
		// to get right: the best API is the "SENS" api but it's
		// not always available; there are wininet functions but
		// they don't always return good values.  Lastly, 
		// AOL 9.0 Broadband messes us up
		//
		// The end-user can set HKLM Software\WildTangent\ConnectionStatusOverride
		// to a value
		static bool WTCall IsOnline();
	};

} // end namespace Net
} // end namespace System
} // end namespace WT
