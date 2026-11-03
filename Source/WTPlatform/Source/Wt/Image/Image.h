/***************************************************************************************
 *
 * Image.h
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 6/28/2005: Gary Hinger: created file 
 *
 ***************************************************************************************/

#pragma once

#include "d3d9.h"
#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/System/SmartPointer.h"
#include "WT/System/IO/Stream.h"
#include "WT/Image/ImageSerializer.h"

namespace WT
{
namespace Graphics
{


//The supported image formats (not the same as pixel formats).
namespace EImageFormat
{
	enum Type
	{
		Unknown, //When reading an image file, used to request autodetection
		TGA,
		PNG,
		JPG,
		WJ2,
		DDS,
		Count, //Number of image formats
	};
};

/// The format of pixels used by a CImage object.
namespace EPixelFormat
{
	enum Type
	{
		None,      ///< Indicates an uninitialized image.
		X8R8G8B8,  ///< Each pixel is stored as blue, green, red, and unused bytes.
		A8R8G8B8,  ///< Each pixel is stored as blue, green, red, and alpha bytes.
		L8,        ///< Each pixel consists of a luminance (grayscale) value.
		P8_R8G8B8, /**< Each pixel is an 8-bit index to a palette entry. Each
						palette entry consists of red, green, and blue bytes.*/
		R32F,			///< Each pixel consists of a float32 red value.
		A32B32G32R32F,	///< Each pixel consists of float32 R, G, B, and A values.
		A16B16G16R16F,	///< Each pixel consists of float16 R, G, B, and A values.
		DXT1,			///< DXT1-compressed data
		DXT3,			///< DXT3-compressed data
		DXT5,			///< DXT5-compressed data
		Count			///< The number of formats - not a valid format itself.
	};
};

/// The method used to lock a CImage object's data.
namespace EGraphicsResourceLock
{
	enum Type
	{
		ReadOnly,
		WriteOnly,
		WriteAppendDataOnly,
		ReadWrite,
		Count
	};
};

//Forward declarations
class CImage;
typedef System::SmartPointer<class CImage> HImage;
class CImageSerializer;

//CImageLock is used to hold a lock to image pixel or palette data. When the 
//CImageLock is destructed, the image will automatically be unlocked.
typedef System::SmartPointer<class CImageLock> HImageLock;
class CImageLock : public System::CObject
{
public:
	WT_CLASS_NO_CREATOR( CImageLock, System::CObject );

	uint8* GetDataPtr() const;

	~CImageLock();

private:
	//Construct an CImageLock object which owns no lock
	CImageLock( CImage& Image, uint8* pData, bool bPaletteLock );

	HImage m_hImage;
	uint8* m_pData;
	bool m_bPalette; //true if a palette lock

	friend CImage;
};

/// CImage is an interface to a 2-D bitmap with a specific width, height,
/// depth, image format, and optional palette. The user of this class can Lock()
/// a pointer to the raw data to read or manipulate it. Using the accompanying
///	CreateImage() functions, one can create a new CImage based on given
/// specifications, or by loading a file using the specified cmISerializer.
class CImage : public System::CObject
{
public:
	WT_CLASS( CImage, System::CObject );

    /// CImage description - this structure describes all of a CImage's properties.
	struct Desc
	{
        /// Width of an image.
		int32 Width;

        /// Height of an image.
        int32 Height;

		/// Depth of an image.
		int32 Depth;

        /** Number of palette entries. With image formats that do not support 
        palettes, this should be 0.*/
        int32 PaletteSize;

        /** The row pitch - the number of bytes used to represent 
        each row of image pixels.*/
        int32 RowPitch;

        /** The slice pitch - the number of bytes used to represent 
        all image pixels in a two-dimensional slice of constant depth.*/
        int32 SlicePitch;

		/// The number of bytes used to represent the pixel data.
		/// This is typically the SlicePitch multiplied by the Depth of the image.
		int32 ImagePitch;

        ///Image format.
		EPixelFormat::Type Format;
	};

	~CImage();

	/// Create a new image, returning NULL on failure. Until you call Init(), the
	/// new image will have format EPixelFormat::None, zero width, height, and depth
	/// and no palette entries.
	static HImage WTCall CreateImage();

	/// Create an image by duplicating an existing image.
	static HImage WTCall CreateImage( CImage const& I );

	/// Create a new image; return NULL on failure. The new image has the specified
	/// properties and fill color.
	static HImage WTCall CreateImage( CImage::Desc const& Desc, 
		/**< The properties to create the image with. Note that 
			any Pitch values you supply are ignored. Valid Pitch values 
			will automatically be chosen for the image. For format 
			EPixelFormat::None, specify zero width, height, depth,
			and palette size. For other formats, you must specify 
			a positive width, height, and depth and either zero or 
			nonzero palette size depending on whether the format 
			supports palettes.*/
		uint32 ARGB = 0xffffffffu
		/**< A color to fill the image with. This value is encoded 
			as (alpha << 24) | (red << 16) | (green << 8) | blue, 
			where alpha, red, green, and blue are integers in the 
			range [0,255]. For palettized images, all palette 
			entries will be filled with this color and all pixels 
			will be initialized to index 0. For grayscale images, 
			only the blue value is used.*/
		);

	/// Create and return an image from the specified stream using
	/// the specified serializer. If pSerializer is NULL, then autodetection is used.
	static HImage WTCall CreateImage( System::IO::CStream * pStream, CImageSerializer* pSerializer = NULL );

	/// Create and return an image from the specified local file using
	/// the specified serializer. If pSerializer is NULL, then autodetection is used.
	static HImage WTCall CreateImage( string const& FileName, CImageSerializer* pSerializer = NULL );

	/// Create a new image from a file object; return NULL on failure.
	/// If ImageFormat is EImageFormat::Unknown, then autodetection is used. Otherwise,
	/// the specified format is used for decoding.
	static HImage WTCall CreateImage(
		System::IO::CStream * pStream,	//< A stream to decode from.
		EImageFormat::Type ImageFormat		//< The image format, if known.
		);

	/// Create a new image from a local file; return NULL on failure.
	/// If ImageFormat is EImageFormat::Unknown, then autodetection is used. Otherwise,
	/// the specified format is used for decoding.
	static HImage WTCall CreateImage(
		string const& FileName, ///< A local file name (not a URL).
		EImageFormat::Type ImageFormat //< The image format, if known.
		);

	/// Encode an image to a stream. Returns the number of bytes written or 0 on failure.
	int32 Encode(
		System::IO::CStream* pStream, ///< A stream to encode to.
		CImageSerializer* pSerializer ///< A serializer to encode with.
		) const;

	/// Encode an image as a local file. Returns the number of bytes written or 0 on failure.
	int32 Encode(
		string const& FileName, ///< A local file name (not a URL).
		CImageSerializer* pSerializer ///< A serializer to encode with.
		) const;

	/// Encode an image to a stream. Returns the number of bytes written or 0 on failure.
	int32 Encode(
		System::IO::CStream* pStream, ///< A stream to encode to.
		EImageFormat::Type ImageFormat ///< The image format to encode.
		) const;

	/// Encode an image to a stream. Returns the number of bytes written or 0 on failure.
	int32 Encode(
		string const& FileName, ///< A stream to encode to.
		EImageFormat::Type ImageFormat ///< The image format to encode.
		) const;

    /// Set the image's properties and fill color. This function may be called
    /// repeatedly on a given Image object.
 	void Init( Desc const& desc,
                            /**< The properties to set the image to. Note that any 
								 Pitch values you supply are ignored. Valid Pitch values 
								 will automatically be chosen for the image. For format 
                                 EPixelFormat::None, specify zero width, height, depth, 
                                 and palette size. For other formats, you must specify 
                                 a positive width, height, and depth and either zero or 
                                 nonzero palette size depending on whether the format 
                                 supports palettes.*/
                          uint32 ARGB = 0xffffffff
                            /**< A color to fill the image with. This value is encoded 
                                 as (alpha << 24) | (red << 16) | (green << 8) | blue, 
                                 where alpha, red, green, and blue are integers in the 
                                 range [0,255]. For palettized images, all palette 
                                 entries will be filled with this color and all pixels 
                                 will be initialized to index 0. For grayscale images, 
                                 only the blue value is used.*/
                         );

    /// Return the image's description.
   	Desc const& GetDesc() const;

    /// Get the width of the image in pixels.
    int32 GetWidth() const;

    /// Get the height of the image in pixels.
    int32 GetHeight() const;

	/// Get the depth of the image in pixels.
	int32 GetDepth() const;

    /// Return the number of entries in the palette. If there
    /// is no palette, 0 is returned.
    int32 GetPaletteSize() const;

    /// Get the row pitch of the image. The row pitch is the number
    /// of bytes used to represent each row of image pixels. After
    /// retrieving a pointer to the image data by calling Lock(), you
    /// can point to subsequent rows of pixel data by advancing the
    /// pointer by a number of bytes equal to the row pitch.
	int32 GetRowPitch() const;

    /// Get the slice pitch of the image. The slice pitch is the number 
	///	of bytes used to represent the pixels in a 2D-slice of the image.
	int32 GetSlicePitch() const;

    /// Get the image pitch of the image. The image pitch is the
    /// number of bytes used to represent all of the image pixels.
    /// By making a successful call to Lock(), you can obtain a
    /// pointer to the image data consisting of a number of bytes 
	/// equal to the image pitch.
	int32 GetImagePitch() const;

    /// Get the format of the image.
	EPixelFormat::Type GetFormat() const;

	/// Convert current image format to that given
	bool ConvertToFormat( EPixelFormat::Type NewFormat )
	{
		// TODO - Implement this fully

		if ( NewFormat != EPixelFormat::A8R8G8B8 )
			return false;

		if ( m_Desc.Format == NewFormat )
		{
			return true;
		}
		if ( m_Desc.Format == EPixelFormat::X8R8G8B8 )
		{
			m_Desc.Format = NewFormat;
			return true;
		}

		return false;
	}

	/// Lock the image data for reading and/or writing. A Image may only
    /// have one image data lock at a time. If the image is already locked,
    /// an additional call to Lock() will immediately fail. After a successful
    /// lock is obtained, use Unlock() to relinquish access to the image data.
    /// Lock() may change the row pitch of the image; use GetRowPitch() to get
    /// the new row pitch. If the function is successful, KOk is returned and
    /// *ppData is set to point to a buffer of slice pitch bytes, beginning with
    /// the upper left pixel and followed by the pixel to the right.
	HImageLock Lock( EGraphicsResourceLock::Type LockFlags
                          /**< A flag specifying how the locked data will be used. 
                               For best performance, request only the minimum needed 
                               access. Note: this parameter is currently unused. */
                         );

    /// Check whether the image data is locked.
    bool IsLocked() const;

	/// Lock the palette data for reading and/or writing. A Image may only
    /// have one palette data lock at a time. If the palette data is already
    /// locked, an additional call to LockPalette() will immediately fail.
    /// After a successful lock is obtained, call UnlockPalette() to relinquish
    /// access to the palette data. If the function is successful, KOk is
    /// returned and *ppPalData is set to point to the first palette entry. Note: 
	/// LockPalette() can succeed even if there are zero palette entries.
	HImageLock LockPalette(EGraphicsResourceLock::Type LockFlags
                                 /**< A flag specifying how the loked data will be used. 
                                      For best performance, request only the minimum needed 
                                      access. Note: this parameter is currently unused. */
                                );

    /// Check whether the palette data is locked, without actually locking it.
    bool IsPaletteLocked() const;

	/// Get the color of the element with horizontal coordinate X, vertical coordinate Y,
	/// and depth coordinate Z. On failure, the default color 0xffffffffu is returned.
	uint32 GetPixelColor( int32 X, int32 Y, int32 Z = 0 ) const;

	/// Set the color of the element with horizontal coordinate X, vertical coordinate Y
	/// and depth coordinate Z to the specified color. The color must be exactly representable 
	/// in the current format. For example, when dealing with an image with format 
	/// EPixelFormat::L8, it is required that A = 0xff, R == G, and R == B.
	void SetPixelColor( int32 X, int32 Y, int32 Z, uint32 ARGB );

	/// Get the palette index of the element with horizontal coordinate X, vertical coordinate
	/// Y, and depth coordinate Z. On failure, -1 is returned.
	int32 GetPixelPaletteIndex( int32 X, int32 Y, int32 Z = 0 ) const;

	/// Set the palette index of the element with horizontal coordinate X, vertical coordinate
	/// Y, and depth coordinate Z to the specified value.
	void SetPixelPaletteIndex( int32 X, int32 Y, int32 Z, uint32 PaletteIndex );

	/// Locks the image and returns a device context. NULL is returned on failure. 
	/// This method is not applicable to all platforms.
	HDC LockDC();

	/// Unlocks the image and releases the device context.
	void UnlockDC();

	/// Fill the image with a color.
    void FillColor(
                               uint32 ARGB = 0xffffffffu
                               /**< A color to fill the image with. This value is encoded 
                                    as (alpha << 24) | (red << 16) | (green << 8) | blue, 
                                    where alpha, red, green, and blue are integers in the 
									range [0,255]. For palettized images, all palette 
                                    entries will be filled with this color. For grayscale 
                                    images, only the blue value is used.*/
                              );

	/// Suggest a format for D3D similar to that of this image. Note: conversions to the 
	/// other D3D formats might not be implemented in CImage, so using this D3DFORMAT is 
	/// suggested.
	static D3DFORMAT WTCall ConvertFormatToD3D( EPixelFormat::Type const& Format );

	/// Suggest an image format similar to the specified D3D format.
	static EPixelFormat::Type WTCall ConvertFormatFromD3D( D3DFORMAT Format );

	//Flags passed to CreateD3DTexture
	enum ECreateD3DTextureFlags
	{
		KCreateD3DTextureFlagsAsCubeMap		= (1 << 0), //Create a cube map instead of a volume texture of
														//depth 6 from a DDS format image. (DDS format images
														//do not specify if they represent cube maps.)
		KCreateD3DTextureFlagsAsOpacityMap  = (1 << 1)  //Fill in the alpha channel using luminance information.
	};

	//Create a D3D base texture from the image. Call SafeRelease() on the result when you are finished
	//using it. Use the default value for Format, D3DFMT_UNKNOWN, to request that a suggested D3D format 
	//be chosen for you automatically.
	LPDIRECT3DBASETEXTURE9 CreateD3DTexture( LPDIRECT3DDEVICE9 pDevice, 
		DWORD Usage = D3DUSAGE_AUTOGENMIPMAP, D3DFORMAT Format = D3DFMT_UNKNOWN, 
		D3DPOOL Pool = D3DPOOL_MANAGED, uint32 CreateFlags = 0 ) const;

protected:
	CImage();
	CImage( CImage const& );
	void Unlock();
	void UnlockPalette();

private:
    Desc m_Desc;
	std::vector<uint8> m_Data;
	std::vector<uint8> m_Palette;
	bool m_bImageDataLocked : 1;
	bool m_bPaletteDataLocked : 1;
    CRITICAL_SECTION m_Mutex; //Mutex enabling reliable locking

	HDC m_hMemoryDC;
	HBITMAP m_hBitmap;
	HGDIOBJ m_hOrigObj;
	void ReleaseDC();

	friend CImageLock;
};

//Combine the RGB of the first image and the alpha of the second image.
HImage WTCall ImageCombineRGBAlpha( CImage* pRGBImage, CImage* pOpacityImage );

//Cobmine the RGB of the first image and the alpha of the second image (computed as the luminance 
//of the second image RGB values).
HImage WTCall ImageCombineRGBLuminance( CImage* pRGBImage, CImage* pOpacityImage );

} //namespace Graphics
} //namespace WT
