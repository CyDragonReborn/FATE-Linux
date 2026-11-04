/*
	submodelsegment.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _SUBMODELSEGMENT_H
#define _SUBMODELSEGMENT_H

#include "d3dx8_shim.h"
#include "submodel.h"
//class	CSubModel;
class	CMaterial;
class	CRefManager;
class	CSettings;

/// submodelsegments are essentially the separate portions of the index-buffer
/// of a submodel (the pieces that have different materials)
/// we keep them separate for clarity, and so that we can
/// set specific user data on these sub-pieces (i.e. disabling
/// sub materials from the render)
class CSubModelSegment
{
public:

			CSubModelSegment( CRefManager& pRefManager,	// pointer to the reference manager, for tracking submaterials
							  CSettings& pSettings );		// pointer to the settings manager, for keeping track of card/machine capabilities

			~CSubModelSegment( void );


	void	SetMaterial( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
						 std::string TexName,			// path to texture
						 bool Colorkey = false );		// is texture colorkeyed?

	void	SetMaterial( CMaterial* pMaterial );		// material to add

	void	GenerateMesh( LPDIRECT3DINDEXBUFFER8 pIB,			// buffer to add indices to
						  uint32& CurrentIndex,					// index within the buffer
						  std::vector <D3DXVECTOR3>& Vertices,	// list of Vertices in the mesh
						  std::vector <D3DXVECTOR4>& Triangles,	// list of Triangle indices for the mesh
						  std::vector <uint32>& VertexIndices,	// reference map of Triangle indices, to actual Vertices
						  uint32 MaterialID,					// global MaterialID id for this submesh
						  int32 MaterialType,					// type of MaterialID
						  bool DoubleSided );					// is the mesh double-sided?

	void	UnSet( LPDIRECT3DDEVICE8 pD3DDevice );				// direct3d device

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice );				// direct3d device


	void	RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	int32	MaterialID( void )					{	return m_MaterialID;		};
	int32	MaterialType( void )				{	return m_MaterialType;		};

	uint32	TotalFaces( void )					{	return m_TotalFaces;		};
	void	SetTotalFaces( uint32 Faces )		{	m_TotalFaces = Faces;		};

	void	RemoveMaterial( void );

	CMaterial* GetMaterial( void )				{	return m_pMaterial;		};

	const D3DXVECTOR3& MaxBounds( void )		{	return m_MaxBounds;		};

	const D3DXVECTOR3& MinBounds( void )		{	return m_MinBounds;		};

	uint32	FirstVertIndex( void )				{	return m_FirstIndex;	};
	uint32	LastVertIndex( void )				{	return m_LastIndex;	};

	void	SetFirstVertIndex( uint32 Index )	{	m_FirstIndex = Index;	};
	void	SetLastVertIndex( uint32 Index )	{	m_LastIndex = Index;	};

	void	SetIBIndex( uint32 Index )			{	m_IBIndex = Index;		};

	uint32	IBIndex( void )						{	return m_IBIndex;	};

	void			SetVisible( bool State )		{	m_Visible = State;	};

	bool			Visible( void )					{	return m_Visible;	};

private:
	bool					m_Visible;

	CRefManager&			m_pRefManager;
	CSettings&				m_pSettings;


	D3DXVECTOR3				m_MaxBounds;	// maximum local Bounds of the mesh
	D3DXVECTOR3				m_MinBounds;	// minimum local Bounds of the mesh
	CMaterial*				m_pMaterial;	// reference to the material used on this mesh
	uint32					m_TotalFaces;	// total faces in this mesh

	uint32					m_IBIndex;		// index within the parent IB

	int32					m_MaterialID;		// id of the material applied to this
	int32					m_MaterialType;		// type of material (standard, alpha, illumination, etc.)

	uint32					m_FirstIndex;		// first vert index in the vert buffer that we reference
	uint32					m_LastIndex;		// last vert index in the vert buffer that we reference

};

#endif