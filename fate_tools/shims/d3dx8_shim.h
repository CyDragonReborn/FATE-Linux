#ifndef D3DX8_SHIM_H
#define D3DX8_SHIM_H
#pragma once
#include <windows.h>
#include <math.h>
#include "d3d8_headers/d3d8.h"

struct ID3DXBuffer : public IUnknown {
    STDMETHOD_(LPVOID, GetBufferPointer)(THIS) = 0;
    STDMETHOD_(DWORD, GetBufferSize)(THIS) = 0;
};
typedef struct ID3DXBuffer* LPD3DXBUFFER;

typedef struct _D3DXATTRIBUTERANGE {
    DWORD AttribId, FaceStart, FaceCount, VertexStart, VertexCount;
} D3DXATTRIBUTERANGE, *LPD3DXATTRIBUTERANGE;

struct ID3DXBaseMesh : public IUnknown {
    STDMETHOD(DrawSubset)(THIS_ DWORD AttribId) = 0;
    STDMETHOD_(DWORD, GetNumFaces)(THIS) = 0;
    STDMETHOD_(DWORD, GetNumVertices)(THIS) = 0;
    STDMETHOD_(DWORD, GetFVF)(THIS) = 0;
    STDMETHOD(GetDeclaration)(THIS_ DWORD Declaration[24]) = 0;
    STDMETHOD_(DWORD, GetOptions)(THIS) = 0;
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE8* ppDevice) = 0;
    STDMETHOD(CloneMeshFVF)(THIS_ DWORD Options, DWORD FVF, LPDIRECT3DDEVICE8 pD3DDevice, struct ID3DXMesh** ppCloneMesh) = 0;
    STDMETHOD(CloneMesh)(THIS_ DWORD Options, const DWORD* pDeclaration, LPDIRECT3DDEVICE8 pD3DDevice, struct ID3DXMesh** ppCloneMesh) = 0;
    STDMETHOD(GetVertexBuffer)(THIS_ LPDIRECT3DVERTEXBUFFER8* ppVB) = 0;
    STDMETHOD(GetIndexBuffer)(THIS_ LPDIRECT3DINDEXBUFFER8* ppIB) = 0;
    STDMETHOD(LockVertexBuffer)(THIS_ DWORD Flags, BYTE** ppData) = 0;
    STDMETHOD(UnlockVertexBuffer)(THIS) = 0;
    STDMETHOD(LockIndexBuffer)(THIS_ DWORD Flags, BYTE** ppData) = 0;
    STDMETHOD(UnlockIndexBuffer)(THIS) = 0;
    STDMETHOD(GetAttributeTable)(THIS_ D3DXATTRIBUTERANGE* pAttribTable, DWORD* pAttribTableSize) = 0;
    STDMETHOD(ConvertPointRepsToAdjacency)(THIS_ const DWORD* pPRep, DWORD* pAdjacency) = 0;
    STDMETHOD(ConvertAdjacencyToPointReps)(THIS_ const DWORD* pAdjacency, DWORD* pPRep) = 0;
    STDMETHOD(GenerateAdjacency)(THIS_ FLOAT Epsilon, DWORD* pAdjacency) = 0;
    STDMETHOD(UpdateSemantics)(THIS_ DWORD Declaration[24]) = 0;
};

struct ID3DXMesh : public ID3DXBaseMesh {
    STDMETHOD(LockAttributeBuffer)(THIS_ DWORD Flags, DWORD** ppData) = 0;
    STDMETHOD(UnlockAttributeBuffer)(THIS) = 0;
    STDMETHOD(Optimize)(THIS_ DWORD Flags, const DWORD* pAdjacencyIn, DWORD* pAdjacencyOut, DWORD* pFaceRemap, LPD3DXBUFFER* ppVertexRemap, ID3DXMesh** ppOptMesh) = 0;
    STDMETHOD(OptimizeInplace)(THIS_ DWORD Flags, const DWORD* pAdjacencyIn, DWORD* pAdjacencyOut, DWORD* pFaceRemap, LPD3DXBUFFER* ppVertexRemap) = 0;
    STDMETHOD(SetAttributeTable)(THIS_ const D3DXATTRIBUTERANGE* pAttribTable, DWORD cAttribTableSize) = 0;
};
typedef struct ID3DXMesh* LPD3DXMESH;

struct ID3DXRenderToSurface : public IUnknown {
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE8* ppDevice) = 0;
    STDMETHOD(GetDesc)(THIS_ void* pDesc) = 0;
    STDMETHOD(BeginScene)(THIS_ LPDIRECT3DSURFACE8 pSurface, const D3DVIEWPORT8* pViewport) = 0;
    STDMETHOD(EndScene)(THIS) = 0;
    STDMETHOD(OnLostDevice)(THIS) = 0;
    STDMETHOD(OnResetDevice)(THIS) = 0;
};
typedef struct ID3DXRenderToSurface* LPD3DXRENDERTOSURFACE;

#ifndef D3DX_DEFAULT
#define D3DX_DEFAULT ((UINT)-1)
#endif
#ifndef D3DX_DEFAULT_NONPOW2
#define D3DX_DEFAULT_NONPOW2 ((UINT)-2)
#endif

typedef enum _D3DXIMAGE_FILEFORMAT {
    D3DXIFF_BMP = 0, D3DXIFF_JPG = 1, D3DXIFF_TGA = 2, D3DXIFF_PNG = 3,
    D3DXIFF_DDS = 4, D3DXIFF_PPM = 5, D3DXIFF_DIB = 6, D3DXIFF_FORCE_DWORD = 0x7fffffff
} D3DXIMAGE_FILEFORMAT;

typedef struct _D3DXIMAGE_INFO {
    UINT Width, Height, Depth, MipLevels;
    D3DFORMAT Format; D3DRESOURCETYPE ResourceType; D3DXIMAGE_FILEFORMAT ImageFileFormat;
} D3DXIMAGE_INFO;

#define D3DX_FILTER_NONE (1 << 0)
#define D3DX_FILTER_POINT (2 << 0)
#define D3DX_FILTER_LINEAR (3 << 0)
#define D3DX_FILTER_TRIANGLE (4 << 0)
#define D3DX_FILTER_BOX (5 << 0)

#define D3DXMESH_SYSTEMMEM 0x010
#define D3DXMESH_MANAGED 0x020
#define D3DXMESH_WRITEONLY 0x008
#define D3DXMESH_32BIT 0x001

enum _D3DXMESHOPT {
    D3DXMESHOPT_COMPACT = 0x01, D3DXMESHOPT_ATTRSORT = 0x02, D3DXMESHOPT_VERTEXCACHE = 0x04,
    D3DXMESHOPT_STRIPREORDER = 0x08, D3DXMESHOPT_IGNORE = 0x10, D3DXMESHOPT_SHAREGEOMETRY = 0x20
};

#ifndef ID3DXRenderToEnvMap_DEFINED
#define ID3DXRenderToEnvMap_DEFINED
struct ID3DXRenderToEnvMap : public IUnknown {
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE8* ppDevice) = 0;
    STDMETHOD(GetDesc)(THIS_ void* pDesc) = 0;
    STDMETHOD(BeginCube)(THIS_ LPDIRECT3DCUBETEXTURE8 pCubeTexture) = 0;
    STDMETHOD(BeginSphere)(THIS_ LPDIRECT3DTEXTURE8 pTex) = 0;
    STDMETHOD(BeginHemisphere)(THIS_ LPDIRECT3DTEXTURE8 pTexZPos, LPDIRECT3DTEXTURE8 pTexZNeg) = 0;
    STDMETHOD(BeginParaboloid)(THIS_ LPDIRECT3DTEXTURE8 pTexZPos, LPDIRECT3DTEXTURE8 pTexZNeg) = 0;
    STDMETHOD(Face)(THIS_ D3DCUBEMAP_FACES Face) = 0;
    STDMETHOD(End)(THIS) = 0;
    STDMETHOD(OnLostDevice)(THIS) = 0;
    STDMETHOD(OnResetDevice)(THIS) = 0;
};
typedef struct ID3DXRenderToEnvMap* LPD3DXRENDERTOENVMAP;
#endif

#include "d3dx8math.h"

typedef void (*LPD3DXFILL2D)(D3DXVECTOR4* pOut, D3DXVECTOR2* pTexCoord, D3DXVECTOR2* pTexScale, void* pData);

#ifdef __cplusplus
extern "C" {
#endif

HRESULT WINAPI D3DXCreateTexture(LPDIRECT3DDEVICE8 pDevice, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DTEXTURE8* ppTexture);
HRESULT WINAPI D3DXCreateTextureFromFileInMemory(LPDIRECT3DDEVICE8 pDevice, LPCVOID pSrcData, UINT SrcDataSize, LPDIRECT3DTEXTURE8* ppTexture);
HRESULT WINAPI D3DXCreateTextureFromFileInMemoryEx(LPDIRECT3DDEVICE8 pDevice, LPCVOID pSrcData, UINT SrcDataSize, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO* pSrcInfo, PALETTEENTRY* pPalette, LPDIRECT3DTEXTURE8* ppTexture);
HRESULT WINAPI D3DXCreateTextureFromFileExA(LPDIRECT3DDEVICE8 pDevice, LPCSTR pSrcFile, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO* pSrcInfo, PALETTEENTRY* pPalette, LPDIRECT3DTEXTURE8* ppTexture);
#ifndef D3DXCreateTextureFromFileEx
#define D3DXCreateTextureFromFileEx D3DXCreateTextureFromFileExA
#endif

HRESULT WINAPI D3DXCreateCubeTexture(LPDIRECT3DDEVICE8 pDevice, UINT Size, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DCUBETEXTURE8* ppCubeTexture);
HRESULT WINAPI D3DXCreateCubeTextureFromFileExA(LPDIRECT3DDEVICE8 pDevice, LPCSTR pSrcFile, UINT Size, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO* pSrcInfo, PALETTEENTRY* pPalette, LPDIRECT3DCUBETEXTURE8* ppCubeTexture);
#ifndef D3DXCreateCubeTextureFromFileEx
#define D3DXCreateCubeTextureFromFileEx D3DXCreateCubeTextureFromFileExA
#endif

HRESULT WINAPI D3DXSaveSurfaceToFileA(LPCSTR pDestFile, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DSURFACE8 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect);
#ifndef D3DXSaveSurfaceToFile
#define D3DXSaveSurfaceToFile D3DXSaveSurfaceToFileA
#endif

HRESULT WINAPI D3DXSaveTextureToFileA(LPCSTR pDestFile, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DBASETEXTURE8 pSrcTexture, const PALETTEENTRY* pSrcPalette);
#ifndef D3DXSaveTextureToFile
#define D3DXSaveTextureToFile D3DXSaveTextureToFileA
#endif

HRESULT WINAPI D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE8 pDestSurface, const PALETTEENTRY* pDestPalette, const RECT* pDestRect, LPDIRECT3DSURFACE8 pSrcSurface, const PALETTEENTRY* pSrcPalette, const RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey);
HRESULT WINAPI D3DXFillTexture(LPDIRECT3DTEXTURE8 pTexture, LPD3DXFILL2D pFunction, void* pData);
HRESULT WINAPI D3DXFilterTexture(LPDIRECT3DBASETEXTURE8 pBaseTexture, const PALETTEENTRY* pPalette, UINT SrcLevel, DWORD Filter);
HRESULT WINAPI D3DXCreateRenderToEnvMap(LPDIRECT3DDEVICE8 pDevice, UINT Size, D3DFORMAT Format, BOOL DepthBuffer, D3DFORMAT DepthFormat, LPD3DXRENDERTOENVMAP* ppRenderToEnvMap);
HRESULT WINAPI D3DXCreateRenderToSurface(LPDIRECT3DDEVICE8 pDevice, UINT Width, UINT Height, D3DFORMAT Format, BOOL DepthBuffer, D3DFORMAT DepthFormat, LPD3DXRENDERTOSURFACE* ppRenderToSurface);
HRESULT WINAPI D3DXCreateMeshFVF(DWORD NumFaces, DWORD NumVertices, DWORD Options, DWORD FVF, LPDIRECT3DDEVICE8 pD3DDevice, LPD3DXMESH* ppMesh);


#define D3DX_COMP_TANGENT_NONE 0

HRESULT WINAPI D3DXComputeTangent(LPD3DXMESH pMeshIn, DWORD TexStageIn, LPD3DXMESH pMeshOut, DWORD TexStageOut, DWORD Options, BOOL ComputeBinormals, const DWORD* pAdjacency);

#ifdef __cplusplus
}
#endif
#endif // D3DX8_SHIM_H
