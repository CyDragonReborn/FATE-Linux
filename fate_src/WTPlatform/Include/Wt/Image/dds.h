#pragma once

/***************************************************************************************
 *
 * dds.h
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 ***************************************************************************************/

// This header defines constants and structures that are useful when parsing
// DDS files.  DDS files were originally designed to use several structures
// and constants that are native to DirectDraw and are defined in ddraw.h,
// such as DDSURFACEDESC2 and DDSCAPS2.  This file defines similar
// (compatible) constants and structures so that one can use DDS files
// without needing to include ddraw.h.

// Radu M: Copied from the XDK. DDS_HEADER is essentially a DDSURFACEDESC2. 
//		this redefinition removes dependency on DX header files.

#include "WT/System/Platform.h"

namespace WT
{
namespace Graphics
{

struct DDS_PIXELFORMAT
{
    uint32 dwSize;
    uint32 dwFlags;
    uint32 dwFourCC;
    uint32 dwRGBBitCount;
    uint32 dwRBitMask;
    uint32 dwGBitMask;
    uint32 dwBBitMask;
    uint32 dwABitMask;

	bool	operator ==( DDS_PIXELFORMAT pf ) const 
	{ 
		//Check everything except for dwRGBBitCount
		return 
			dwSize == pf.dwSize &&
			dwFlags == pf.dwFlags &&
			dwFourCC == pf.dwFourCC &&
			dwRBitMask == pf.dwRBitMask &&
			dwGBitMask == pf.dwGBitMask &&
			dwBBitMask == pf.dwBBitMask &&
			dwABitMask == pf.dwABitMask;
	}
};

#define DDS_FOURCC		0x00000004  // DDPF_FOURCC
#define DDS_RGB			0x00000040  // DDPF_RGB
#define DDS_ALPHAPIXELS 0x00000001  // DDPF_ALPHAPIXELS
#define DDS_ALPHA	    0x00000002  // DDPF_ALPHA
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
	((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |       \
	((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */

//
// Old Xbox dds files use these constants
#define DDSFMT_DXT1_X       0x0000000C
#define DDSFMT_DXT2_X       0x0000000E
#define DDSFMT_DXT4_X       0x0000000F

const DDS_PIXELFORMAT DDSPF_DXT1 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D', 'X', 'T', '1'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT2 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D', 'X', 'T', '2'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT3 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D', 'X', 'T', '3'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT4 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D', 'X', 'T', '4'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT5 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D', 'X', 'T', '5'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT1_X =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, DDSFMT_DXT1_X, 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT2_X =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, DDSFMT_DXT2_X, 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT4_X =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, DDSFMT_DXT4_X, 0, 0, 0, 0, 0 };

#define DDS_HEADER_FLAGS_TEXTURE    0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
#define DDS_HEADER_FLAGS_MIPMAP     0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME     0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH      0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE 0x00080000  // DDSD_LINEARSIZE

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP			  0x00000200 // DDSCAPS2_CUBEMAP
#define DDS_CUBEMAP_POSITIVEX 0x00000400 // DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000800 // DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001000 // DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002000 // DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004000 // DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008000 // DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME


struct DDS_HEADER
{
	uint32 dwMagic;	// this is not part of the surfacedesc2 struct, but part of the DDS file format
    uint32 dwSize;
    uint32 dwHeaderFlags;
    uint32 dwHeight;
    uint32 dwWidth;
    uint32 dwPitchOrLinearSize;
    uint32 dwDepth; // only if DDS_HEADER_FLAGS_VOLUME is set in dwHeaderFlags
    uint32 dwMipMapCount;
    uint32 dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32 dwSurfaceFlags;
    uint32 dwCubemapFlags;
    uint32 dwReserved2[3];
};

} //namespace Graphics
} //namespace WT
