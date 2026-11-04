//
// SystemFont.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/UI/Font.h"
#include "WT/System/IO/Stream.h"
#include "WT/Image/Image.h"
#include <vector>


namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace WT::System::IO;

class CSystemFont : public CFont
{
public:

	WT_CLASS(CSystemFont, CFont)

	CSystemFont();
	CSystemFont( const CSystemFont& Other );
	virtual ~CSystemFont();


	// Locks the font so that additional characters may be added
	// A successful call to BeginUpdate must be matched by a call to EndUpdate
	// Calls to AddChars are valid between calls to BeginUpdate and EndUpdate 
	virtual bool BeginUpdate( LPDIRECT3DDEVICE9 pDevice )
	{
		if( pDevice == NULL || m_pDevice != NULL )
		{
			return false;
		}
		m_pDevice = pDevice;

		// Initialize the font

		DWORD const dwWeight		= TranslateFontWeight( m_Weight );
		DWORD const dwItalic		= m_Italic	  ? TRUE : FALSE;
		DWORD const dwUnderline		= m_Underline ? TRUE : FALSE;
		DWORD const dwStrikeout		= m_Strikeout ? TRUE : FALSE;
		DWORD const dwCharacterSet	= m_CharacterSet;

		HDC hDC = ::CreateCompatibleDC( NULL );
		::SetMapMode( hDC, MM_TEXT );
		INT const iHeight = -::MulDiv( m_FontHeight, ::GetDeviceCaps(hDC, LOGPIXELSY), 72 );

		// Create the font
		// By specifying ANTIALIASED_QUALITY, we might get an antialiased font, but this is not guaranteed.
		m_hFont = ::CreateFont( iHeight, 0, 0, 0, 
								dwWeight, dwItalic, dwUnderline, dwStrikeout, dwCharacterSet,	
								OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, 
								m_Face.c_str( ) );

		::DeleteDC ( hDC );

		m_LastWritePos.x = 1;
		m_LastWritePos.y = 1;

		if( m_hFont == NULL )
		{
			return false;
		}

		return true;
	}

	// Signals that updates to the font are complete so that the font object can be unlocked
	// Calls to AddChars are no longer valid after this method is called
	virtual bool EndUpdate( )
	{
		if( m_pDevice == NULL )
		{
			return false;
		}

		bool Result = true;

		if( m_pCurrentTexture )
		{
			// Unlock the current texture, apply its alpha channel and add to the font sheets collection			
			Result = FinishFontTexture( m_pCurrentTexture );

			// Invalidate the current working texture
			SafeDelete( m_pCurrentTexture );
		}					

		m_pDevice = NULL;

		if( m_hFont != NULL )
		{
			::DeleteObject ( m_hFont );
			m_hFont = NULL;
		}

		m_Initialized = true;

		return Result;
	}

	// Adds all characters within the provided range to the font
	virtual bool AddChars ( WideLetter StartCharacter, WideLetter EndCharacter );

	// Adds all unique characters within the provided string to the font
	virtual bool AddChars ( const wstring& Chars );

	// Adds all unique characters within the provided stream to the font
	virtual bool AddChars ( System::IO::CStream* pGlyphStream );

	// Retrieves the weight of the font
	virtual EFontWeight::Weight GetWeight( ) const
	{
		return m_Weight;
	}

	// Determines whether the font exhibits the italicized property
	virtual bool GetItalic( ) const
	{
		return m_Italic;
	}

	// Determines whether the font exhibits the underlined property
	virtual bool GetUnderline( ) const
	{
		return m_Underline;
	}

	// Determines whether the font exhibits the strikeout property
	virtual bool GetStrikeout( ) const
	{
		return m_Strikeout;
	}

	// Retrieves the logical name of system font
	virtual string GetFace( ) const
	{
		return m_Face;
	}

	// Retrieves the character set fitting the target locale
	virtual ECharacterSet::Set GetCharacterSet( ) const
	{
		return m_CharacterSet;
	}

	// Determines whether the font was created with the border effect
	virtual bool GetBorderEffect( ) const
	{
		return m_BorderEffect;
	}

	// Determines whether the font was created with the shadow effect
	virtual bool GetShadowEffect( ) const
	{
		return m_ShadowEffect;
	}

	void SetHeight( int32 Height )
	{
		m_FontHeight = Height;
	}

	virtual int32 GetHeight() const
	{
		return m_FontHeight + (m_BorderEffect ? 2 : 0);
	}

	void SetWeightEnum( int32 WeightEnum )
	{
		m_Weight = static_cast<EFontWeight::Weight>( WeightEnum );
	}
	int32 GetWeightEnum( ) const
	{
		return static_cast<int32>( m_Weight );
	}

	void SetItalic( bool Italic )
	{
		m_Italic = Italic;
	}

	void SetUnderline( bool Underline )
	{
		m_Underline = Underline;
	}

	void SetStrikeout( bool Strikeout )
	{
		m_Strikeout = Strikeout;
	}

	void SetFace( const string& Face )
	{
		m_Face = Face;
	}

	void SetCharacterSetEnum( int32 CharacterSet )
	{
		m_CharacterSet = static_cast<ECharacterSet::Set>( CharacterSet );
	}
	int32 GetCharacterSetEnum( ) const
	{
		return static_cast<int32>( m_CharacterSet );
	}

	void SetBorderEffect( bool BorderEffect )
	{
		m_BorderEffect = BorderEffect;
	}

	void SetShadowEffect( bool ShadowEffect )
	{
		m_ShadowEffect = ShadowEffect;
	}

	// Initializes the font with the provided parameters
	virtual void Initialize ( const string& Face, uint32 Height, ECharacterSet::Set CharacterSet, 
		uint32 TexWidth, uint32 TexHeight,
        EFontWeight::Weight Weight, bool Italic, bool Underline, bool Strikeout,
		bool BorderEffect, bool ShadowEffect )
	{
		m_Face = Face;
		m_FontHeight = Height;
		m_CharacterSet = CharacterSet;
		m_TextureWidth = TexWidth;
		m_TextureHeight = TexHeight;
		m_Weight = Weight;
		m_Italic = Italic;
		m_Underline = Underline;
		m_Strikeout = Strikeout;
		m_BorderEffect = BorderEffect;
		m_ShadowEffect = ShadowEffect;
	}


private: // classes

	class CFontTexture
	{
	public:
		~CFontTexture()
		{
			Release();
		}

		static CFontTexture* Create( LPDIRECT3DDEVICE9 pDevice, uint32 Width, uint32 Height )
		{
			if ( !pDevice )
				return NULL;

			CImage::Desc TexDesc;
			TexDesc.Width = Width;
			TexDesc.Height = Height;
			TexDesc.Depth = 1;
			TexDesc.Format = EPixelFormat::A8R8G8B8;
			TexDesc.ImagePitch = 0;
			TexDesc.SlicePitch = 0;
			TexDesc.PaletteSize = 0;
			TexDesc.RowPitch = 0;

			HImage hImage;
			try
			{
				hImage = CImage::CreateImage(TexDesc);
			}
			catch(...)
			{
				return NULL;
			}
	
			if ( !hImage )
			{
				return NULL;
			}

			return new CFontTexture( pDevice, hImage );
		}

		HDC GetDC()
		{
			if ( m_hTexDC )
				return m_hTexDC;

			try
			{
				m_hTexDC = m_hImage->LockDC();
			}
			catch(...)
			{
				m_hTexDC = NULL;
			}

			return m_hTexDC;
		}

		void ReleaseDC()
		{
			if (m_hTexDC)
			{
				try
				{
					m_hImage->UnlockDC();
				}
				catch(...)
				{
				}

				m_hTexDC = NULL;
			}
		}

		HImageLock GetImageLock()
		{
			if ( m_hImage->IsLocked() )
				return NULL;

			HImageLock Result;
			try
			{
				Result = m_hImage->Lock( EGraphicsResourceLock::ReadWrite );
			}
			catch(...)
			{
				assert( Result == NULL );
			}

			return Result;
		}

		bool GetDesc( CImage::Desc& TexDesc )
		{
			try
			{
				TexDesc = m_hImage->GetDesc();
			}
			catch(...)
			{
				return false;
			}

			return true;
		}

		LPDIRECT3DBASETEXTURE9 GetTexture()
		{
			if ( !m_pTexture )
			{
				CreateTexture();
			}

			return m_pTexture;
		}

	private:
		CFontTexture()
			: m_pDevice(NULL)
			, m_pTexture(NULL)
			, m_hTexDC(NULL)
		{
		}

		CFontTexture( LPDIRECT3DDEVICE9 pDevice, CImage* pImage )
			: m_pDevice(pDevice)
			, m_hImage(pImage)
			, m_pTexture(NULL)
			, m_hTexDC(NULL)
		{
		}

		bool CreateTexture()
		{
			SafeRelease( m_pTexture );

			if ( !m_pDevice )
				return false;

			try
			{
				m_pTexture = m_hImage->CreateD3DTexture( m_pDevice );
			}
			catch(...)
			{
				return false;
			}

			return (m_pTexture != NULL) ? true : false;
		}

		void Release()
		{
			if ( m_hTexDC )
			{
				try
				{
					m_hImage->UnlockDC();
				}
				catch(...)
				{}
				m_hTexDC = NULL;
			}

			m_hImage = NULL;
			SafeRelease( m_pTexture );
		}

	private:
		HImage					m_hImage;
		LPDIRECT3DDEVICE9		m_pDevice;
		LPDIRECT3DBASETEXTURE9	m_pTexture;
		HDC						m_hTexDC;
	};


private: // methods

	// Fills the sent vector stream with the given range of characters, including the NULL character
	bool BuildCharacterStream( WideLetter Start, WideLetter End, std::vector<WideLetter>& CharacterStream ) const;
	// Fills the sent vector stream with all unique characters found in the given string, including the NULL character
	bool BuildCharacterStream( const wstring& Chars, std::vector<WideLetter>& CharacterStream ) const;
	// Fills the sent vector stream with all unique characters found in the given stream, including the NULL character
	bool BuildCharacterStream( System::IO::CStream* pGlyphStream, std::vector<WideLetter>& CharacterStream ) const;

	// Determines whether the given vector stream contains the given character
	static bool StreamContainsCharacter( const std::vector<WideLetter>& CharacterStream, WideLetter c );

	// Translates the provided EFontWeight::Weight value to a Win32 font weight
	static inline DWORD TranslateFontWeight( EFontWeight::Weight FontWeight )
	{
		switch( FontWeight )
		{	
		case EFontWeight::DontCare:
		default:
			return FW_DONTCARE;

		case EFontWeight::Thin:
			return FW_THIN;

		case EFontWeight::ExtraLight:
			return FW_EXTRALIGHT;

		case EFontWeight::Light:
			return FW_LIGHT;

		case EFontWeight::Normal:
			return FW_NORMAL;

		case EFontWeight::Medium:
			return FW_MEDIUM;

		case EFontWeight::SemiBold:
			return FW_SEMIBOLD;

		case EFontWeight::Bold:
			return FW_BOLD;

		case EFontWeight::ExtraBold:
			return FW_EXTRABOLD;

		case EFontWeight::Heavy:
			return FW_HEAVY;
		}
	}

	// Finalizes updates to the current working texture and adds the next one to the fontsheet collection
	bool AddNextTexture( LPDIRECT3DDEVICE9 pDevice );

	// Renders all glyphs starting at the provided iterator to the given device context
	bool RenderGlyphs ( HDC hDC, std::vector<WideLetter>::const_iterator& it, std::vector<WideLetter>::const_iterator& end );

	// Applies an alpha channel to the provided device texture
	static bool WTCall ApplyAlphaChannel( CFontTexture* pTexture );

	// Internal AddChars method; adds all characters in the provided stream to the font
	// All public AddChars methods are funnelled to this method
	bool AddChars ( LPDIRECT3DDEVICE9 pDevice, const std::vector<WideLetter>& CharacterStream );

	// Release Font texture image DC handle, apply alpha to image bits, and add device texture to font sheet
	// collection.
	bool FinishFontTexture( CFontTexture * pFontTexture );


private: // data

	string					m_Face;				// font face
	ECharacterSet::Set		m_CharacterSet;		// character set fitting target locale

	uint32					m_TextureWidth;		// size of texture to generate
	uint32					m_TextureHeight;

	EFontWeight::Weight		m_Weight;			// font weight
	bool	 				m_Italic;			// flags to generate an italic font
	bool	 				m_Underline;		// flags to generate an underlined font
	bool	 				m_Strikeout;		// flags to generate a strikeout font
	
	bool					m_BorderEffect;		// flags "border" effect
	bool					m_ShadowEffect;		// flags "shadow" effect

	HFONT					m_hFont;
	HDC						m_hDC;
	POINT					m_LastWritePos;

	CFontTexture*			m_pCurrentTexture;
	LPDIRECT3DDEVICE9		m_pDevice;

	bool					m_Initialized;
};


} // namespace UI
} // namespace Graphics
} // namespace WT
