/*
	model.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/9/2004

*/

#ifndef _MODELTEMPLATE_H
#define _MODELTEMPLATE_H



#include "../MEDIA/materialconstants.h"

#include "../OBJECTS/datapoint.h"
#include "../OBJECTS/datalight.h"
#include "../OBJECTS/path.h"

// forward declarations
class	CMaterial;
class	CRefManager;
class	CFaceList;

/// loads information for a model, but DOES NOT CREATE THE GEOMETRY!

class CModelTemplate
{
public:

			CModelTemplate( CRefManager& pRefManager );		// pointer to the reference manager, for tracking submaterials
			
			~CModelTemplate( void );

	void	Load( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
				  std::string FileName,			// path to the mesh file
				  std::string TexturePath,		// path to the textures (if "" is supplied, will use the mesh path)
				  bool LocalSpace,				// ignore worldspace positioning in the file?
				  bool LoadTextures,			// should the mesh load and apply textures
				  bool FlipNormals = kFalse );	// flip the normals at load time?

	bool	RayCollision( const D3DXVECTOR3& Start,		// start of collision ray
						  const D3DXVECTOR3& End,		// end of collision ray
						  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
						  D3DXVECTOR3& ImpactNormal );	// impact normal to be filled

	bool	SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
							 const D3DXVECTOR3& End,		// end of collision ray
							 float32 Radius,				// radius of sphere to check along collision ray
							 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
							 D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							 D3DXVECTOR3& ImpactNormal );	// impact normal to be filled

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	CFaceList*	FaceList( void )				{	return m_pCollisionList;		};

	/// check for the existence of the named material on this mesh
	bool		HasMaterial( const std::string& Name );	// name of material to check existence of

	uint32		GetMaterials( void )			{	return m_MaterialNames.size();	};

	CMaterial*	GetMaterial( uint32 Index );		// index of material to retrieve
	
	void		SetMaterial( uint32 Index,				// index of material to overwrite
							 CMaterial* pMaterial );	// material to replace it with

	std::string	GetMaterialName( uint32 Index )	{	return m_MaterialNames[Index];	};

	bool		IsTextured( void )				{	return m_IsTextured;		};
	bool		HasAlpha( void )				{ 	return m_HasAlpha;			};
	bool		HasReflection( void );

	uint32		GetPathCount( void )			{	return m_Paths.size();		};
	uint32		GetPointCount( void )			{	return m_Points.size();		};
	uint32		GetLightCount( void )			{	return m_Lights.size();		};

	CPath&		GetPath( uint32 Index );	// index of path to retrieve
	CDataPoint&	GetPoint( uint32 Index );	// index of point to retrieve
	CDataLight&	GetLight( uint32 Index );	// index of light to retrieve

	const D3DXVECTOR3&		GetMinBounds( void )	{	return m_LocalMinBounds;	};
	const D3DXVECTOR3&		GetMaxBounds( void )	{	return m_LocalMaxBounds;	};

	bool	GetBoundsOfMaterial( const std::string& Name,	// name of the material to check for
								 D3DXVECTOR3* Min,			// min bounds to fill
								 D3DXVECTOR3* Max );		// max bounds to fill

	const std::vector <D3DXVECTOR3>&	Vertices( void ) const			{	return m_Vertices;				};
	const std::vector <uint32>&			VertexReferences( void ) const	{	return m_VertexReferences;		};
	const std::vector <D3DXVECTOR3>&	Normals( void ) const			{	return m_Normals;				};
	const std::vector <uint32>&			NormalReferences( void ) const	{	return m_NormalReferences;		};
	const std::vector <D3DXVECTOR4>&	Triangles( void ) const			{	return m_Triangles;				};
	const std::vector <uint32>&			UVReferences( void ) const		{	return m_UVReferences;			};
	const std::vector <D3DXVECTOR3>&	UVs( void ) const				{	return m_UVs;					};

	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////

	/// set a user data value on a specific material id
	void	SetUserData( uint32 Id,				// id of material to set user data on
						 int32 Value );			// value to set for user data

	/// set a user data value on a specific material name
	void	SetUserData( std::string Name,		// name of material to set user data on
						 int32 Value );			// value to set for user data

private:

	std::vector< CDataPoint* >						m_Points;
	std::vector< CPath* >							m_Paths;
	std::vector< CDataLight* >						m_Lights;

	std::vector <bool>								m_MaterialReferenced;
	std::vector <CMaterial*>						m_pMaterialReferences;

	std::vector <D3DXVECTOR3>						m_Vertices;
	std::vector <uint32>							m_VertexReferences;
	std::vector <D3DXVECTOR4>						m_Triangles;
	std::vector <uint32>							m_UVReferences;
	std::vector <D3DXVECTOR3>						m_UVs;
	std::vector <D3DXVECTOR3>						m_Normals;
	std::vector <uint32>							m_NormalReferences;
	std::vector <D3DXVECTOR3>						m_VertexColors;

	CRefManager&									m_pRefManager;

	D3DXVECTOR3										m_LocalMaxBounds;
	D3DXVECTOR3										m_LocalMinBounds;

	D3DXVECTOR3										m_MaxBounds;
	D3DXVECTOR3										m_MinBounds;
	D3DXVECTOR3										m_Bounds[8];

	uint32											m_TotalMaterials;
	uint32											m_TotalFaces;
	uint32											m_TotalSubMeshes;

	bool											m_HasAlpha;
	bool											m_IsTextured;
	bool											m_HasReflection;
	bool											m_CastsShadows;

	std::vector <std::string>						m_MaterialNames;
	std::vector <int32>								m_UserData;

	CFaceList*										m_pCollisionList;	

};
#endif
