/*
	CDRM3.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	8/24/2004

*/

#include <assert.h>
#include "../utilities/macros.h"
#include "../utilities/constants.h"
#include "../utilities/utilities.h"
#include "../utilities/textfile.h"
#include "../utilities/spawn.h"
#include "cRuntimeAttribute.h"

#include "DRM3.h"
#include "Base64.h"



CDRM3::CDRM3( const std::string& CommandLine ) :	// command line parameters
			  m_LoadedSuccessfully( kTrue )
{

#ifdef _TRIMEDIA
	
#ifdef _TRIMEDIA_VERSION_52

    AmLicenseStatus status;
    memset( &status, 0, sizeof( AmLicenseStatus ) );
    status.size = sizeof(AmLicenseStatus);
    status.flags = AM_MODE_INFORM;
    //status.expiration_uses[1] = 5;
    ActiveMARK_setLicenseRules( &status );
#endif

#elif _KEYHOLE

	// init keyhole
	KeyholeInit();

	// init ranger
	WTRErrorCode res = RangerInit();
	assert( res == ERR_SUCCESS );

#else

	// create license
	m_pLicense = new CDRM3License();
	// this GUID is specific to this game.
	m_pLicense->SetLicenseInfo( "WT", 
								"FAE5F8B1-E98F-48ca-A6DA-5516E6011963" );

#endif

	
#ifdef ONLINEVERSION
	m_OnlineVersion = true;
#else
	m_OnlineVersion = false;
#endif

	m_UseLeaderboard = true;
	m_VersionString = "error";
	m_ContentName = "game";
	m_DPName = "error";
	m_ProductID = "";
	m_ContentID = "error";
	m_DPID = "error";
	m_BuyURL = "http://games.wildtangent.com";
	m_MoreInfoURL = "";
	m_UseTracking = kTrue;
	m_LaunchWindowed = kFalse;
	m_Language = "en";
	m_UpsellErrorCount = 0;

	// now deal with command line options for the downloaded versions
	if( CommandLine.length() > 0 )
	{

		// locals
		CTextFile			File;
		queue<std::string>	Tokens;

		// tokenize the command line
		TokenizeString( &Tokens, CommandLine, ",", '#' );
		
		// the first param is always the URL - we don't need that here
		Tokens.pop();

		// get version std::string
		if( !Tokens.empty() )
		{
			m_VersionString = Tokens.front();
			Tokens.pop();
		}
		
		// get content name
		if( !Tokens.empty() )
		{
			m_ContentName = Tokens.front();
			Tokens.pop();
		}
		
		// get DP name
		if( !Tokens.empty() )
		{
			m_DPName = Tokens.front();
			Tokens.pop();
		}

		// get product ID
		if( !Tokens.empty() )
		{
			m_ProductID = Tokens.front();
			Tokens.pop();
		}
		
		// get content ID
		if( !Tokens.empty() )
		{
			m_ContentID = Tokens.front();
			Tokens.pop();
		}

		// get DP ID
		if( !Tokens.empty() )
		{
			m_DPID = Tokens.front();
			Tokens.pop();
		}

		// get 'use leaderboard' setting
		if( !Tokens.empty() )
		{
			m_UseLeaderboard = ( atoi( Tokens.front().c_str() ) == 1 ? true : false );
			Tokens.pop();
		}

		// get buy url
		if( !Tokens.empty() )
		{
			m_BuyURL = Tokens.front();
			Tokens.pop();
		}
/*
		// get more info url
		if( !Tokens.empty() )
		{
			m_MoreInfoURL = Tokens.front();
			Tokens.pop();
		}

		if( !Tokens.empty() )
		{
			Tokens.pop();
		}
		if( !Tokens.empty() )
		{
			Tokens.pop();
		}

		// get language
		if( !Tokens.empty() )
		{
			m_Language = Tokens.front();
			Tokens.pop();
		}*/

		// get tracking
		if( !Tokens.empty() )
		{
			m_UseTracking = ( atoi( Tokens.front().c_str() ) == 1 ? true : false );
			Tokens.pop();
		}

		// get launch windowed
		if( !Tokens.empty() )
		{
			m_LaunchWindowed = ( atoi( Tokens.front().c_str() ) == 1 ? true : false );
			Tokens.pop();
		}
		// Clear the queue
		ClearQueue( Tokens );
	}

	InitLicense();

} // CDRM3::CDRM3()

CDRM3::~CDRM3()
{

#ifdef _TRIMEDIA

	// doesn't do anything

#elif _KEYHOLE

	// free keyhole
	KeyholeFree();

	// free ranger
	RangerDone();

#else

	// if its locked, then do as follows...
	int Unlocked;
	GETUNLOCKED( m_pLicense, Unlocked );
	if ( Unlocked == VALUE_GAMEISLOCKED || m_OnlineVersion )
	{

#ifndef _KEYHOLE
#ifndef _TRIMEDIA_VERSION_52
#ifndef TENFOOT

#endif
#endif
#endif
	}

#endif

#ifndef _TRIMEDIA
#ifndef _TRIMEDIA_VERSION_52
#ifndef _KEYHOLE
	// delete the license
	DELETE_SAFELY( m_pLicense );
#endif
#endif
#endif
} // CDRM3::~CDRM3()


#ifdef _KEYHOLE

bool CDRM3::IsGameLocked( void )
{
	char Buffer[255] = "unknown";
	KeyholeGetInfo( KEYHOLE_INFO_MODE, Buffer, sizeof( Buffer ) );
	if( strcmp( "BUY", Buffer ) == 0 || strcmp( "RENT", Buffer ) == 0 )
	{
		return false;//VALUE_GAMEISUNLOCKED;
	}

	return true;//VALUE_GAMEISLOCKED;
}

///////////////////////////////////////////////////////////////////////////////////////
//	Do ranger related stuff.
///////////////////////////////////////////////////////////////////////////////////////
void CDRM3::ProcessRanger( WTR_Mode	Mode )	// ranger mode
{

	// locals
	HRANGER handle = NULL;

	// get a handle
	if ( Mode == WTR_SUBSCRIPTION )
	{
		char UserName[255];
		strcpy( UserName, "unknown" );
		KeyholeGetInfo( KEYHOLE_INFO_GAMEBLAST_USERNAME, UserName, sizeof( UserName ) );
		handle = RangerOpenSubscriptionA( UserName );
	}
	else
	{
		handle = RangerOpenRetail();
	}

	// setup the properties
	RangerSetProperty(handle, WTR_PROD_GUID, m_ProductID.c_str() );
	RangerSetProperty(handle, WTR_PROD_KEY, m_ContentName.c_str() );
	RangerSetProperty(handle, WTR_DP_KEY, m_DPName.c_str() );

	// send the Data
	WTRErrorCode res = RangerSendData(handle,  "http://ranger.wildtangent.com/webservices/ProcessUnlock.asmx/Unlock");

	// all done with ranger
	RangerClose( handle );
	assert( res == ERR_SUCCESS );

} // CDRM3::ProcessRanger()

#endif



///////////////////////////////////////////////////////////////////////////////////////
//	Init license.
///////////////////////////////////////////////////////////////////////////////////////
void CDRM3::InitLicense( void )
{

#ifdef _TRIMEDIA

	// setup defaults
	m_OnlineVersion = false;

#elif _KEYHOLE

	// don't cache drm Data anymore

#endif

	// get product info
	LoadProdInfo();

#ifdef _KEYHOLE
	{

		// setup defaults
		m_OnlineVersion = false;

		// get app info
		char Buffer[255];	
		strcpy( Buffer, "unknown" );
		KeyholeGetInfo( KEYHOLE_INFO_MODE, Buffer, sizeof( Buffer ) );

		// deal with a bought app...
		if ( strcmp( "BUY", Buffer ) == 0 )
		{
//			m_AppType = DRM3_APPTYPE_FULL;
			ProcessRanger( WTR_RETAIL );
		}
		// ...or a rented app...
		else if ( strcmp( "RENT", Buffer ) == 0 )
		{
//			m_AppType = DRM3_APPTYPE_FULL;
			ProcessRanger( WTR_SUBSCRIPTION );
		}
		// ...or default handling
		else
		{
//			m_AppType = DRM3_APPTYPE_DOWNLOADDEMO;
		}
	}
#endif

} // CDRM3::InitLicense()

///////////////////////////////////////////////////////////////////////////////////////
//	Unlock the game.
///////////////////////////////////////////////////////////////////////////////////////
bool CDRM3::UnlockGame( void )
{

	// locals
	bool	UnlockFailed = true;

#ifdef _TRIMEDIA

#ifdef _TRIMEDIA_VERSION_52

	// launch the purchase process
	ActiveMARK_showInterface( NULL );
#else

	// locals
	BrowserInterface	b;

	// setup the struct
	b.type         = WBC;
	b.width        = 680;
	b.height       = 570;
	b.fullscreen   = false;
	b.resizable    = false;

	// launch the purchase process
	ActiveMARKtriggerPurchaseProcess( 1, &b, NULL, NULL, NULL, 0 );
#endif

	// determine if the unlock failed
	int16 Unlocked;
	GETUNLOCKED( Unlocked );
	UnlockFailed = ( Unlocked == VALUE_GAMEISLOCKED ? true : false );

#elif _KEYHOLE

	// doesn't support unlocking in game

#else

	// attempt to unlock with the current unlock std::string
	int32 m_UnlockError = m_pLicense->Unlock( m_UnlockCode.c_str(), m_ContentID.c_str(), m_ProductID.c_str() );

	// Clear out error strings
	for ( int32 i = 0; i < 4; i++ )
	{
		m_UpsellError[i]="";
		m_UpsellErrorCount = 0;
	}

	// update the error strings based on unlock result
	switch( m_UnlockError )
	{

		case DRM3_FAIL:
		{
			m_UpsellError[0] = "Invalid unlock code.";
			m_UpsellErrorCount = 1;
			break;
		}
	
		case DRM3_FAIL_OPEN_INTERNET:
		{
			m_UpsellError[0] = "You must be connected";
			m_UpsellError[1] = "to the Internet to unlock.";
			m_UpsellError[2] = "Please connect and try";
			m_UpsellError[3] = "again.";
			m_UpsellErrorCount = 4;
			break;
		}

		case DRM3_FAIL_INTERNET_CONNECT:
		{
			m_UpsellError[0] = "Could not connect to the";
			m_UpsellError[1] = "Unlock Server. Please try";
			m_UpsellError[2] = "again later or visit";
			m_UpsellError[3] = "wildtangent.com/support.";
			m_UpsellErrorCount = 4;
			break;
		}

		case DRM3_FAIL_INTERNET_REQUEST:
		{
			m_UpsellError[0] = "Could not connect to the";
			m_UpsellError[1] = "Unlock Server. Please try";
			m_UpsellError[2] = "again later or visit";
			m_UpsellError[3] = "wildtangent.com/support.";
			m_UpsellErrorCount = 4;
			break;
		}
	
		default:
		{
			m_UpsellError[0] = "Unknown error";
			m_UpsellErrorCount = 1;
			break;
		}
	}

	// determine if the unlock failed
	int Unlocked;
	GETUNLOCKED( m_pLicense, Unlocked );
	UnlockFailed = ( Unlocked == VALUE_GAMEISLOCKED ? true : false );


#endif

	// return success or failure
	return !UnlockFailed;

} // CDRM3::UnlockGame()



///////////////////////////////////////////////////////////////////////////////////////
//	Updates session Data.
///////////////////////////////////////////////////////////////////////////////////////
void CDRM3::DecrementSessionCount( void )
{

#ifdef _TRIMEDIA

	// doesn't do anything

#elif _KEYHOLE

	// doesn't do anything

#elif ONLINEVERSION

	// doesn't do anything

#else

	// adjust session remaining count, only for download demo
	int Unlocked;
	GETUNLOCKED( m_pLicense, Unlocked );
	if ( Unlocked == VALUE_GAMEISLOCKED )
	{
		int	Remaining, Sessions, Max;
		GETSESSIONS( m_pLicense, Remaining, Sessions, Max, 15 );
		if ( Max != VALUE_DOESNOTAPPLY )
		{
			Sessions = __min( ( Sessions + 1 ), Max );
			SETDATA( m_pLicense, QUERY_SESSIONS, Sessions );
		}
	}

#endif

} // CDRM3::DecrementSessionCount()



///////////////////////////////////////////////////////////////////////////////////////
//	Load product info.
///////////////////////////////////////////////////////////////////////////////////////
void CDRM3::LoadProdInfo( void )
{

	cRuntimeAttribute Attribute;

	try
	{
		TCHAR ReturnString[256];
		std::string ScratchString;
		Attribute.GetString( "ProductDisplayName", ReturnString, 256 );
		m_ContentName = ReturnString;

		Attribute.GetString( "ProductGUID", ReturnString, 256 );
		m_ProductID = ReturnString;

		Attribute.GetString( "BuyURL", ReturnString, 256 );
		m_BuyURL = ReturnString;

		Attribute.GetString( "DPGUID", ReturnString, 256 );
		m_DPID = ReturnString;


		Attribute.GetString( "UseLeaderboard", ReturnString, 256 );
		ScratchString = ReturnString;
		ScratchString = StringUpper( ScratchString );
		if( ScratchString == "TRUE" )
		{
			m_UseLeaderboard = kTrue;
		}
		else
		{
			m_UseLeaderboard = kFalse;
		}

		Attribute.GetString( "UseTracking", ReturnString, 256 );
		ScratchString = ReturnString;
		ScratchString = StringUpper( ScratchString );
		if( ScratchString == "TRUE" )
		{
			m_UseTracking = kTrue;
		}
		else
		{
			m_UseTracking = kFalse;
		}
	}
	catch ( ... )
	{
		MessageBox( NULL,
					"Product Data Read Error",
					"Registry entry not found.",
					MB_ICONERROR | MB_OK );

		m_LoadedSuccessfully = kFalse;

	}
/*
	// locals
	CTextFile			File;
	queue<std::string>	Tokens;
	std::string			Path = AssembleAbsolutePath( GetLocalPath(),
													 "scripts/prodinfo.dat" );
	
	// parse the file
	if ( File.Open( Path.c_str() ) )
	{

		// get the version std::string
		File.TokenizeNextLine( &Tokens, ",", '#' );	
		if( !Tokens.empty() )
		{
			Tokens.pop();	
		}
		if( !Tokens.empty() )
		{
			m_VersionString = Tokens.front();
			Tokens.pop();
		}
		ClearQueue( Tokens );

		// get the content name std::string
		File.TokenizeNextLine( &Tokens, ",", '#' );	
		if( !Tokens.empty() )
		{
			Tokens.pop();	
		}
		if( !Tokens.empty() )
		{
			m_ContentName = Tokens.front();
			Tokens.pop();
		}
		ClearQueue( Tokens );

		// get the DP name std::string
		File.TokenizeNextLine( &Tokens, ",", '#' );	
		if( !Tokens.empty() )
		{
			Tokens.pop();	
		}
		if( !Tokens.empty() )
		{
			m_DPName = Tokens.front();
			Tokens.pop();
		}
		ClearQueue( Tokens) ;

		// get the product ID std::string
		File.TokenizeNextLine( &Tokens, ",", '#' );	
		if( !Tokens.empty() )
		{
			Tokens.pop();	
		}
		if( !Tokens.empty() )
		{
			m_ProductID = Tokens.front();
			Tokens.pop();
		}
		ClearQueue( Tokens );

		// get the content ID std::string
		File.TokenizeNextLine( &Tokens, ",", '#' );	
		if( !Tokens.empty() )
		{
			Tokens.pop();	
		}
		if( !Tokens.empty() )
		{
			m_ContentID = Tokens.front();
			Tokens.pop();
		}
		ClearQueue( Tokens );

		// get the DP ID std::string
		File.TokenizeNextLine( &Tokens, ",", '#' );	
		if( !Tokens.empty() )
		{
			Tokens.pop();	
		}
		if( !Tokens.empty() )
		{
			m_DPID = Tokens.front();
			Tokens.pop();
		}
		ClearQueue( Tokens );

		// get the 'use leaderboard' setting
		File.TokenizeNextLine( &Tokens, ",", '#' );	
		if( !Tokens.empty() )
		{
			Tokens.pop();	
		}
		if( !Tokens.empty() )
		{
			m_UseLeaderboard = ( atoi( Tokens.front().c_str() ) == 1 ? true : false );
			Tokens.pop();
		}
		ClearQueue( Tokens );

		// get the buy url std::string
		//File.TokenizeNextLine( &Tokens, ",", '#' );	
		if( !Tokens.empty() )
		{
			Tokens.pop();	
		}
		if( !Tokens.empty() )
		{
			m_BuyURL = Tokens.front();
			Tokens.pop();
		}
		ClearQueue( Tokens );

		if( !File.IsEOF() )
		{
			// get the more info url std::string
			File.TokenizeNextLine( &Tokens, ",", '#' );	
			if( !Tokens.empty() )
			{
				Tokens.pop();	
			}
			if( !Tokens.empty() )
			{
				m_MoreInfoURL = Tokens.front();
				Tokens.pop();
			}
			ClearQueue( Tokens );
		}


		File.TokenizeNextLine( &Tokens, ",", '#' );	
		ClearQueue( Tokens );
		File.TokenizeNextLine( &Tokens, ",", '#' );	
		ClearQueue( Tokens );
		File.TokenizeNextLine( &Tokens, ",", '#' );	
		ClearQueue( Tokens );

		if( !File.IsEOF() )
		{
			// get the language std::string
			File.TokenizeNextLine( &Tokens, ",", '#' );	
			if( !Tokens.empty() )
			{
				Tokens.pop();	
			}
			if( !Tokens.empty() )
			{
				m_Language = Tokens.front();
				Tokens.pop();
			}
			ClearQueue( Tokens );
		}

		if( !File.IsEOF() )
		{
			// get the 'use tracking' setting
			File.TokenizeNextLine( &Tokens, ",", '#' );	
			if( !Tokens.empty() )
			{
				Tokens.pop();	
			}
			if( !Tokens.empty() )
			{
				m_UseTracking = ( atoi( Tokens.front().c_str() ) == 1 ? true : false );
				Tokens.pop();
			}
			ClearQueue( Tokens );
		}

		if( !File.IsEOF() )
		{
			// get the 'launch windowed' setting
			File.TokenizeNextLine( &Tokens, ",", '#' );	
			if( !Tokens.empty() )
			{
				Tokens.pop();	
			}
			if( !Tokens.empty() )
			{
				m_LaunchWindowed = ( atoi( Tokens.front().c_str() ) == 1 ? true : false );
				Tokens.pop();
			}
			ClearQueue( Tokens );
		}

		// close the file
		File.Close();
	}
*/
} // CDRM3::LoadProdInfo()



///////////////////////////////////////////////////////////////////////////////////////
//	Spawns the buy webpage.
///////////////////////////////////////////////////////////////////////////////////////
void CDRM3::SpawnBuyPage( void )
{
	
	// spawn the buy webpage
	SpawnApp( (char *)m_BuyURL.c_str(), "", KSpawnAsync );

} // CDRM3::SpawnBuyPage()

///////////////////////////////////////////////////////////////////////////////////////
//	Spawns the upsell webpage.
///////////////////////////////////////////////////////////////////////////////////////
void CDRM3::SpawnUpsellPage( void )
{
	
	// spawn the buy webpage
	SpawnApp( (char *)AssembleAbsolutePath( GetLocalPath(), "//upsell.hta" ).c_str(),
			  "", 
			  KSpawnAsync );

} // CDRM3::SpawnUpsellPage()


///////////////////////////////////////////////////////////////////////////////////////
//	View online scores.
///////////////////////////////////////////////////////////////////////////////////////
void CDRM3::ViewScores( void )
{

	// build url
	std::string PostUrl = "http://wildboards.wildtangent.com/Redirector/Games/LeaderboardLookup/Decoder.aspx?dp=" + m_DPName + "&g=" + m_ContentName;
	
	// spawn the url
	SpawnApp( "iexplore.exe", PostUrl.c_str(), KSpawnAsync );

} // CDRM3::ViewScores()



///////////////////////////////////////////////////////////////////////////////////////
//	Post score online.
///////////////////////////////////////////////////////////////////////////////////////
void CDRM3::PostScore( int32 LevelIndex,	// index of level (0-based)
					   int32 Score )		// score to post
{

	// setup secret Key
	std::string secretKey = "fndrnt85";

	// build header std::string
	int32 random = rand() % 10000;
	std::string header = Encrypt( IntegerToString( random ), secretKey );

	std::string LeaderboardCode =  "";
	LeaderboardCode = LeaderboardCode + (char)( 65 + LevelIndex );
	// build payload std::string
	std::string Data = "HSR" + LeaderboardCode + "=";

	Data += IntegerToString( Score );

	std::string payload = Encrypt ( Data, IntegerToString( random ) );

	// build Checksum std::string
	int32 Length = payload.length() + secretKey.length();
	std::string computedChecksum = "" + IntegerToString( Hash( ( payload + secretKey ).c_str(), Length ) );

	// build the post url std::string
	std::string PostUrl = "http://wildboards.wildtangent.com/Redirector/Games/LeaderboardLookup/Decoder.aspx?dp=" + 
						m_DPName + "&u=name&g=" + m_ContentName + "&d=" + URLEncode( header.c_str(), header.length() + 10 ) + 
						"%24" + URLEncode( payload.c_str(), payload.length() + 10 ) + "%24" +
						URLEncode( computedChecksum.c_str(), computedChecksum.length() + 10 );
	
	// spawn the post page
	SpawnApp( "iexplore.exe", PostUrl.c_str(), KSpawnAsync );

} // CDRM3::PostScore()



///////////////////////////////////////////////////////////////////////////////////////
//	Encrypt a std::string.
///////////////////////////////////////////////////////////////////////////////////////
std::string CDRM3::Encrypt( const std::string&	Clear,	// std::string to Encrypt
						   const std::string&	Key )	// Key
{

	// XOR Flip all the bytes using the Key
	char* result =XORFlip( Clear.c_str(), Key.c_str(), Clear.length(), Key.length());
	return Base64::Encode( result, Clear.length() );

} // CDRM3::Encrypt()



///////////////////////////////////////////////////////////////////////////////////////
//	Twiddle a std::string, for Encryption purposes.
///////////////////////////////////////////////////////////////////////////////////////
char * CDRM3::XORFlip( const char*	ToFlip,		// std::string to mod
					  const char*	Key,		// Key
					  int32			Length1,	// Length of std::string to mod
					  int32			Length2 )	// Length of Key
{

	// build the output byte array
	char* result = new char[ Length1 + 1 ];
	
	int32 keyIdx = 0;
	

	// Cycle through each byte of the ToFlip array
	// and XOR it with a byte in the Key array.
	// Increment and wrap the keyIdx pointer.
	for ( int32 ii = 0; ii < Length1; ii++ )
	{
	    result[ii] = ( ToFlip[ii] ^ Key[keyIdx] );
	    keyIdx = ( keyIdx + 1 ) % Length2;
	}

	//
	return result;

} // CDRM3::xorFlip()



///////////////////////////////////////////////////////////////////////////////////////
//	Encode a url std::string.
///////////////////////////////////////////////////////////////////////////////////////
char* CDRM3::URLEncode( const char *Source,		// std::string to encode
					   unsigned	Max )			// Max Length
{

	// locals
	static const char *hex = "0123456789abcdef";
	unsigned len = 0;
	unsigned char ch;
	char *dest = new char[Max];
	char *ret = dest;

	//
	while( len < Max - 4 && *Source )
	{
		ch = (unsigned char)*Source;
		if( *Source == ' ' )
		{
			*(dest++) = '+';
		}
		else if( isalnum( *Source ) || strchr( "/.-", *Source ) )
		{
			*(dest++) = *Source;
		}
		else
		{
			*(dest++) = '%';
			// char in C++ can be more than 8bit
			*(dest++) = hex[(ch >> 4)&0xF];
			*(dest++) = hex[ch % 16];
		}	
		++Source;
	}
	*dest = 0;

	//
	return ret;

} // CDRM3::URLEncode()

///////////////////////////////////////////////////////////////////////////////////////
//	Get hash value.
///////////////////////////////////////////////////////////////////////////////////////
int32 CDRM3::Hash( const char*	Data,		// Data std::string
				  int32			Length )	// its Length
{	
	
	// Sanity check our Data to hash
	if ( Data == NULL )
	{
		return 0;
	}
	
	//
	int32 Checksum = 0;
	for ( int32 ii = 0; ii < Length; ii++ )
	{
		Checksum = ( Checksum << 2 ) ^ (int32)Data[ii];
	}
	
	// We don't want negative values
	return ( Checksum & 0x7FFFFFFF );

} // CDRM3::Hash()


