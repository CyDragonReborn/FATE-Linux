#pragma once

#include "WT/System/Object.h"
#include "WT/System/IO/TextStreamReader.h"
#include "WT/System/IO/EncodedTextStream.h"
#include "WT/System/Xml/XmlException.h"

namespace WT
{
namespace System
{
namespace Xml
{

using namespace WT::System::IO;

namespace NodeType
{
	enum Type
	{
		Unset,
		StartElementNode,
		EndElementNode,
		TextNode,
		PINode, // Processing Instruction node (eg <? ... ?>
		NoMoreNodes,
		Error // encountered an error, trying to read the next node, unrecoverable
	};
} // namespace NodeType

class CTextStreamUtf8Reader;

typedef SmartPointer<class CXmlReader> HXmlReader;
class CXmlReader : public CObject
{
public:

	WT_CLASS_NO_CREATOR(CXmlReader, CObject)

	CXmlReader ( IO::CStream* stream );
	virtual ~CXmlReader();
	virtual IO::CEncodedTextStream* GetEncodedTextStream();

	// Gets the number of attributes on the current node. 
	virtual int32 AttributeCount() const;

	// Gets the depth of the current node in the XML document. 
	virtual int32 Depth() const;

	// Gets a value indicating whether the current node has any attributes. 
	virtual bool HasAttributes() const;

	// Gets the qualified name of the current node. 
	virtual wstring const& Name() const;

	// Gets the type of the current node. 
	virtual NodeType::Type NodeType() const;

	// Gets the text value of the current node. 
	virtual wstring const& Value() const;

	// Overloaded. When overridden in a derived class, gets the value of an attribute. 
	// returns true if the attribute existed and was placed into outValue
	virtual bool GetAttribute(wstring const & attribName, wstring& outValue) const;

	// returns the value (in outValue) of the attribute at the given index
	// return false if there was no attribute at that index
	virtual bool GetAttributeValue(int32 attribIdx, wstring& outValue) const;

	// returns the name (in outName) of the attribute at the given index
	// return false if there was no attribute at that index
	virtual bool GetAttributeName(int32 attribIdx, wstring& outName) const;

	// Overloaded. Tests if the current content node is a start tag. 
	virtual bool IsStartElement() const;

	// Reads the next node from the stream. 
	// returns true if it was able to move to the next node.
	virtual bool Read();

	// Skips the children of the current node. 
	virtual void Skip();

protected:
	// resets all data associated with node info back to a defaul value.
	// 
	void ClearNodeData();

	// all of these methods will throw a XMLReaderException() if things go wrong.
	// these may also throw UnexpectedEndOfDataException
	NodeType::Type ReadNextNode();
	void InitPINode(wstring const & name);
	void InitStartElementNode(wstring const & name);
	void InitEndElementNode(wstring const & name);
	void InitTextNode(wstring const & text);
	void AddAttribute(wstring const & attribName, wstring const & attribValue);
	wstring ReadXmlName();
private:

	NodeType::Type m_NodeType;
	wstring m_NodeName; // valid for start element, end element and PI node tyoes
	wstring m_TextNodeStr;
	std::map<wstring, wstring> m_ElementNodeAttribNames2Values;
	std::vector<wstring> m_AttribNames;

	// if true, XmlReader will inject a close element node on the next ReadNextNode() call.
	// this is used to normalize the handling of elements that look like this: <eltname />
	bool m_AutoCloseElement; 


	std::list<wstring> m_ElementNameStack;

	IO::CStream* m_BaseStream;
	CTextEncoder* m_pTextEncoder;
	CEncodedTextStream* m_pEncodedStream;
	CTextStreamReader* m_TextStreamReader;
	CTextStreamUtf8Reader* m_TextStreamUtf8Reader;
};

}
}
}