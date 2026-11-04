/***************************************************************************************
 *
 * AudioSystem.cpp
 *
 * Copyright (C) 2003-2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 12/08/2003: Jason G:	Creation
 *
 ***************************************************************************************/
// -----------------------------------------------------------------
// Include Files
// -----------------------------------------------------------------
#include "WT/stdafx.h"
#include "WT/Audio/AudioProperties.h"
#include "WT/Audio/AudioSystem.h"
#include "WT/Audio/AudioDevice.h"
#include "WT/Audio/MarkerFile.h"

#ifdef _DEBUG
#pragma comment( lib, "fmodstaticSD" )
#else
#pragma comment( lib, "fmodstaticS" )
#endif

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CAudioSystem)

CAudioSystem::CAudioSystem( const CAudioSystemProperties& InitProps )
	: m_FindCdRomDriveLetter( KFindCdRomDriveMin - 1 )
{
	m_hAudioDevice = new CAudioDevice( InitProps );
	m_hDspSystem = new CDspSystem( m_hAudioDevice->GetDspEngine() );
}

CAudioSystem::~CAudioSystem()
{
}


/*
uint32 CAudioSystem::GetDriverCount( )
{
    assert( m_DriverMgr != NULL );

    return m_DriverMgr->GetDriverCount( AudioDriverId ) );
}

bool CAudioSystem::GetDriverInfo( uint32 Index, AudioDriverInfo* Info )
{
    return true;
}

uint32 CAudioSystem::GetDriverDeviceCount( uint32 DriverIndex )
{
    assert( m_DriverMgr != NULL );

    uint32 deviceCnt = 0;
    uint32 driverCnt = m_DriverMgr->GetDriverCount( AudioDriverId ) );

    assert( DriverIndex < driverCnt );

    const DriverIdentifier* pDrI = m_DriverMgr->GetDriverIdentifier( AudioDriverId ), DriverIndex );
    if( pDrI != NULL )
    {
        IAudioDriverHnd driverHnd = m_DriverMgr->CreateDriver( pDrI );
        if( driverHnd != NULL )
        {
            deviceCnt = driverHnd->GetDeviceCount();
        }
    }

    return deviceCnt;
}

bool CAudioSystem::GetDriverDeviceInfo( uint32 DriverIndex, uint32 DeviceIndex, AudioDeviceInfo* Info )
{
    return true;
}

bool CAudioSystem::ActivateDriver( uint32 DriverIndex, uint32 DeviceIndex, IPropertyBag* DeviceProps )
{
    assert( m_DriverMgr != NULL );

    ShutdownDriver();

    uint32 deviceCnt = 0;
    uint32 driverCnt = m_DriverMgr->GetDriverCount( AudioDriverId ) );

    assert( DriverIndex < driverCnt );

    const DriverIdentifier* pDrI = m_DriverMgr->GetDriverIdentifier( AudioDriverId ), DriverIndex );
    if( pDrI != NULL )
    {
        m_AudioDriver = m_DriverMgr->CreateDriver( pDrI );
        if( m_AudioDriver != NULL )
        {
            deviceCnt = m_AudioDriver->GetDeviceCount();
            assert( DeviceIndex < deviceCnt );

            const DeviceIdentifier* pDI = m_AudioDriver->GetDeviceIdentifier( DeviceIndex );
            if( pDI != NULL )
            {
                m_hAudioDevice = m_AudioDriver->CreateDevice( pDI );
                if( m_hAudioDevice != NULL )
                {
                    // TODO: need to allow audio engine properties to be
                    // overridden. Right now we just use defaults.
                    return m_hAudioDevice->Init( DeviceProps ); //NULL );
                }
            }
        }
    }

    return false;
}

bool CAudioSystem::ShutdownDriver( )
{
    if( m_hAudioDevice != NULL )
    {
        m_hAudioDevice->Shutdown();
        m_hAudioDevice = NULL;
    }
    if( m_AudioDriver != NULL )
    {
        m_AudioDriver = NULL;
    }
    return true;
}
*/



void CAudioSystem::Update()
{
	m_hAudioDevice->Update();
}

void CAudioSystem::SetSpeakerMode( ESpeakerMode SpeakerMode )
{
	m_hAudioDevice->SetSpeakerMode( SpeakerMode );
}

void CAudioSystem::SetPanSeparation( float32 PanSeparation )
{
	m_hAudioDevice->SetPanSeparation( PanSeparation );
}

void CAudioSystem::SetMasterVolume( float32 Volume )
{
	m_hAudioDevice->SetMasterVolume( Volume );
}

float32 CAudioSystem::GetMasterVolume( ) const
{
	return m_hAudioDevice->GetMasterVolume( );
}

HAudioClip CAudioSystem::LoadAudioClip(	const std::string& Filename,
												AudioModeFlags Mode )
{
	return m_hAudioDevice->LoadAudioClip( Filename, Mode );
}

HAudioStream CAudioSystem::OpenAudioStream(	const std::string& Filename,
													AudioModeFlags Mode,
													uint32 BufferMs )
{
	HAudioStream hStream = m_hAudioDevice->OpenAudioStream(	Filename,
																Mode,
																BufferMs );
	assert( hStream != NULL );

	CMarkerFileReader mrkFileReader;
	{
		std::string mrkFile( Filename );
		mrkFile = StringLower( mrkFile );
		const std::string::size_type npos = static_cast<std::string::size_type>(-1);
		const std::string::size_type pos = mrkFile.rfind( "." );
		if( pos != npos )
		{
			mrkFile = Filename.substr(0,pos);
			mrkFile += ".wtmrkr";
			mrkFileReader.LoadMarkerData( mrkFile );
			{
				MarkerData mrkData;
				uint32 mrkCnt = mrkFileReader.GetNumMarkers();
				for( uint32 i=0; i<mrkCnt; i++ )
				{
					mrkFileReader.GetMarkerInfo( i, mrkData );
					hStream->AddMarker( std::string( mrkData.MarkerName ), mrkData.MarkerOffset );
				}
			}
		}
	}

	return hStream;
}

HAudioCD CAudioSystem::OpenAudioCD( const std::string& driveLetter, uint32 BufferMs )
{
	return m_hAudioDevice->OpenAudioCD( driveLetter, BufferMs );
}

HMusic CAudioSystem::LoadMusic(	const std::string& Filename,
										AudioModeFlags Mode )
{
	return m_hAudioDevice->LoadMusic( Filename, Mode );
}

CDspSystem* CAudioSystem::GetDspSystem( )
{
	return m_hDspSystem;
}

HChannel CAudioSystem::CreateReservedChannel()
{
	return m_hAudioDevice->CreateReservedChannel( );
}

// return KFail on Failure
// On Success, FindDriveLetter contains a drive path string "D" etc.
void CAudioSystem::FindFirstCdRomDriveLetter( std::string& FindDriveLetter )
{
	m_FindCdRomDriveLetter = KFindCdRomDriveMin;				// prime the search
	return FindNextCdRomDriveLetter( FindDriveLetter );
}

void CAudioSystem::FindNextCdRomDriveLetter( std::string& FindDriveLetter )
{
	int8 path[4] = "C:\\";

    while(	m_FindCdRomDriveLetter >= KFindCdRomDriveMin &&
			m_FindCdRomDriveLetter < KFindCdRomDriveMax )
	{
		path[0] = (int8)('A' + m_FindCdRomDriveLetter++);

        if( GetDriveType( path ) == DRIVE_CDROM )
        {
			FindDriveLetter = path[0];
            return;
        }
    }

	WT_THROW( CAudioException, "failed" );
}

}	// namespace Audio
}	// namespace WT
