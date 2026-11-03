/*
	IRenderable.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/

#pragma once

#include "d3d9.h"
#include "WT/System/IInterface.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/Serialization/SerializationContext.h"
#include "WT/System/Serialization/Formatters/Agnostic/AgnosticFormatter.h"
#include "WT/Graphics/Camera.h"

namespace WT
{
namespace Graphics
{

class IRenderable;

//Render categories - renderable elements sharing the same render category are 
//grouped together and can be rendered as a group. The render categories
//are essentially equivalent from the point of view of WTPlatform;
//the descriptions here are only suggested uses.
namespace ERenderCategory
{
	enum Type
	{
		PreOpaque, //Pre-opaque render category
		Opaque, //Opaque render category (or 1-bit transparency point-sampled)
		PostOpaque, //Post-opaque render category; recommended for translucent elements
		Count, //Number of render categories
	};
};

//RenderContext provides information relevant to rendering such as
//the camera used and the render category. RenderContext also specifies
//whether there are translucent elements that require special
//treatment, including back-to-front triangle sorting, alpha blending, and 
//Z-buffer write disable.
typedef System::SmartPointer<class CRenderContext> HRenderContext;
class CRenderContext : public System::CObject
{
public:
	WT_CLASS_NO_CREATOR( CRenderContext, System::CObject );

	CRenderContext( CCamera const* Camera = NULL, 
		ERenderCategory::Type RenderCategory = ERenderCategory::Opaque, bool Translucent = false )
		: m_Camera( Camera )
		, m_RenderCategory( RenderCategory )
		, m_Translucent( Translucent )
		, m_UseMaterials( true )
		, m_TranslucentZWrite ( false )
	{
	};

	CCamera const* GetCamera() const {return m_Camera;}

	void SetRenderCategory( ERenderCategory::Type const& RenderCategory )
	{
		if (RenderCategory < 0 || RenderCategory >= ERenderCategory::Count)
		{
			throw std::invalid_argument( "render category" );
		}

		m_RenderCategory = RenderCategory;
	}

	ERenderCategory::Type GetRenderCategory() const {return m_RenderCategory;}

	bool GetUseMaterials() const {return m_UseMaterials;}
	void SetUseMaterials( bool UseMaterials ) {m_UseMaterials = UseMaterials;}

	bool GetTranslucent() const {return m_Translucent;}
	void SetTranslucent( bool Translucent ) {m_Translucent = Translucent;}

	bool GetTranslucentZWrite() const { return m_TranslucentZWrite; }
	void SetTranslucentZWrite( bool TranslucentZWrite ) { m_TranslucentZWrite = TranslucentZWrite; }

	inline virtual bool ShouldRender( IRenderable const& Renderable ) const;

	virtual void SetWorldTransform( LPDIRECT3DDEVICE9 pDevice, CMatrix4f const& Transform ) const
	{
		assert( pDevice != NULL );
		pDevice->SetTransform( D3DTS_WORLD, Transform.GetD3DMatrixPtr() );
	}

	virtual void SetTexture( LPDIRECT3DDEVICE9 pDevice, int32 Stage, LPDIRECT3DBASETEXTURE9 pTexture ) const
	{
		assert( pDevice != NULL );
		pDevice->SetTexture( Stage, pTexture );
	}
  	
private:
	CCamera const*        m_Camera;
	ERenderCategory::Type m_RenderCategory;
	bool				  m_UseMaterials		: 1;
	bool				  m_Translucent			: 1;
	bool                  m_TranslucentZWrite	: 1;

	CRenderContext& operator=( CRenderContext const& );
};

// Interface to a renderable object.
class IRenderable : public System::IInterface
{
public:

	WT_BEGIN_INTERFACE( IRenderable );
		WT_BEGIN_PROPERTIES();
			WT_PROP_RW_BYVAL( Visible, bool );
			WT_PROP_RW_BYVAL( ReceivesShadows, bool );
			WT_PROP_RW_BYVAL( CastsShadows, bool );
			WT_PROP_RW_BYVAL( CastsReflections, bool );
		WT_END_PROPERTIES();
	WT_END_INTERFACE();

	//Render the object. Render() is responsible for checking the render context
	//and any of the object's flags (such as GetVisible()) to determine whether
	//anything needs rendering.
	virtual void Render( LPDIRECT3DDEVICE9 pDevice , CRenderContext const& Context ) = 0;

	//Test whether the object is visible.
	virtual bool GetVisible() const = 0;

	//Set whether the object is visible. This defaults to true.
	virtual void SetVisible( bool Visible ) = 0;

	//Test whether the object receives shadows.
	virtual bool GetReceivesShadows() const = 0;

	//Set whether the object receives shadows. This defaults to true.
	virtual void SetReceivesShadows( bool ReceivesShadows ) = 0;

	//Test whether the object casts shadows.
	virtual bool GetCastsShadows() const = 0;

	//Set whether the object casts shadows. This defaults to true.
	virtual void SetCastsShadows( bool CastsShadows ) = 0;

	//Test whether the object casts reflections.
	virtual bool GetCastsReflections() const = 0;

	//Set whether the object casts reflections. This defaults to true.
	virtual void SetCastsReflections( bool CastsReflections ) = 0;
};

class CRenderableImpl : public IRenderable
{
public:
	virtual bool GetVisible() const {return m_Visible;}
	virtual void SetVisible( bool Visible ) {m_Visible = Visible;}
	virtual bool GetReceivesShadows() const {return m_ReceivesShadows;}
	virtual void SetReceivesShadows( bool ReceivesShadows ) {m_ReceivesShadows = ReceivesShadows;}
	virtual bool GetCastsShadows() const {return m_CastsShadows;}
	virtual void SetCastsShadows( bool CastsShadows ) {m_CastsShadows = CastsShadows;}
	virtual bool GetCastsReflections() const {return m_CastsReflections;}
	virtual void SetCastsReflections( bool CastsReflections ) {m_CastsReflections = CastsReflections;}

protected:
	CRenderableImpl()
		: m_Visible( true )
		, m_ReceivesShadows( true )
		, m_CastsShadows( true )
		, m_CastsReflections( true )
	{
	}

	void InitPropertyValues()
	{
		m_Visible = true;
		m_ReceivesShadows = true;
		m_CastsShadows = true;
		m_CastsReflections = true;
	}

private:
	bool m_Visible : 1;
	bool m_ReceivesShadows : 1;
	bool m_CastsShadows : 1;
	bool m_CastsReflections : 1;
};

/*virtual*/ bool CRenderContext::ShouldRender( IRenderable const& Renderable ) const
{
	return Renderable.GetVisible();
}

}
}
