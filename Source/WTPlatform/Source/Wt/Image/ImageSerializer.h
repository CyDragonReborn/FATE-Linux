/***************************************************************************************
 *
 * ImageSerializer.h
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 7/22/2005: Gary Hinger: created file 
 *
 ***************************************************************************************/

#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/System/IO/Stream.h"

namespace WT
{

namespace Graphics
{
	class CImage;
}

template<> struct CCanCastPointer<Graphics::CImage, System::CReferenceCounted const> {typedef TrueType Result;};

namespace Graphics
{

/// CImageSerializer is an interface to a deserializer/serializer designed for 
/// a specific image file format. Interfaces derived from CImageSerializer may offer 
/// special decoding or encoding options related to their specific file formats.
typedef System::SmartPointer<class CImageSerializer> HImageSerializer;
class CImageSerializer : public System::CObject
{
public:
	WT_ABSTRACT_CLASS( CImageSerializer, System::CObject );

	virtual void Decode( System::IO::CStream* pStream, CImage* pImage ) const = 0;
	virtual int32 Encode( System::IO::CStream* pStream, CImage const* pImage ) const = 0;

	//Validate that the stream is in a format that can be read by this serializer.
	virtual bool Validate( System::IO::CStream* pStream ) const = 0;
};

typedef System::SmartPointer<class CImageSerializerTGA> HImageSerializerTGA;
class CImageSerializerTGA : public CImageSerializer
{
public:
	WT_CLASS( CImageSerializerTGA, CImageSerializer );

	virtual void Decode( System::IO::CStream* pStream, CImage* pImage ) const;
	virtual int32 Encode( System::IO::CStream* pStream, CImage const* pImage ) const;
	virtual bool Validate( System::IO::CStream* pStream ) const;
};

typedef System::SmartPointer<class CImageSerializerPNG> HImageSerializerPNG;
class CImageSerializerPNG : public CImageSerializer
{
public:
	WT_CLASS( CImageSerializerPNG, CImageSerializer );

	virtual void Decode( System::IO::CStream* pStream, CImage* pImage ) const;
	virtual int32 Encode( System::IO::CStream* pStream, CImage const* pImage ) const;
	virtual bool Validate( System::IO::CStream* pStream ) const;
};

typedef System::SmartPointer<class CImageSerializerJPG> HImageSerializerJPG;
class CImageSerializerJPG : public CImageSerializer
{
public:
	WT_CLASS( CImageSerializerJPG, CImageSerializer );

	CImageSerializerJPG();

	virtual void Decode( System::IO::CStream* pStream, CImage* pImage ) const;
	virtual int32 Encode( System::IO::CStream* pStream, CImage const* pImage ) const;
	virtual bool Validate( System::IO::CStream* pStream ) const;

    /// Get the quality of the JPEG image compression. The result will range from    
    /// 0.0 (lowest quality) through 1.0 (highest quality).
    float32 GetQuality() const;

    /// Set the quality of the JPEG image compression. Acceptable values range from
    /// 0.0 (lowest quality) through 1.0 (highest quality). The default quality
    /// for a new CImageSerializerJPG is 1.0. 
    bool SetQuality( float32 Quality );

private:
    float32 m_Quality; //Encoding quality
};

typedef System::SmartPointer<class CImageSerializerWJ2> HImageSerializerWJ2;
class CImageSerializerWJ2 : public CImageSerializer
{
public:
	WT_CLASS( CImageSerializerWJ2, CImageSerializer );

	CImageSerializerWJ2();

	virtual void Decode( System::IO::CStream* pStream, CImage* pImage ) const;
	virtual int32 Encode( System::IO::CStream* pStream, CImage const* pImage ) const;
	virtual bool Validate( System::IO::CStream* pStream ) const;

    ///Gets the compression rate.
    float32 GetCompressionRate();

    ///Gets whether a particular component is to be losslessly compressed.
    bool IsComponentLossless( int32 ComponentIndex );

    ///Gets the luminance weight.
    float32 GetLuminanceWeight();

    ///Set the compression rate. The default value is 1, signifying
    ///lossless compression. Specify a non-negative value less than 1 to 
    ///guarantee a resulting size no larger than that value 
    ///multiplied by the raw size. (Note: components specially marked 
    ///for lossless compression are exempt from this size restriction.)
    //The image will never be less than required format overhead.
    bool SetCompressionRate( float32 Rate );

    ///Sets whether a particular component is to be losslessly
    ///compressed. The default value is false. These values 
    ///only apply when the compression rate is less than 1.
    bool SetComponentLossless( int32 ComponentIndex, bool bLossless );

    ///Sets the luminance weight. This value defaults to 1.0 and determines 
    ///the weight of the luminance component. Typical values range from 0.1
    ///through 10.0. Larger values lead to more accurate luminance.
    bool SetLuminanceWeight( float32 LuminanceWeight );

private:
	void DecodeJasStream( void* JasPerStream, CImage* pImage ) const;
	int32 EncodeJasStream( CImage const* pImage, void* JasPerStream ) const;

    //Encoding options
    float32 m_CompressionRate;
    bool m_bComponentsLossless[4];
    float32 m_LuminanceWeight;
};

typedef System::SmartPointer<class CImageSerializerDDS> HImageSerializerDDS;
class CImageSerializerDDS : public CImageSerializer
{
public:
	WT_CLASS( CImageSerializerDDS, CImageSerializer );

	virtual void Decode( System::IO::CStream* pStream, CImage* pImage ) const;
	virtual int32 Encode( System::IO::CStream* pStream, CImage const* pImage ) const;
	virtual bool Validate( System::IO::CStream* pStream ) const;
};

} //namespace Graphics
} //namespace WT
