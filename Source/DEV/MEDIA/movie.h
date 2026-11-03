/*
	movie.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _MOVIE_H
#define _MOVIE_H

#include <vfw.h>
#include <D3D8.h>

#include "../UTILITIES/constants.h"

// the movie class decompresses an AVI to a D3D texture
class CMovie
{

public:


						CMovie( const std::string& FileName,		// path to movie to open
								LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								bool Looping = kTrue );				// is this a looping movie?
						~CMovie();

	void				Reset();

	void				Update( float32 TimeElapsed );				// time elapsed in seconds
	void				SetFrame( uint32 Frame );					// frame to advance to


	bool				IsValid()			{	return m_Valid;		};

	LPDIRECT3DTEXTURE8	GetTexture()		{	return m_pTexture;	};

	uint16				Width()				{	return m_Width;		};

	uint16				Height()			{	return m_Height;	};

private:
	std::string			m_Path;

	bool				m_Looping;						 
	bool				m_Valid;

	uint16				m_FPS;						
	uint16				m_Width;
	uint16				m_Height;

	uint32				m_CurrentFrame;				
	uint32				m_MovieFrames;  			

	float32				m_Timer;
	float32				m_FrameLength;


	LPDIRECT3DTEXTURE8	m_pTexture;

	RECT				m_SourceRect;
	CRITICAL_SECTION	m_AccessBuffer;

	PAVISTREAM			MovieAVIStream; 		// the Handle to the AVI-Videostream
	AVISTREAMINFO		MovieAVIStreamInfo; 	// The Video-Information
	
	LPBYTE				m_InputBuffer;			// Decompressor Input -Buffer
	LPBYTE				m_OutputBuffer; 		// Decompressor Output-Buffer

	HIC					m_DecompressorHandle;	// Handle to the AVI-Decompressor
	
	int32				m_BufferLength;			// Input-Bufferlength
	int32				m_LinePitch;   			// Bitmap-Linepitch
	
	LPBITMAPINFOHEADER	m_SourceFormat; 		// Format of VideoInput
	LPBITMAPV4HEADER	m_TargetFormat;			// Format of Surface
	
	bool				Initialize( LPDIRECT3DDEVICE8 pD3DDevice );					// direct3d device

	void				ReadFrameToBuffer( uint32 Frame );							// frame to read
	void				CopyBufferToTexture( LPDIRECT3DTEXTURE8 TargetTexture );	// target texture

	void				CloseMovie();

};

#pragma comment(lib,"vfw32.lib")

#endif
