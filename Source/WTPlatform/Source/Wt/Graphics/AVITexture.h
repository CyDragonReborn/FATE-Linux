//
// AVITexture.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/Graphics/IUpdateable.h"
#include "WT/Graphics/IDeviceObject.h"

#pragma warning( push )
#pragma warning( disable : 4201 ) //nonstandard extension used : nameless struct/union
#include <mmsystem.h> //Used internally by vfw.h
#pragma warning( pop )

#include <vfw.h>

namespace WT
{
namespace Graphics
{

//This class updates a D3D texture based on a playing AVI movie file
typedef System::SmartPointer<class CAVITexture> HAVITexture;
class CAVITexture : public System::CObject, public IDeviceObject, public IUpdateable
{
public:
	WT_BEGIN_CLASS_NO_CREATOR( CAVITexture, System::CObject )
		WT_INHERITS_INTERFACE( IDeviceObject );
		WT_INHERITS_INTERFACE( IUpdateable );
	WT_END_CLASS()

	CAVITexture( const std::string& Path,		// path to movie to open
			LPDIRECT3DDEVICE9 pDevice,		// direct3d device
			bool Looping = true );				// is this a looping movie?
	virtual ~CAVITexture();

	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	LPDIRECT3DTEXTURE9	GetTexture() const		{return m_pTexture;};
	uint16				Width() const			{return m_Width;};
	uint16				Height() const			{return m_Height;};

	bool				GetPause() const {return m_bPause;}
	void				SetPause( bool bPause ) {m_bPause = bPause;}

	int32				GetFrameCount() const {return m_FrameCount;}
	int32				GetFrame() const {return m_CurrentFrame;}

	void				Reset(); //Reset to beginning of movie
	void				SetFrame( uint32 Frame ); // frame to move to
	void				SetTimeSeconds( float32 TimeSeconds ); //time to move to

	void				Update( float32 ElapsedTimeSeconds );				// time elapsed in seconds

private:

#ifndef NDEBUG
	std::string			m_Path;
#endif

	uint16				m_Width;
	uint16				m_Height;

	int32				m_CurrentFrame;
	int32				m_FrameCount;

	float32				m_CurrentTime; //Current time in the movie
	float32				m_FrameLength; //Length of each frame in seconds

	LPDIRECT3DTEXTURE9	m_pTexture;

	CRITICAL_SECTION	m_AccessBuffer;

	PAVISTREAM			m_MovieAVIStream; 		// the Handle to the AVI-Videostream
	
	std::vector<BYTE>	m_InputBuffer;			// Decompressor Input -Buffer
	std::vector<BYTE>	m_OutputBuffer; 		// Decompressor Output-Buffer

	HIC					m_DecompressorHandle;	// Handle to the AVI-Decompressor
	
	int32				m_LinePitch;   			// Bitmap-Linepitch
	
	std::vector<uint8>  m_SourceFormat; 		// Format of VideoInput
	std::vector<uint8>	m_TargetFormat;			// Format of Surface
	
	bool				m_bLooping : 1;
	bool				m_bPause : 1;

	void				Initialize( std::string const& Path, LPDIRECT3DDEVICE9 pDevice );					// direct3d device
	void				InitializeTexture( LPDIRECT3DDEVICE9 pDevice );
	static void			Error( std::string const& Path, std::string const& Message );
	void				Destroy();
	void				ReadFrameToTexture();
};


} // namespace Graphics
} // namespace WT
