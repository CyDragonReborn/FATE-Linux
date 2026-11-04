/******************************************************************************
	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved

	History:
	2/13/2006	Rui Jiang	Createed

	Description:  This is a very small easy to use xml parser. However, 
	it has limited functionalities. 
******************************************************************************/
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Util/Util.h"
#include "WT/System/Platform.h"

#include <map>
#include <string>
#include <iostream>

namespace WT
{
namespace System
{
namespace Xml
{

class RStringBuffer
{
public:
	RStringBuffer(const char* s)
	{
		buf = s;
		pos = 0;
	}
	char getChar()
	{
		if (buf == NULL) return 0;
		char c = buf[pos];
		if (c != 0) pos++;
		return c;
	}
	//Read first non white char.
	char getnc()
	{
		ignoreSpaces();
		return getChar();
	}
	void ungetc()
	{
		if (pos > 0) pos--;
	}
	//Eat white spaces until non-white.
	void ignoreSpaces()
	{
		char c;
		while((c = getChar()) != 0)
		{
			if (c < 0 || !isspace(c)) 
			{
				ungetc();
				break;
			}
		}
	}
	//Read until the sub std::string appears. If it never appear, then false. Position not changed.
	bool readUntil(const char* substr, std::string* res = NULL)
	{
		if (buf == NULL) return false;
		char const * start = strstr(buf + pos, substr);
		
		if (start == NULL) return false;
		else
		{
			if (res != NULL)
				res->assign(buf + pos, start - (buf + pos));
			pos = (int)(start + strlen(substr) - buf);
			return true;
		}
	}
	//Read tag names or attribute names. For xml parsing.
	bool readName(std::string* s)
	{
		char c;
		while((c = getChar()) != 0)
		{
			if (isalpha(c) || isdigit(c) || c == ':' || c == '-' || c == '_')
				(*s) += c;
			else
			{
				ungetc();
				break;			
			}
		}
		return (s->size() > 0);
	}
	bool eof() { return buf == NULL || buf[pos] == 0; } 
protected:
	const char* buf;
	int	pos;
};

///////////////////////////////////////////////////////////////////////////////
// Simpler xml. 

class CSmallXml
{
public:
	CSmallXml();
	//Clone another one.
	CSmallXml(const CSmallXml* p);
	//This one will parse the std::string. if not successful, throw an exception.
	CSmallXml(const std::string& xml);
	~CSmallXml();
	//Parse xml string. Note you must clear this node if it already has some content. 
	//Otherwise parse will fail.
	bool parse(const std::string& input);
	bool parse(const char* input);
	//Clear all names, children etc.
	void clear();
public:
	typedef Util::autoreleaselist<std::vector<CSmallXml*> > ChildrenListType;
	typedef std::map<std::string, std::string> AttributeListType;
	
	///////////////////////////////////////////////////////////////////////////
	//My implementation.
	std::string& getName() { return tag; }
	void	setName(std::string newNodeName) { if(newNodeName.size() > 0) tag = newNodeName; }
	std::string& getText() { return text; }
	void	setText(std::string textNodeValue = "") { text = textNodeValue; }
	CSmallXml*	getParent() { return parent; } 

	//Miscellanous functions to get parameters.
	int paramSize() const { return attrs == NULL? 0 : (int)attrs->size(); }
	bool getParam(const std::string& name, std::string& param) const ;
	std::string getParam(const std::string& name) const ;
	bool getParam(const std::string& name, int& param) const ;
	int getParamInt(const std::string& name) const;
	std::string paramtoString();
	void addParam(const std::string& key, const std::string& value);
	void removeParam(const std::string& key);

	//Stream out the whole tree.
	void output(std::ostream& os) const;
	void outputParams(std::ostream& os) const;

	//Format output with indent.
	void output(std::ostream& os, int thisindent, int indent)  const;
	//Output children only with indent.
	void outputChildren(std::ostream& os, int thisindent, int indent)  const;

	void HRoutput(std::ostream& os) const;
	void HRoutputParams(std::ostream & os) const;
	void toString(std::string& out) const;
	void toString(std::string& out, bool humanReadable, int indent = 0) const;
	std::string toString() const;
	//Now only formatted output available.
	void childrenToString(std::string & out, int initialIndent, int indent) const;
	
	//Chidlren count.
	int		size() const { return children == NULL? 0 : (int)children->size();}
	//Get one of the children.
	CSmallXml*	operator[](int index); 
	//Get child by path. like: "Smil/to". Tag names seperated by / or "\"
	CSmallXml*	getChild(const std::string& path);
	int		getChildInt(const std::string& path, int defaultValue = 0);
	std::string	getChildString(const std::string& path, const std::string& defaultValue = "");
	std::string	getChildParamString(const std::string& path, const std::string& param, const std::string& defaultValue = "");
	
	CSmallXml* find(const std::string& path) { return getChild(path); }
	//Add child. if pos is -1, then to the end.
	void addChild(CSmallXml* child, int pos = -1);
	//Remove child at pos..
	void removeChild(int pos);
	void removeAllChildren();
	//Get my position in the parent's children.
	int	getPos();

	//Remove myself from parent.
	void subtreeMe();

protected:
	//Only create when necessary.
	ChildrenListType*	children;
	//Only create when necessary.
	AttributeListType*	attrs;
	//Parent.
	CSmallXml*	parent;
	//Tag of myself.
	std::string	tag;
	std::string	text;
	//Parent's text. Parent might have text between children.
	std::string	parentText;

	CSmallXml* getDirectChild(const std::string& tagName);
	void Init()
	{
		children = NULL;
		attrs = NULL;
		parent = NULL;
	};
	bool parse(RStringBuffer* buf);
	//Read in attribute in the form: name = '...'
	bool parseAttr(RStringBuffer* buf);
	//Eat one child. starting from <..> to <..> or in the form <../>
	bool parseChild(RStringBuffer* buf);
	//Eat the close tag: </...> It must match open tag.
	bool parseCloseTag(RStringBuffer* buf);
};


std::ostream& operator << (std::ostream& os, const CSmallXml& xml);

}}} //namespace