//
// TextureFont.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/UI/Font.h"
#include "WT/System/IO/Stream.h"


namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace WT::System::IO;

class CTextureFont : public CFont
{
public:

	WT_CLASS(CTextureFont, CFont)

	CTextureFont();
	CTextureFont( const CTextureFont& Other );
	virtual ~CTextureFont();


	// Adds a new texture sheet from the files referenced by the given texture and glyph database paths
	virtual bool AddSheet ( LPDIRECT3DBASETEXTURE9 pTexture, const string& DatabasePath );

	// Adds a new texture sheet from the provided texture and glyph database file
	virtual bool AddSheet ( LPDIRECT3DBASETEXTURE9 pTexture, CStream* pDatabaseStream );


private:	// methods

	// Loads the font sheet at the provided index with information from the provided file
	// This method assumes the data is formatted in FontMaker version 5 format
	bool LoadFontMakerFormat5( CStream* pDatabaseStream, uint32 FontSheetIndex );

};


} // namespace UI
} // namespace Graphics
} // namespace WT
