/******************************************************************************
	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved

	History:
	2/13/2006	Rui Jiang	Createed

	Description:  This is a very small easy to use xml parser. However, 
	it has limited functionalities. 
******************************************************************************/
#include "WT/stdafx.h"
#include "SmallXml.h"

#include <strstream>
#include <assert.h>

using namespace std;

#pragma warning(disable:4127) //warning C4127: conditional expression is constant

namespace WT
{
namespace System
{
namespace Xml
{


CSmallXml::CSmallXml()
{
	Init();
}

CSmallXml::~CSmallXml()
{
	clear();
}

//Clear all names, children etc.
void CSmallXml::clear()
{
	delete children;
	children = NULL;
	delete attrs;
	attrs = NULL;
	parent = NULL;
	tag.clear();
	text.clear();
	parentText.clear();
}

//Clone another one.
CSmallXml::CSmallXml(const CSmallXml* p)
{
	Init();
	if (p == NULL) throw "CSmallXml::CSmallXml -- Error copying from NULL node";
	else
	{
		//std::string s = p->toString();
		std::string s = "";
		p->toString(s,false);
		if (!parse(s))
			throw "Error parse xml.";
	}
}

//This one will parse the std::string. if not successful, throw an exception.
CSmallXml::CSmallXml(const std::string& xml)
{
	Init();
	if (!parse(xml))
		throw "Error parse xml.";
}

bool CSmallXml::parse(const std::string& input)
{
	return parse((const char*)input.c_str());
}

bool CSmallXml::parse(const char* input)
{
	RStringBuffer buf(input);
	return parse(&buf);
}

bool CSmallXml::parse(RStringBuffer* buf)
{
	char c;
	c = buf->getnc();

	if (c != '<') return false;
	c = buf->getChar();
	if (c == '?') 
	{
		if (!buf->readUntil("?>")) return false;
		buf->ignoreSpaces();
		c = buf->getChar();
		if (c != '<') return false;
	}
	else
		buf->ungetc();
	
	buf->ignoreSpaces();
	//Now, read in tag name.
	if (!buf->readName(&tag)) return false;

	//after getting name. Read in attributes.
	while(true)
	{
		c = buf->getnc();
		if (c == '/')
		{
			if (buf->getnc() == '>') return true;
			else return false;
		}
		else if (c == '>')
		{
			while(true)
			{
				//Read in body.
				c = buf->getnc();
				if (c == '<')
				{
					//Child or closing tag?
					c = buf->getnc();
					if (c == '/')
					{
						//Here there might be problem when there some space between
						// "< abc" Fix later !!!!!!!!!!!!!!!!!!!!!1
						buf->ungetc();
						buf->ungetc();
						return parseCloseTag(buf);
					}
					else
					{
						//Here there might be problem when there some space between
						// "< abc" Fix later !!!!!!!!!!!!!!!!!!!!!1
						buf->ungetc();
						buf->ungetc();
						if (!parseChild(buf)) return false;
					}
				}
				else
				{
					//In Text.
					buf->ungetc();
					while(true)
					{
						std::string tmp;
						if (buf->readUntil("<", &tmp))
						{
							text += tmp;
							c = buf->getnc();
							if (c == '/')
							{
								//ending.
								buf->ungetc();
								buf->ungetc();
								UnEscapeXmlString(text);
								return parseCloseTag(buf);
							}
							else
							{
								//Starting of another child?
								buf->ungetc();
								buf->ungetc();
								if (!parseChild(buf)) return false;
							}							
						}
						else
						{
							//Not cat end? 
							return false;
						}
					}
					/*
					if (buf->readUntil("</", &text)) 
					{
						buf->ungetc();
						buf->ungetc();
						UnEscapeString(text);
						return parseCloseTag(buf);
					}
					else return false;
					*/
				}
			}
		}
		else //Not / >, start of atttribute.
		{
			buf->ungetc();
			if (!parseAttr(buf)) return false;
		}
	}
	return false;
}
//Read in attribute in the form: name = '...'
bool CSmallXml::parseAttr(RStringBuffer* buf)
{
	char c;
	std::string name, attr;
	
	if (buf->readName(&name))
	{
		c = buf->getnc();
		if (c == '=')
		{
			//Read in value.
			c = buf->getnc();
			if (c == '\'' || c == '\"')
			{
				char end[2];
				end[0] = c;
				end[1] = 0;
				if (buf->readUntil(end, &attr)) 
				{
					if (attrs == NULL)
						attrs = new AttributeListType;
					(*attrs)[name] = attr;
					return true;
				}
			}
		}
	}
	return false;
}
//Eat one child. starting from <..> to <..> or in the form <../>
bool CSmallXml::parseChild(RStringBuffer* buf)
{
	//Child.
	CSmallXml* child;

	child = new CSmallXml;
	if (child->parse(buf))
	{
		addChild(child);
		return true;
	}
	else
	{
		delete child;
		return false;
	}
}
//Eat the close tag: </...> It must match open tag.
bool CSmallXml::parseCloseTag(RStringBuffer* buf)
{
	//Closing tag.
	char c;
	c = buf->getnc();
	if (c != '<') return false;
	c = buf->getnc();
	if (c != '/') return false;

	std::string endtag;
	buf->readName(&endtag);
	if (endtag != tag) return false;
	
	if (buf->getnc() != '>') return false;
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// CSmallXml Implementation.

//Add child. if pos is -1, then to the end.
void CSmallXml::addChild(CSmallXml* child, int pos)
{
	if (children == NULL)
		children = new ChildrenListType();
	if (pos < 0)
		children->push_back(child);
	else
	{
		assert(pos <= (int)children->size());
		children->insert(children->begin() + pos, child);
	}
	child->parent = this;
}

//Remove child at pos..
void CSmallXml::removeChild(int pos)
{
	if (pos < 0) return;
	if (children == NULL) return;
	if (pos >= (int)children->size()) return;
	children->erase( children->begin() + pos );
}

void CSmallXml::removeAllChildren()
{
	if (children == NULL) return;
	children->clear();
}

void CSmallXml::addParam(const std::string& key, const std::string& value)
{
	if (attrs == NULL)
		attrs = new AttributeListType();
	(*attrs)[key] = value;
}

void CSmallXml::removeParam(const std::string& key)
{
	if(attrs->find(key) != attrs->end())
	{
		attrs->erase(attrs->find(key));
	}
}

bool CSmallXml::getParam(const std::string& name, std::string& param) const
{ 
	if (attrs)
	{
		AttributeListType::iterator it;
		if ((it = attrs->find(name)) != attrs->end())
		{
			param = (it->second);
			return true;
		}
	}
	return false;
}

std::string CSmallXml::getParam(const std::string& name) const
{ 
	std::string res;
	getParam(name, res);
	return res;
}

bool CSmallXml::getParam(const std::string& name, int& param) const
{
	std::string res;
	if (!getParam(name, res)) return false;
	param = atoi(res.c_str());
	return true;
}

int CSmallXml::getParamInt(const std::string& name) const
{
	int n;
	if (!getParam(name, n)) return 0;
	return n;
}

std::string CSmallXml::paramtoString()
{
	ostrstream os;
	
	outputParams(os);

	std::string res(os.str(), os.pcount());
	delete [] os.str();
	return res;
}


void CSmallXml::outputParams(std::ostream& os) const
{
	if (attrs != NULL)
	{
		AttributeListType::iterator it;
		for(it = attrs->begin(); it != attrs->end(); it++)
		{
			os << " " << it->first << "=\"" << EscapeStringForXml(it->second) << "\"";
		}
	}
}

void CSmallXml::HRoutputParams(std::ostream& os) const
{
	if (attrs != NULL)
	{
		AttributeListType::iterator it;
		for(it = attrs->begin(); it != attrs->end(); it++)
		{
			os << " " << it->first << "=\"" << it->second << "\"";
		}
	}
}

void CSmallXml::output(std::ostream& os) const
{
	if (tag.size() == 0)
		os<<EscapeStringForXml(text);
	else
	{
		os<<"<" << tag;
		outputParams(os);
		if (text.size() > 0 || size() > 0)
		{
			os<<">";
			os<<EscapeStringForXml(text);
			ChildrenListType::iterator it;
			if (children != NULL)
			for(it = children->begin(); it != children->end(); it++)
			{
				os<<(**it)<< (*it)->parentText;
			}
			os<<"</" << tag << ">";
		}
		else
			os<<"/>";
	}
}

//Output children only with indent.
void CSmallXml::outputChildren(std::ostream& os, int thisindent, int indent)  const
{
	ChildrenListType::iterator it;
	if (children != NULL)
	for(it = children->begin(); it != children->end(); it++)
	{
		(*it)->output(os, thisindent, indent);
	}
}

//Format output with indent.
void CSmallXml::output(std::ostream& os, int thisindent, int indent) const
{
	std::string s(thisindent, ' ');
	if (tag.size() == 0)
		os<< ConvertToWindowsString(text);
	else
	{
		os<< s << "<" << tag;
		HRoutputParams(os);
		if (size() == 0)
		{
			if (text.size())
				os<<">" << ConvertToWindowsString(text) << "</" << tag << ">" << "\r\n";
			else
				os<< "/>" << "\r\n";
		}
		else
		{
			os<<">";
			os << "\r\n";
			outputChildren(os, thisindent + indent, indent);
			os<< s << "</" << tag << ">" << "\r\n";
		}
	}
}

void CSmallXml::HRoutput(std::ostream& os) const
{
	if (tag.size() == 0)
		os<<text;
	else
	{
		os<<"<" << tag;
		HRoutputParams(os);
		if (text.size() > 0 || size() > 0)
		{
			os<<">\r\n";
			if(size() == 0)
			{
				os<<text;
			}
			ChildrenListType::iterator it;
			if (children != NULL)
			for(it = children->begin(); it != children->end(); it++)
			{
				(*it)->HRoutput(os);
			}
			os<<"</" << tag << ">\r\n";
		}
		else
			os<<"/>\r\n";
	}
}

void CSmallXml::toString(std::string& out) const
{
	strstream os;
	os<<(*this);
	out.assign(os.str(), os.pcount());
	delete [] os.str();
}

//Now only formatted output available.
void CSmallXml::childrenToString(std::string & out, int initialIndent, int indent) const
{
	strstream os;
	outputChildren(os, initialIndent, indent);
	out.assign(os.str(), os.pcount());
	delete [] os.str();
}

void CSmallXml::toString(std::string& out, bool humanReadable, int indent) const
{
	strstream os;
	if(humanReadable)
	{
		if (indent == 0)
			this->HRoutput(os);
		else
			this->output(os, 0, indent);
	}
	else
	{
		os<<(*this);
	}
	out.assign(os.str(), os.pcount());
	delete [] os.str();
}

std::string CSmallXml::toString() const
{
	std::string res;
	toString(res);
	return res;
}

std::ostream& operator << (std::ostream& os, const CSmallXml& xml)
{
	xml.output(os);
	return os;
}

//Get one of the children.
CSmallXml* CSmallXml::operator[](int index)
{
	if (index < 0 || index >= size()) 
	{
		throw "CSmallXml::operator[] out of range.";
	}
	else
	{
		return (*children)[index];
	}
}

//Case insensative.
CSmallXml* CSmallXml::getDirectChild(const std::string& tagName)
{
	CSmallXml* p;
	if (children == NULL || children->size() == 0) return NULL;
	for(int i = 0; i < (int)children->size(); i++)
	{
		p = (*children)[i];
		if (strieq(p->tag, tagName)) return p;
	}
	return NULL;
}

//Find child
CSmallXml* CSmallXml::getChild(const std::string& path)
{
	vector<std::string> sv;
	SplitByChars(path, std::string("\\/"), sv);

	CSmallXml* p = this, * child;
	for(int i = 0; i < (int)sv.size(); i++)
	{
		child = p->getDirectChild(sv[i]);
		if (child == NULL) return NULL;
		p = child;
	}
	return p;
}

int	CSmallXml::getChildInt(const std::string& path, int defaultValue)
{
	CSmallXml* p = getChild(path);
	if (p == NULL) 
		return defaultValue;
	else return atoi(p->getText().c_str());
}

std::string CSmallXml::getChildString(const std::string& path, const std::string& defaultValue)
{
	CSmallXml* p = getChild(path);
	if (p == NULL) 
		return defaultValue;
	else return p->getText();
}

std::string	CSmallXml::getChildParamString(const std::string& path, const std::string& param, const std::string& defaultValue)
{
	std::string res;
	CSmallXml* p = getChild(path);
	if (p && p->getParam(param, res)) return res;
	return defaultValue;
}

//Remove myself from parent.
void CSmallXml::subtreeMe()
{
	if (parent == NULL) return;
	if (parent->children == NULL) return;
	ChildrenListType::iterator it;
	for(it = parent->children->begin(); it != parent->children->end(); it++)
		if ((*it) == this)
		{
			*it = NULL;
			parent->children->erase(it);
			break;
		}
	parent = NULL;
}

//Get my position in the parent's children.
int	CSmallXml::getPos()
{
	if (parent == NULL) return 0;
	if (parent->children == NULL) 
	{
		assert(false);
		return 0;
	}
	for(int i = 0; i < parent->size(); i++)
	{
		if ((*parent)[i] == this) return i;
	}
	assert(false);
	return -1;
}

/*
class Test
{
public:
	Test()
	{
		CSmallXml xml;
		XMLPlatformUtils::Initialize();

		xml.parse("<Template Path=\"C:\\temp\" p=\"\">\nHello, This is a test.<My attr=\"He\">Hello</My>\n</Template>");
		cout<<xml<<endl;
		cout<<xml[0]<<endl;
		cout<<xml.toString()<<endl;
		cout<<*xml.getChild("My")<<endl;
	}
};

Test test;

*/

}}} //namespace
