#pragma once

#include "WT/System/Object.h"
#include "WT/System/IO/EncodedTextStream.h"

namespace WT
{
namespace System
{
namespace IO
{
	class CStream;
}

namespace Xml
{



// Used internally by XmlWriter
namespace XmlEntryName
{
	enum Type
	{
		StartDocument1,
		Utf8EncodingName,
		Utf16EncodingName,
		StartDocument2,
		Tab,
		Newline,
		NodeStart,
		NodeEnd,
		EndSlash,
		Space,
		Equals,
		DQuote,
		Bang,
		Dash,
	};
}

typedef SmartPointer<class CXmlReader> HXmlReader;
class CXmlWriter : public CObject
{
public:

	WT_CLASS_NO_CREATOR(CXmlWriter, CObject)
	CXmlWriter ( IO::CStream* stream, StringEncoding::Type encoding );
	virtual ~CXmlWriter();
	virtual IO::CEncodedTextStream* GetEncodedTextStream();

	// Closes this stream and the underlying stream. 
	virtual void Close();

	//  Flushes whatever is in the buffer to the underlying streams and also flushes the underlying stream. 
	virtual void Flush ();

	//  Returns the closest prefix defined in the current namespace scope for the namespace URI. 
//	LookupPrefix();

	// Writes out all the attributes found at the current position in the XmlReader. 
//	WriteAttributes();

	// Overloaded. Writes an attribute with the specified value. 
	virtual void WriteAttributeString(wstring const & attribName, wstring const & attribValue);

    // Encodes the specified binary bytes as base64 and writes out the resulting text. 
	virtual void WriteBase64(uint8* pBuffer, uint32 offset, uint32 count);

	// Encodes the specified binary bytes as binhex and writes out the resulting text. 
//	WriteBinHex();

	// Writes out a <![CDATA[...]]> block containing the specified text. 
//	WriteCData();

	// Forces the generation of a character entity for the specified Unicode character value. 
//	WriteCharEntity();

	// Writes text one buffer at a time. 
//	WriteChars();

	// Writes out a comment <!--...--> containing the specified text. 
	virtual void WriteComment(wstring const &text);

	// Writes the DOCTYPE declaration with the specified name and optional attributes. 
//	WriteDocType();
/*
	// Overloaded. Writes an element containing a string value. 
	WriteElementString();

	// Closes the previous WriteStartAttribute call. 
	WriteEndAttribute();
*/
	// Closes any open elements or attributes and puts the writer back in the Start state. 
	virtual void WriteEndDocument();

	// Closes one element and pops the corresponding namespace scope. 
	virtual void WriteEndElement();
/*
	// Writes out an entity reference as &name;. 
	WriteEntityRef();

	// Closes one element and pops the corresponding namespace scope. 
	WriteFullEndElement();

	// Writes out the specified name, ensuring it is a valid name according to the W3C XML 1.0 recommendation (http://www.w3.org/TR/1998/REC-xml-19980210#NT-Name). 
	WriteName();

	// Writes out the specified name, ensuring it is a valid NmToken according to the W3C XML 1.0 recommendation (http://www.w3.org/TR/1998/REC-xml-19980210#NT-Name). 
	WriteNmToken();

	// Copies everything from the reader to the writer and moves the reader to the start of the next sibling. 
	WriteNode();

	// Writes out a processing instruction with a space between the name and text as follows: <?name text?>. 
//	WriteProcessingInstruction();

	// Writes out the namespace-qualified name. This method looks up the prefix that is in scope for the given namespace. 
	WriteQualifiedName();

	// Overloaded. Writes raw markup manually. 
	WriteRaw();

	// Overloaded. Writes the start of an attribute. 
	WriteStartAttribute();
*/
	// Overloaded. Writes the XML declaration. 
	virtual void WriteStartDocument();

	// Overloaded. Writes the specified start tag.
	virtual void WriteStartElement(wstring const & eltName);

	// Writes the given text content. 
	virtual void WriteString(wstring const & text);
/*
	// Generates and writes the surrogate character entity for the surrogate character pair. 
	WriteSurrogateCharEntity();

	// Writes out the given white space. 
	WriteWhitespace();
*/
private:
	struct ElementInfo
	{
		ElementInfo(wstring const & name) : m_Name(name), m_Leaf(true)
		{
		}
		ElementInfo(ElementInfo const & right) : m_Name(right.m_Name), m_Leaf(right.m_Leaf)
		{
		}

		wstring m_Name;
		bool m_Leaf;
	};

	// a stack that keeps track of all the elements that are above us.
	// This class also updates whether or not an element is a leaf element or not
	// based on whether or not sub elements were eventially added benith it.
	class CElementInfoStack 
	{
	public:
		void Push(wstring const & eltName);
		ElementInfo Pop();
		bool IsEmpty();
	private:
		std::list<ElementInfo> m_Stack;
	};

	void IncrementIndent();
	void DecrimentIndent();
	void CloseOpenStartElement(bool TriggeredBySubElement);

	void Write(XmlEntryName::Type entry);
	void Write( const wstring& Text );
	void WriteIndent();

	IO::CTextEncoder* m_pEncoder;
	IO::CEncodedTextStream* m_EncodedTextStream;
	int32 m_IndentAmt;

	// true after WriteStartElement() until WriteString(), WriteEndElement() or someother 
	// method that forces the completed element (no more attribs can be added) to the stream.
	// this flag is needed so that attributes can still be added to an element
	// after the WriteStartElement() call.
	bool m_StartEltOpen;
	CElementInfoStack m_ElementInfoStack;
	StringEncoding::Type m_Encoding;
};

}
}
}