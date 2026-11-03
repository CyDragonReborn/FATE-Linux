#include "WT/stdafx.h"
#include <wininet.h>
#include <winsock.h>
#include <Sensapi.h>

#include "NetInfo.h"

typedef BOOL (WINAPI* PFN_ISNETWORKALIVE)(LPDWORD);
typedef BOOL (WINAPI* PFN_INTERNETGETCONNECTEDSTATEEX)(LPDWORD,LPTSTR, DWORD, DWORD);
typedef HINTERNET (WINAPI* PFN_INTERNETOPEN)(LPCSTR,DWORD,LPCSTR,LPCSTR,DWORD);
typedef BOOL (WINAPI *PFN_INTERNETGETCONNECTEDSTATE)(LPDWORD,DWORD);
typedef BOOL (WINAPI *PFN_INTERNETCLOSEHANDLE)(HINTERNET);

namespace WT
{
namespace System
{
namespace Net
{
	namespace EOnlineStatus
	{
		enum OnlineStatus
		{
			Connected,
			NotConnected,
			NotSupported
		};
	}



	// Cache of data used by CNetInfo.  This includes opening up libraries, 
	// getting pointers to functions, etc.
	class CNetInfoCache
	{
	public:
		CNetInfoCache()
		{
			m_hSensAPI = NULL;
			m_hWinInet = NULL;
			m_IgcsTime = 0;
			m_IgcsInfo = 0;

			m_pfnIsNetworkAlive = NULL;

			if((m_hSensAPI = ::LoadLibrary("SENSAPI.DLL")) != NULL)
			{
				m_pfnIsNetworkAlive = (PFN_ISNETWORKALIVE)::GetProcAddress(m_hSensAPI, "IsNetworkAlive");
			}

			m_pfnInternetGetConnectedStateEx = NULL;
			m_pfnInternetOpen = NULL;
			m_pfnInternetCloseHandle = NULL;
			m_pfnInternetGetConnectedState = NULL;
			if((m_hWinInet = ::LoadLibrary("WININET.DLL")) != NULL)
			{
				m_pfnInternetGetConnectedStateEx = (PFN_INTERNETGETCONNECTEDSTATEEX)GetProcAddress(m_hWinInet, "InternetGetConnectedStateEx");
				m_pfnInternetOpen = (PFN_INTERNETOPEN)GetProcAddress(m_hWinInet, "InternetOpenA");
				m_pfnInternetCloseHandle = (PFN_INTERNETCLOSEHANDLE)GetProcAddress(m_hWinInet, "InternetCloseHandle");
				m_pfnInternetGetConnectedState = (PFN_INTERNETGETCONNECTEDSTATE)GetProcAddress(m_hWinInet, "InternetGetConnectedState");
			}
		}

		~CNetInfoCache()
		{
			if (m_hSensAPI)
			{
				::FreeLibrary(m_hSensAPI);
				m_hSensAPI = NULL;
			}
			if (m_hWinInet)
			{
				::FreeLibrary(m_hWinInet);
				m_hWinInet = NULL;
			}
		}

		HMODULE m_hSensAPI;
		HMODULE m_hWinInet;

		PFN_ISNETWORKALIVE m_pfnIsNetworkAlive;

		PFN_INTERNETGETCONNECTEDSTATEEX m_pfnInternetGetConnectedStateEx;
		PFN_INTERNETOPEN m_pfnInternetOpen;
		PFN_INTERNETCLOSEHANDLE m_pfnInternetCloseHandle;
		PFN_INTERNETGETCONNECTEDSTATE m_pfnInternetGetConnectedState;

		DWORD m_IgcsTime;	// tick count of last call to InternetGetConnectedStateEx
		DWORD m_IgcsInfo;	// Info from last call

	};

	//
	// Trivial RAII wrapper for the global CInfoCache object.  The idea
	// here is that the CInfoCache should only be created if it's used,
	// but that on shutdown the global cache object has to go away.
	//
	class CRAIICInfoCache
	{
	public:
		CRAIICInfoCache(CNetInfoCache* p):m_p(p) { }
		~CRAIICInfoCache() { Free(); }
		void Free() { free (m_p); m_p = 0; }
		void Set(CNetInfoCache* p) { Free(); m_p = p; }

	private:
		CNetInfoCache* m_p;
	};
	CNetInfoCache* g_pInfoCache = 0;
	CRAIICInfoCache g_RAIICInfoCache (g_pInfoCache);

	//
	// We allow for an override via the HKLM\\Software\\WildTangent\\ConnectionStatusOverride integer
	//
	static EOnlineStatus::OnlineStatus IsConnected_Override(void)
	{
		HKEY Key=0;
		const LONG OpenStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\WildTangent", 0, KEY_READ, &Key);
		if (OpenStatus != ERROR_SUCCESS)
		{
			return EOnlineStatus::NotSupported;
		}
		DWORD Value;
		DWORD ValueSize = sizeof (Value);
		DWORD ValueType = REG_DWORD;
		const LONG QueryStatus = RegQueryValueEx (Key, "ConnectionStatusOverride", NULL, &ValueType, (LPBYTE)&Value, &ValueSize);
		if (ValueType != REG_DWORD)
		{
			return EOnlineStatus::NotSupported;
		}
		if (QueryStatus != ERROR_SUCCESS)
		{
			return EOnlineStatus::NotSupported;
		}
		if (Value < 0 || Value >= 3)
		{
			return EOnlineStatus::NotSupported;
		}
		return (EOnlineStatus::OnlineStatus)Value;
	}


	//
	// Functions that determine what our network state is.
	// The first set are low-level functions that don't attempt
	// to be clever at all.
	//
	static EOnlineStatus::OnlineStatus IsConnected_SensAPI(void)
	{
		EOnlineStatus::OnlineStatus rv = EOnlineStatus::NotSupported;

		bool SensLANIsConnected  (false);
		bool SensRASIsConnected (false);
		DWORD SensNetworkIsConnected (0);
		bool SensCalledOK (false);
		
		if(g_pInfoCache->m_pfnIsNetworkAlive)
		{
			DWORD dwFlags = 0;
			SensNetworkIsConnected = g_pInfoCache->m_pfnIsNetworkAlive(&dwFlags);
			SensLANIsConnected = (dwFlags & NETWORK_ALIVE_LAN) ? true : false;
			SensRASIsConnected = (dwFlags & NETWORK_ALIVE_WAN) ? true : false;
			SensCalledOK = true;
		}


		bool IgcsConnectViaLAN (false);
		bool IgcsConnectViaRAS (false);
		bool IgcsConnectionConfigured (false);
		bool IgcsCalledOK (false);

		if (g_pInfoCache->m_pfnInternetGetConnectedStateEx) 
		{
			// It turns out that GetConnectedStateEx is more reliable than GetConnectedState.
			// There's one machine (win98se in David Erickson's office) where the flags returned are different
			// with the two APIs.

			// It also turns out that this call can be very slow, and the values don't change often,
			// so we only call it every second.

			DWORD dwVar;
			DWORD tcNow = GetTickCount();
			if ((tcNow - g_pInfoCache->m_IgcsTime) > 1000)
			{
				g_pInfoCache->m_IgcsTime = tcNow;

				char buff [256];
				g_pInfoCache->m_pfnInternetGetConnectedStateEx(&dwVar, buff, 256, (BYTE)0);
				g_pInfoCache->m_IgcsInfo = dwVar;
			}
			else
			{
				dwVar = g_pInfoCache->m_IgcsInfo;
			}
			IgcsConnectViaLAN = (dwVar & INTERNET_CONNECTION_LAN) ? true : false;
			IgcsConnectViaRAS = (dwVar & INTERNET_CONNECTION_MODEM) ? true : false;
			IgcsConnectionConfigured = (dwVar & INTERNET_CONNECTION_CONFIGURED) ? true : false;
			IgcsCalledOK = true;
		}

		//
		// And now for the complex logic.
		// Firstly, if everything looks good, then just go with it.
		// However, there are cases where InternetGetConnectedState claims that the internet connected
		// has been configured, but that connection will happen with neither the modem or the lan.
		// This is caused, perhaps, by anomolous and broken setups.  If we detect that this has
		// happend, they we "just believe" the results of the SENS api.
		//
		if (SensCalledOK && IgcsCalledOK)
		{
			if (IgcsConnectViaLAN && SensLANIsConnected)
				rv = EOnlineStatus::Connected;
			else if (IgcsConnectViaRAS && SensRASIsConnected)
				rv = EOnlineStatus::Connected;
			else if (IgcsConnectionConfigured && !IgcsConnectViaLAN && !IgcsConnectViaRAS)
			{
				if (SensNetworkIsConnected)
					rv = EOnlineStatus::Connected;
				else
					rv = EOnlineStatus::NotConnected;

				// How does this happen?  They're configured, but not really....
				// Assume that the return value from SENS is correct.
			}
			else 
				rv = EOnlineStatus::NotConnected;
		}

		// Log reporting when available
		//char buff [256];  // Size check: string < 70 chars long; has 4 ints max size 11 char=44 char max size < 115 which is much smaller than 256.
		//wsprintf (buff, "InternetOnlineStatus: SENS called ok %d returned %d LAN %d WAN %d",
		//	SensCalledOK, SensNetworkIsConnected, SensLANIsConnected, SensRASIsConnected);
		
		//wsprintf (buff, "InternetOnlineStatus: IGCS called ok %d LAN %d RAS %d Configured %d",
		//	IgcsCalledOK, IgcsConnectViaLAN, IgcsConnectViaRAS, IgcsConnectionConfigured);

		return rv;
	}

	// Forward declaration: internal helper function
	static bool DetectLocalIPAddress(void);

	EOnlineStatus::OnlineStatus IsConnected_WinInet(void)
	{
		EOnlineStatus::OnlineStatus rv = EOnlineStatus::Connected;
		DWORD dwVar=0;
		HINTERNET hInternet = NULL;	

		if (g_pInfoCache->m_pfnInternetOpen && g_pInfoCache->m_pfnInternetCloseHandle && g_pInfoCache->m_pfnInternetGetConnectedState)
		{
			hInternet = g_pInfoCache->m_pfnInternetOpen(	"InetDetect",	// agent doesn't matter, should be no INET HTTP requests from this fn.
					INTERNET_OPEN_TYPE_PRECONFIG,	// access
					NULL,							// proxy server doesn't matter  - same reason... 
					NULL,							// defaults
					0);								// synchronousity

			// get the connected state
			if (!g_pInfoCache->m_pfnInternetGetConnectedState(&dwVar,(BYTE)0))
			{
				rv = EOnlineStatus::NotConnected;
			}

			// close the internet handle
			g_pInfoCache->m_pfnInternetCloseHandle(hInternet);

			if ((rv == EOnlineStatus::NotConnected) && (dwVar & 0x10) && !(dwVar & 0x40) && !(dwVar & INTERNET_CONNECTION_MODEM) )
			{	// We possibly want to override our current idea that we aren't connected
				// if we have some other evidence that we have a local IP address
				if ( DetectLocalIPAddress() )
				{
					rv = EOnlineStatus::Connected;
				}
			}
		}
		else
		{
			rv = EOnlineStatus::Connected; // how to tell when we're dealing with IE3.0??? 
		}

		return rv;
	}

	static bool InitLocalIPAddressDetection(void)
	{
		bool bResult = FALSE;

		// Open up Winsock stack
		WSADATA wsaData;
		if ( 0 == WSAStartup( MAKEWORD(1,1), &wsaData ) )
		{
			bResult = TRUE;
		}	
		return bResult;
	}


	static void ShutdownLocalIPAddressDetection(void)
	{
		WSACleanup();
	}


	// Function to return either yes-or-no answer on the whole connection front...
	static bool DetectLocalIPAddress(void)
	{
		bool bConnected = FALSE;
		char hostname[256];

		if ( !InitLocalIPAddressDetection() )
			return FALSE;

		// See what IP addresses are present on the local machine -
		// gethostname() returns something that gethostbyname() should
		// be able to resolve (hopefully *without* doing a DNS lookup
		// if we're disconnected...)
		if ( 0 == gethostname( hostname, 256 ) )
		{
			// Have the hostname, get all the host information we can from
			// the IP stack

			struct hostent *pHostEnt;

			pHostEnt = gethostbyname( hostname );
			if ( NULL != pHostEnt )
			{
				if ( sizeof(struct in_addr) == pHostEnt->h_length )
				{
					struct in_addr *pAddr;
					int i = 0;
					// Loop through all the addresses, printing them out
					while( (pAddr=(struct in_addr *)(pHostEnt->h_addr_list[i])) != NULL )
					{
						u_long addr = ntohl(pAddr->S_un.S_addr);

						// If we have at least one address that is not 0.0.0.0
						// or 127.0.0.1, then we assume that we're connected
						if ( (0x00000000 != addr) && (0x7F000001 != addr) )
						{
							bConnected = TRUE;
							break;
						}
						i++;  // Step on to next address
					}
				}

			}
		}

		ShutdownLocalIPAddressDetection();

		return bConnected;
	}



	//
	// All of the CNetInfo class "proper"
	//

	WT_DEFINE_CLASS(CNetInfo)

	static void InitCache()
	{
		if (!g_pInfoCache)
		{
			g_pInfoCache = new CNetInfoCache;
			g_RAIICInfoCache.Set (g_pInfoCache);
		}
	}

	CNetInfo::CNetInfo()
	{
		InitCache();
	}

	CNetInfo::~CNetInfo()
	{
	}

	bool WTCall CNetInfo::IsOnline()
	{
		InitCache();
		if (!g_pInfoCache)
		{
			g_pInfoCache = new CNetInfoCache;
			g_RAIICInfoCache.Set (g_pInfoCache);
		}

		EOnlineStatus::OnlineStatus Status = EOnlineStatus::NotSupported;
		if (Status == EOnlineStatus::NotSupported)
		{
			Status = IsConnected_Override();
		}
		if (Status == EOnlineStatus::NotSupported)
		{
			Status = IsConnected_SensAPI();
		}
		if (Status == EOnlineStatus::NotSupported)
		{
			Status = IsConnected_WinInet();
		}
		const bool Retval = (Status == EOnlineStatus::Connected);

		return Retval;
	}


} // end namespace Net
} // end namespace System
} // end namespace WT