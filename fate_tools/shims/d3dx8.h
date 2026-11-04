/*
 * d3dx8.h  --  replacement for the parts of D3DX8 that Fate uses.
 *
 *   math          : header-only (d3dx8math.h)
 *   textures      : implemented in d3dx8_shim.cpp (stb_image for PNG/JPG/BMP/TGA)
 *   meshes, render-to-surface, render-to-envmap, cube-from-file, tangents:
 *                 declared and STUBBED (return D3DERR_NOTAVAILABLE) until the
 *                 game's usage of them is known.
 */
#ifndef D3DX8_SHIM_H
#define D3DX8_SHIM_H

#include <windows.h>
#include <d3d8.h>
#include "d3dx8math.h"

#ifdef __cplusplus
extern "C++" {
#endif

#define D3DX_DEFAULT ((UINT)-1)

#define D3DX_FILTER_NONE      (1 << 0)
#define D3DX_FILTER_POINT     (2 << 0)
#define D3DX_FILTER_LINEAR    (3 << 0)
#define D3DX_FILTER_TRIANGLE  (4 << 0)
#define D3DX_FILTER_BOX       (5 << 0)
#define D3DX_FILTER_MIRROR_U  (1 << 16)
#define D3DX_FILTER_MIRROR_V  (2 << 16)
#define D3DX_FILTER_MIRROR_W  (4 << 16)
#define D3DX_FILTER_MIRROR    (7 << 16)
#define D3DX_FILTER_DITHER    (1 << 19)

#define D3DX_COMP_TANGENT_NONE 0xFFFFFFFF

typedef enum _D3DXIMAGE_FILEFORMAT {
    D3DXIFF_BMP = 0, D3DXIFF_JPG = 1, D3DXIFF_TGA = 2, D3DXIFF_PNG = 3,
    D3DXIFF_DDS = 4, D3DXIFF_PPM = 5, D3DXIFF_DIB = 6, D3DXIFF_FORCE_DWORD = 0x7fffffff
} D3DXIMAGE_FILEFORMAT;

typedef struct _D3DXIMAGE_INFO {
    UINT Width, Height, Depth, MipLevels;
    D3DFORMAT Format;
    D3DRESOURCETYPE ResourceType;
    D3DXIMAGE_FILEFORMAT ImageFileFormat;
} D3DXIMAGE_INFO;

typedef VOID (*LPD3DXFILL2D)(D3DXVECTOR4 *pOut, D3DXVECTOR2 *pTexCoord, D3DXVECTOR2 *pTexelSize, LPVOID pData);

/* mesh flags (values as in D3DX8) */
#define D3DXMESH_32BIT            0x001
#define D3DXMESH_DONOTCLIP        0x002
#define D3DXMESH_POINTS           0x004
#define D3DXMESH_RTPATCHES        0x008
#define D3DXMESH_NPATCHES         0x4000
#define D3DXMESH_VB_SYSTEMMEM     0x010
#define D3DXMESH_VB_MANAGED       0x020
#define D3DXMESH_VB_WRITEONLY     0x040
#define D3DXMESH_VB_DYNAMIC       0x080
#define D3DXMESH_VB_SOFTWAREPROCESSING 0x8000
#define D3DXMESH_IB_SYSTEMMEM     0x100
#define D3DXMESH_IB_MANAGED       0x200
#define D3DXMESH_IB_WRITEONLY     0x400
#define D3DXMESH_IB_DYNAMIC       0x800
#define D3DXMESH_SYSTEMMEM        0x110
#define D3DXMESH_MANAGED          0x220
#define D3DXMESH_WRITEONLY        0x440
#define D3DXMESH_DYNAMIC          0x880
#define D3DXMESHOPT_COMPACT       0x01000000
#define D3DXMESHOPT_ATTRSORT      0x02000000
#define D3DXMESHOPT_VERTEXCACHE   0x04000000
#define D3DXMESHOPT_STRIPREORDER  0x08000000
#define D3DXMESHOPT_IGNOREVERTS   0x10000000
#define D3DXMESHOPT_SHAREVB       0x1000

typedef struct _D3DXATTRIBUTERANGE {
    DWORD AttribId, FaceStart, FaceCount, VertexStart, VertexCount;
} D3DXATTRIBUTERANGE;
typedef D3DXATTRIBUTERANGE* LPD3DXATTRIBUTERANGE;

/* Interfaces intentionally left incomplete: any code that calls a method on
 * them fails to compile, which tells us exactly what needs implementing. */
struct ID3DXMesh;           typedef ID3DXMesh*           LPD3DXMESH;
struct ID3DXBuffer;         typedef ID3DXBuffer*         LPD3DXBUFFER;
struct ID3DXRenderToSurface; typedef ID3DXRenderToSurface* LPD3DXRENDERTOSURFACE;
struct ID3DXRenderToEnvMap;  typedef ID3DXRenderToEnvMap*  LPD3DXRENDERTOENVMAP;

/* ------------------------------------------------------------ textures */
HRESULT D3DXCreateTexture(LPDIRECT3DDEVICE8 pDevice, UINT Width, UINT Height, UINT MipLevels, DWORD Usage,
                          D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DTEXTURE8 *ppTexture);
HRESULT D3DXCreateCubeTexture(LPDIRECT3DDEVICE8 pDevice, UINT Size, UINT MipLevels, DWORD Usage,
                              D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DCUBETEXTURE8 *ppCubeTexture);
HRESULT D3DXCreateTextureFromFileEx(LPDIRECT3DDEVICE8 pDevice, LPCSTR pSrcFile, UINT Width, UINT Height,
                                    UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter,
                                    DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo,
                                    PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE8 *ppTexture);
HRESULT D3DXCreateCubeTextureFromFileEx(LPDIRECT3DDEVICE8 pDevice, LPCSTR pSrcFile, UINT Size, UINT MipLevels,
                                        DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter,
                                        D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette,
                                        LPDIRECT3DCUBETEXTURE8 *ppCubeTexture);
HRESULT D3DXFillTexture(LPDIRECT3DTEXTURE8 pTexture, LPD3DXFILL2D pFunction, LPVOID pData);
HRESULT D3DXFilterTexture(LPDIRECT3DBASETEXTURE8 pBaseTexture, CONST PALETTEENTRY *pPalette, UINT SrcLevel, DWORD Filter);
HRESULT D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE8 pDestSurface, CONST PALETTEENTRY *pDestPalette, CONST RECT *pDestRect,
                                   LPDIRECT3DSURFACE8 pSrcSurface, CONST PALETTEENTRY *pSrcPalette, CONST RECT *pSrcRect,
                                   DWORD Filter, D3DCOLOR ColorKey);
HRESULT D3DXSaveSurfaceToFile(LPCSTR pDestFile, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DSURFACE8 pSrcSurface,
                              CONST PALETTEENTRY *pSrcPalette, CONST RECT *pSrcRect);
HRESULT D3DXSaveTextureToFile(LPCSTR pDestFile, D3DXIMAGE_FILEFORMAT DestFormat, LPDIRECT3DBASETEXTURE8 pSrcTexture,
                              CONST PALETTEENTRY *pSrcPalette);

/* ----------------------------------------------------- stubs (TODO) */
HRESULT D3DXCreateMeshFVF(DWORD NumFaces, DWORD NumVertices, DWORD Options, DWORD FVF,
                          LPDIRECT3DDEVICE8 pD3DDevice, LPD3DXMESH *ppMesh);
HRESULT D3DXCreateRenderToSurface(LPDIRECT3DDEVICE8 pDevice, UINT Width, UINT Height, D3DFORMAT Format,
                                  BOOL DepthStencil, D3DFORMAT DepthStencilFormat, LPD3DXRENDERTOSURFACE *ppRenderToSurface);
HRESULT D3DXCreateRenderToEnvMap(LPDIRECT3DDEVICE8 pDevice, UINT Size, D3DFORMAT Format, BOOL DepthStencil,
                                 D3DFORMAT DepthStencilFormat, LPD3DXRENDERTOENVMAP *ppRenderToEnvMap);
HRESULT D3DXComputeTangent(LPD3DXMESH InMesh, DWORD TexStage, LPD3DXMESH OutMesh, DWORD TexStageUVec,
                           DWORD TexStageVVec, DWORD Wrap, DWORD *pAdjacency);

#ifdef __cplusplus
}
#endif
#endif /* D3DX8_SHIM_H */
