/*
	cdamanager.cpp

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	8/20/2004
	
*/

#ifdef ONLINEVERSION

#include <math.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"

#include "../UTILITIES/datafile.h"
#include "../UTILITIES/llist.h"

#include "cdamanager.h"

typedef void   ( __cdecl *cdaProcessFP ) ( long Millis );

typedef cdaDownloadRequest ( __cdecl *cdaInitDownloadFP ) 
						   ( const char* URL,				// url to retrieve
							 cdaECacheUsage CacheUsage,		// type of cache usage
							 BOOL ResumeCache,				// allow resumption of download?
							 long Secure,					// secure file?
							 cdaDownloadCallback Callback,	// file download callback
							 void* CallbackData );			// void point32er to callback data
 
typedef cdaDownloadRequest ( __cdecl *cdaInitDownloadToFileFP )
						   ( const char* URL,				// url to retrieve
						     const char* FileName,			// filename to save to
							 cdaECacheUsage CacheUsage,		// type of cache usage
							 BOOL ResumeCache,				// allow resumption of download?
							 long Secure,					// secure file?
							 cdaDownloadCallback Callback,	// file download callback
							 void* CallbackData );			// void point32er to callback data
 
typedef void   ( __cdecl *cdaEngineInitFP ) ( void );

typedef void   ( __cdecl *cdaEngineFreeFP ) ( void );

typedef void   ( __cdecl *cdaCancelDownloadFP ) 
			   ( cdaDownloadRequest Request );	// download DownloadRequestuest

typedef unsigned long	( __cdecl *cdaGetBytesDownloadedFP ) 
						( cdaDownloadRequest Request );			// download DownloadRequestuest

typedef cdaCabHandle	( __cdecl *cdaExtractCabAsyncFP )  
						( const char* FileName,					// cab file		
						  const char* DestinationDirectory );	// directory to extract to

typedef void	( __cdecl *cdaPauseDownloadFP ) 
				( cdaDownloadRequest Request );			// download DownloadRequestuest

typedef void	( __cdecl *cdaResumeDownloadFP ) 
				( cdaDownloadRequest Request );			// download DownloadRequestuest

typedef void	( __cdecl *cdaReleaseDownloadRequestFP ) 
				( cdaDownloadRequest Request );			// download DownloadRequestuest

typedef bool	( __cdecl *cdaIsCabFileFP )  
				( const char* FileName );	// file to check

typedef bool	( __cdecl *cdaIsCabExtractDoneFP )  
				( cdaCabHandle Handle );	// handle to cab to check

typedef bool	( __cdecl *cdaGetCachedFilenameFP )  
				( const char* URL,					// url to check for cached file of
				  char* Buffer,						// buffer to fill with filename
				  const unsigned long BufferSize ); // length of buffer

typedef void	( __cdecl *cdaReleaseCabHandleFP ) 
				( cdaCabHandle Handle );			// handle to release

typedef void	( __cdecl *cdaSetResumeCacheFP ) 
				( const char* LocalPath );			// local path to resume cache file

// set up function pointers
struct CDAFunctions
{
 cdaProcessFP					Process;
 cdaInitDownloadFP				InitDownload;
 cdaInitDownloadToFileFP		InitDownloadToFile;
 cdaCancelDownloadFP			CancelDownload;
 cdaGetBytesDownloadedFP		GetBytesDownloaded;
 cdaExtractCabAsyncFP			ExtractCabAsync;
 cdaPauseDownloadFP				PauseDownload;
 cdaResumeDownloadFP			ResumeDownload;
 cdaReleaseDownloadRequestFP	ReleaseDownload;
 cdaIsCabFileFP					IsCabFile;
 cdaGetCachedFilenameFP			GetCachedFilename;
 cdaIsCabExtractDoneFP			IsCabExtractDone;
 cdaReleaseCabHandleFP			ReleaseCabHandle;
 cdaEngineInitFP				EngineInit;
 cdaEngineFreeFP				EngineFree;
 cdaSetResumeCacheFP			SetResumeCache;
} m_CDAFunctions;

CCDAManager::CCDAManager( const std::string& FileManifest,		// manifest containing file descriptions
						  const std::string& RootURL,			// root that download paths are relative to
						  const std::string& ExtractionPath,	// root path to extract cabs to
						  const std::string& CacheFolder ) :	// path where files will be cached in the CDA directory
																m_ExtractionPath( ExtractionPath ),
																m_RootURL( RootURL ),
																m_CDAModule( NULL ),
																m_DownloadFailed( kFalse ),
																m_EstimateTimer( 0.0f ),
																m_EstimatedTime( 0.0f ),
																m_LastEstimatedTime( 0.0f ),
																m_ActiveDownloads( 0 ),
																m_RequestedKB( 0 ),
																m_DownloadedKB( 0 ),
																m_LastDownloadedKB( 0 ),
																m_DownloadingName( "" ),
																m_EstimatedTimeString( "" ),
																m_FailedFile( "" )
{
	// create a templatized linked list for all active downloads
	m_pDownloadList = new CLList< CActiveDownload >();

	// create a templatized linked list for all descriptions
	m_pDescriptionList = new CLList< DownloadDescription >();

	std::string	CDAVersionTag( "0404" );
	HKEY hCDAKey( NULL );
	char CDAPath[MAX_PATH+1];

	CDAPath[MAX_PATH] = '\0';
	
	// retrieve the registry key for CDA, if any
	LRESULT Result = ::RegOpenKeyEx( HKEY_LOCAL_MACHINE,
									 "Software\\WildTangent\\CDA\\", 
									 0, 
									 KEY_READ, 
									 &hCDAKey );
	
	// if we found it, copy out the path to CDA
	if( Result == ERROR_SUCCESS )		
	{		
		DWORD Type( 0 );		
		DWORD Count( MAX_PATH );		
		if( ERROR_SUCCESS != ::RegQueryValueEx( hCDAKey, 
												( std::string( "FileName" ) +
												  CDAVersionTag ).c_str(), 
												0, 
												&Type, 
												(BYTE*)CDAPath, 
												&Count ) )			
		{			
			strcpy( CDAPath, "" );			
		}		
		::RegCloseKey( hCDAKey );		
	}	
	else	
	{
		strcpy( CDAPath, "" );
	}

	// try to load the lib
	m_CDAModule = LoadLibrary( CDAPath );
	// if it worked, assign addresses to all our function pointers
	if( m_CDAModule )
	{
		m_CDAFunctions.EngineInit =			(cdaEngineInitFP)GetProcAddress( m_CDAModule, "cdaEngineInit" );
		m_CDAFunctions.EngineFree =			(cdaEngineFreeFP)GetProcAddress( m_CDAModule, "cdaEngineFree" );
		m_CDAFunctions.Process =			(cdaProcessFP)GetProcAddress( m_CDAModule, "cdaProcess" );
		m_CDAFunctions.InitDownload =		(cdaInitDownloadFP)GetProcAddress( m_CDAModule, "cdaInitDownload" );
		m_CDAFunctions.InitDownloadToFile = (cdaInitDownloadToFileFP)GetProcAddress( m_CDAModule, "cdaInitDownloadToFile" );
		m_CDAFunctions.CancelDownload =		(cdaCancelDownloadFP)GetProcAddress( m_CDAModule, "cdaCancelDownload" );
		m_CDAFunctions.GetBytesDownloaded = (cdaGetBytesDownloadedFP)GetProcAddress( m_CDAModule, "cdaGetBytesDownloaded" );
		m_CDAFunctions.PauseDownload =		(cdaPauseDownloadFP)GetProcAddress( m_CDAModule, "cdaPauseDownload" );
		m_CDAFunctions.ResumeDownload =		(cdaResumeDownloadFP)GetProcAddress( m_CDAModule, "cdaResumeDownload" );
		m_CDAFunctions.ReleaseDownload =	(cdaReleaseDownloadRequestFP)GetProcAddress( m_CDAModule, "cdaReleaseDownloadRequest" );
		m_CDAFunctions.IsCabFile =			(cdaIsCabFileFP)GetProcAddress( m_CDAModule, "cdaIsCabFile" );
		m_CDAFunctions.ExtractCabAsync =	(cdaExtractCabAsyncFP)GetProcAddress( m_CDAModule, "cdaExtractCabAsync" );
		m_CDAFunctions.GetCachedFilename =	(cdaGetCachedFilenameFP)GetProcAddress( m_CDAModule, "cdaGetCachedFilename" );
		m_CDAFunctions.IsCabExtractDone =	(cdaIsCabExtractDoneFP)GetProcAddress( m_CDAModule, "cdaIsCabExtractDone" );
		m_CDAFunctions.ReleaseCabHandle =	(cdaReleaseDownloadRequestFP)GetProcAddress( m_CDAModule, "cdaReleaseCabHandle" );
		m_CDAFunctions.SetResumeCache =		(cdaSetResumeCacheFP)GetProcAddress( m_CDAModule, "cdaSetResumeCache" );
	}

	m_CDAFunctions.EngineInit();
	m_CDAFunctions.SetResumeCache( CacheFolder.c_str() );


	LoadManifest( FileManifest );

} // CCDAManager::CCDAManager()

CCDAManager::~CCDAManager( void )
{
	DELETE_SAFELY( m_pDownloadList );
	DELETE_SAFELY( m_pDescriptionList );

	if( m_CDAModule != NULL )
	{
		m_CDAFunctions.EngineFree();
		FreeLibrary( m_CDAModule );
	}

} // CCDAManager::~CCDAManager()


void CCDAManager::LoadManifest(  const std::string& ManifestPath )	// path to manifest.dat file
{

	// get the root path to the manifest file
	std::string RootPath = AssembleAbsolutePath( GetLocalPath(),
												 ManifestPath );

	CDataFile*	pDataFile = new CDataFile( RootPath );

	// parse all races from the manifest data file


	uint32 Files = pDataFile->DataGroups( "FILE" );
	for( uint32 i = 0; i < Files; i++ )
	{
		std::string Name( "Unknown" );
		std::string URL( "" );
		std::string LocalPath( "" );
		int32		Priority( -1 );

		CDataGroup* pGroup = pDataFile->GetDataGroup( "FILE", i );

		CDataGroup* pSubGroup = NULL;

		pSubGroup = pGroup->GetSubDataGroup( "NAME" );
		if( pSubGroup != NULL )
		{
			Name = pSubGroup->Item( 0 );
		}

		pSubGroup = pGroup->GetSubDataGroup( "URL" );
		if( pSubGroup != NULL )
		{
			// prepend our root URL to this
			URL = m_RootURL + "/" + pSubGroup->Item( 0 );

			if( URL.length() > 0 &&
				StringUpper( URL ).substr( 0, 7 ) == "HTTP://" )
			{
				URL = URL.substr( 7, URL.length() );
			}
			// replace duplicate slashes - some servers HATE these
			while( URL.find( "//" ) != -1 )
			{
				URL.replace( URL.find( "//" ),
							 2,
							 "/" );
			}
			URL = "http://" + URL;
		}

		pSubGroup = pGroup->GetSubDataGroup( "DESTINATION" );
		if( pSubGroup != NULL )
		{
			LocalPath = pSubGroup->Item( 0 );
		}

		pSubGroup = pGroup->GetSubDataGroup( "PRIORITY" );
		if( pSubGroup != NULL )
		{
			Priority = atoi( pSubGroup->Item( 0 ).c_str() );
		}

		DownloadDescription* pDescription = new DownloadDescription( Name,
																	 URL,
																	 LocalPath,
																	 Priority );

		uint32 Keywords = pGroup->SubDataGroups( "KEYWORD" );
		for( uint32 j = 0; j < Keywords; j++ )
		{
			pSubGroup = pGroup->GetSubDataGroup( "KEYWORD", j );
			pDescription->m_Keywords.push_back( StringUpper( pSubGroup->Item( 0 ) ) );
		}
		m_pDescriptionList->Queue( pDescription );
	}

	DELETE_SAFELY( pDataFile );

} // CCDAManager::LoadManifest()

void CCDAManager::Update( float32 TimeElapsed )	// Time elapsed in seconds
{
	ProcessLoading( TimeElapsed );
	ProcessQueue( TimeElapsed );
} // CCDAManager::Update()

// update the string representation of remaining time and filesize
void CCDAManager::UpdateEstimatedTime( void )
{

	int32 Seconds( (int32)floor( m_EstimatedTime + .5f ) );
	if(Seconds < 0)
	{
		Seconds = 0;
	}
	int32 Minutes( 0 );
	int32 Hours( 0 );
	while( Seconds > 60 )
	{
		Seconds -= 60;
		Minutes ++;
	}
	while( Minutes > 60 )
	{
		Minutes -= 60;
		Hours ++;
	}
	if( Hours > 0 )
	{
		m_EstimatedTimeString= IntegerToString( Hours ) + 
							   "Hours " + 
							   IntegerToString( Minutes ) + 
							   "Minutes";
	}
	else if( Minutes > 0 )
	{
		if( Seconds > 9 )
		{
			m_EstimatedTimeString = IntegerToString( Minutes ) + 
									":" + 
									IntegerToString( Seconds );
		}
		else
		{
			m_EstimatedTimeString = IntegerToString( Minutes ) + 
									":0" + 
									IntegerToString( Seconds );
		}
	}
	else
	{
		m_EstimatedTimeString = IntegerToString( Seconds ) + 
								" Seconds";
	}

} // CCDAManager::UpdateEstimatedTime()

void CCDAManager::ProcessLoading( float32 TimeElapsed )	// time elapsed in seconds
{
	// clear out download information, which will be
	// updated by an active download
	m_ActiveDownloads	= 0;
	m_RequestedKB		= 0;
	m_DownloadedKB		= 0;
	m_DownloadingName	= "?";

	CLLNode<CActiveDownload> *pDownload = m_pDownloadList->GetFirst();
	CLLNode<CActiveDownload> *pNext = NULL;
	// go through all active downloads and update their state
	while( pDownload != NULL )
	{
		pNext = pDownload->Next();

		// if a download failed, we need to notify the app that this is the case
		// NO DOWNLOADS ARE ALLOWED TO FAIL!
		if( pDownload->Data()->m_Failed )
		{
			m_DownloadFailed = kTrue;
			m_FailedFile = pDownload->Data()->m_pDownloadDescription->m_URL;
			return;
		}

		// if the download is complete
		if( pDownload->Data()->m_DownloadDoneEvent )
		{
			// and it is not marked as loaded yet 
			if( !pDownload->Data()->m_pDownloadDescription->m_Loaded )
			{
				// mark it as loaded
				m_DownloadedKB = 0;
				m_LastDownloadedKB = 0;
				m_EstimateTimer = 0;
				m_EstimatedTime = 0;
				m_LastEstimatedTime = 0;
				pDownload->Data()->m_pDownloadDescription->m_Loaded = kTrue;
				
				// make sure to release the download handle
				m_CDAFunctions.ReleaseDownload( pDownload->Data()->m_DownloadRequest );
			}

			// if the loaded file is not currently extracting
			if( !pDownload->Data()->m_Extracting )
			{
				char	Path[MAX_PATH];
				// see if we can find the cached downloaded file
				if( m_CDAFunctions.GetCachedFilename( pDownload->Data()->m_pDownloadDescription->m_URL.c_str(),
													  Path,
													  MAX_PATH ) )
				{
					// if it is a cab
					if( m_CDAFunctions.IsCabFile( Path ) )
					{
						// start extracting it to our requested extraction path
						pDownload->Data()->m_CabHandle = m_CDAFunctions.ExtractCabAsync( Path,
																						  m_ExtractionPath.c_str() );
						// let the download handle know it is extracting
						pDownload->Data()->m_Extracting = kTrue;
					}
					else
					{
						// let the description know it is done
						pDownload->Data()->m_pDownloadDescription->m_Loaded = kTrue;
						pDownload->Data()->m_pDownloadDescription->m_Extracted = kTrue;
						// as well as the download handle
						pDownload->Data()->m_Extracted=true;
					}
					
				}
			}
			else	// otherwise, if it is extracting
			{
				// give the rest of the CPU some room to perform the async extraction
				Sleep( 20 );

				m_DownloadingName = pDownload->Data()->m_pDownloadDescription->m_Name;
				m_RequestedKB = -1;
				if( m_CDAFunctions.IsCabExtractDone( pDownload->Data()->m_CabHandle ) )
				{
					m_CDAFunctions.ReleaseCabHandle( pDownload->Data()->m_CabHandle );
					pDownload->Data()->m_Extracted=true;
					pDownload->Data()->m_Extracting=false;
					pDownload->Data()->m_pDownloadDescription->m_Extracted=true;
				}
			}
			
			// if this item is extracted, we're done with it
			// and it can be removed from the list
			if( pDownload->Data()->m_Extracted )
			{
				m_pDownloadList->Remove( pDownload );
			}

		}	// if the download isn't paused, then it is the active download
		else if( !pDownload->Data()->m_Paused )
		{
			// update all data about the download
			m_DownloadingName = pDownload->Data()->m_pDownloadDescription->m_Name;
			m_RequestedKB = pDownload->Data()->m_FileKB;
			// sometimes you get bad data
			if( m_RequestedKB < 0 )
			{
				m_RequestedKB = 0;
			}
			m_DownloadedKB = pDownload->Data()->m_DownloadedKB;
			m_ActiveDownloads++;
		}
		pDownload = pNext;
	}

	// keep track of our download time
	m_EstimateTimer += TimeElapsed;
	// every second, we re-estimate the download rate
	if( m_EstimateTimer > 1 )
	{
		// delta of download KBs over the last second
		float32 DeltaKB = (float32)( m_DownloadedKB - m_LastDownloadedKB );
		// if there was any change
		if( DeltaKB != 0 )
		{
			// calculate the remaining time
			float32 Remaining = (float32)( m_RequestedKB - m_DownloadedKB) / DeltaKB;
			Remaining *= m_EstimateTimer;
			m_EstimatedTime = Remaining;
			// average our estimate with the previous estimate, so
			// that we don't jump around too much
			m_EstimatedTime = ( m_EstimatedTime + m_LastEstimatedTime) * .5f;
		}
		m_LastDownloadedKB = m_DownloadedKB;
		m_EstimateTimer = 0;
		m_LastEstimatedTime = m_EstimatedTime;
	}

	// if any downloads are going, let cda process them
	if( m_ActiveDownloads > 0 )
	{
		m_CDAFunctions.Process( 3 );
	}
	else	// otherwise, check for resumption of previous paused downloads
	{
		// if we have some items remaining in queue
		if( m_pDownloadList->GetFirst() != NULL )
		{
			CActiveDownload* pHighestPriority = NULL;
			pDownload = m_pDownloadList->GetFirst();
			while( pDownload != NULL )
			{
				// find the highest priority paused item
				if( pHighestPriority == NULL || 
					( pDownload->Data()->m_pDownloadDescription->m_Priority < pHighestPriority->m_pDownloadDescription->m_Priority && 
					  pDownload->Data()->m_pDownloadDescription->m_Priority != -1 ) ||
					( pDownload->Data()->m_pDownloadDescription->m_Priority > -1 && 
					  pHighestPriority->m_pDownloadDescription->m_Priority == -1 ) )
				{
					pHighestPriority = pDownload->Data();
				}

				pDownload = pDownload->Next();
			}

			// if we found one
			if( pHighestPriority != NULL )
			{
				// and it is in fact paused
				if( pHighestPriority->m_Paused )
				{
					// un-pause it
					pHighestPriority->m_Paused = kFalse;
					// if it isn't already extracted/extracting
					if( !pHighestPriority->m_Extracted && 
						!pHighestPriority->m_Extracting )
					{
						// resume the download
						m_CDAFunctions.ResumeDownload( pHighestPriority->m_DownloadRequest );
					}
				}
				m_ActiveDownloads++;
			}

		}
	}

	UpdateEstimatedTime();
} // CCDAManager::ProcessLoading()

void CCDAManager::ProcessQueue( float32 TimeElapsed )	// time elapsed in seconds
{

	CLLNode<DownloadDescription>*	pDownload = m_pDescriptionList->GetFirst();
	CLLNode<DownloadDescription>*	pNext = NULL;

	// the highest priority description in the list
	DownloadDescription* pHighestPriority = NULL;
	// go through all the descriptions in queue, and find
	// the highest-priority un-downloaded file
	while( pDownload != NULL )
	{
		pNext = pDownload->Next();

		// if we find a description whose file has already been downloaded,
		// we remove it from the list - we don't need it any more
		if( pDownload->Data()->m_Loaded && 
			pDownload->Data()->m_Extracted )
		{
			m_pDescriptionList->Remove( pDownload );
		}
		else
		{
			// if we have no best description, or this
			// description is of a higher priority, keep it
			if( pHighestPriority == NULL || 
				( pDownload->Data()->m_Priority < pHighestPriority->m_Priority 
				  && pDownload->Data()->m_Priority != -1 ) ||
				( pDownload->Data()->m_Priority > -1 && 
				  pHighestPriority->m_Priority == -1 ) )
			{
				pHighestPriority = pDownload->Data();
			}
		}
		pDownload = pNext;
	}

	// if no files are currently downloading, and we found
	// a description to download, add it to the queue and start
	// it downloading
	if( m_ActiveDownloads < 1 && 
		pHighestPriority != NULL )
	{
		AddToDownloadQueue( pHighestPriority );
		ProcessLoading( TimeElapsed );
	}
} // CCDAManager::ProcessQueue()

void CCDAManager::AddToDownloadQueue( DownloadDescription* pDownloadDescription )	// file we want to queue up
{

	CActiveDownload* pDownload = m_pDownloadList->Queue( new CActiveDownload( pDownloadDescription ) );

	// Use cdaCheckForUpdate to download if not in the cache or if a newer version exists
	cdaECacheUsage CacheParameter = cdaKCheckForUpdate;

	// if the download description has a destination path, make
	// sure it exists!
	if( pDownloadDescription->m_LocalPath.length() > 0 )
	{
		// assemble the absolute path to the destination
		std::string FullPath = AssembleAbsolutePath( GetLocalPath(), pDownloadDescription->m_LocalPath );
		std::string BaseDirectory = RemoveFileName( FullPath );

		CreateLocalDirectory( BaseDirectory );
	}
	pDownload->InitializeDownload();

} // CCDAManager::AddToDownloadQueue()

bool CCDAManager::ItemsLoaded( const std::string& Keyword )		// keyword to look for
{
	CLLNode<DownloadDescription>*	pDownload = m_pDescriptionList->GetFirst();
	std::string UpperKeyword( StringUpper( Keyword ) );
	// go through all the descriptions in queue, and find
	// any with the given keyword
	while( pDownload != NULL )
	{

		if( pDownload->Data()->HasKeyword( UpperKeyword ) )
		{
			return kFalse;
		}

		pDownload = pDownload->Next();
	}
	return kTrue;
} // CCDAManager::ItemsLoaded()


// ACTIVE DOWNLOAD METHODS

// callback method for DownloadRequest
void CDA_CALL FileDownloadCallback( cdaDownloadRequest DownloadRequest,	// the download request
									cdaEDownloadStatus DownloadStatus,	// current status of the download
									void* CallbackData,					// void pointer to callback data
									unsigned char** ppBuffer,			// ? buffer
									unsigned long BufferSize )			// length of buffer
{
	CActiveDownload* pActiveDownload = (CActiveDownload*)CallbackData;
	uint32 downloaded( 0 );

	// if this download has already started, simply update its download progress
	if ( DownloadStatus == cdaKStatusStarted )
	{
		// In this DownloadStatus only, the Buffersize tells us the size of the file to be downloaded
		pActiveDownload->m_FileKB = BufferSize / 1024;
	}	

	// keep the download handle's download status synced with the callback
	pActiveDownload->m_DownloadStatus = DownloadStatus; // update DownloadStatus

	if ( DownloadStatus != cdaKStatusCancelled && 
		 DownloadStatus != cdaKStatusSuccess && 
		 DownloadStatus != cdaKStatusFailed ) // these are the only "end-states"
	{
		// if the download DownloadRequestuest isn't in an "end state" then check to see if we should cancel the download.
		// must listen to see if a cancel event has been sent to this download

		// Get the progress of the download
		pActiveDownload->m_DownloadedKB = m_CDAFunctions.GetBytesDownloaded(pActiveDownload->m_DownloadRequest)/1024;

	}
	else
	{
		// the download is in an "end state"

		if( DownloadStatus == cdaKStatusFailed )
		{
			pActiveDownload->m_Failed = kTrue;
		}


		// set the m_DownloadDoneEvent that the thread is waiting on.
		pActiveDownload->m_DownloadDoneEvent = kTrue;
	}
} // FileDownloadCallback()

void CActiveDownload::InitializeDownload( void )
{
		
	cdaECacheUsage CacheParameter( cdaKCheckForUpdate );
	// Use cdaCheckForUpdate to download if not in the cache or if a newer version exists
	m_DownloadRequest = m_CDAFunctions.InitDownloadToFile( m_pDownloadDescription->m_URL.c_str(), 
														   AssembleAbsolutePath( GetLocalPath(), m_pDownloadDescription->m_LocalPath ).c_str(),
														   CacheParameter, 
														   kTrue, 
														   0, 
														   FileDownloadCallback, 
														   this );
} // CActiveDownload::InitializeDownload()

#endif