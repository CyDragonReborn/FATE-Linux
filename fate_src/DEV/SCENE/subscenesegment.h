/*
	subscenesegment.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _SUBSCENESEGMENT_H
#define _SUBSCENESEGMENT_H

#include "d3dx8_shim.h"

class	CSubScene;
class	CMaterial;
class	CRefManager;
class	CSettings;

/// subscenesegments are essentially the separate index-buffers
/// of a subscene (the pieces that have different materials)
/// we keep them separate for clarity, and so that we can
/// set specific user data on these sub-pieces (i.e. disabling
/// sub materials from the render)
class CSubSceneSegment
{
public:

			CSubSceneSegment( CRefManager& pRefManager,	// reference to refmanager
							  CSettings& pSettings,		// pointer to the settings manager, for keeping track of card/machine capabilities
							  CSubScene* pParent );		// parent subscene

			~CSubSceneSegment( void );


	void	SetMaterial( CMaterial* pMaterial );		// material to add

	void	SetShadowMaterial( CMaterial* pMaterial );		// material to add

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

	void	SetShadowMap( LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device
	void	UnSetShadowMap( LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device

	int32 MaterialID( void )					{	return m_MaterialID;	};
	int32 MaterialType( void )					{	return m_MaterialType;	};

	uint32	TotalFaces( void )					{	return m_TotalFaces;		};
	void	SetTotalFaces( uint32 Faces )		{	m_TotalFaces = Faces;		};


	void	RemoveMaterial( void );
	CMaterial* GetMaterial( void )				{	return m_pMaterial;			};
	CMaterial* GetShadowMaterial( void )		{	return m_pShadowMaterial;	};

	const D3DXVECTOR3& MaxBounds( void )		{	return m_MaxBounds;		};
	const D3DXVECTOR3& MinBounds( void )		{	return m_MinBounds;		};
	
	const D3DXVECTOR3& CenterPoint( void )		{	return m_CenterPoint;	};

	uint32	FirstVertIndex( void )				{	return m_FirstIndex;	};
	uint32	LastVertIndex( void )				{	return m_LastIndex;	};

	void	SetFirstVertIndex( uint32 Index )	{	m_FirstIndex = Index;	};
	void	SetLastVertIndex( uint32 Index )	{	m_LastIndex = Index;	};

	void	SetIBIndex( uint32 Index )			{	m_IBIndex = Index;		};

	uint32	IBIndex( void )						{	return m_IBIndex;	};


	void		CalculateBounds( void )
	{
		m_CullingBounds[0] = D3DXVECTOR3( m_MinBounds.x, m_MinBounds.y, m_MinBounds.z );
		m_CullingBounds[1] = D3DXVECTOR3( m_MaxBounds.x, m_MinBounds.y, m_MinBounds.z );
		m_CullingBounds[2] = D3DXVECTOR3( m_MinBounds.x, m_MaxBounds.y, m_MinBounds.z );
		m_CullingBounds[3] = D3DXVECTOR3( m_MaxBounds.x, m_MaxBounds.y, m_MinBounds.z );
		m_CullingBounds[4] = D3DXVECTOR3( m_MinBounds.x, m_MinBounds.y, m_MaxBounds.z );
		m_CullingBounds[5] = D3DXVECTOR3( m_MaxBounds.x, m_MinBounds.y, m_MaxBounds.z );
		m_CullingBounds[6] = D3DXVECTOR3( m_MinBounds.x, m_MaxBounds.y, m_MaxBounds.z );
		m_CullingBounds[7] = D3DXVECTOR3( m_MaxBounds.x, m_MaxBounds.y, m_MaxBounds.z );

	} // CalculateBounds()

	D3DXVECTOR3*	GetCullingBounds( void )		{	return m_CullingBounds;	};

	float32			ZDistance( void )				{	return m_ZDistance;		};
	void			SetZDistance( float32 Value )	{	m_ZDistance = Value;	};

	void			SetSubIndex( uint32 Index )		{	m_SubIndex = Index;		};
	uint32			SubIndex( void )				{	return m_SubIndex;		};


	CSubScene*		Parent( void )			{	return m_pParent;	};

	void			SetStream( LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device

	void			SetVisible( bool State )		{	m_Visible = State;	};

	bool			Visible( void )					{	return m_Visible;	};

private:
	bool					m_Visible;

	CRefManager&			m_pRefManager;
	CSettings&				m_pSettings;

	CSubScene*				m_pParent;

	uint32					m_SubIndex;		// index of the parent subscene for use in sorting

	D3DXVECTOR3				m_CenterPoint;	//center point of the mesh
	D3DXVECTOR3				m_MaxBounds;	// maximum local Bounds of the mesh
	D3DXVECTOR3				m_MinBounds;	// minimum local Bounds of the mesh
	D3DXVECTOR3				m_CullingBounds[8];

	CMaterial*				m_pMaterial;	// reference to the material used on this mesh
	CMaterial*				m_pShadowMaterial;	// reference to the material used for shadow mapping on this mesh

	uint32					m_TotalFaces;	// total faces in this mesh

	uint32					m_IBIndex;		// index within the parent IB

	int32					m_MaterialID;	// id of the material applied to this
	int32					m_MaterialType;	// type of material (standard, alpha, illumination, etc.)

	uint32					m_FirstIndex;		// first vert index in the vert buffer that we reference
	uint32					m_LastIndex;		// last vert index in the vert buffer that we reference

	float32					m_ZDistance;	// distance from near frustum plane
};

#endif 