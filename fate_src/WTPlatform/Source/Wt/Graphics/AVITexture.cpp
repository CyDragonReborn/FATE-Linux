/***************************************************************************************
 *
 * AVITexture.h
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          09/06/2005
 * Notes:         Original code copied from Travis' dungeon\dev\media\movie.*
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include "WT/Graphics/AVITexture.h"

#pragma comment( lib, "vfw32" )

using namespace WT::System;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CAVITexture )

CAVITexture::CAVITexture( const std::string& Path,		// path to movie to open
				LPDIRECT3DDEVICE9 pDevice,		// direct3d device
				bool Looping ) :					// is this a looping movie?
				m_MovieAVIStream( NULL ),
				m_DecompressorHandle( NULL ),
				m_CurrentTime( 0.0f ),
				m_CurrentFrame( 0 ),
				m_FrameCount( 0 ),
				m_bLooping( Looping ),
				m_bPause( false ),
				m_pTexture( NULL )
{
	AVIFileInit();  
	InitializeCriticalSection( &m_AccessBuffer );

	try
	{
		Initialize( Path, pDevice );
		InitializeTexture( pDevice );
	} catch (...) {
		Destroy();
		throw;
	}
}

CAVITexture::~CAVITexture()
{
	Destroy();
}

void CAVITexture::Destroy()
{
	if (m_pTexture != NULL)
	{
		m_pTexture->Release();
	}

	if (m_DecompressorHandle != NULL)
	{
		ICDecompressEnd( m_DecompressorHandle );
		ICClose( m_DecompressorHandle );
	}

	if (m_MovieAVIStream != NULL)
	{
		AVIStreamRelease( m_MovieAVIStream );
	}

	DeleteCriticalSection( &m_AccessBuffer );
	AVIFileExit();
}

// set up color format conversion, get information about the AVI stream,
// set up the decompressor, and create an appropriate D3D texture to receive the data

void CAVITexture::Error( std::string const& Path, std::string const& Message )
{
	throw std::runtime_error( Message + " failed for file '" + Path + '\'' );
}

void CAVITexture::InvalidateDeviceObjects()
{
	SafeRelease( m_pTexture );
}

void CAVITexture::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	if (m_pTexture != NULL)
	{
		return;
	}

	InitializeTexture( pDevice );
}

bool CAVITexture::IsValid() const
{
	return m_pTexture != NULL;
}

void CAVITexture::Initialize( std::string const& Path, LPDIRECT3DDEVICE9 pDevice )	// direct3d device
{
#ifndef NDEBUG
	m_Path = Path;
#endif

	if (AVIStreamOpenFromFile( &m_MovieAVIStream,
								Path.c_str(),
								streamtypeVIDEO,
								0,
								OF_READ,
								NULL ))
	{
		Error( Path, "AVIStreamOpenFromFile" );
	}

	// get the size of the format information
	LONG	FormatLength;
	HRESULT hr = AVIStreamFormatSize( m_MovieAVIStream, 0, &FormatLength );
	if (hr)
	{
		Error( Path, "AVIStreamFormatSize" );
	}

	// allocate space to store the format information
	m_SourceFormat.resize( FormatLength );
	m_TargetFormat.resize( std::max( FormatLength, static_cast<LONG>(sizeof( BITMAPV4HEADER ) ) ) );

	BITMAPINFOHEADER& SourceFormat = *reinterpret_cast<LPBITMAPINFOHEADER>( &m_SourceFormat[0] );
	BITMAPV4HEADER& TargetFormat = *reinterpret_cast<LPBITMAPV4HEADER>( &m_TargetFormat[0] );

	ZeroMemory( &TargetFormat, sizeof TargetFormat );

	// read in the target format data
	hr = AVIStreamReadFormat( m_MovieAVIStream, 0, &SourceFormat, &FormatLength );
	if (hr)
	{
		Error( Path, "AVIStreamReadFormat" );
	}

	// get the number of frames in the AVI
	m_FrameCount = AVIStreamLength( m_MovieAVIStream );
	if (m_FrameCount == -1)
	{
		Error( Path, "AVIStreamLength" );
	}

	// load the AVI Stream info
	AVISTREAMINFO MovieAVIStreamInfo;
	hr = AVIStreamInfo( m_MovieAVIStream, &MovieAVIStreamInfo, sizeof( AVISTREAMINFO ) );
	if (hr)
	{
		Error( Path, "AVIStreamInfo" );
	}

	// copy source format to target format
	memcpy( &TargetFormat, &SourceFormat, FormatLength );

	TargetFormat.bV4Size = std::max( FormatLength, static_cast<LONG>( sizeof( BITMAPV4HEADER ) ) );
	// try for 32 bit color
	TargetFormat.bV4BitCount = 32;
	TargetFormat.bV4V4Compression = BI_RGB;
	TargetFormat.bV4ClrUsed = 0;
	TargetFormat.bV4RedMask = 0x1F;
	TargetFormat.bV4GreenMask = 0x3F;
	TargetFormat.bV4BlueMask = 0x1F;
	TargetFormat.bV4AlphaMask = 0;
	TargetFormat.bV4SizeImage = ((TargetFormat.bV4Width + 3) & 0xFFFFFFFC) * 
								TargetFormat.bV4Height * 
								(TargetFormat.bV4BitCount >> 3);

	// calulate the buffer size based upon width height and color depth
	int32 BufferLength = TargetFormat.bV4Width * 
					 SourceFormat.biHeight * 
					 (SourceFormat.biBitCount >> 3);

	// if a buffer size has been suggested, use that 
	if ( MovieAVIStreamInfo.dwSuggestedBufferSize )
	{
		BufferLength = static_cast<LONG>( MovieAVIStreamInfo.dwSuggestedBufferSize );
	}

	// try to make a decompressor handle
	m_DecompressorHandle = ICDecompressOpen( ICTYPE_VIDEO,
											 MovieAVIStreamInfo.fccHandler,
											 &SourceFormat,
											 reinterpret_cast<LPBITMAPINFOHEADER>( &m_TargetFormat[0] ) );

	// if we failed to create a decompressor, try another color format and see 
	// if we can pull it off there
	if (m_DecompressorHandle == NULL)
	{
		TargetFormat.bV4Size = std::max( FormatLength, static_cast<LONG>( sizeof( BITMAPV4HEADER ) ) );
		TargetFormat.bV4BitCount = 16;
		TargetFormat.bV4V4Compression = BI_BITFIELDS;

		TargetFormat.bV4ClrUsed = 0;

		TargetFormat.bV4RedMask = 0x1F;
		TargetFormat.bV4GreenMask = 0x3F;
		TargetFormat.bV4BlueMask = 0x1F;
		TargetFormat.bV4AlphaMask = 0;
		TargetFormat.bV4SizeImage = ((TargetFormat.bV4Width + 3) & 0xFFFFFFFC) * 
									TargetFormat.bV4Height * (TargetFormat.bV4BitCount >> 3);

		// try one  more time
		m_DecompressorHandle = ICDecompressOpen( ICTYPE_VIDEO,
											  MovieAVIStreamInfo.fccHandler,
											  &SourceFormat,
											  ( LPBITMAPINFOHEADER )
											  &TargetFormat );
	}



	// if we couldn't get a decompressor, then this isn't going to work
	if (m_DecompressorHandle == NULL)
	{
		Error( Path, "ICDecompressOpen" );
	}

	// allocate an appropriate size, and clear out the input buffer
	m_InputBuffer.resize( BufferLength, 0 );

	// allocate an appropriate size, and clear out the output buffer
	m_OutputBuffer.resize( TargetFormat.bV4SizeImage, 0 );

	// calculate the line pitch
	m_LinePitch = TargetFormat.bV4Width * 
				  (TargetFormat.bV4BitCount >> 3);


	if (ICDecompressBegin( m_DecompressorHandle,
						   &SourceFormat,
						   reinterpret_cast<LPBITMAPINFOHEADER>( &m_TargetFormat[0] ) ) == ICERR_BADFORMAT)
	{
		Error( Path, "ICDecompressBegin" );
	}

	//Determine how long, in seconds, each frame will be displayed
	m_FrameLength = static_cast<float32>( MovieAVIStreamInfo.dwScale ) / MovieAVIStreamInfo.dwRate;
	m_CurrentTime = 0.0f;

	m_Width = static_cast<uint16>( TargetFormat.bV4Width );
	m_Height = static_cast<uint16>( TargetFormat.bV4Height );
}

void CAVITexture::InitializeTexture( LPDIRECT3DDEVICE9 pDevice )
{
	BITMAPV4HEADER& TargetFormat = *reinterpret_cast<LPBITMAPV4HEADER>( &m_TargetFormat[0] );

	// given the format of the movie, create a texture that is most compatible with that format
	D3DFORMAT const Format = TargetFormat.bV4BitCount == 16 ? D3DFMT_R5G6B5 : D3DFMT_A8R8G8B8;

	HRESULT hr = pDevice->CreateTexture(
		m_Width, m_Height, 1,
		D3DUSAGE_DYNAMIC, Format, D3DPOOL_DEFAULT,
		&m_pTexture, NULL );
	if (FAILED( hr ))
	{
		throw std::runtime_error( "CreateTexture failure" );
	}
}

// reset the movie to the beginning
void CAVITexture::Reset()
{
	SetTimeSeconds( 0.0f );
}

// manually set the current frame
void CAVITexture::SetFrame( uint32 Frame )	// frame to set to
{
	SetTimeSeconds( Frame * m_FrameLength );
}

void CAVITexture::SetTimeSeconds( float32 TimeSeconds )
{
	m_CurrentTime = TimeSeconds;

	if (!m_FrameCount)
	{
		m_CurrentFrame = -1;
		return;
	}

	int32 NewFrame = Float32ToInt32( floorf( m_CurrentTime / m_FrameLength ) );
	
	if (m_bLooping)
	{
		NewFrame %= m_FrameCount;
		if (NewFrame < 0) NewFrame += m_FrameCount;
	} else {
		if (NewFrame < 0)
		{
			NewFrame = 0;
		} else if (NewFrame >= m_FrameCount) {
			NewFrame = m_FrameCount - 1;
		}
	}

	if (m_CurrentFrame != NewFrame)
	{
		m_CurrentFrame = NewFrame;

		// pull the current frame index to the output buffer
		// blast the buffer to the texture
		ReadFrameToTexture();
	}
}

// update the movie - this involves advancing the time,
// tracking the individual frames, and copying the data to the texture
void CAVITexture::Update( float32 ElapsedTimeSeconds )	// time elapsed in seconds
{
	if (m_bPause)
	{
		return;
	}

	SetTimeSeconds( m_CurrentTime + ElapsedTimeSeconds );
}

// read frame pulls an image from the AVI at the requested location, and dumps it to the 
// Output Buffer
void CAVITexture::ReadFrameToTexture()
{
	assert( m_CurrentFrame >= 0 );

	if (!m_FrameCount)
	{
		return;
	}

	assert( m_CurrentFrame < m_FrameCount );

	AVIStreamRead( m_MovieAVIStream,
					m_CurrentFrame,
					1,
					&m_InputBuffer[0],
					static_cast<LONG>( m_InputBuffer.size() ),
					NULL,
					NULL );

	EnterCriticalSection( &m_AccessBuffer );
	
	ICDecompress( m_DecompressorHandle,
					0,
					reinterpret_cast<LPBITMAPINFOHEADER>( &m_SourceFormat[0] ),
					&m_InputBuffer[0],
					reinterpret_cast<LPBITMAPINFOHEADER>( &m_TargetFormat[0] ),
					&m_OutputBuffer[0] );

	LeaveCriticalSection( &m_AccessBuffer );

	// copies image data from the output buffer to the target texture -
	// make sure to readframe something into the output buffer first

	// we have to lock the area to copy to
	D3DLOCKED_RECT Locked;
	HRESULT Result = m_pTexture->LockRect( 0, &Locked, NULL, D3DLOCK_DISCARD );

	if (FAILED( Result ))
	{
		return;
	}

	LPBYTE	DestBits	= static_cast<LPBYTE>( Locked.pBits );
	LPBYTE	SourceBits	= &m_OutputBuffer[0] +
						  m_LinePitch * (reinterpret_cast<LPBITMAPV4HEADER>( &m_TargetFormat[0] )->bV4Height - 1);
	
	EnterCriticalSection( &m_AccessBuffer );

	// loop through all the rows of the image
	for (int32 i = 0; i < m_Height; i++)
	{
		// blast across each line
		memcpy( DestBits, SourceBits, m_LinePitch );
		// make sure to adjust for line pitch when skipping to the next line
		DestBits += Locked.Pitch;

		SourceBits -= m_LinePitch;
	}

	LeaveCriticalSection( &m_AccessBuffer );

	// unlock our rect
	m_pTexture->UnlockRect( 0 );
}

}
}
