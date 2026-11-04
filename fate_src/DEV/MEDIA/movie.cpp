#include "d3dx8_shim.h"
#include "d3dx8.h"
/*
	movie.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#include <stdexcept>
#include <D3D8.h>
#include <D3DX8.h>


using namespace std;
#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"

#include "movie.h"


CMovie::CMovie( const std::string& FileName,		// path to movie to open
				LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
				bool Looping ) :					// is this a looping movie?
				MovieAVIStream( 0 ),
				m_InputBuffer( 0 ),
				m_OutputBuffer( 0 ),
				m_DecompressorHandle( 0 ),
				m_Timer( 0.0f ),
				m_CurrentFrame( 0 ),
				m_MovieFrames( 0 ),
				m_Looping( Looping ),
				m_Valid( kFalse ),
				m_Path( FileName ),
				m_SourceFormat( NULL ),
				m_TargetFormat( NULL ),
				m_pTexture( NULL ),
				m_FPS( 0 ),
				m_Width( 0 ),
				m_Height( 0 ),
				m_FrameLength( 0.0f ),
				m_BufferLength( 0 ),
				m_LinePitch( 0 )
{
	ZeroMemory( &m_SourceRect, sizeof( m_SourceRect ));
	ZeroMemory( &m_AccessBuffer, sizeof( m_AccessBuffer ));
	ZeroMemory( &MovieAVIStream, sizeof( MovieAVIStream ));
	ZeroMemory( &MovieAVIStreamInfo, sizeof( MovieAVIStreamInfo ));

	AVIFileInit();  

	InitializeCriticalSection( &m_AccessBuffer );

	if ( AVIStreamOpenFromFile( &MovieAVIStream,
								FileName.c_str(),
								streamtypeVIDEO,
								0,
								OF_READ,
								NULL ) )
	{
		AVIFileExit();
		return;
	}
	if ( !Initialize( pD3DDevice ) )
	{
		return;
	}
} // CMovie::CMovie()

CMovie::~CMovie()
{
	DeleteCriticalSection( &m_AccessBuffer );
	CloseMovie();
	AVIFileExit();
	RELEASE_SAFELY( m_pTexture );
} // CMovie::~CMovie()


// set up color format conversion, get information about the AVI stream,
// set up the decompressor, and create an appropriate D3D texture to receive the data

bool CMovie::Initialize( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	LONG	FormatLength( 0 );

	// get the size of the format information
	AVIStreamFormatSize( MovieAVIStream, 0, &FormatLength );

	// allocate space to store the format information
	m_SourceFormat = ( LPBITMAPINFOHEADER ) malloc( FormatLength );
	m_TargetFormat = ( LPBITMAPV4HEADER ) malloc( max( FormatLength, sizeof( BITMAPV4HEADER ) ) );

	ZeroMemory( m_TargetFormat, sizeof( BITMAPV4HEADER ) );

	// read in the target format data
	AVIStreamReadFormat( MovieAVIStream, 0, m_SourceFormat, &FormatLength );

	// get the number of frames in the AVI
	m_MovieFrames = AVIStreamLength( MovieAVIStream );

	// load the AVI Stream info
	AVIStreamInfo( MovieAVIStream, &MovieAVIStreamInfo, sizeof( AVISTREAMINFO ) );

	// copy source format to target format
	memcpy( m_TargetFormat, m_SourceFormat, FormatLength );

	m_TargetFormat->bV4Size = max( FormatLength, sizeof( BITMAPV4HEADER ) );
	// try for 32 bit color
	m_TargetFormat->bV4BitCount = 32;
	m_TargetFormat->bV4V4Compression = BI_RGB;
	m_TargetFormat->bV4ClrUsed = 0;
	m_TargetFormat->bV4RedMask = 0x1F;
	m_TargetFormat->bV4GreenMask = 0x3F;
	m_TargetFormat->bV4BlueMask = 0x1F;
	m_TargetFormat->bV4AlphaMask = 0;
	m_TargetFormat->bV4SizeImage = ( ( m_TargetFormat->bV4Width + 3 ) & 0xFFFFFFFC ) * 
								m_TargetFormat->bV4Height * 
								( m_TargetFormat->bV4BitCount >> 3 );

	// calulate the buffer size based upon width height and color depth
	m_BufferLength = m_TargetFormat->bV4Width * 
					 m_SourceFormat->biHeight * 
					 ( m_SourceFormat->biBitCount >> 3 );

	// if a buffer size has been suggested, use that 
	if ( MovieAVIStreamInfo.dwSuggestedBufferSize )
	{
		m_BufferLength = ( LONG ) MovieAVIStreamInfo.dwSuggestedBufferSize;
	}

	// try to make a decompressor handle
	m_DecompressorHandle = ICDecompressOpen( ICTYPE_VIDEO,
											 MovieAVIStreamInfo.fccHandler,
											 m_SourceFormat,
											 ( LPBITMAPINFOHEADER ) m_TargetFormat );

	// if we failed to create a decompressor, try another color format and see 
	// if we can pull it off there
	if ( m_DecompressorHandle == NULL )
	{
		m_TargetFormat->bV4Size = max( FormatLength, sizeof( BITMAPV4HEADER ) );
		m_TargetFormat->bV4BitCount = 16;
		m_TargetFormat->bV4V4Compression = BI_BITFIELDS;

		m_TargetFormat->bV4ClrUsed = 0;

		m_TargetFormat->bV4RedMask = 0x1F;
		m_TargetFormat->bV4GreenMask = 0x3F;
		m_TargetFormat->bV4BlueMask = 0x1F;
		m_TargetFormat->bV4AlphaMask = 0;
		m_TargetFormat->bV4SizeImage = ( ( m_TargetFormat->bV4Width + 3 ) & 0xFFFFFFFC ) * 
									m_TargetFormat->bV4Height * ( m_TargetFormat->bV4BitCount >> 3 );

		// try one  more time
		m_DecompressorHandle = ICDecompressOpen( ICTYPE_VIDEO,
											  MovieAVIStreamInfo.fccHandler,
											  m_SourceFormat,
											  ( LPBITMAPINFOHEADER )
											  m_TargetFormat );
	}



	// if we couldn't get a decompressor, then this isn't going to work
	if ( !m_DecompressorHandle )
	{
		return kFalse;
	}

	// allocate an appropriate size, and clear out the input buffer
	m_InputBuffer = ( BYTE * ) calloc( m_BufferLength, 1 );
	ZeroMemory( m_InputBuffer, m_BufferLength );

	// allocate an appropriate size, and clear out the output buffer
	m_OutputBuffer = ( BYTE * ) calloc( m_TargetFormat->bV4SizeImage, 1 );
	ZeroMemory( m_OutputBuffer, m_TargetFormat->bV4SizeImage );

	// calculate th eline pitch
	m_LinePitch = m_TargetFormat->bV4Width * 
				  ( m_TargetFormat->bV4BitCount >>  3 );

	if( ICDecompressBegin( m_DecompressorHandle,
						   m_SourceFormat,
						   ( LPBITMAPINFOHEADER ) m_TargetFormat ) == ICERR_BADFORMAT )
	{
		return kFalse;
	}

	// calculate the FPS
	m_FPS = (uint16) ( MovieAVIStreamInfo.dwRate / MovieAVIStreamInfo.dwScale );

	// determine how long, in seconds, each frame will be displayed
	m_FrameLength = 1.0F / ( float32 ) m_FPS;
	m_Timer = m_FrameLength;

	m_Width = (uint16) m_TargetFormat->bV4Width;
	m_Height = (uint16) m_TargetFormat->bV4Height;

	// lay out the rectangle for out texture for locking later
	m_SourceRect.left = 0;
	m_SourceRect.top = 0;
	m_SourceRect.right = m_Width;
	m_SourceRect.bottom = m_Height;

	// given the format of the movie, create a texture that is most compatible with that format
	if ( m_TargetFormat->bV4BitCount == 16 )
	{
		D3DXCreateTexture( pD3DDevice,
						   m_Width,
						   m_Height,
						   1,
						   0,
						   D3DFMT_R5G6B5,
						   D3DPOOL_MANAGED,
						   &m_pTexture );
	}
	else
	{
		D3DXCreateTexture( pD3DDevice,
						   m_Width,
						   m_Height,
						   1,
						   0,
						   D3DFMT_A8R8G8B8,
						   D3DPOOL_MANAGED,
						   &m_pTexture );

		// get a surface description so that we can verify that this is an OK color format
		D3DSURFACE_DESC	desc;
		HRESULT			hr	= m_pTexture->GetLevelDesc( 0, &desc );
		if ( FAILED( hr ) )
		{
			m_Valid = kFalse;
			return kFalse;
		}

		// if the resulting image doesn't match a requested format, then this isn't going to work...
		if ( desc.Format != D3DFMT_A8R8G8B8 && desc.Format != D3DFMT_X8R8G8B8 )
		{
			m_Valid = kFalse;
			return kFalse;
		}
	}

	m_Valid = kTrue;
	return kTrue;
} // CMovie::Initialize()

// reset the movie to the beginning
void CMovie::Reset()
{
	m_Timer = m_FrameLength;
	m_CurrentFrame = 0;

	// pull the current frame index to the output buffer
	ReadFrameToBuffer( m_CurrentFrame );
	// blast the buffer to the texture
	CopyBufferToTexture( m_pTexture );

} // CMovie::Reset()

// manually set the current frame
void CMovie::SetFrame( uint32 Frame )	// frame to set to
{
	m_Timer = m_FrameLength;
	m_CurrentFrame = Frame;

	// pull the current frame index to the output buffer
	ReadFrameToBuffer( m_CurrentFrame );
	// blast the buffer to the texture
	CopyBufferToTexture( m_pTexture );

} // CMovie::SetFrame()

// shut down all handles to the AVI
void CMovie::CloseMovie()
{
	if ( m_DecompressorHandle )
	{
		ICDecompressEnd( m_DecompressorHandle );
		ICClose( m_DecompressorHandle );
	}

	if ( m_SourceFormat )
	{
		free( m_SourceFormat );
	}
	if ( m_TargetFormat )
	{
		free( m_TargetFormat );
	}
	if ( m_InputBuffer )
	{
		free( m_InputBuffer );
	}
	if ( m_OutputBuffer )
	{
		free( m_OutputBuffer );
	}

	AVIStreamRelease( MovieAVIStream );
} // CMovie::CloseMovie()

// update the movie - this involves advancing the time,
// tracking the individual frames, and copying the data to the texture
void CMovie::Update( float32 TimeElapsed )	// time elapsed in seconds
{
	m_Timer += TimeElapsed;

	// if we haven't yet rolled over a frame, do nothing
	if ( m_Timer < m_FrameLength )
	{
		return;
	}
	// once we've passed the frame threshhold
	while ( m_Timer >= m_FrameLength )
	{
		m_Timer -= m_FrameLength;
		m_CurrentFrame++;
		if ( m_CurrentFrame >= m_MovieFrames - 1 )
		{
			if( m_Looping )
			{
				m_CurrentFrame = 0;
			}
			else
			{
				m_CurrentFrame--;
				return;
			}
		}
	}
	// pull the current frame index to the output buffer
	ReadFrameToBuffer( m_CurrentFrame );
	// blast the buffer to the texture
	CopyBufferToTexture( m_pTexture );
} // Update()

// copies image data from the output buffer to the target texture -
// make sure to readframe something into the output buffer first
void CMovie::CopyBufferToTexture( LPDIRECT3DTEXTURE8 TargetTexture )	// texture to copy to
{
	HRESULT			Result;
	D3DLOCKED_RECT	Locked;

	// we have to lock the area to copy to
	Result = TargetTexture->LockRect( 0, &Locked, NULL, 0 );

	if( FAILED( Result ) )
	{
		return;
	}

	LPBYTE	DestBits	= ( LPBYTE ) Locked.pBits;	 
	LPBYTE	SourceBits	= m_OutputBuffer +
						  m_LinePitch * ( m_TargetFormat->bV4Height - 1 );
	
	EnterCriticalSection( &m_AccessBuffer );

	// loop through all the rows of the image
	for ( int32 i = 0; i < m_SourceRect.bottom; i++ )
	{
		// blast across each line
		memcpy( DestBits, SourceBits, m_LinePitch );
		// make sure to adjust for line pitch when skipping to the next line
		DestBits += Locked.Pitch;

		SourceBits -= m_LinePitch;
	}

	LeaveCriticalSection( &m_AccessBuffer );

	// unlock our rect
	TargetTexture->UnlockRect( 0 );

} // CMovie::CopyBufferToTexture()

// read frame pulls an image from the AVI at the requested location, and dumps it to the 
// Output Buffer
void CMovie::ReadFrameToBuffer( uint32 Frame )	// which frame to decompress form the AVI
{
	if ( Frame < m_MovieFrames )
	{
		AVIStreamRead( MovieAVIStream,
					   Frame,
					   1,
					   m_InputBuffer,
					   m_BufferLength,
					   NULL,
					   NULL );

		EnterCriticalSection( &m_AccessBuffer );
		
		ICDecompress( m_DecompressorHandle,
					  0,
					  m_SourceFormat,
					  m_InputBuffer,
					  ( LPBITMAPINFOHEADER ) m_TargetFormat,
					  m_OutputBuffer );

		LeaveCriticalSection( &m_AccessBuffer );
	}

} // CMovie::ReadFrame()

