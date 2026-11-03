/*
	HuffmanEncoding.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#include "WT/stdafx.h"

#include <algorithm>
#include <cassert>
#include "WT/Mesh/HuffmanEncoding.h"

namespace WT
{
namespace Graphics
{

namespace
{

//This function is used for the Huffman compression below.
//
//Note: if the array element is a negative value,
//it's a pointer to another array element. Otherwise,
//it's a symbol frequency. (This rule is just to aid 
//in debugging.)
//
//Move the top element down through the heap as necessary
//to ensure the heap condition, namely that each parent 
//has less frequency than either of its children.
void sift(int32 a[], int32 n)
{
    int32 i = 0;
    for (;;)
    {
        int32 child1 = i * 2 + 1;

        if (child1 >= n)
        {
            //No child
            break;
        }
        
        int32 child2 = child1 + 1;

        if (child2 >= n) {
            //One child
            if (a[-a[i]] > a[-a[child1]])
            {
                int32 t = a[i];
                a[i] = a[child1];
                a[child1] = t;
            }
            break;
        }

        //Two children
        if (a[-a[i]] > a[-a[child1]] || a[-a[i]] > a[-a[child2]])
        {
            int32 child = child1;
            if (a[-a[child2]] < a[-a[child1]])
            {
                child = child2;
            }

            int32 t = a[i];
            a[i] = a[child];
            a[child] = t;
            i = child;
        } else {
            break;
        }
    }
}

//This function is used for the Huffman compression below.
//Add a value to the heap. We add it to then end of the
//heap and then move it up as necessary to maintain the heap
//condition.
void addheap(int32 a[], int32 n, int32 value)
{
    a[n] = value;

    for (int32 i = n; i > 0;)
    {
        int32 parent = (i - 1) / 2;
        if (a[-a[i]] > a[-a[parent]])
        {
            break;
        }

        int32 t = a[i];
        a[i] = a[parent];
        a[parent] = t;
        i = parent;
    }
}

} //anonymous namespace

//Construct Huffman codes based on the frequency of the symbols
void ComputeHuffmanCodes(int32 NumSymbols, int32 freq[], int32 length[], int32 huff[])
{
    //First construct the Huffman code using a heap
    //(See p. 74 of Data Compression 2nd Edition)

    int32* a = new int32[NumSymbols * 2];
    for (int32 i = 0; i < NumSymbols; ++i)
    {
        a[NumSymbols + i] = freq[i];

        //Add the pointer to the heap
        addheap(a, i, -NumSymbols - i);
    }

    for (int32 HeapSize = NumSymbols; HeapSize > 1; HeapSize--)
    {
        int32 smallest_ptr = -a[0];
        a[0] = a[HeapSize - 1];
        sift( a, HeapSize - 1 );
        int32 next_smallest_ptr = -a[0];
        a[HeapSize - 1] = a[smallest_ptr] + a[next_smallest_ptr];
        int32 PtrToFreqSum = -(HeapSize - 1);
        a[smallest_ptr] = PtrToFreqSum;
        a[next_smallest_ptr] = PtrToFreqSum;
        a[0] = PtrToFreqSum;
        sift(a, HeapSize - 2 );
    }    

    //Determine length of each Huffman code
    //by traversing the heap from each symbol's element
    //until the root is reached.
    for (int32 i = 0; i < NumSymbols; ++i)
    {
        length[i] = 0;
        for (int32 j = i + NumSymbols; j != 1; j = -a[j])
        {
            ++length[i];
        }
    }

    delete[] a;

    //Determine the number of codes with each length
    int32 const MaxLength = NumSymbols;
    int32* numl = new int32[MaxLength + 1];
    std::fill(numl, numl + MaxLength + 1, 0);
    int32 Longest = 0;
    for (int32 i = 0; i < NumSymbols; ++i)
    {
        assert( length[i] < MaxLength );
        if (length[i] > Longest)
        {
            Longest = length[i];
        }
        ++numl[length[i]];
    }

    //Now choose Huffman codes for each symbol
    int32* first;
    first = new int32[MaxLength + 1];
    first[Longest] = 0;
    for (int32 i = Longest - 1; i >= 1; --i)
    {
        first[i] = (first[i + 1] + numl[i + 1]) / 2;
    }

    delete[] numl;

    for (int32 i = 0; i < NumSymbols; ++i)
    {
        huff[i] = first[length[i]];
        ++first[length[i]];
    }

    delete[] first;
}

} //namespace Graphics
} //namespace WT
