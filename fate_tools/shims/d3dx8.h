#ifndef __D3DX8_H__
#define __D3DX8_H__

#include <windows.h>
#include "d3d8.h"
#include "d3d8types.h"

typedef struct ID3DXBuffer* LPD3DXBUFFER;
typedef struct ID3DXMesh* LPD3DXMESH;
typedef struct ID3DXRenderToSurface* LPD3DXRENDERTOSURFACE;
typedef struct ID3DXRenderToEnvMap* LPD3DXRENDERTOENVMAP;

typedef struct _D3DXATTRIBUTERANGE {
    DWORD AttribId;
    DWORD FaceStart;
    DWORD FaceCount;
    DWORD VertexStart;
    DWORD VertexCount;
} D3DXATTRIBUTERANGE;

struct ID3DXBuffer : public IUnknown {
    STDMETHOD_(LPVOID, GetBufferPointer)(THIS) PURE;
    STDMETHOD_(DWORD, GetBufferSize)(THIS) PURE;
};

struct ID3DXMesh : public IUnknown {
    STDMETHOD(LockVertexBuffer)(THIS_ DWORD Flags, BYTE** ppData) PURE;
    STDMETHOD(UnlockVertexBuffer)(THIS) PURE;
    STDMETHOD(LockIndexBuffer)(THIS_ DWORD Flags, BYTE** ppData) PURE;
    STDMETHOD(UnlockIndexBuffer)(THIS) PURE;
    STDMETHOD_(DWORD, GetNumVertices)(THIS) PURE;
    STDMETHOD_(DWORD, GetNumFaces)(THIS) PURE;
    STDMETHOD_(DWORD, GetFVF)(THIS) PURE;
    STDMETHOD(GetVertexBuffer)(THIS_ LPDIRECT3DVERTEXBUFFER8* ppVB) PURE;
    STDMETHOD(GetIndexBuffer)(THIS_ LPDIRECT3DINDEXBUFFER8* ppIB) PURE;
    STDMETHOD(LockAttributeBuffer)(THIS_ DWORD Flags, DWORD** ppData) PURE;
    STDMETHOD(UnlockAttributeBuffer)(THIS) PURE;
    STDMETHOD(GenerateAdjacency)(THIS_ FLOAT Epsilon, DWORD* pAdjacency) PURE;
    STDMETHOD(OptimizeInplace)(THIS_ DWORD Flags, CONST DWORD* pAdjacencyIn, DWORD* pAdjacencyOut, DWORD* pFaceRemap, LPD3DXBUFFER* ppVertexBuffer) PURE;
    STDMETHOD(GetAttributeTable)(THIS_ D3DXATTRIBUTERANGE* pAttribTable, DWORD* pAttribTableSize) PURE;
    STDMETHOD(CloneMeshFVF)(THIS_ DWORD Options, DWORD FVF, LPDIRECT3DDEVICE8 pDevice, LPD3DXMESH* ppCloneMesh) PURE;
};

struct ID3DXRenderToSurface : public IUnknown {
    STDMETHOD(BeginScene)(THIS_ LPDIRECT3DSURFACE8 pSurface, CONST D3DVIEWPORT8* pViewport) PURE;
    STDMETHOD(EndScene)(THIS) PURE;
};

struct ID3DXRenderToEnvMap : public IUnknown {
    STDMETHOD(BeginCube)(THIS_ LPDIRECT3DCUBETEXTURE8 pCubeTexture) PURE;
    STDMETHOD(Face)(THIS_ DWORD FaceTarget, DWORD Filter) PURE;
    STDMETHOD(End)(THIS_ DWORD Filter) PURE;
    STDMETHOD(EndCube)(THIS_ DWORD Filter) PURE;
};

#endif // __D3DX8_H__
