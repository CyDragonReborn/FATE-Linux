/*
	HuffmanEncoding.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#pragma once

#include "WT/System/Platform.h"

namespace WT
{
namespace Graphics
{

//Construct Huffman codes based on the frequency of the symbols
void ComputeHuffmanCodes(int32 NumSymbols, int32 freq[], int32 length[], int32 huff[]);

} //namespace Graphics
} //namespace WT
