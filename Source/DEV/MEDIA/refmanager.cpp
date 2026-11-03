/*
	refmanager.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	Change History :

	Travis Baldree
	4/21/2004		-	added support for cubemaps

	Travis Baldree
	5/17/2004		-	all materials were being created unique by default - fixed

	Travis Baldree
	5/27/2004	-	for scenes - now must specify whether you allow automatic dynamic reflections at load time
					this is useful for games, like racing games, where you are generating one
					reflection for the entire scene, and don't need an individual reflection per material.
					That way the rendertotextures don't get erroneously created for all reflective
					materials. 

	6/29/2004	-	removed the d3ddevice requirement of addmaterial when addign a previously
					existing material. This was required for the new setmaterial methods on
					models and animated meshes
	
	7/12/2004	-	added support for font metrics

	9/25/2004	-	Fixed a bug with skinned animations - they were not properly refmanaging, and were always duplicating

    10/8/2004	-	Added assertion checks when requesting files
*/

#include <D3D8.h>
#include <D3DX8.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"


#include "../UTILITIES/settings.h"
#include "../UTILITIES/capabilities.h"
#include "refmanager.h"

#ifdef _SUPPORT_SOUND
#include "soundmanager.h"
#include "fmod.h"

SoundRef::~SoundRef()			{	FSOUND_Sample_Free( Sample );	}
#endif

#ifdef _SUPPORT_MODEL
#include "../MODEL/model.h"
ModelRef::~ModelRef()			{	DELETE_SAFELY( Model );			}
#endif

#ifdef _SUPPORT_SCENE
#include "../Scene/scene.h"
SceneRef::~SceneRef()			{	DELETE_SAFELY( Scene );			}
#endif

#ifdef _SUPPORT_COLLISIONSCENE
#include "../Scene/collisionscene.h"
CollisionSceneRef::~CollisionSceneRef()			{	DELETE_SAFELY( CollisionScene );			}
#endif

#ifdef _SUPPORT_ANIMATEDMESH
#include "../ANIMATEDMESH/animatedmesh.h"
#include "../ANIMATEDMESH/animatedgeometry.h"
AnimatedGeometryRef::~AnimatedGeometryRef()		{	DELETE_SAFELY( AnimatedGeometry );		}
AnimatedMeshRef::~AnimatedMeshRef()				{	DELETE_SAFELY( AnimatedMesh );		}
#endif

#ifdef _SUPPORT_SKINNEDMESH
#include "../SKINNEDMESH/skinnedmesh.h"
#include "../SKINNEDMESH/skinnedgeometry.h"
#include "../SKINNEDMESH/skinnedanimation.h"
SkinnedGeometryRef::~SkinnedGeometryRef()		{	DELETE_SAFELY( SkinnedGeometry );		}
SkinnedMeshRef::~SkinnedMeshRef()				{	DELETE_SAFELY( SkinnedMesh );			}
SkinnedAnimationRef::~SkinnedAnimationRef()		{	DELETE_SAFELY( SkinnedAnimation );		}
#endif

#ifdef _SUPPORT_MATERIAL
#include "material.h"
MaterialRef::~MaterialRef()		{	DELETE_SAFELY( Material );		}
#endif

#ifdef _SUPPORT_MOVIE
#include "movie.h"
MovieRef::~MovieRef()			{	DELETE_SAFELY( Movie );			}
#endif

#ifdef _SUPPORT_FONTMETRIC
#include "../UI/fontmetric.h"
FontMetricRef::~FontMetricRef()			{	DELETE_SAFELY( FontMetric );			}
#endif

#ifdef _SUPPORT_CUBEMAP
#include "cubemap.h"
CubeMapRef::~CubeMapRef()		{	DELETE_SAFELY( CubeMap );			}
#endif

CRefManager::CRefManager( CSettings& pSettings,					// pointer to a settings manager
						  std::string ExecutablePath ) :		// the base Path of this executable
						  m_pSettings( pSettings ),
						  m_ExecutablePath( ExecutablePath )
{
}  // CRefManager::CRefManager()

CRefManager::~CRefManager()
{
	RemoveAll();
} // CRefManager::~CRefManager()



////////////////////////////////////////////////////////////////////////////////
// MATERIAL reference management
////////////////////////////////////////////////////////////////////////////////

#ifdef _SUPPORT_MATERIAL
// add a complex Material - can contain alpha and illumination
CMaterial* CRefManager::AddMaterial( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									 std::string TexturePath,			// Path to the base texture ("" if none)
									 std::string AlphaPath,				// Path to the alpha texture ("" if none)
									 std::string IlluminationPath,		// Path to the illumination texture ("" if none)
									 std::string NormalPath,			// Path to the normal map texture ("" if none)
									 std::string SpherePath,			// Path to the normal map texture ("" if none)
									 bool IsUnique,						// force this to be a unique material
									 bool AllowTextureCompression )		// allow DXT texture compression?
{
	if( AllowTextureCompression && 
		( !m_pSettings.Capabilities().IsCapable( KCanDoDXT1 ) ||
		  !m_pSettings.Capabilities().IsCapable( KCanDoDXT3 ) ) )
	{
		AllowTextureCompression = kFalse;
	}

	std::string	OriginalPath( TexturePath );

	// assemble the absolute paths, all in uppercase


	TexturePath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, TexturePath ) );
	TexturePath = CleanPath( TexturePath );
	AlphaPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, AlphaPath ) );
	AlphaPath = CleanPath( AlphaPath );
	IlluminationPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, IlluminationPath ) );
	IlluminationPath = CleanPath( IlluminationPath );
	NormalPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, NormalPath ) );
	NormalPath = CleanPath( NormalPath );
	SpherePath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, SpherePath ) );
	SpherePath = CleanPath( SpherePath );

	// check for the existence of this Material
	MaterialRef*	MediaExists	= GetMaterial( TexturePath, 
											   AlphaPath, 
											   IlluminationPath, 
											   NormalPath, 
											   SpherePath );

	// if it doesn't currently exist in the list
	if ( MediaExists == NULL || IsUnique )
	{
		MaterialRef* MediaReference	= new MaterialRef();
		m_Materials.push_back( MediaReference );

		MediaReference->Material = new CMaterial( *this, AllowTextureCompression );

		// texture has an alpha mask
		if ( AlphaPath.length() > 0 )
		{
			MediaReference->Material->Create( pD3DDevice, TexturePath, AlphaPath );

			// add an illumination map if one is requested
			if ( IlluminationPath.length() > 0 )
			{
				MediaReference->Material->CreateIllumination( pD3DDevice, IlluminationPath );
			}
		}
		else // no alpha mask
		{
			if( TexturePath.length() > 0 )
			{
				MediaReference->Material->Create( pD3DDevice, TexturePath );
			}

			// add an illumination map if one is requested
			if ( IlluminationPath.length() > 0 )
			{
				MediaReference->Material->CreateIllumination( pD3DDevice, IlluminationPath );
			}
		}

		// add a normal map if one is requested
		if ( NormalPath.length() > 0 )
		{
			MediaReference->Material->CreateNormalMap( pD3DDevice, NormalPath );
		}

		// add a normal map if one is requested
		if ( SpherePath.length() > 0 )
		{
			MediaReference->Material->CreateSphereMap( pD3DDevice, SpherePath );
		}

		// set up the basics for this Material reference
		MediaReference->FileName = OriginalPath;
		MediaReference->References = 1;
		MediaReference->Path = TexturePath;
		MediaReference->AlphaPath = AlphaPath;
		MediaReference->IlluminationPath = IlluminationPath;
		MediaReference->NormalPath = NormalPath;
		MediaReference->SpherePath = SpherePath;

		// return a pointer to the actual Material
		return MediaReference->Material;
	}
	else
	{
		// increment the reference count
		MediaExists->References++;
		// return a pointer to the actual Material
		return MediaExists->Material;
	}
} // CRefManager::AddMaterial()


// add a base material - MAY contain a colorkey
CMaterial* CRefManager::AddMaterial( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
									 std::string TexturePath,		// path to texture
									 bool ColorKey,					// user colorkey? (kFalse is default)
									 DWORD ColorKeyValue,			// defaults to magenta
									 bool IsUnique,						// force this to be a unique material
									 bool AllowTextureCompression )		// allow DXT texture compression?
{
	if( AllowTextureCompression && 
		( !m_pSettings.Capabilities().IsCapable( KCanDoDXT1 ) ||
		  !m_pSettings.Capabilities().IsCapable( KCanDoDXT3 ) ) )
	{
		AllowTextureCompression = kFalse;
	}

	std::string	OriginalPath( TexturePath );

	// assemble the absolute path, all in uppercase
	TexturePath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, TexturePath ) );
	TexturePath = CleanPath( TexturePath );

	// check for the existence of this Material
	MaterialRef* MediaExists	= GetMaterial( TexturePath, "", "", "", "" );

	// if it doesn't currently exist in the list
	if ( MediaExists == NULL || 
		IsUnique || 
		( MediaExists != NULL && ColorKey && !MediaExists->Material->HasAlpha() ) ||
		( MediaExists != NULL && !ColorKey && MediaExists->Material->HasAlpha() ) )
	{
		MaterialRef* MediaReference	= new MaterialRef();
		m_Materials.push_back( MediaReference );

		MediaReference->Material = new CMaterial( *this, AllowTextureCompression );
		MediaReference->Material->Create( pD3DDevice, TexturePath, ColorKey, ColorKeyValue );
		MediaReference->References = 1;
		MediaReference->Path = TexturePath;

		MediaReference->FileName = OriginalPath;

		// return a pointer to the actual Material
		return MediaReference->Material;
	}
	else
	{
		// increment the reference count
		MediaExists->References++;
		// return a pointer to the actual Material
		return MediaExists->Material;
	}
} // CRefManager::AddMaterial()

// add a texture with opacity mask
CMaterial* CRefManager::AddMaterialAlpha( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
										  std::string TexturePath,			// path to texture
										  std::string AlphaPath,			// path to alpha
										  bool IsUnique,					// force this to be a unique material
										  bool AllowTextureCompression )	// allow DXT texture compression?
{
	return AddMaterial( pD3DDevice, TexturePath, AlphaPath, "", "", "", IsUnique, AllowTextureCompression );

} // CRefManager::AddMaterial()

// add an existing material to the manager
CMaterial* CRefManager::AddMaterial( CMaterial* Material )				// material to add
{
	// check for the existence of this Material
	MaterialRef* MediaExists	= GetMaterial( Material );

	// if it doesn't currently exist in the list
	if ( MediaExists == NULL )
	{
		MaterialRef* MediaReference	= new MaterialRef();
		m_Materials.push_back( MediaReference );

		MediaReference->Material = Material;
		MediaReference->References = 1;
		MediaReference->Path = StringUpper( Material->GetPath() );

		MediaReference->FileName = MediaReference->Path;

		// return a pointer to the actual Material
		return MediaReference->Material;
	}
	else
	{
		MediaExists->References++;
		// return a pointer to the actual Material
		return MediaExists->Material;
	}
} // CRefManager::AddMaterial()

// remove a Material, by reference
void CRefManager::RemoveMaterial( CMaterial* Material )		// reference to the Material to remove from the list
{
	// if it doesn't currently exist in the list
	if ( Material == NULL )
	{
		return;
	}
	MaterialRef* MediaExists	= GetMaterial( Material );

	// if it exists, decrement the references
	if ( MediaExists != NULL )
	{
		MediaExists->References--;
	}
} // CRefManager::RemoveMaterial()

// retrieve an existing Material by reference
MaterialRef* CRefManager::GetMaterial( CMaterial* Material )		// reference to the Material to remove from the list
{
	for( uint32 i = 0; i < m_Materials.size() ; i++ )
	{
		if ( m_Materials[i]->Material == Material )
		{
			return m_Materials[i];
		}
	}

	return NULL;
}

// retrieve an existing Material by texture, alpha, and illumination paths
MaterialRef* CRefManager::GetMaterial( std::string TexturePath,			// path to texture
									   std::string AlphaPath,			// path to alpha texture
									   std::string IlluminationPath,	// path to illumination texture
									   std::string NormalPath,			// path to normal texture
									   std::string SpherePath )			// path to sphere texture
{
	for( uint32 i = 0; i < m_Materials.size() ; i++ )
	{
		if ( m_Materials[i]->Path == TexturePath && 
			 m_Materials[i]->AlphaPath == AlphaPath &&
			 m_Materials[i]->IlluminationPath == IlluminationPath &&
			 m_Materials[i]->SpherePath == SpherePath &&
			 m_Materials[i]->NormalPath == NormalPath )
		{
			return m_Materials[i];
		}
	}

	return NULL;
} // CRefManager::GetMaterial()

// update any material animations
void CRefManager::UpdateMaterials( float32 TimeElapsed )	// time elapsed, in seconds
{
#ifdef _SUPPORT_MOVIE
	for( uint32 i = 0; i < m_Movies.size() ; i++ )
	{
		if( m_Movies[i]->AutoUpdate )
		{
			m_Movies[i]->Movie->Update( TimeElapsed );
		}
	}
#endif

	for( i = 0; i < m_Materials.size() ; i++ )
	{
		m_Materials[i]->Material->Update( TimeElapsed );
	}
} // CRefManager::UpdateMaterials()

// set Data ID on a texture - certain objects may use this to determine whether it should be
// rendered or not

void CRefManager::SetMaterialDataID( std::string Path,			// path to the texture
									 uint32 ID )				// LOD level below which this should render
{
	Path = StringUpper( Path );

	for( uint32 i = 0; i < m_Materials.size() ; i++ )
	{
		if( StringUpper( m_Materials[i]->FileName ) == Path )
		{
			m_Materials[i]->Material->SetDataID( ID );
		}
	}

} // CRefManager::SetDataID()
#endif

////////////////////////////////////////////////////////////////////////////////
// MOVIE reference management
////////////////////////////////////////////////////////////////////////////////

#ifdef _SUPPORT_MOVIE

CMovie* CRefManager::AddMovie( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
							   std::string MoviePath,				// path to movie (AVI)
							   bool AutoUpdate )					// automatically update movie? default kTrue
{
	std::string OriginalPath( MoviePath );

	MoviePath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, MoviePath ) );
	MoviePath = CleanPath( MoviePath );

	MovieRef*	MediaExists	= GetMovie( MoviePath );

	// if it doesn't already exist in the list
	if ( MediaExists == NULL )
	{
		assert( FileExists( MoviePath ) );
		// create it
		MovieRef*	MediaReference	= new MovieRef();
		m_Movies.push_back( MediaReference );

		MediaReference->Movie = new CMovie( MoviePath, pD3DDevice );
		MediaReference->References = 1;
		MediaReference->Path = MoviePath;
		MediaReference->FileName = OriginalPath;
		MediaReference->AutoUpdate = AutoUpdate;

		// return a pointer to the actual Material
		return MediaReference->Movie;
	}
	else
	{
		// increment the reference count
		MediaExists->References++;
		// return a pointer to the actual Material
		return MediaExists->Movie;
	}
} // CRefManager::AddMovie()

// remove a movie from the manager
void CRefManager::RemoveMovie( CMovie* Movie )		// reference to movie
{
	MovieRef*	MediaExists	= GetMovie( Movie );

	if ( MediaExists != NULL )
	{
		// decrement the reference count
		MediaExists->References--;
	}
} // CRefManager::RemoveMovie()

// retrieve a movie by path
MovieRef* CRefManager::GetMovie( std::string TexturePath )	// path to movie to find
{
	for( uint32 i = 0; i < m_Movies.size() ; i++ )
	{
		if ( m_Movies[i]->Path == TexturePath )
		{
			return m_Movies[i];
		}
	}

	return NULL;
} // CRefManager::GetMovie()

// retrieve a movie by reference
MovieRef* CRefManager::GetMovie( CMovie* Movie )	// pointer to movie to find
{
	for( uint32 i = 0; i < m_Movies.size() ; i++ )
	{
		if ( m_Movies[i]->Movie == Movie )
		{
			return m_Movies[i];
		}
	}

	return NULL;
} // CRefManager::GetMovie()
#endif

////////////////////////////////////////////////////////////////////////////////
// FONTMETRIC reference management
////////////////////////////////////////////////////////////////////////////////

#ifdef _SUPPORT_FONTMETRIC

CFontMetric* CRefManager::AddFontMetric( std::string FontMetricPath )	// path to FontMetric (AVI)
{
	std::string OriginalPath( FontMetricPath );

	FontMetricPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, FontMetricPath ) );
	FontMetricPath = CleanPath( FontMetricPath );

	FontMetricRef*	MediaExists	= GetFontMetric( FontMetricPath );

	// if it doesn't already exist in the list
	if ( MediaExists == NULL )
	{
		assert( FileExists( FontMetricPath ) );
		// create it
		FontMetricRef*	MediaReference	= new FontMetricRef();
		m_FontMetrics.push_back( MediaReference );

		MediaReference->FontMetric = new CFontMetric( FontMetricPath );
		MediaReference->References = 1;
		MediaReference->Path = FontMetricPath;
		MediaReference->FileName = OriginalPath;

		// return a pointer to the actual Material
		return MediaReference->FontMetric;
	}
	else
	{
		// increment the reference count
		MediaExists->References++;
		// return a pointer to the actual Material
		return MediaExists->FontMetric;
	}
} // CRefManager::AddFontMetric()

// remove a FontMetric from the manager
void CRefManager::RemoveFontMetric( CFontMetric* FontMetric )		// reference to FontMetric
{
	FontMetricRef*	MediaExists	= GetFontMetric( FontMetric );

	if ( MediaExists != NULL )
	{
		// decrement the reference count
		MediaExists->References--;
	}
} // CRefManager::RemoveFontMetric()

// retrieve a FontMetric by path
FontMetricRef* CRefManager::GetFontMetric( std::string Path )	// path to FontMetric to find
{
	for( uint32 i = 0; i < m_FontMetrics.size() ; i++ )
	{
		if ( m_FontMetrics[i]->Path == Path )
		{
			return m_FontMetrics[i];
		}
	}

	return NULL;
} // CRefManager::GetFontMetric()

// retrieve a FontMetric by reference
FontMetricRef* CRefManager::GetFontMetric( CFontMetric* FontMetric )	// pointer to FontMetric to find
{
	for( uint32 i = 0; i < m_FontMetrics.size() ; i++ )
	{
		if ( m_FontMetrics[i]->FontMetric == FontMetric )
		{
			return m_FontMetrics[i];
		}
	}

	return NULL;
} // CRefManager::GetFontMetric()
#endif

////////////////////////////////////////////////////////////////////////////////
// CUBEMAP reference management
////////////////////////////////////////////////////////////////////////////////

#ifdef _SUPPORT_CUBEMAP

CCubeMap* CRefManager::AddCubeMap( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								   std::string CubeMapPath )				// path to CubeMap 
{
	std::string OriginalPath( CubeMapPath );

	CubeMapPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, CubeMapPath ) );
	CubeMapPath = CleanPath( CubeMapPath );

	CubeMapRef*	MediaExists	= GetCubeMap( CubeMapPath );

	// if it doesn't already exist in the list
	if ( MediaExists == NULL )
	{
		assert( FileExists( CubeMapPath ) );
		// create it
		CubeMapRef*	MediaReference	= new CubeMapRef();
		m_CubeMaps.push_back( MediaReference );

		MediaReference->CubeMap = new CCubeMap();
		MediaReference->CubeMap->Create( pD3DDevice, CubeMapPath );
		MediaReference->References = 1;
		MediaReference->Path = CubeMapPath;
		MediaReference->FileName = OriginalPath;

		// return a pointer to the actual Material
		return MediaReference->CubeMap;
	}
	else
	{
		// increment the reference count
		MediaExists->References++;
		// return a pointer to the actual Material
		return MediaExists->CubeMap;
	}
} // CRefManager::AddCubeMap()

// empty cubemaps are always created uniquely
CCubeMap* CRefManager::AddCubeMap( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								   uint32 Width,						// width of cube
								   ECubeMapType Type )					// render target or standard
{
	// create it
	CubeMapRef*	MediaReference	= new CubeMapRef();
	m_CubeMaps.push_back( MediaReference );

	MediaReference->CubeMap = new CCubeMap();
	MediaReference->CubeMap->Create( pD3DDevice, Width, Type );
	MediaReference->References = 1;
	MediaReference->Path = "NONE";
	MediaReference->FileName = "NONE";
	// return a pointer to the actual Material
	return MediaReference->CubeMap;
} // CRefManager::AddCubeMap()

CCubeMap* CRefManager::AddCubeMap( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								   std::string Right,						// path to face texture
								   std::string Left,						// path to face texture
								   std::string Up,							// path to face texture
								   std::string Down,						// path to face texture
								   std::string Front,						// path to face texture
								   std::string Back )						// path to face texture
{

	std::string RightPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, Right ) );
	RightPath = CleanPath( RightPath );
	std::string LeftPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, Left ) );
	LeftPath = CleanPath( LeftPath );
	std::string UpPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, Up ) );
	UpPath = CleanPath( UpPath );
	std::string DownPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, Down ) );
	DownPath = CleanPath( DownPath );
	std::string FrontPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, Front ) );
	FrontPath = CleanPath( FrontPath );
	std::string BackPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, Back ) );
	BackPath = CleanPath( BackPath );

	CubeMapRef*	MediaExists	= GetCubeMap( RightPath,
										  LeftPath,
										  UpPath,
										  DownPath,
										  FrontPath,
										  BackPath );

	// if it doesn't already exist in the list
	if ( MediaExists == NULL )
	{
		// create it
		CubeMapRef*	MediaReference	= new CubeMapRef();
		m_CubeMaps.push_back( MediaReference );

		MediaReference->CubeMap = new CCubeMap();
		MediaReference->CubeMap->Create( pD3DDevice, 
										 RightPath,
										 LeftPath,
										 UpPath,
										 DownPath,
										 FrontPath,
										 BackPath );
		MediaReference->References = 1;
		MediaReference->Path = "";
		MediaReference->RightPath = RightPath;
		MediaReference->LeftPath = LeftPath;
		MediaReference->UpPath = UpPath;
		MediaReference->DownPath = DownPath;
		MediaReference->FrontPath = FrontPath;
		MediaReference->BackPath = BackPath;
		MediaReference->FileName = Right;

		// return a pointer to the actual Material
		return MediaReference->CubeMap;
	}
	else
	{
		// increment the reference count
		MediaExists->References++;
		// return a pointer to the actual Material
		return MediaExists->CubeMap;
	}
}

// remove a CubeMap from the manager
void CRefManager::RemoveCubeMap( CCubeMap* CubeMap )		// reference to CubeMap
{
	CubeMapRef*	MediaExists	= GetCubeMap( CubeMap );

	if ( MediaExists != NULL )
	{
		// decrement the reference count
		MediaExists->References--;
	}
} // CRefManager::RemoveCubeMap()

// retrieve a CubeMap by path
CubeMapRef* CRefManager::GetCubeMap( std::string TexturePath )	// path to CubeMap to find
{
	for( uint32 i = 0; i < m_CubeMaps.size() ; i++ )
	{
		if ( m_CubeMaps[i]->Path == TexturePath )
		{
			return m_CubeMaps[i];
		}
	}

	return NULL;
} // CRefManager::GetCubeMap()

// retrieve a CubeMap by path
CubeMapRef* CRefManager::GetCubeMap( std::string Right,		// path to CubeMap to find
									 std::string Left,		// path to CubeMap to find
									 std::string Up,		// path to CubeMap to find
									 std::string Down,		// path to CubeMap to find
									 std::string Front,		// path to CubeMap to find
									 std::string Back )		// path to CubeMap to find
{
	for( uint32 i = 0; i < m_CubeMaps.size() ; i++ )
	{
		if ( m_CubeMaps[i]->RightPath == Right &&
			 m_CubeMaps[i]->LeftPath == Left &&
			 m_CubeMaps[i]->UpPath == Up &&
			 m_CubeMaps[i]->DownPath == Down &&
			 m_CubeMaps[i]->FrontPath == Front &&
			 m_CubeMaps[i]->BackPath == Back )
		{
			return m_CubeMaps[i];
		}
	}

	return NULL;
} // CRefManager::GetCubeMap()

// retrieve a CubeMap by reference
CubeMapRef* CRefManager::GetCubeMap( CCubeMap* CubeMap )	// pointer to CubeMap to find
{
	for( uint32 i = 0; i < m_CubeMaps.size() ; i++ )
	{
		if ( m_CubeMaps[i]->CubeMap == CubeMap )
		{
			return m_CubeMaps[i];
		}
	}

	return NULL;
} // CRefManager::GetCubeMap()
#endif

////////////////////////////////////////////////////////////////////////////////
// MODEL reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_MODEL

// add a model to the manager
CModel* CRefManager::AddModel( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
							   std::string ModelPath,				// path to the model
							   std::string TexturePath,				// path at which the model's textures are found
							   bool LocalSpace,						// ignore global positioning in the file?
							   bool LoadTextures,					// load any referenced textures?
							   bool IsUnique )						// is it unique? (must have no other references made to it)  default is false
{


	// find the absolute path to the model
	ModelPath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, ModelPath ) );
	ModelPath = CleanPath( ModelPath );

	ModelRef*	MediaExists	= GetModel( ModelPath );

	// if it's not in the manager, or we are requesting a unique item, create
	// a new one and add it
	if ( MediaExists == NULL || IsUnique )
	{
		assert( FileExists( ModelPath ) );
		ModelRef*	MediaReference = new ModelRef();
		m_Models.push_back( MediaReference );

		MediaReference->Model = new CModel( *this, m_pSettings );
		MediaReference->Model->Load( pD3DDevice,
									 ModelPath,
									 TexturePath,
									 LocalSpace,
									 LoadTextures,
									 kFalse );
		MediaReference->References = 1;
		MediaReference->Path = ModelPath;
		// return a pointer to the actual model
		return MediaReference->Model;
	}
	else
	{
		// increment the reference count
		MediaExists->References++;
		// return a pointer to the actual model
		return MediaExists->Model;
	}
} // CRefManager::AddModel()

void CRefManager::RemoveModel( CModel* Model )			// model to remove
{
	ModelRef*	MediaExists	= GetModel( Model );

	if ( MediaExists != NULL )
	{
		// decrement the reference count
		MediaExists->References--;
	}

} // CRefManager::RemoveModel()

ModelRef* CRefManager::GetModel( std::string ModelPath )		// path of model to retrive
{

	for( uint32 i = 0; i < m_Models.size() ; i++ )
	{
		if ( m_Models[i]->Path == ModelPath )
		{
			return m_Models[i];
		}
	}


	return NULL;
} // CRefManager::GetModel()


ModelRef* CRefManager::GetModel( CModel* Model )		// reference to model to retrieve
{
	for( uint32 i = 0; i < m_Models.size() ; i++ )
	{
		if ( m_Models[i]->Model == Model )
		{
			return m_Models[i];
		}
	}
	return NULL;
} // CRefManager::GetModel()
#endif

////////////////////////////////////////////////////////////////////////////////
// SCENE reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_SCENE

// add a model to the manager
CScene* CRefManager::AddScene( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
							   std::string ScenePath,			// path to the model
							   std::string TexturePath,			// path at which the model's textures are found
							   bool IsCollideable,				// can it be collided with?
							   bool AllowRealtimeReflection,	// allow automatic realtime reflection? ( mirrors )
							   ESortType SortType,				// type of sorting to use (quadtree is default)
							   bool IsUnique )					// is it unique? (must have no other references made to it)
{


	// find the absolute path to the model
	ScenePath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, ScenePath ) );
	ScenePath = CleanPath( ScenePath );

	SceneRef*	MediaExists	= GetScene( ScenePath );

	// if it's not in the manager, or we are requesting a unique item, create
	// a new one and add it
	if ( MediaExists == NULL || IsUnique )
	{
		SceneRef*	MediaReference = new SceneRef();
		m_Scenes.push_back( MediaReference );

		MediaReference->Scene = new CScene( *this, m_pSettings );
		MediaReference->Scene->Load( pD3DDevice,
									 ScenePath,
									 TexturePath,
									 IsCollideable,
									 AllowRealtimeReflection,
									 SortType );
		MediaReference->References = 1;
		MediaReference->Path = ScenePath;
		// return a pointer to the actual model
		return MediaReference->Scene;
	}
	else
	{
		// increment the reference count
		MediaExists->References++;
		// return a pointer to the actual model
		return MediaExists->Scene;
	}
} // CRefManager::AddScene()

void CRefManager::RemoveScene( CScene* Scene )			// model to remove
{
	SceneRef*	MediaExists	= GetScene( Scene );

	if ( MediaExists != NULL )
	{
		// decrement the reference count
		MediaExists->References--;
	}

} // CRefManager::RemoveScene()

SceneRef* CRefManager::GetScene( std::string ScenePath )		// path of model to retrive
{

	for( uint32 i = 0; i < m_Scenes.size() ; i++ )
	{
		if ( m_Scenes[i]->Path == ScenePath )
		{
			return m_Scenes[i];
		}
	}


	return NULL;
} // CRefManager::GetScene()


SceneRef* CRefManager::GetScene( CScene* Scene )		// reference to model to retrieve
{
	for( uint32 i = 0; i < m_Scenes.size() ; i++ )
	{
		if ( m_Scenes[i]->Scene == Scene )
		{
			return m_Scenes[i];
		}
	}
	return NULL;
} // CRefManager::GetScene()
#endif


////////////////////////////////////////////////////////////////////////////////
// COLLISIONSCENE reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_COLLISIONSCENE

// add a model to the manager
CCollisionScene* CRefManager::AddCollisionScene( std::string CollisionScenePath,	// path to the collision scene
												 ESortType SortType,				// type of sorting to use (quadtree is default)
												 bool IsUnique )					// is it unique? (must have no other references made to it)
{


	// find the absolute path to the model
	CollisionScenePath = StringUpper( AssembleAbsolutePath( m_ExecutablePath, CollisionScenePath ) );
	CollisionScenePath = CleanPath( CollisionScenePath );

	CollisionSceneRef*	MediaExists	= GetCollisionScene( CollisionScenePath );

	// if it's not in the manager, or we are requesting a unique item, create
	// a new one and add it
	if ( MediaExists == NULL || IsUnique )
	{
		CollisionSceneRef*	MediaReference = new CollisionSceneRef();
		m_CollisionScenes.push_back( MediaReference );

		MediaReference->CollisionScene = new CCollisionScene();
		MediaReference->CollisionScene->Load( CollisionScenePath,
											  SortType );
		MediaReference->References = 1;
		MediaReference->Path = CollisionScenePath;
		// return a pointer to the actual model
		return MediaReference->CollisionScene;
	}
	else
	{
		// increment the reference count
		MediaExists->References++;
		// return a pointer to the actual model
		return MediaExists->CollisionScene;
	}
} // CRefManager::AddCollisionScene()

void CRefManager::RemoveCollisionScene( CCollisionScene* CollisionScene )			// model to remove
{
	CollisionSceneRef*	MediaExists	= GetCollisionScene( CollisionScene );

	if ( MediaExists != NULL )
	{
		// decrement the reference count
		MediaExists->References--;
	}

} // CRefManager::RemoveCollisionScene()

CollisionSceneRef* CRefManager::GetCollisionScene( std::string CollisionScenePath )		// path of model to retrive
{

	for( uint32 i = 0; i < m_CollisionScenes.size() ; i++ )
	{
		if ( m_CollisionScenes[i]->Path == CollisionScenePath )
		{
			return m_CollisionScenes[i];
		}
	}


	return NULL;
} // CRefManager::GetCollisionScene()


CollisionSceneRef* CRefManager::GetCollisionScene( CCollisionScene* CollisionScene )		// reference to model to retrieve
{
	for( uint32 i = 0; i < m_CollisionScenes.size() ; i++ )
	{
		if ( m_CollisionScenes[i]->CollisionScene == CollisionScene )
		{
			return m_CollisionScenes[i];
		}
	}
	return NULL;
} // CRefManager::GetCollisionScene()
#endif


////////////////////////////////////////////////////////////////////////////////
// AnimatedMesh reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_ANIMATEDMESH

CAnimatedGeometry* CRefManager::AddAnimatedGeometry( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
													 std::string Path,				// path to the animated geometry
													 std::string TexturePath )		// path to the textures (if "" is supplied, will use the mesh path)
{

	// find the absolute path to the model
	Path = StringUpper( AssembleAbsolutePath( m_ExecutablePath, Path ) );
	Path = CleanPath( Path );

	// find any existing copies of the mesh
	AnimatedGeometryRef*	MediaExists	= GetAnimatedGeometry( Path );

	// if there are none, create a new one
	if ( MediaExists == NULL )
	{
		assert( FileExists( Path ) );
		AnimatedGeometryRef*	MediaReference = new AnimatedGeometryRef();
		m_Animations.push_back( MediaReference );

		MediaReference->AnimatedGeometry = new CAnimatedGeometry( *this , m_pSettings );
		MediaReference->AnimatedGeometry->Load( pD3DDevice, Path, TexturePath );
		MediaReference->References = 1;
		MediaReference->Path = Path;

		// return a pointer to the actual mesh
		return MediaReference->AnimatedGeometry;
	} 
	else
	{
		// increment reference count
		MediaExists->References++;
		// return a pointer to the actual mesh
		return MediaExists->AnimatedGeometry;
	}
} // CRefManager::AddAnimatedGeometry()

void CRefManager::RemoveAnimatedGeometry( CAnimatedGeometry* AnimatedGeometry )	// geometry to remove
{
	AnimatedGeometryRef*	MediaExists	= GetAnimatedGeometry( AnimatedGeometry );
	
	if ( MediaExists != NULL )
	{
		MediaExists->References--;
	}
} // CRefManager::RemoveAnimatedGeometry()

// retrieve an existing AnimatedGeometry given its path and index within parent model
AnimatedGeometryRef* CRefManager::GetAnimatedGeometry( std::string Path )		// path to the animated mesh
{

	for( uint32 i = 0; i < m_Animations.size() ; i++ )
	{
		if( m_Animations[i]->Path == Path )
		{
			return m_Animations[i];
		}
	}

	return NULL;
} // CRefManager::GetAnimatedGeometry()


// retrieve an existing AnimatedGeometry 
AnimatedGeometryRef* CRefManager::GetAnimatedGeometry( CAnimatedGeometry* AnimatedGeometry ) // animatedmesh to retrieve
{
	for( uint32 i = 0; i < m_Animations.size() ; i++ )
	{
		if( m_Animations[i]->AnimatedGeometry == AnimatedGeometry )
		{
			return m_Animations[i];
		}
	}

	return NULL;
} // CRefManager::GetAnimatedGeometry()


// these are always unique - it's the geometry that gets instanced
CAnimatedMesh* CRefManager::AddAnimatedMesh( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
											 std::string Path,				// path to the animated mesh
											 std::string TexturePath,		// path to the textures (if "" is supplied, will use the mesh path)
											 std::string AnimationPath )	// path to animation file
{

	// find the absolute path to the model
	Path = StringUpper( AssembleAbsolutePath( m_ExecutablePath, Path ) );
	Path = CleanPath( Path );


	AnimatedMeshRef*	MediaReference = new AnimatedMeshRef();
	m_AnimatedMeshes.push_back( MediaReference );

	MediaReference->AnimatedMesh = new CAnimatedMesh( *this, m_pSettings );
	MediaReference->AnimatedMesh->Load( pD3DDevice, Path, TexturePath, AnimationPath );
	MediaReference->References = 1;
	// return a pointer to the actual mesh
	return MediaReference->AnimatedMesh;

} // CRefManager::AddAnimatedMesh()

void CRefManager::RemoveAnimatedMesh( CAnimatedMesh* AnimatedMesh )	// mesh to remove
{
	AnimatedMeshRef*	MediaExists	= GetAnimatedMesh( AnimatedMesh );
	
	if ( MediaExists != NULL )
	{
		MediaExists->References--;
	}
} // CRefManager::RemoveAnimatedMesh()


// retrieve an existing AnimatedMesh 
AnimatedMeshRef* CRefManager::GetAnimatedMesh( CAnimatedMesh* AnimatedMesh ) // animatedmesh to retrieve
{
	for( uint32 i = 0; i < m_AnimatedMeshes.size() ; i++ )
	{
		if( m_AnimatedMeshes[i]->AnimatedMesh == AnimatedMesh )
		{
			return m_AnimatedMeshes[i];
		}
	}

	return NULL;
} // CRefManager::GetAnimatedMesh()


#endif

////////////////////////////////////////////////////////////////////////////////
// SkinnedMesh reference management
////////////////////////////////////////////////////////////////////////////////
#ifdef _SUPPORT_SKINNEDMESH

CSkinnedAnimation* CRefManager::AddSkinnedAnimation( const std::string& Name,		// name of the animation
												     std::string Path,				// path to SMA animation file
												     CSkinnedGeometry* pParent,		// geometry loading the animation
												     float32 FPS )					// frames per second to play at
{

	// find the absolute path to the model
	Path = StringUpper( AssembleAbsolutePath( m_ExecutablePath, Path ) );
	Path = CleanPath( Path );

	// find any existing copies of the mesh
	SkinnedAnimationRef*	MediaExists	= GetSkinnedAnimation( Name, Path, FPS );

	// if there are none, create a new one
	if ( MediaExists == NULL )
	{
		assert( FileExists( Path ) );
		SkinnedAnimationRef*	MediaReference = new SkinnedAnimationRef();
		m_SkinnedAnimations.push_back( MediaReference );

		MediaReference->SkinnedAnimation = new CSkinnedAnimation( Name, Path, pParent, FPS );
		MediaReference->References = 1;
		MediaReference->Path = Path;
		MediaReference->Name = Name;
		MediaReference->FPS = FPS;

		// return a pointer to the actual mesh
		return MediaReference->SkinnedAnimation;
	} 
	else
	{
		// increment reference count
		MediaExists->References++;
		// return a pointer to the actual mesh
		return MediaExists->SkinnedAnimation;
	}
} // CRefManager::AddSkinnedAnimation()

void CRefManager::RemoveSkinnedAnimation( CSkinnedAnimation* SkinnedAnimation )	// Animation to remove
{
	SkinnedAnimationRef*	MediaExists	= GetSkinnedAnimation( SkinnedAnimation );
	
	if ( MediaExists != NULL )
	{
		MediaExists->References--;
	}
} // CRefManager::RemoveSkinnedAnimation()

// retrieve an existing SkinnedAnimation given its path and index within parent model
SkinnedAnimationRef* CRefManager::GetSkinnedAnimation( const std::string& Name,			// name of the animation
												       std::string Path,				// path to SMA animation file
												       float32 FPS )					// frames per second to play at
{

	for( uint32 i = 0; i < m_SkinnedAnimations.size() ; i++ )
	{
		if( m_SkinnedAnimations[i]->Path == Path &&
			m_SkinnedAnimations[i]->Name == Name &&
			m_SkinnedAnimations[i]->FPS == FPS )
		{
			return m_SkinnedAnimations[i];
		}
	}

	return NULL;
} // CRefManager::GetSkinnedAnimation()


// retrieve an existing SkinnedAnimation 
SkinnedAnimationRef* CRefManager::GetSkinnedAnimation( CSkinnedAnimation* SkinnedAnimation ) // skinnedmesh to retrieve
{
	for( uint32 i = 0; i < m_SkinnedAnimations.size() ; i++ )
	{
		if( m_SkinnedAnimations[i]->SkinnedAnimation == SkinnedAnimation )
		{
			return m_SkinnedAnimations[i];
		}
	}

	return NULL;
} // CRefManager::GetSkinnedAnimation()

CSkinnedGeometry* CRefManager::AddSkinnedGeometry( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
													 std::string Path,				// path to the skinned geometry
													 std::string TexturePath )		// path to the textures (if "" is supplied, will use the mesh path)
{

	// find the absolute path to the model
	Path = StringUpper( AssembleAbsolutePath( m_ExecutablePath, Path ) );
	Path = CleanPath( Path );

	// find any existing copies of the mesh
	SkinnedGeometryRef*	MediaExists	= GetSkinnedGeometry( Path );

	// if there are none, create a new one
	if ( MediaExists == NULL )
	{
		assert( FileExists( Path ) );
		SkinnedGeometryRef*	MediaReference = new SkinnedGeometryRef();
		m_Skins.push_back( MediaReference );

		MediaReference->SkinnedGeometry = new CSkinnedGeometry( *this , m_pSettings );
		MediaReference->SkinnedGeometry->Load( pD3DDevice, Path, TexturePath );
		MediaReference->References = 1;
		MediaReference->Path = Path;

		// return a pointer to the actual mesh
		return MediaReference->SkinnedGeometry;
	} 
	else
	{
		// increment reference count
		MediaExists->References++;
		// return a pointer to the actual mesh
		return MediaExists->SkinnedGeometry;
	}
} // CRefManager::AddSkinnedGeometry()

void CRefManager::RemoveSkinnedGeometry( CSkinnedGeometry* SkinnedGeometry )	// geometry to remove
{
	SkinnedGeometryRef*	MediaExists	= GetSkinnedGeometry( SkinnedGeometry );
	
	if ( MediaExists != NULL )
	{
		MediaExists->References--;
	}
} // CRefManager::RemoveSkinnedGeometry()

// retrieve an existing SkinnedGeometry given its path and index within parent model
SkinnedGeometryRef* CRefManager::GetSkinnedGeometry( std::string Path )		// path to the skinned mesh
{

	for( uint32 i = 0; i < m_Skins.size() ; i++ )
	{
		if( m_Skins[i]->Path == Path )
		{
			return m_Skins[i];
		}
	}

	return NULL;
} // CRefManager::GetSkinnedGeometry()


// retrieve an existing SkinnedGeometry 
SkinnedGeometryRef* CRefManager::GetSkinnedGeometry( CSkinnedGeometry* SkinnedGeometry ) // skinnedmesh to retrieve
{
	for( uint32 i = 0; i < m_Skins.size() ; i++ )
	{
		if( m_Skins[i]->SkinnedGeometry == SkinnedGeometry )
		{
			return m_Skins[i];
		}
	}

	return NULL;
} // CRefManager::GetSkinnedGeometry()


// these are always unique - it's the geometry that gets instanced
CSkinnedMesh* CRefManager::AddSkinnedMesh( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
											 std::string Path,				// path to the skinned mesh
											 std::string TexturePath,		// path to the textures (if "" is supplied, will use the mesh path)
											 std::string AnimationPath )	// path to animation file
{

	// find the absolute path to the model
	Path = StringUpper( AssembleAbsolutePath( m_ExecutablePath, Path ) );
	Path = CleanPath( Path );


	SkinnedMeshRef*	MediaReference = new SkinnedMeshRef();
	m_SkinnedMeshes.push_back( MediaReference );

	MediaReference->SkinnedMesh = new CSkinnedMesh( *this, m_pSettings );
	MediaReference->SkinnedMesh->Load( pD3DDevice, Path, TexturePath, AnimationPath );
	MediaReference->References = 1;
	// return a pointer to the actual mesh
	return MediaReference->SkinnedMesh;

} // CRefManager::AddSkinnedMesh()

void CRefManager::RemoveSkinnedMesh( CSkinnedMesh* SkinnedMesh )	// mesh to remove
{
	SkinnedMeshRef*	MediaExists	= GetSkinnedMesh( SkinnedMesh );
	
	if ( MediaExists != NULL )
	{
		MediaExists->References--;
	}
} // CRefManager::RemoveSkinnedMesh()


// retrieve an existing SkinnedMesh 
SkinnedMeshRef* CRefManager::GetSkinnedMesh( CSkinnedMesh* SkinnedMesh ) // skinnedmesh to retrieve
{
	for( uint32 i = 0; i < m_SkinnedMeshes.size() ; i++ )
	{
		if( m_SkinnedMeshes[i]->SkinnedMesh == SkinnedMesh )
		{
			return m_SkinnedMeshes[i];
		}
	}

	return NULL;
} // CRefManager::GetSkinnedMesh()


#endif

////////////////////////////////////////////////////////////////////////////////
// SAMPLE reference management
////////////////////////////////////////////////////////////////////////////////

#ifdef _SUPPORT_SOUND

FSOUND_SAMPLE* CRefManager::AddSample( std::string SamplePath,	//  path to sound sample
									   bool Looping )			// looping sample? (default = kFalse)
{
	// find the absolute path to the model
	SamplePath = AssembleAbsolutePath( m_ExecutablePath, SamplePath );
#ifdef _WIN32
	SamplePath = StringUpper( SamplePath );
#endif
	SamplePath = CleanPath( SamplePath );

	// find a previously existing sample
	SoundRef*	MediaExists	= GetSample( SamplePath );
	// if it doesn't exist, make a new one
	if ( MediaExists == NULL )
	{
		assert( FileExists( SamplePath ) );
		SoundRef*	MediaReference	= new SoundRef();
		if ( Looping )
		{
			MediaReference->Sample = FSOUND_Sample_Load( FSOUND_FREE,
											SamplePath.c_str(),
											FSOUND_NORMAL |
											FSOUND_LOOP_NORMAL |
											FSOUND_NORMAL,
											0,
											0 );
		}
		else
		{
			MediaReference->Sample = FSOUND_Sample_Load( FSOUND_FREE,
											SamplePath.c_str(),
											FSOUND_NORMAL |
											FSOUND_LOOP_OFF |
											FSOUND_NORMAL,
											0,
											0 );
		}
		if( MediaReference->Sample != NULL )
		{
			m_Sounds.push_back( MediaReference );

			MediaReference->References = 1;
			MediaReference->Path = SamplePath;
			// return a pointer to the actual sample
			return MediaReference->Sample;
		}
		else
		{
			DELETE_SAFELY( MediaReference );
			return NULL;
		}
	}
	else
	{
		// increment the reference count
		MediaExists->References++;
		// return a pointer to the actual sample
		return MediaExists->Sample;
	}
} // CRefManager::AddSample()

// remove an existing sample from the manager
void CRefManager::RemoveSample( FSOUND_SAMPLE* Sample )		// Sample to remove
{
	SoundRef*	MediaExists	= GetSample( Sample );
	if ( MediaExists != NULL )
	{
		MediaExists->References--;
	}
} // CRefManager::RemoveSample()

SoundRef* CRefManager::GetSample( std::string SamplePath )	// path to sample to retrieve
{
	for( uint32 i = 0; i < m_Sounds.size() ; i++ )
	{
		if( m_Sounds[i]->Path == SamplePath )
		{
			return m_Sounds[i];
		}
	}

	return NULL;
} // CRefManager::GetSample()


SoundRef* CRefManager::GetSample( FSOUND_SAMPLE* Sample )	// sample to retrieve
{
	for( uint32 i = 0; i < m_Sounds.size() ; i++ )
	{
		if( m_Sounds[i]->Sample == Sample )
		{
			return m_Sounds[i];
		}
	}

	return NULL;
} // CRefManager::GetSample()
#endif
////////////////////////////////////////////////////////////////////////////////
// GENERAL FUNCTIONALITY
////////////////////////////////////////////////////////////////////////////////

// remove any media items with a reference count of zero
void CRefManager::RemoveUnused()
{

#ifdef _SUPPORT_MODEL
	for( uint32 i = 0; i < m_Models.size() ; i++ )
	{
		if ( i < m_Models.size() && 
			m_Models[i]->References <= 0 )
		{
			DELETE_SAFELY( m_Models[i] );
			m_Models[i] = m_Models[m_Models.size() - 1];
			m_Models.pop_back();
			i--;
		}
	}
#endif

#ifdef _SUPPORT_SCENE
	for( i = 0; i < m_Scenes.size() ; i++ )
	{
		if ( i < m_Scenes.size() && 
			m_Scenes[i]->References <= 0 )
		{
			DELETE_SAFELY( m_Scenes[i] );
			m_Scenes[i] = m_Scenes[m_Scenes.size() - 1];
			m_Scenes.pop_back();
			i--;
		}
	}
#endif

#ifdef _SUPPORT_COLLISIONSCENE
	for( i = 0; i < m_CollisionScenes.size() ; i++ )
	{
		if ( i < m_CollisionScenes.size() && 
			m_CollisionScenes[i]->References <= 0 )
		{
			DELETE_SAFELY( m_CollisionScenes[i] );
			m_CollisionScenes[i] = m_CollisionScenes[m_CollisionScenes.size() - 1];
			m_CollisionScenes.pop_back();
			i--;
		}
	}
#endif

#ifdef _SUPPORT_ANIMATEDMESH
	for( i = 0; i < m_AnimatedMeshes.size() ; i++ )
	{
		if ( i < m_AnimatedMeshes.size() && 
			m_AnimatedMeshes[i]->References <= 0 )
		{
			DELETE_SAFELY( m_AnimatedMeshes[i] );
			m_AnimatedMeshes[i] = m_AnimatedMeshes[m_AnimatedMeshes.size() - 1];
			m_AnimatedMeshes.pop_back();
			i--;
		}
	}

	for( i = 0; i < m_Animations.size() ; i++ )
	{
		if ( i < m_Animations.size() && 
			m_Animations[i]->References <= 0 )
		{
			DELETE_SAFELY( m_Animations[i] );
			m_Animations[i] = m_Animations[m_Animations.size() - 1];
			m_Animations.pop_back();
			i--;
		}
	}
#endif

#ifdef _SUPPORT_SKINNEDMESH
	for( i = 0; i < m_SkinnedMeshes.size() ; i++ )
	{
		if ( i < m_SkinnedMeshes.size() && 
			m_SkinnedMeshes[i]->References <= 0 )
		{
			DELETE_SAFELY( m_SkinnedMeshes[i] );
			m_SkinnedMeshes[i] = m_SkinnedMeshes[m_SkinnedMeshes.size() - 1];
			m_SkinnedMeshes.pop_back();
			i--;
		}
	}
	for( i = 0; i < m_Skins.size() ; i++ )
	{
		if ( i < m_Skins.size() && 
			m_Skins[i]->References <= 0 )
		{
			DELETE_SAFELY( m_Skins[i] );
			m_Skins[i] = m_Skins[m_Skins.size() - 1];
			m_Skins.pop_back();
			i--;
		}
	}
	for( i = 0; i < m_SkinnedAnimations.size() ; i++ )
	{
		if ( i < m_SkinnedAnimations.size() && 
			m_SkinnedAnimations[i]->References <= 0 )
		{
			DELETE_SAFELY( m_SkinnedAnimations[i] );
			m_SkinnedAnimations[i] = m_SkinnedAnimations[m_SkinnedAnimations.size() - 1];
			m_SkinnedAnimations.pop_back();
			i--;
		}
	}

#endif

#ifdef _SUPPORT_MATERIAL
	for( i = 0; i < m_Materials.size() ; i++ )
	{
		if ( i < m_Materials.size() && 
			m_Materials[i]->References <= 0 )
		{
			DELETE_SAFELY( m_Materials[i] );
			m_Materials[i] = m_Materials[m_Materials.size() - 1];
			m_Materials.pop_back();
			i--;
		}
	}
#endif

#ifdef _SUPPORT_SOUND
	for( i = 0; i < m_Sounds.size() ; i++ )
	{
		if ( i < m_Sounds.size() && 
			m_Sounds[i]->References <= 0 )
		{
			DELETE_SAFELY( m_Sounds[i] );
			m_Sounds[i] = m_Sounds[m_Sounds.size() - 1];
			m_Sounds.pop_back();
			i--;
		}
	}
#endif
 
#ifdef _SUPPORT_MOVIE
	for( i = 0; i < m_Movies.size() ; i++ )
	{
		if ( i < m_Movies.size() && 
			m_Movies[i]->References <= 0 )
		{
			DELETE_SAFELY( m_Movies[i] );
			m_Movies[i] = m_Movies[m_Movies.size() - 1];
			m_Movies.pop_back();
			i--;
		}
	}
#endif

#ifdef _SUPPORT_CUBEMAP
	for( i = 0; i < m_CubeMaps.size() ; i++ )
	{
		if ( i < m_CubeMaps.size() && 
			m_CubeMaps[i]->References <= 0 )
		{
			DELETE_SAFELY( m_CubeMaps[i] );
			m_CubeMaps[i] = m_CubeMaps[m_CubeMaps.size() - 1];
			m_CubeMaps.pop_back();
			i--;
		}
	}
#endif

#ifdef _SUPPORT_FONTMETRIC
	for( i = 0; i < m_FontMetrics.size() ; i++ )
	{
		if ( i < m_FontMetrics.size() && 
			m_FontMetrics[i]->References <= 0 )
		{
			DELETE_SAFELY( m_FontMetrics[i] );
			m_FontMetrics[i] = m_FontMetrics[m_FontMetrics.size() - 1];
			m_FontMetrics.pop_back();
			i--;
		}
	}
#endif

} // CRefManager::RemoveUnused()

void CRefManager::RemoveAll()
{

#ifdef _SUPPORT_MODEL
	for( uint32 i = 0; i < m_Models.size() ; i++ )
	{
		DELETE_SAFELY( m_Models [i]);
	}
	m_Models.clear();
#endif

#ifdef _SUPPORT_SCENE
	for( i = 0; i < m_Scenes.size() ; i++ )
	{
		DELETE_SAFELY( m_Scenes [i]);
	}
	m_Scenes.clear();
#endif

#ifdef _SUPPORT_COLLISIONSCENE
	for( i = 0; i < m_CollisionScenes.size() ; i++ )
	{
		DELETE_SAFELY( m_CollisionScenes [i]);
	}
	m_CollisionScenes.clear();
#endif

#ifdef _SUPPORT_ANIMATEDMESH
	for( i = 0; i < m_AnimatedMeshes.size() ; i++ )
	{
		DELETE_SAFELY( m_AnimatedMeshes[i] );
	}

	for( i = 0; i < m_Animations.size() ; i++ )
	{
		DELETE_SAFELY( m_Animations[i] );
	}

	m_AnimatedMeshes.clear();
	m_Animations.clear();
#endif

#ifdef _SUPPORT_SKINNEDMESH
	for( i = 0; i < m_SkinnedMeshes.size() ; i++ )
	{
		DELETE_SAFELY( m_SkinnedMeshes[i] );
	}

	for( i = 0; i < m_Skins.size() ; i++ )
	{
		DELETE_SAFELY( m_Skins[i] );
	}

	for( i = 0; i < m_SkinnedAnimations.size() ; i++ )
	{
		DELETE_SAFELY( m_SkinnedAnimations[i] );
	}

	m_SkinnedMeshes.clear();
	m_Skins.clear();
#endif

#ifdef _SUPPORT_MATERIAL
	for( i = 0; i < m_Materials.size() ; i++ )
	{
		DELETE_SAFELY( m_Materials[i] );
	}
	m_Materials.clear();
#endif

#ifdef _SUPPORT_SOUND
	for( i = 0; i < m_Sounds.size() ; i++ )
	{
		DELETE_SAFELY( m_Sounds[i] );
	}
	m_Sounds.clear();
#endif

#ifdef _SUPPORT_MOVIE
	for( i = 0; i < m_Movies.size() ; i++ )
	{
		DELETE_SAFELY( m_Movies[i] );
	}
	m_Movies.clear();
#endif

#ifdef _SUPPORT_CUBEMAP
	for( i = 0; i < m_CubeMaps.size() ; i++ )
	{
		DELETE_SAFELY( m_CubeMaps[i] );
	}
	m_CubeMaps.clear();
#endif

#ifdef _SUPPORT_FONTMETRIC

	for( i = 0; i < m_FontMetrics.size() ; i++ )
	{
		DELETE_SAFELY( m_FontMetrics[i] );
	}
	m_FontMetrics.clear();
#endif
} // CRefManager::RemoveAll()

void CRefManager::InvalidateDeviceObjects( void )
{
	for( uint32 i = 0; i < m_Animations.size() ; i++ )
	{
		m_Animations[i]->AnimatedGeometry->InvalidateDeviceObjects();
	}

	for( i = 0; i < m_Skins.size() ; i++ )
	{
		m_Skins[i]->SkinnedGeometry->InvalidateDeviceObjects();
	}

	for( i = 0; i < m_Models.size() ; i++ )
	{
		m_Models[i]->Model->InvalidateDeviceObjects();
	}

	for( i = 0; i < m_CubeMaps.size() ; i++ )
	{
		m_CubeMaps[i]->CubeMap->InvalidateDeviceObjects();
	}

	for( i = 0; i < m_Materials.size() ; i++ )
	{
		m_Materials[i]->Material->InvalidateDeviceObjects();
	}

} // CRefManager::InvalidateDeviceObjects()

void CRefManager::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	for( uint32 i = 0; i < m_Animations.size() ; i++ )
	{
		m_Animations[i]->AnimatedGeometry->RestoreDeviceObjects( pD3DDevice );
	}

	for( i = 0; i < m_Skins.size() ; i++ )
	{
		m_Skins[i]->SkinnedGeometry->RestoreDeviceObjects( pD3DDevice );
	}

	for( i = 0; i < m_Models.size() ; i++ )
	{
		m_Models[i]->Model->RestoreDeviceObjects( pD3DDevice );
	}

	for( i = 0; i < m_CubeMaps.size() ; i++ )
	{
		m_CubeMaps[i]->CubeMap->RestoreDeviceObjects( pD3DDevice );
	}

	for( i = 0; i < m_Materials.size() ; i++ )
	{
		m_Materials[i]->Material->RestoreDeviceObjects( pD3DDevice );
	}

} // CRefManager::RestoreDeviceObjects()


