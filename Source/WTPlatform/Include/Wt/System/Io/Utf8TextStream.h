#pragma once

#include "WT/System/Object.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/IO/IOException.h"

namespace WT
{
namespace System
{
namespace IO
{

class Utf8TextStream : public Object
{
public:
	WT_CLASS( Utf8TextStream, Object );

	Utf8TextStream(Stream* BaseStream);
	virtual ~Utf8TextStream();
/*
	// Overloaded. Overridden. Calculates the number of bytes required to store the results of encoding a set of characters from a specified String or Unicode character array. 
	virtual int32 GetByteCount(wstring const & wstr);
	virtual int GetByteCount(wstring::valuetype[] wstrbuf, int32 offset, int32 size);
	// from encoding:
	virtual int GetByteCount(wstring::valuetype[] wstrbuf);

	// Overloaded. Overridden. Encodes a specified range of characters from a Unicode character array or a String and stores the results in a specified byte array. 
	virtual int GetBytes(string, int32, int32, byte[], int32);
	virtual byte[] GetBytes(string);
	virtual int GetBytes(char[], int32, int32, byte[], int32);
	// from encoding:
	virtual byte[] GetBytes(char[]);
	virtual byte[] GetBytes(char[], int32, int32);

	// Overloaded. Overridden. Calculates the number of characters that would result from decoding a set of elements in a byte array. 
	virtual int GetCharCount(byte[], int32, int32);
	// from encoding:
	virtual int GetCharCount(byte[]);

	// Overloaded. Overridden. Decodes a set of elements from a specified byte array and stores the result into a specified set of elements in a Unicode character array. 
	virtual int GetChars(byte[], int32, int32, char[], int32);
	// from encoding:
	virtual char[] GetChars(byte[]);
	virtual char[] GetChars(byte[], int32, int32);

	// Overridden. Obtains a decoder that can convert a UTF-8 encoded sequence of bytes into a sequence of Unicode characters. 
	//public override Decoder GetDecoder();

	// Overridden. Obtains an encoder that can convert a sequence of Unicode characters into a UTF-8 encoded sequence of bytes. 
	//public override Encoder GetEncoder();

	// Overridden. Calculates the maximum number of bytes required to encode a specified number of characters. 
	virtual int GetMaxByteCount( int32 charCount );

	// Overridden. Calculates the maximum number of characters that can result from decoding a specified number of bytes. 
	virtual int GetMaxCharCount( int32 byteCount );

	// Overridden. Returns a Unicode byte order mark encoded in UTF-8 format, if the constructor for this instance requested byte order mark support. 
	//public override byte[] GetPreamble();

	// Overloaded. Decodes the specified byte array into a string.
	// (inherited from Encoding) 
	virtual string GetString(byte[]);
	virtual string GetString(byte[], int32, int32);
*/

	// TODO: make this non-pulbic
	virtual uint32 ReadChar(uint32& charByteWidth);
protected:
	Utf8TextStream();

	
private:
	Stream* m_BaseStream;
};

}
}
}