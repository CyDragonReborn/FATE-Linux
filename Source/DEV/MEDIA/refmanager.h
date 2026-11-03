/*
	refmanager.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _REFMANAGER_H
#define _REFMANAGER_H

#include "../UTILITIES/math3d.h"

// you can disable any media type by undefining it here
#define _SUPPORT_MOVIE
#define _SUPPORT_MATERIAL
#define _SUPPORT_CUBEMAP
#define _SUPPORT_ANIMATEDMESH
#define _SUPPORT_SKINNEDMESH
#define _SUPPORT_MODEL
#define _SUPPORT_SCENE
#define _SUPPORT_SOUND
#define _SUPPORT_COLLISIONSCENE
#define _SUPPORT_FONTMETRIC

#ifdef _SUPPORT_SOUND
#include "fmod.h"
struct	FSOUND_SAMPLE;
struct	FSOUND_STREAM;
#endif

class	CSettings;

class	CMaterial;
class	CCubeMap;
class	CMovie;
class	CAnimatedGeometry;
class	CAnimatedMesh;
class	CSkinnedGeometry;
class	CSkinnedAnimation;
class	CSkinnedMesh;
class	CModel;
class	CFontMetric;
class	CScene;
class	CCollisionScene;


// these structs hold reference counts and pathnames as well as pointers to
// the contained media for all the media types tracked by the refmanager
#ifdef _SUPPORT_FONTMETRIC
struct FontMetricRef
{
	CFontMetric*	FontMetric;
	uint32			References;
	std::string		Path;
	std::string		FileName;

	FontMetricRef( void )		{	};
	~FontMetricRef( void );
};
#endif

#ifdef _SUPPORT_MOVIE
struct MovieRef
{
	CMovie*		Movie;
	uint32		References;
	std::string	Path;
	std::string	FileName;
	bool		AutoUpdate;

	MovieRef( void )		{	};
	~MovieRef( void );
};
#endif

#ifdef _SUPPORT_MATERIAL
struct MaterialRef
{
	CMaterial*	Material;
	uint32		References;
	std::string	FileName;
	std::string	Path;
	std::string	AlphaPath;
	std::string	IlluminationPath;
	std::string	NormalPath;
	std::string	SpherePath;

	MaterialRef( void )		{	};
	~MaterialRef( void );
};
#endif

#ifdef _SUPPORT_CUBEMAP

#include "cubemap.h"

struct CubeMapRef
{
	CCubeMap*	CubeMap;
	uint32		References;
	std::string	FileName;
	std::string	Path;
	std::string	RightPath;
	std::string	LeftPath;
	std::string	UpPath;
	std::string	DownPath;
	std::string	FrontPath;
	std::string	BackPath;

	CubeMapRef( void )		{	};
	~CubeMapRef( void );
};
#endif

#ifdef _SUPPORT_ANIMATEDMESH
struct AnimatedGeometryRef
{
	CAnimatedGeometry*	AnimatedGeometry;
	int32				Index;
	uint32				References;
	std::string			Path;

	AnimatedGeometryRef( void )		{	};
	~AnimatedGeometryRef( void );
};

struct AnimatedMeshRef
{
	CAnimatedMesh*		AnimatedMesh;
	int32				Index;
	uint32				References;

	AnimatedMeshRef( void )		{	};
	~AnimatedMeshRef( void );
};
#endif

#ifdef _SUPPORT_SKINNEDMESH
struct SkinnedGeometryRef
{
	CSkinnedGeometry*	SkinnedGeometry;
	int32				Index;
	uint32				References;
	std::string			Path;

	SkinnedGeometryRef( void )		{	};
	~SkinnedGeometryRef( void );
};

struct SkinnedAnimationRef
{
	CSkinnedAnimation*	SkinnedAnimation;
	int32				Index;
	uint32				References;
	std::string			Path;
	std::string			Name;
	float32				FPS;

	SkinnedAnimationRef( void )		{	};
	~SkinnedAnimationRef( void );
};

struct SkinnedMeshRef
{
	CSkinnedMesh*		SkinnedMesh;
	int32				Index;
	uint32				References;

	SkinnedMeshRef( void )		{	};
	~SkinnedMeshRef( void );
};
#endif

#ifdef _SUPPORT_MODEL
struct ModelRef
{
	CModel*		Model;
	uint32		References;
	std::string	Path;

	ModelRef( void )		{	};
	~ModelRef( void );

};
#endif

#ifdef _SUPPORT_SCENE
struct SceneRef
{
	CScene*		Scene;
	uint32		References;
	std::string	Path;

	SceneRef( void )		{	};
	~SceneRef( void );

};
#endif

#ifdef _SUPPORT_COLLISIONSCENE
struct CollisionSceneRef
{
	CCollisionScene*		CollisionScene;
	uint32		References;
	std::string	Path;

	CollisionSceneRef( void )		{	};
	~CollisionSceneRef( void );

};
#endif

#ifdef _SUPPORT_SOUND
struct SoundRef
{
	FSOUND_SAMPLE*	Sample;
	uint32			References;
	std::string		Path;

	SoundRef( void )		{	Sample = NULL; };
	~SoundRef( void );
};
#endif

// the RefManager keeps track of various media types to prevent them from being duplicated in memory.
// it provides a way to retrieve any piece of media by path or reference
// complex media types (models,scenes,skinned meshes) depend upon the existence of a refmanager
// to handle their sub-media elements.
class CRefManager
{
public:




						CRefManager( CSettings& pSettings,				// pointer to a settings manager
									 std::string ExecutablePath );		// the base Path of this executable

						~CRefManager();

////////////////////////////////////////////////////////////////////////////////
// MATERIAL reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_MATERIAL
	CMaterial*			AddMaterial( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									 std::string TexturePath,			// Path to the base texture ("" if none)
									 std::string AlphaPath,				// Path to the alpha texture ("" if none)
									 std::string IlluminationPath,		// Path to the illumination texture ("" if none)
									 std::string NormalPath,			// Path to the normal texture ("" if none)
									 std::string SpherePath,			// Path to the sphere texture ("" if none)
									 bool IsUnique = kFalse,			// force this to be a non-unique material
									 bool AllowTextureCompression = kTrue );	// allow DXT texture compression?

	CMaterial*			AddMaterial( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									 std::string TexturePath,			// path to texture
									 bool ColorKey = kFalse,			// user colorkey? (kFalse is default)
									 DWORD ColorKeyValue = D3DCOLOR_ARGB( 255, 255, 0, 255 ), 	// defaults to magenta, if present
									 bool IsUnique = kFalse,			// force this to be a non-unique material
									 bool AllowTextureCompression = kTrue );	// allow DXT texture compression?

	CMaterial*			AddMaterialAlpha( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
										  std::string TexturePath,		// path to texture
										  std::string AlphaPath,		// path to alpha
										  bool IsUnique = kFalse,			// force this to be a non-unique material
										  bool AllowTextureCompression = kTrue );	// allow DXT texture compression?

	CMaterial*			AddMaterial( CMaterial* Material );				// material to add

	void				RemoveMaterial( CMaterial* Material );			// reference to the Material to remove from the list

	MaterialRef*		GetMaterial( CMaterial* Material );				// reference to the Material to remove from the list

	MaterialRef*		GetMaterial( std::string TexturePath,			// path to texture
									 std::string AlphaPath,				// path to alpha texture
									 std::string IlluminationPath,		// path to illumination texture
									 std::string NormalPath,			// path to illumination texture
									 std::string SpherePath );			// path to illumination texture

	void				UpdateMaterials( float32 TimeElapsed );		// time elapsed, in seconds

	void				SetMaterialDataID( std::string Path,			// path to the texture
										   uint32 ID );					// data to set on the texture

	uint32				CountMaterials( void )	{	return m_Materials.size();	};
#endif

////////////////////////////////////////////////////////////////////////////////
// FONTMETRIC reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_FONTMETRIC

	CFontMetric*		AddFontMetric( std::string Path );				// path to font metric (ini )

	void				RemoveFontMetric( CFontMetric* FontMetric );	// reference to metric

	FontMetricRef*		GetFontMetric( std::string Path );					// path to metric to find

	FontMetricRef*		GetFontMetric( CFontMetric* FontMetric );			// pointer to metric to find

	uint32				CountFontMetrics( void )	{	return m_FontMetrics.size();		};

#endif

////////////////////////////////////////////////////////////////////////////////
// MOVIE reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_MOVIE

	CMovie*				AddMovie( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								  std::string MoviePath,				// path to movie (AVI)
								  bool AutoUpdate = kTrue );			// automatically update movie?

	void				RemoveMovie( CMovie* Movie );					// reference to movie

	MovieRef*			GetMovie( std::string TexturePath );			// path to movie to find

	MovieRef*			GetMovie( CMovie* Movie );						// pointer to movie to find

	uint32				CountMovies( void )		{	return m_Movies.size();		};

#endif

////////////////////////////////////////////////////////////////////////////////
// CUBEMAP reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_CUBEMAP

	CCubeMap*			AddCubeMap( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								    std::string Path );						// path to cubemap dds

	CCubeMap*			AddCubeMap( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								    uint32 Width,							// width of cube
									ECubeMapType Type );					// render target or standard

	CCubeMap*			AddCubeMap( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								    std::string Right,						// path to face texture
								    std::string Left,						// path to face texture
								    std::string Up,							// path to face texture
								    std::string Down,						// path to face texture
								    std::string Front,						// path to face texture
								    std::string Back );						// path to face texture

	void				RemoveCubeMap( CCubeMap* CubeMap );					// reference to movie

	CubeMapRef*			GetCubeMap( std::string TexturePath );				// path to movie to find

	CubeMapRef*			GetCubeMap( std::string Right,		// path to CubeMap to find
									 std::string Left,		// path to CubeMap to find
									 std::string Up,		// path to CubeMap to find
									 std::string Down,		// path to CubeMap to find
									 std::string Front,		// path to CubeMap to find
									 std::string Back );	// path to CubeMap to find

	CubeMapRef*			GetCubeMap( CCubeMap* CubeMap );					// pointer to movie to find

	uint32				CountCubeMaps( void )		{	return m_CubeMaps.size();		};

#endif

////////////////////////////////////////////////////////////////////////////////
// MODEL reference management
////////////////////////////////////////////////////////////////////////////////

#ifdef _SUPPORT_MODEL

	CModel*				AddModel( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
								  std::string ModelPath,			// path to the model
								  std::string TexturePath,			// path at which the model's textures are found
								  bool LocalSpace,					// ignore global positioning in the file?
								  bool LoadTextures,				// load any referenced textures?
								  bool IsUnique = kFalse);			// is it unique? (must have no other references made to it) default = false

	void				RemoveModel( CModel* Model );				// model to remove

	ModelRef*			GetModel( std::string ModelPath );			// path of model to retrive

	ModelRef*			GetModel( CModel* Model );					// reference to model to retrieve
	uint32				CountModels( void )		{	return m_Models.size();		};

#endif

////////////////////////////////////////////////////////////////////////////////
// SCENE reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_SCENE

	CScene*				AddScene( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
								  std::string ScenePath,			// path to the model
								  std::string TexturePath,			// path at which the model's textures are found
								  bool IsCollideable,				// can it be collided with?
								  bool AllowRealtimeReflection,		// allow automatic realtime reflection? ( mirrors )
								  ESortType SortType,				// type of sorting to use (quadtree is default)
								  bool IsUnique = kFalse);			// is it unique? (must have no other references made to it)

	void				RemoveScene( CScene* Scene );				// scene to remove

	SceneRef*			GetScene( std::string ScenePath );			// path of scene to retrive

	SceneRef*			GetScene( CScene* Scene );					// reference to scene to retrieve
	uint32				CountScenes( void )		{	return m_Scenes.size();		};

#endif

////////////////////////////////////////////////////////////////////////////////
// COLLISIONSCENE reference management
////////////////////////////////////////////////////////////////////////////////

#ifdef _SUPPORT_COLLISIONSCENE

	CCollisionScene*	AddCollisionScene( std::string CollisionScenePath,	// path to the collision scene
										   ESortType SortType,				// type of sorting to use (quadtree is default)
										   bool IsUnique = kFalse);			// is it unique? (must have no other references made to it)

	void				RemoveCollisionScene( CCollisionScene* CollisionScene );// scene to remove

	CollisionSceneRef*	GetCollisionScene( std::string ScenePath );				// path of scene to retrive

	CollisionSceneRef*	GetCollisionScene( CCollisionScene* CollisionScene );	// reference to scene to retrieve
	uint32				CountCollisionScenes( void )		{	return m_CollisionScenes.size();		};
#endif

////////////////////////////////////////////////////////////////////////////////
// ANIMATED MESH reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_ANIMATEDMESH
	CAnimatedGeometry*			AddAnimatedGeometry( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
													 std::string Path,				// path to the animated geometry
													 std::string TexturePath );		// path to the textures (if "" is supplied, will use the mesh path)

	void					RemoveAnimatedGeometry( CAnimatedGeometry* AnimatedGeometry );//  geometry to remove

	AnimatedGeometryRef*	GetAnimatedGeometry( std::string Path );					// path to the animated geometry mesh

	AnimatedGeometryRef*	GetAnimatedGeometry( CAnimatedGeometry* AnimatedGeometry );	// animated geometry to retrieve


	CAnimatedMesh*			AddAnimatedMesh( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
											 std::string Path,				// path to the animated mesh
											 std::string TexturePath,		// path to the textures (if "" is supplied, will use the mesh path)
											 std::string AnimationPath = "" );		// path to the animation file ( optional )

	void					RemoveAnimatedMesh( CAnimatedMesh* AnimatedMesh );// mesh to remove

	AnimatedMeshRef*	GetAnimatedMesh( CAnimatedMesh* AnimatedMesh );	// animated mesh to retrieve

	uint32				CountAnimatedMeshes( void )	{	return m_AnimatedMeshes.size();	};

	uint32				CountAnimatedGeometries( void )	{	return m_Animations.size();	};
#endif

	////////////////////////////////////////////////////////////////////////////////
// SKINNED MESH reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_SKINNEDMESH

	CSkinnedAnimation*			AddSkinnedAnimation( const std::string& Name,		// name of the animation
												     std::string Path,				// path to SMA animation file
												     CSkinnedGeometry* pParent,		// geometry loading the animation
												     float32 FPS );					// frames per second to play at

	void					RemoveSkinnedAnimation( CSkinnedAnimation* SkinnedAnimation );//  Animation to remove

	SkinnedAnimationRef*	GetSkinnedAnimation( const std::string& Name,		// name of the animation
												 std::string Path,				// path to SMA animation file
												 float32 FPS );					// frames per second to play at

	SkinnedAnimationRef*	GetSkinnedAnimation( CSkinnedAnimation* SkinnedAnimation );	// animated Animation to retrieve

	CSkinnedGeometry*			AddSkinnedGeometry( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
													 std::string Path,				// path to the animated geometry
													 std::string TexturePath );		// path to the textures (if "" is supplied, will use the mesh path)

	void					RemoveSkinnedGeometry( CSkinnedGeometry* SkinnedGeometry );//  geometry to remove

	SkinnedGeometryRef*	GetSkinnedGeometry( std::string Path );					// path to the animated geometry mesh

	SkinnedGeometryRef*	GetSkinnedGeometry( CSkinnedGeometry* SkinnedGeometry );	// animated geometry to retrieve


	CSkinnedMesh*			AddSkinnedMesh( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
											 std::string Path,				// path to the animated mesh
											 std::string TexturePath,		// path to the textures (if "" is supplied, will use the mesh path)
											 std::string AnimationPath = "" );		// path to the animation file ( optional )

	void					RemoveSkinnedMesh( CSkinnedMesh* SkinnedMesh );// mesh to remove

	SkinnedMeshRef*	GetSkinnedMesh( CSkinnedMesh* SkinnedMesh );	// animated mesh to retrieve

	uint32				CountSkinnedMeshes( void )	{	return m_SkinnedMeshes.size();	};

	uint32				CountSkinnedGeometries( void )	{	return m_Skins.size();	};

	uint32				CountSkinnedAnimations( void )	{	return m_SkinnedAnimations.size();	};

#endif

////////////////////////////////////////////////////////////////////////////////
// SOUND reference management
////////////////////////////////////////////////////////////////////////////////

#ifdef _SUPPORT_SOUND
	FSOUND_SAMPLE*		AddSample( std::string SamplePath,			//  path to sound sample
								   bool Looping = kFalse );			// looping sample? (default = kFalse)

	void				RemoveSample( FSOUND_SAMPLE* Sample );		// Sample to remove

	SoundRef*			GetSample( std::string SamplePath );		// path to sample to retrieve

	SoundRef*			GetSample( FSOUND_SAMPLE* Sample );			// sample to retrieve
	uint32				CountSounds( void )		{	return m_Sounds.size();	};

#endif

////////////////////////////////////////////////////////////////////////////////
// General purpose
////////////////////////////////////////////////////////////////////////////////

	void				InvalidateDeviceObjects( void );
	void				RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void				RemoveUnused();
	void				RemoveAll();

private:
	CSettings&					m_pSettings;

	std::string					m_ExecutablePath;	// path to the actual executable

#ifdef _SUPPORT_MODEL
	std::vector <ModelRef*>					m_Models;
#endif

#ifdef _SUPPORT_SCENE
	std::vector <SceneRef*>					m_Scenes;
#endif

#ifdef _SUPPORT_COLLISIONSCENE
	std::vector <CollisionSceneRef*>		m_CollisionScenes;
#endif

#ifdef _SUPPORT_MATERIAL
	std::vector <MaterialRef*>				m_Materials;
#endif

#ifdef _SUPPORT_ANIMATEDMESH
	std::vector <AnimatedMeshRef*>			m_AnimatedMeshes;
	std::vector <AnimatedGeometryRef*>		m_Animations;
#endif

#ifdef _SUPPORT_SKINNEDMESH
	std::vector <SkinnedMeshRef*>			m_SkinnedMeshes;
	std::vector <SkinnedGeometryRef*>		m_Skins;
	std::vector <SkinnedAnimationRef*>		m_SkinnedAnimations;
#endif

#ifdef _SUPPORT_MOVIE
	std::vector <MovieRef*>					m_Movies;
#endif

#ifdef _SUPPORT_FONTMETRIC
	std::vector <FontMetricRef*>			m_FontMetrics;
#endif

#ifdef _SUPPORT_CUBEMAP
	std::vector <CubeMapRef*>				m_CubeMaps;
#endif

#ifdef _SUPPORT_SOUND
	std::vector <SoundRef*>					m_Sounds;
#endif
};

#endif