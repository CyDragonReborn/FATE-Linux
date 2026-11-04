//
// FontUtil.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/FontUtil.h"
#include "WT/Graphics/UI/Font.h"
#include "windows.h"

namespace WT
{
namespace Graphics
{
namespace UI
{


void WTCall CFontUtil::ReorderString ( wstring& Text )
{
	//calculate position of characters for the line itself
	HDC dc = GetDC ( NULL );

	GCP_RESULTSW GCPResults;
	memset ( &GCPResults , 0 , sizeof ( GCP_RESULTS ));
	WideLetter* pResult = new WideLetter[Text.size()+1];
	memset ( pResult , 0 , sizeof(WideLetter) * (Text.size()+1));
	GCPResults.lStructSize = sizeof ( GCPResults );
	GCPResults.nGlyphs = static_cast<UINT>( Text.size() );
	GCPResults.lpOutString = pResult;
	GetCharacterPlacementW( dc, Text.c_str(), static_cast<int>(Text.size()), 0, &GCPResults, GCP_REORDER );
	ReleaseDC ( NULL , dc );
	Text = pResult;
	delete[] pResult;
}


} // namespace UI
} // namespace Graphics
} // WT
