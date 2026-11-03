/*
	cdamanager.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	8/20/2004
	
*/

#ifdef ONLINEVERSION

#ifndef _CDAMANAGER_H
#define _CDAMANAGER_H

#include "../UTILITIES/llist.h"
#include "../UTILITIES/constants.h"
#include "../../CDASDK/INCLUDE/cdaEngineInterface.h"

class	CActiveDownload;

// this is a description of the file to be downloaded,
// its key data, and its priority information
struct	DownloadDescription
{
	DownloadDescription( const std::string& Name,		// name to display
						 const std::string& URL,		// url to download
						 const std::string& LocalPath,	// local path to save to (can be blank)
						 int32 Priority ) :				// priority (smaller is higher, -1 is last)
											m_Name( Name ),
											m_URL( URL ),
											m_LocalPath( LocalPath ),
											m_Priority( Priority ),
											m_Loaded( kFalse ),
											m_Extracted( kFalse )
	{
	}

	~DownloadDescription( void )
	{
	}


	bool	HasKeyword( const std::string& Keyword )
	{
		for( uint32 i = 0; i < m_Keywords.size(); i++ )
		{
			if( Keyword == m_Keywords[i] )
			{
				return kTrue;
			}
		}
		return kFalse;
	}

	std::string	m_Name;
	std::string m_URL;
	std::string m_LocalPath;
	
	std::vector< std::string >	m_Keywords;

	int32	m_Priority;

	bool	m_Loaded;
	bool	m_Extracted;

};


// manager for active CDA downloads,
// which allows you to check for progress of keyed
// items, and pause items that are not needed.  Ties in
// with gamestate manager for easy streaming of content
// based upon application context
class CCDAManager
{
public:


			CCDAManager( const std::string& FileManifest,		// manifest containing file descriptions
						 const std::string& RootURL,			// root that download paths are relative to
						 const std::string& ExtractionPath,	// root path to extract cabs to
						 const std::string& CacheFolder );	// path where files will be cached in the CDA directory

			~CCDAManager( void );

	void	Update( float32 TimeElapsed );	// Time elapsed in seconds

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	bool	ItemsLoaded( const std::string& Keyword );	// keyword to look for

	uint32	ActiveDownloads( void )				{	return m_ActiveDownloads;		};

	const	std::string& Name( void )			{	return m_DownloadingName;		};

	const	std::string& EstimatedTime( void )	{	return m_EstimatedTimeString;	};

	uint32	DownloadedKB( void )				{	return m_DownloadedKB;			};

	uint32	RequestedKB( void )					{	return m_RequestedKB;			};

	bool	DownloadFailed( void )				{	return m_DownloadFailed;		};

	const std::string& FailedFile( void )		{	return m_FailedFile;			};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////


private:
	void	LoadManifest(  const std::string& ManifestPath );	// path to manifest file

	void	AddToDownloadQueue( DownloadDescription* pDownloadDescription );	// file we want to queue up

	void	UpdateEstimatedTime( void );

	void	ProcessLoading( float32 TimeElapsed );	// time elapsed in seconds

	void	ProcessQueue( float32 TimeElapsed );	// time elapsed in seconds

	CLList<CActiveDownload>*		m_pDownloadList;	// list of pointers to all active downloads
	CLList<DownloadDescription>*	m_pDescriptionList;	// list of pointers to all file descriptions to be downloaded

	HINSTANCE		m_CDAModule;

	float32			m_EstimateTimer;

	float32			m_EstimatedTime;
	float32			m_LastEstimatedTime;

	uint32			m_ActiveDownloads;
	uint32			m_RequestedKB;
	uint32			m_DownloadedKB;
	uint32			m_LastDownloadedKB;
	std::string		m_DownloadingName;

	std::string		m_ExtractionPath;
	std::string		m_RootURL;

	std::string		m_EstimatedTimeString;

	bool			m_DownloadFailed;
	std::string		m_FailedFile;

};

// this is a handle to an active download of a file description
class CActiveDownload 
{
public :

	CActiveDownload( DownloadDescription* pDownloadDescription ) :	// package reference
					 m_pDownloadDescription( pDownloadDescription ),
					 m_FileKB( 0 ),
					 m_DownloadedKB( 0 ),
					 m_DownloadDoneEvent( kFalse ),
					 m_Failed( kFalse ),
					 m_Paused( kFalse ),
					 m_Extracted( kFalse ),
					 m_Extracting( kFalse )

	{
	}

	~CActiveDownload( void )
	{
	};

	void InitializeDownload( void );

	DownloadDescription*			m_pDownloadDescription;

	cdaEDownloadStatus	m_DownloadStatus;
	cdaDownloadRequest  m_DownloadRequest;
	cdaCabHandle		m_CabHandle;


	bool				m_Failed;
	bool				m_DownloadDoneEvent;
	bool				m_Extracted;
	bool				m_Extracting;
	bool				m_Paused;
	
	int32				m_DownloadedKB;
	int32				m_FileKB;
};

#endif
#endif