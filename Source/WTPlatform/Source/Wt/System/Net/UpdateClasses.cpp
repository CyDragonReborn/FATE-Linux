//TODO: File marker

#include "WT/stdafx.h"
#include "WT/System/CRC.h"
#include "WT/System/Platform.h"
#include "WT/System/IO/IO.h"
#include "WT/System/Net/Net.h"
#include "WT/System/Serialization/Formatters/Xml/XmlFormatter.h"
#include "WT/System/SystemException.h"
#include "UpdateClasses.h" 
#include <direct.h>


#define READ_OBJECT(_PVARIABLE,_TYPE,_FILENAME)\
	{	WT::System::IO::CFileStream inf (_FILENAME, FileMode::Open, FileAccess::Read);\
		WT::System::Serialization::CXmlFormatter _XmlFormatter;\
		CObject* pObj = _XmlFormatter.Deserialize (&inf);\
		_PVARIABLE = DynamicCast<_TYPE>(pObj);	}

#define READ_OBJECT_NOTHROW(_VARIABLE,_TYPE,_FILENAME)\
		try\
		{\
			READ_OBJECT(_VARIABLE,_TYPE,_FILENAME);\
		}\
		catch (WT::System::IO::CIOException ex)\
		{\
			_VARIABLE = 0;\
			printf( "Error: exception %s\n", ex.ToString().c_str() );\
		}

#define WRITE_OBJECT(_PVARIABLE, _FILENAME)\
	{	WT::System::IO::CFileStream outf (_FILENAME, WT::System::IO::FileMode::Create);\
		WT::System::Serialization::CXmlFormatter _XmlFormatter;\
		_XmlFormatter.Serialize ( &outf, _PVARIABLE );\
		outf.Close();		}

namespace WT
{
namespace System
{
namespace Net
{
	static int test_CStringI18N();
	static int test_localecmp();

#ifdef _DEBUG
	int WTCall test_Updates()
	{
		int NError = 0;
		NError += test_localecmp();
		NError += test_CStringI18N();
		if (NError)
		{
			fprintf (stderr, "ERROR: WT::System::Net::test_Updates FAILED (%d errors)\n", NError);
		}
		return NError;
	}

#endif

	bool operator==(const CNameValueI18N& Left, const std::string& Name)
	{
		return Left.GetName() == Name;
	}

	bool operator==(const CNameValueI18N& Left, const CNameValueI18N& Right)
	{
		return Left.GetName() == Right.GetName();
	}

	// Matches two locale strings and returns the degree to which 
	// they match.
	// localecmp ("English_USA", "English_Canada") ==> 1 (the first item matches)
	// localecmp ("English_USA", "English_USA") ==> 2 (the first two items match)
	// localecmp ("English_USA.1252", "English_USA.1252") ==> 3 (the first three items match)
	// localecmp ("English_USA", "French_USA") ==> 0 (the first item does not match)
	//
	// if either or both input strings are null, return 0;
	//
	int localecmp (const char* const S1, const char* const S2)
	{
		if (!S1 || !S2)
		{
			return 0;
		}

		const char KSep[] = { '_', '.', '\0' };

		const char* p1 = S1;
		const char* p2 = S2;
		int SepIdx = 0;
		int Retval=1;

		while (*p1 && *p2)
		{
			if (*p1 != *p2)
			{
				Retval -= 1;
				break;
			}
			if (*p1 == KSep[SepIdx])
			{
				Retval++;
				SepIdx++;
			}
			++p1;
			++p2;
		}
		return Retval;
	}
#ifdef _DEBUG
	// Does a single test of the localecmp routine
	static int test_localecmp_one (const char* const S1, const char* const S2, const int Expected)
	{
		int NError = 0;
		const int Actual = localecmp (S1, S2);
		if (Actual != Expected)
		{
			printf ("Error: localecmp (\"%s\", \"%s\") returns %d; expected %d\n", S1, S2, Actual, Expected);
			++NError;
		}
		return NError;
	}

	// Tests the localecmp routine
	static int test_localecmp()
	{
		int NError = 0;
		NError += test_localecmp_one ("English", "English", 1);
		NError += test_localecmp_one ("English_USA", "English_Canada", 1);
		NError += test_localecmp_one ("English_USA", "English_USA", 2);
		NError += test_localecmp_one ("English_USA", "French_USA", 0);
		NError += test_localecmp_one ("English_USA.1252", "English_USA.1252", 3);
		NError += test_localecmp_one ("English_USA.1252", "English_USA.1251", 2);

		NError += test_localecmp_one (0, 0, 0);
		NError += test_localecmp_one ("English", 0, 0);
		NError += test_localecmp_one (0, "English", 0);

		// Even nonsense values are OK
		NError += test_localecmp_one ("Klingon_Nile.apple", "Klingon_Nile.apple", 3);
		return NError;
	}
#endif
//?
	//								 -------------------------------
	//								 CNameValueI18N_single interface
	//								 -------------------------------
	CNameValueI18N::CNameValueI18N(const std::string& Name, const CStringI18NSet& Value)
		:m_Name(Name),m_Value(Value)
	{
	}

	CNameValueI18N::CNameValueI18N()
	{
	}

	CNameValueI18N::~CNameValueI18N()
	{
	}

	// Adds an internationalized string component.
	void CNameValueI18N::AddValue (const CStringI18N& Value)
	{
		m_Value.Add (Value);
	}

	// Gets the name part of the name=value pair
	const std::string& CNameValueI18N::GetName() const
	{
		return m_Name;
	}

	// Gets the most likely value part of the internationalized value
	const std::wstring& CNameValueI18N::GetValue() const
	{
		const std::wstring& Retval = m_Value.Get().Get();
		return Retval;
	}

	// Gets the raw StringI18N from the name=value pair
	const CStringI18NSet& CNameValueI18N::GetValueI18N () const
	{
		return m_Value;
	}

	// Gets the raw StringI18N from the name=value pair
	CStringI18NSet& CNameValueI18N::GetValueI18N ()
	{
		return m_Value;
	}

	// Sets the name of the name=value pair
	void CNameValueI18N::SetName (const std::string& Name)
	{
		m_Name = Name;
	}

	//
	// ISerializable interface
	//
	void CNameValueI18N::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const
	{
		pInfo->AddValue ("Name", m_Name);
		pInfo->AddValue ("ValueI18N", m_Value);
	}

	void CNameValueI18N::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext )
	{
		pInfo->GetValue ("Name", m_Name);
		pInfo->GetValue ("ValueI18N", m_Value);
	}


//?
	//								 ----------------------------
	//								 CStringI18N interface
	//								 ----------------------------


	CStringI18N::CStringI18N()
		:m_Fontname("")
		,m_Charset(DEFAULT_CHARSET)
		,m_Height(0)
	{ 
	}


	CStringI18N::CStringI18N(const std::string& Language, const std::wstring& Value)
		:m_Language(Language)
		,m_Value(Value) 
		,m_Fontname("")
		,m_Charset(DEFAULT_CHARSET)
		,m_Height(0)
	{ 
	}


	CStringI18N::~CStringI18N() 
	{ 
	}

	const int32 CStringI18N::GetCharset() const
	{
		return m_Charset;
	}


	const std::string& CStringI18N::GetFontname() const
	{
		return m_Fontname;
	}


	const int32 CStringI18N::GetHeight() const
	{
		return m_Height;
	}


	const std::string& CStringI18N::GetLanguage() const 
	{ 
		return m_Language; 
	}


	const std::wstring& CStringI18N::Get() const 
	{ 
		return m_Value; 
	}


	std::string CStringI18N::GetA() const 
	{ 
		// TODO: very inefficient; measure actual performance hit
		// Also note that the upper bits are simply truncated which
		// is sometimes reasonable (extracting a file name) and
		// sometimes not.
		std::string Retval;
		const size_t Len = m_Value.length();
		for (size_t i=0; i<Len; i++)
		{
			const char Ch = (char)m_Value[i];
			assert (Ch == m_Value[i]); // this should only be called when the string is known to be 8-bit only
			Retval.push_back (Ch);
		}
		return Retval; 
	}


	void CStringI18N::SetCharset(const int32 Value)
	{
		m_Charset = Value;
	}

	void CStringI18N::SetFontname(const std::string& Value)
	{
		m_Fontname = Value;
	}

	void CStringI18N::SetHeight(const int32 Value)
	{
		m_Height = Value;
	}

	void CStringI18N::SetLanguage(const std::string& Value)
	{
		m_Language = Value;
	}

	void CStringI18N::SetValue(const std::wstring& Value)
	{
		m_Value = Value;
	}

	static uint8 hextoint (const char ch)
	{
		if (ch >= '0' && ch <= '9')
		{
			return ch-'0';
		}
		else if(ch >= 'a' && ch <= 'f')
		{
			return ch-'a'+10;
		}
		else if(ch >= 'A' && ch <= 'F')
		{
			return ch-'A'+10;
		}
		assert (false);
		return 0;
	}
	void CStringI18N::SetValueA (const std::string& Value, const UnicodeInputMethod::Enum InputMethod)
	{
		// TODO: this isn't very efficient; should measure actual runtime
		// to make sure it's not a bottleneck.
		// Converts U+xxxx into hex char.
		m_Value = L"";
		const size_t Len = Value.length();
		for (size_t i = 0; i<Len; i++)
		{
			wchar_t Ch = Value[i];
			if (InputMethod == UnicodeInputMethod::KUPlus && Ch=='U' && Value[i+1]=='+')
			{
				uint8 C0 = hextoint (Value[i+2]);
				uint8 C1 = hextoint (Value[i+3]);
				uint8 C2 = hextoint (Value[i+4]);
				uint8 C3 = hextoint (Value[i+5]);
				Ch = (C0<<12) + (C1<<8) + (C2<<4) + C3;
				m_Value.push_back (Ch);
				i+=5;
			}
			else
			{
				assert ((Ch & 0x80)==0); // should be 7-bit only
				assert (Ch);
				m_Value.push_back (Ch);
			}
		}
	}

	//
	// ISerializable interface
	//
	void CStringI18N::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const
	{
		pInfo->AddValue ("Charset", m_Charset);
		pInfo->AddValue ("Fontname", m_Fontname);
		pInfo->AddValue ("Height", m_Height);
		pInfo->AddValue ("Language", m_Language);
		pInfo->AddValue ("Value", m_Value);
	}

	void CStringI18N::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext )
	{
		pInfo->GetValue ("Charset", m_Charset);
		pInfo->GetValue ("Fontname", m_Fontname);
		pInfo->GetValue ("Height", m_Height);
		pInfo->GetValue ("Language", m_Language);
		pInfo->GetValue ("Value", m_Value);
	}


	//								 -------------------
	//								 CStringI18NSet methods
	//								 -------------------


	CStringI18NSet::CStringI18NSet ()
	{

	}
	CStringI18NSet::CStringI18NSet (const wstring& Value)
	{
		Add (CStringI18N (string("*"), Value));
	}

	CStringI18NSet::~CStringI18NSet()
	{
		;
	}

	const wchar_t* const CStringI18NSet::c_str() const 
	{ 
		return Get().Get().c_str(); 
	}


	void CStringI18NSet::Add (const CStringI18N& Value)
	{
		m_hStrings.push_back( new CStringI18N( Value ));
	}


	const CStringI18N& CStringI18NSet::Get () const
	{
		string PrefLanguage = GetPreferredLanguage();
		const int32 Len = Length();

		// Matches are better/worse
		int32 BestMatch[4] = { -1, -1, -1, -1 }; // for 0, 1, 2, 3

		for (int32 i=0; i<Len; i++)
		{
			const string& TrialLanguage = m_hStrings[i]->GetLanguage();
			const int32 MatchN = localecmp (PrefLanguage.c_str(), TrialLanguage.c_str());
			assert (MatchN >= 0);
			assert (MatchN <= 3);
			if (BestMatch [MatchN] == -1)
			{
				BestMatch[MatchN] = i;
			}
			else if (strchr (TrialLanguage.c_str(), '*'))
			{
				// A '*' is a match for anyone!
				BestMatch[MatchN] = i;
			}
		}

		for (int32 i=3; i>=0; --i)
		{
			if (BestMatch[i] >= 0)
			{
				const CStringI18N& Retval = *m_hStrings[BestMatch[i]];
				return Retval;
			}
		}
		assert (false);
		static CStringI18N BadReturn ("X", L"");
		return BadReturn;
	}


	const string& CStringI18NSet::GetLanguage (const int32 Index) const
	{
		assert (Index >= 0 && Index < (int32)m_hStrings.size());
		return m_hStrings[Index]->GetLanguage();
	}


	void CStringI18NSet::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const
	{
		uint32 const Count = static_cast<uint32>( m_hStrings.size() );
		pInfo->AddContainerBeginWithCount( "StringList", Count );
		for (uint32 i = 0; i < Count; ++i)
		{ 
			pInfo->AddValue("item", *m_hStrings[i]);
		}
		pInfo->AddContainerEnd();
	}


	const string CStringI18NSet::GetPreferredLanguage ()
	{
		std::string Retval = setlocale (LC_ALL, 0);
		return Retval;
	}


	const CStringI18N& CStringI18NSet::GetValue (const int32 Index) const
	{
		assert (Index >= 0 && Index < (int)m_hStrings.size());
		return *m_hStrings[Index];
	}


	int32 CStringI18NSet::Length() const
	{
		const int32 Retval = (int32)m_hStrings.size();
		return Retval;
	}


	void CStringI18NSet::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext )
	{
		uint32 Count = pInfo->GetContainerBeginWithCount( "StringList" );
		m_hStrings.resize( Count );
		for (uint32 i = 0; i < Count; ++i)
		{ 
			m_hStrings[i] = new CStringI18N();
			pInfo->GetValue("item", *m_hStrings[i]);
		}
		pInfo->GetContainerEnd();
	}

#ifdef _DEBUG

	static int test_CStringI18N_default()
	{
		int NError = 0;

		setlocale (LC_ALL, "French_France");
		const char* const CurrLocaleFr = setlocale (LC_ALL, 0);
		if (strncmp (CurrLocaleFr, "French_France", 13) != 0)
		{
			printf ("Error: set local to French_France but got back %s\n", CurrLocaleFr);
			NError ++;
		}
		CStringI18NSet t;
		t.Add (CStringI18N ("English_Canada", L"en_ca"));
		t.Add (CStringI18N ("English_France", L"en_fr"));
		t.Add (CStringI18N ("English_*", L"en default"));
		t.Add (CStringI18N ("English_Japan", L"en_fr"));
		t.Add (CStringI18N ("*", L"default language"));
		t.Add (CStringI18N ("Spanish", L"sp"));
		const std::wstring& ValueA = t.Get().Get();
		if (ValueA != L"default language")
		{
			NError += 1;
			fprintf (stderr, "Error: test_CStringI18N_default: got value <<%S>>; expected <<default language>>\n", ValueA.c_str());
		}

		setlocale (LC_ALL, "English_United States");
		const char* const CurrLocaleEn = setlocale (LC_ALL, 0);
		if (strncmp (CurrLocaleEn, "English_United States", 11) != 0)
		{
			printf ("Error: set local to English_United States but got back %s\n", CurrLocaleEn);
			NError ++;
		}
		const std::wstring& ValueB = t.Get().Get();
		if (ValueB != L"en default")
		{
			NError += 1;
			fprintf (stderr, "Error: test_CStringI18N_default: got value <<%S>>; expected <<en default>>\n", ValueB.c_str());
		}

		setlocale (LC_ALL, "English_Canada");
		const char* const CurrLocaleEnCa = setlocale (LC_ALL, 0);
		if (strncmp (CurrLocaleEnCa, "English_Canada", 14) != 0)
		{
			printf ("Error: set local to English_Canada but got back %s\n", CurrLocaleEnCa);
			NError ++;
		}
		const std::wstring& ValueC = t.Get().Get();
		if (ValueC != L"en_ca")
		{
			NError += 1;
			fprintf (stderr, "Error: test_CStringI18N_default: got value <<%S>>; expected <<en ca>>\n", ValueC.c_str());
		}

		return NError;
	}

	static int test_CStringI18N()
	{
		int NError = 0;
		NError += test_CStringI18N_default();

		return NError;
	}
#endif


	const void CPatchManifest::AddAboutLong (const CStringI18N& Value)
	{
		m_hAboutLong->Add (Value);
	}


	const void CPatchManifest::AddAboutShort (const CStringI18N& Value)
	{
		m_hAboutShort->Add (Value);
	}

	const void CPatchManifest::AddExpansionString(const std::string& Name, const CStringI18N& Value)
	{
		// If the name already exists, add it in
		if (ExpansionExists (Name))
		{
			CStringI18NSet& String = GetExpansionString (Name);
			String.Add (Value);
		}
		else
		{
			CStringI18NSet NewString;
			NewString.Add (Value);
			AddExpansionString (Name, NewString);
		}
	}


	const void CPatchManifest::AddExpansionString(const std::string& Name, const CStringI18NSet& Value)
	{
		m_Expansion.push_back (CNameValueI18N (Name, Value));
	}

	const void CPatchManifest::DeleteExpansion (const std::string& Name)
	{
		const int32 Len = GetExpansionLength();
		int32 DeleteGap = 0;
		for (int i=0; (i+DeleteGap)<Len; i++)
		{
			if (m_Expansion[i].GetName() == Name)
			{
				++DeleteGap;
			}
			if (DeleteGap && i+DeleteGap<Len)
			{
				m_Expansion[i] = m_Expansion[i+DeleteGap];
			}
		}
		const int32 NewLen = Len - DeleteGap;
		m_Expansion.resize (NewLen);
	}


	bool CPatchManifest::ExpansionExists (const std::string& Name) const
	{
		std::vector< CNameValueI18N >::const_iterator i = find (m_Expansion.begin(), m_Expansion.end(), Name);
		const bool Retval = (i!=m_Expansion.end());
		return Retval;
	}


	const CStringI18N& CPatchManifest::GetAboutLong () const 
	{
		return m_hAboutLong->Get();
	}


	const CStringI18N& CPatchManifest::GetAboutShort() const
	{
		return m_hAboutShort->Get();
	}


	const int32 CPatchManifest::GetExpansionLength () const
	{
		const int32 Retval = (int32)m_Expansion.size();
		assert (m_Expansion.size() < 0x7FFFFFFF);
		return Retval;
	}


	const std::string& CPatchManifest::GetExpansionName (const int32 Index) const
	{
		if (Index < 0 || Index >= GetExpansionLength())
		{
			throw new WT::System::CIndexOutOfRangeException ();
		}
		return m_Expansion[Index].GetName();
	}


	const CStringI18NSet& CPatchManifest::GetExpansionString(const std::string& Name) const
	{
		std::vector< CNameValueI18N >::const_iterator i = find (m_Expansion.begin(), m_Expansion.end(), Name);
		if (i==m_Expansion.end())
		{
			throw new WT::System::CIndexOutOfRangeException ();
		}
		const CStringI18NSet& Retval = i->GetValueI18N();
		return Retval;
	}


	CStringI18NSet& CPatchManifest::GetExpansionString(const std::string& Name) 
	{
		std::vector< CNameValueI18N >::iterator i = find (m_Expansion.begin(), m_Expansion.end(), Name);
		if (i==m_Expansion.end())
		{
			throw new WT::System::CIndexOutOfRangeException ();
		}
		CStringI18NSet& Retval = i->GetValueI18N();
		return Retval;
	}


	const CStringI18NSet& CPatchManifest::GetExpansionValue (const int32 Index) const
	{
		if (Index < 0 || Index >= GetExpansionLength())
		{
			throw new WT::System::CIndexOutOfRangeException ();
		}
		return m_Expansion[Index].GetValueI18N ();
	}


	CStringI18NSet& CPatchManifest::GetExpansionValue (const int32 Index)
	{
		if (Index < 0 || Index >= GetExpansionLength())
		{
			throw new WT::System::CIndexOutOfRangeException ();
		}
		return m_Expansion[Index].GetValueI18N ();
	}


	const std::string& CPatchManifest::GetPatchExeFilename() const
	{
		return m_PatchExeFilename;
	}


	const int32 CPatchManifest::GetPatchExeCRC() const
	{
		return m_CRC;
	}


	const uint32 CPatchManifest::GetPatchExeSize() const
	{
		return m_Size;
	}


	const std::string& CPatchManifest::GetPatchExeUrl() const
	{
		return m_PatchExeUrl;
	}


	const std::string& CPatchManifest::GetVersionTo() const
	{
		return m_VersionTo;
	}


	HPatchManifest CPatchManifest::ReadFromFile (const std::string& Filename)
	{
		WT::System::IO::CFileStream inf (Filename, FileMode::Open, FileAccess::Read);
		WT::System::Serialization::CXmlFormatter _XmlFormatter;
		HObject hObject = _XmlFormatter.Deserialize (&inf);
		HPatchManifest Retval = DynamicCast<CPatchManifest>( hObject.get() );
		return Retval;
	}	


	HPatchManifest CPatchManifest::ReadFromFileNoThrow (const std::string& Filename)
	{
		try
		{
			return ReadFromFile (Filename);
		}
		catch (WT::System::IO::CIOException ex)
		{
			ex;
		}
		return NULL;
	}


	void CPatchManifest::SetAboutLong(const CStringI18NSet& Value)
	{
		*m_hAboutLong = Value;
	}


	void CPatchManifest::SetAboutShort(const CStringI18NSet& Value)
	{
		*m_hAboutShort = Value;
	}


	void CPatchManifest::SetPatchExeFilename (const std::string& Value)
	{
		m_PatchExeFilename = Value;
	}


	void CPatchManifest::SetPatchExeUrl(const std::string& Value)
	{
		m_PatchExeUrl = Value;
	}


	void CPatchManifest::SetVersionTo(const std::string& Value)
	{
		m_VersionTo = Value;
	}


	// Returns true iff Str ends with LookFor.  Returns false if either
	// Str or LookFor are NULL (and asserts, too).  Handles the case of
	// LookFor being longer than Str.
	static bool strendswith(const char* const Str, const char* const LookFor)
	{
		if (!Str)
		{
			assert (false);
			return false;
		}
		if (!LookFor)
		{
			assert (false);
			return false;
		}
		const int32 SLen = (int32)strlen (Str);
		const int32 LLen = (int32)strlen (LookFor);
		const int32 StartIndex = SLen - LLen;
		if (StartIndex < 0)
		{
			return false;
		}
		const bool Retval = (strcmp (Str+StartIndex, LookFor) == 0);
		return Retval;
	}

	// Makes a new string from the left part of an 
	// original string; does not modify the original
	// at all.  If Len is greater than the
	// Str length, will return as much as it can.
	// If Len is negative, will return a string 
	// shortened by that amount.
	// Example: STRING_Left("abc", -1) 
	//    --> STRING_Left ("abc", strlen ("abc") + (-1))
	//    --> STRING_Left ("abc", 2)
	//    --> "ab"

	std::string STRING_Left (const std::string& Str, const int32 Len)
	{
		// Alt: just allow a fixed-size buffer.
		// Could probably do this without the malloc, but
		// it's much easier with it.
		if (Len < 0)
		{
			const int32 SLen = (int32)Str.length();
			const int32 NewLen = (SLen + Len) > 0 ? SLen + Len : 0;
			return STRING_Left (Str, NewLen);
		}
		char* Tmp = new char [Len+1];
		strncpy (Tmp, Str.c_str(), Len);
		Tmp[Len] = '\0';
		std::string Retval (Tmp);
		delete[] Tmp;
		return Retval;
	}

	void CPatchManifest::UpdateFileInfo()
	{
		using namespace WT::System::IO;
		using namespace WT::System;

		// Uses the m_PatchFileInfo
		m_Size = CFile::Size (m_PatchExeFilename);
		m_CRC = CRC::GetFileCRC (m_PatchExeFilename);

		//
		// And now loop through m_Expansion twice.  First time,
		// for each [name]_autoread, clear out [name].
		// Second time, for each [name]_autoread, pull the
		// data into [name] (note that [name]_autoread can
		// include multiple languages)
		//
		const char* const KARStr = "_autoread";
		const int32 KARLen = (int32)strlen (KARStr);

		const int32 ELenStart = GetExpansionLength();
		// First loop: get rid of all old 'name' values 
		// which are part of a 'name'_autoread
		for (int32 i=0; i<ELenStart; i++)
		{
			const std::string& AutoName = GetExpansionName (i);
			if (strendswith (AutoName.c_str(), KARStr))
			{
				const std::string Name = STRING_Left (AutoName, -KARLen);
				DeleteExpansion (Name); // if Name doesn't exist, nothing happens.
			}
		}

		const int32 ELen = GetExpansionLength();
		for (int32 i=0; i<ELen; i++)
		{
			const std::string& AutoName = GetExpansionName (i);
			if (strendswith (AutoName.c_str(), KARStr))
			{
				const std::string Name = STRING_Left (AutoName, -KARLen);
				const CStringI18NSet& Value = GetExpansionValue (i);
				const int ILen = Value.Length(); // number of languages
				CStringI18NSet NewSet;
				for (int j=0; j<ILen; j++)
				{
					const CStringI18N OrigI18N = Value.GetValue (j);
					const std::string& LValue = OrigI18N.GetA();

					// LValue is the file with data
					if (CFile::Exists (LValue))
					{
						CFileStream FS (LValue, FileMode::Open, FileAccess::Read);
						const int32 Len = FS.GetLength();
						uint8* Buffer = new uint8[Len];
						FS.Read (Buffer, Len);
						std::string B64Value = CBase64Encoder::BufferToString (Buffer, 0, Len);
						CStringI18N NewI18N = OrigI18N;
						NewI18N.SetValueA (B64Value, UnicodeInputMethod::KNoUnicode);  // can't have unicode -- it's BASE64
						NewSet.Add (NewI18N);
						delete[] Buffer;
						Buffer = 0;
					}
					else
					{
						throw new CElementNotFoundException ();
					}
				}
				AddExpansionString (Name, NewSet);
			}
		}

#ifdef NEVER_EVER_DEFINED
		this is old code that has been replaced
		for (int32 i=0; i<ELen; i++)
		{
			const std::string& Name = GetExpansionName (i);
			const std::string& AutoName = Name + "_autofile";
			const bool AutoExists = ExpansionExists (AutoName);
			if (AutoExists)
			{
				const CStringI18NSet& Value = GetExpansionValue (i);
				const int ILen = Value.Length(); // number of languages
				for (int j=0; j<ILen; j++)
				{
					const std::string& LLanguage = Value.GetLanguage (j);
					const std::string& LValue = Value.GetValue (j);

					// LValue is the file with data
					if (CFile::Exists (LValue))
					{
						CFileStream FS (LValue, FileMode::Open, FileAccess::Read);
						const int32 Len = FS.GetLength();
						uint8* Buffer = (uint8*)malloc (Len);
						FS.Read (Buffer, 0, Len);
						std::string B64Value = CBase64Encoder::BufferToString (Buffer, 0, Len);
						AddExpansionString (Name, LLanguage, B64Value);
						free (Buffer);
						Buffer = 0;
					}
					else
					{
						throw new CElementNotFoundException ();
					}
				}
			}

		}
#endif
	}

	void CPatchManifest::UpdateFileWrite(const std::string& Directory)
	{
		// Note that the directory should always end in a slash
#ifdef _DEBUG
		const int32 Len = (int32)Directory.length();
		assert (Directory[Len-1] == '\\');
#endif
		const char* const KARStr = "_autowrite";
		const int32 KARLen = (int32)strlen (KARStr);

		const int32 ELen = GetExpansionLength();
		for (int32 i=0; i<ELen; i++)
		{
			const std::string& AutoName = GetExpansionName (i);
			if (strendswith (AutoName.c_str(), KARStr)) // end with _autowrite
			{
				const std::string Name = STRING_Left (AutoName, -KARLen); // just the name
				const std::string Filename = GetExpansionValue (i).Get().GetA(); // Get the current I18N version
				const std::wstring DataB64 = GetExpansionString (Name).Get().Get(); // Gets the data
				const std::string Path = Directory + Filename;
				CFileStream outf (Path, FileMode::OpenOrCreate);
				CBase64Decoder::StringToStream (DataB64, &outf);
			}
		}


	}

	void CPatchManifest::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const
	{
		pInfo->AddValue ("PatchExeUrl", m_PatchExeUrl);
		pInfo->AddValue ("VersionTo", m_VersionTo);
		pInfo->AddValue ("AboutShort", *m_hAboutShort.get());
		pInfo->AddValue ("AboutLong", *m_hAboutLong.get());
		pInfo->AddValue ("PatchExeFilename", m_PatchExeFilename);
		pInfo->AddValue ("Size", m_Size);
		pInfo->AddValue ("CRC", m_CRC);
		pInfo->AddValue ("ExpansionType", m_hExpansionType.get());
		pInfo->AddValue ("Expansion", m_Expansion);
	}

	void CPatchManifest::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext )
	{
		pInfo->GetValue ("PatchExeUrl", m_PatchExeUrl);
		pInfo->GetValue ("VersionTo", m_VersionTo);
		pInfo->GetValue ("AboutShort", *m_hAboutShort);
		pInfo->GetValue ("AboutLong", *m_hAboutLong);
		pInfo->GetValue ("PatchExeFilename", m_PatchExeFilename);
		pInfo->GetValue ("Size", m_Size);
		pInfo->GetValue ("CRC", m_CRC);
		pInfo->GetValue ("ExpansionType", *m_hExpansionType);
		pInfo->GetValue ("Expansion", m_Expansion);
	}

	CUpdateHistory::CUpdateHistory()
		:m_MaxData(0)
	{
	}

	CUpdateHistory::~CUpdateHistory()
	{
	}

	void CUpdateHistory::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const
	{
		pInfo->AddValue ("MaxData", m_MaxData);
		pInfo->AddValue ("Data", m_Data);
	}

	void CUpdateHistory::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext )
	{
		pInfo->GetValue ("MaxData", m_MaxData);
		pInfo->GetValue ("Data", m_Data);
	}


	CUpdateManager::CUpdateManager ()
		:m_StartingDirectory("")
		,m_Version ("")
		,m_pPatchExeRequest(0)
		,m_pPatchManifestRequest(0)
		,m_CurrentState (EUpdateManagerState::KCanDownloadManifest)
	{
	}

	CUpdateManager::~CUpdateManager()
	{
		delete m_pHistory;
	}

	CUpdateManager::CUpdateManager (const std::string& StartingDirectory, const std::string& Version)
		:m_StartingDirectory(StartingDirectory)
		,m_Version(Version)
		,m_pPatchExeRequest(0)
		,m_pPatchManifestRequest(0)
		,m_CurrentState (EUpdateManagerState::KCanDownloadManifest)
	{
#ifdef _DEBUG
		// Must always end in a backslash
		assert (m_StartingDirectory[m_StartingDirectory.length()-1] == '\\');
#endif
		//
		// Make sure my directories exist
		//
		if (!CDirectory::Exists (m_StartingDirectory))
		{
			// The top-level directory must exist
			assert (false);
		}

		if (!CDirectory::Exists (m_StartingDirectory + "updates\\"))
		{
			CDirectory::CreateDirectory (m_StartingDirectory + "updates\\");
		}
		if (!CDirectory::Exists (m_StartingDirectory + "updates\\download\\"))
		{
			CDirectory::CreateDirectory (m_StartingDirectory + "updates\\download\\");
		}
		if (!CDirectory::Exists (m_StartingDirectory + "updates\\log\\"))
		{
			CDirectory::CreateDirectory (m_StartingDirectory + "updates\\log\\");
		}
		m_PatchExeFilename = m_StartingDirectory + "updates\\download\\patch.exe";
		m_PatchManifestFilename = m_StartingDirectory + "updates\\download\\patchmanifest.xml";
		m_MasterUpdateFilename = m_StartingDirectory + "updates\\masterupdate.xml";
		m_UpdateHistoryFilename = m_StartingDirectory + "updates\\log\\updatehistory.xml";
		m_WTRerunProgramUtilityFilename = m_StartingDirectory + "updates\\WTRerunProgramUtility.exe";

		//
		// Read in my history and my masterupdate
		//
		READ_OBJECT_NOTHROW (m_hMaster, CMasterUpdate, m_MasterUpdateFilename);
		READ_OBJECT_NOTHROW (m_pHistory, CUpdateHistory, m_UpdateHistoryFilename);
		if (!m_pHistory)
		{
			m_pHistory = new CUpdateHistory ();
			WRITE_OBJECT (m_pHistory, m_UpdateHistoryFilename);
		}
		m_hPatchManifest = CPatchManifest::ReadFromFileNoThrow (m_PatchManifestFilename);
		if (IsPatchAvailable())
		{
			m_CurrentState = EUpdateManagerState::KCanDownloadPatch;
		}
	}


	void CUpdateManager::Bounce (const std::string& RerunProgram, const std::string& PatchProgram)
	{
		// Don't forget that this is a static routine
		char cwd [MAX_PATH];
		getcwd (cwd, sizeof (cwd));

		STARTUPINFO sinfo;
		memset (&sinfo, 0, sizeof (sinfo));
		sinfo.cb = sizeof (sinfo);
		PROCESS_INFORMATION pinfo;
		memset (&pinfo, 0, sizeof (pinfo));

															//
															// Commands to send to the
															// WTRerunProgramUtility program
															//
		char Buffer [MAX_PATH + MAX_PATH];
		sprintf (Buffer, "D%s", cwd);						// Set the default directory
		SetEnvironmentVariable ("L1", Buffer);

		const DWORD PID = GetCurrentProcessId ();

		sprintf (Buffer, "W%d 10000", PID);
		SetEnvironmentVariable ("L2", Buffer);				// Wait for me to exit!

		sprintf (Buffer, "PW%s", PatchProgram.c_str());		// Run Patch and wait for it to exit
		SetEnvironmentVariable ("L3", Buffer);

		sprintf (Buffer, "PI%s", GetCommandLine());			// Run the original program and don't wait
		SetEnvironmentVariable ("L4", Buffer);
 
		const BOOL CPStatus = CreateProcess (
			RerunProgram.c_str(),	// application name
			0,			// command line
			0, // process attributes
			0, // thread attributes
			FALSE, // inherit handle
			0, // creation flags
			0, // environment
			cwd, // current directory
			&sinfo,// lpstartupinfo
			&pinfo// lprocessinfo
			);
		if (CPStatus)
		{
			// it worked.
			exit (0);
		}
		else
		{
			// it failed.  Carry on as if nothing had happened.
		}
	}


	void CUpdateManager::CancelManifestDownload ()
	{
		if (m_CurrentState != EUpdateManagerState::KAmDownloadingManifest)
		{
			// Anyone who's not actively downloading should exit now
			return;
		}

		if (!m_pPatchManifestRequest)
		{
			assert (false);
		}
		m_pPatchManifestRequest->Abort ();
	}


	void CUpdateManager::CancelPatchDownload ()
	{
		if (m_CurrentState != EUpdateManagerState::KAmDownloadingPatch)
		{
			// Anyone who's not actively downloading should exit now
			return;
		}

		if (!m_pPatchExeRequest)
		{
			assert (false);
		}

		m_pPatchExeRequest->Abort ();

	}


	void CUpdateManager::CheckForUpdatesCallback (const IAsyncResult* AR)
	{
		void* pVoid = AR->GetAsyncState ();
		CUpdateManager* pThis = (CUpdateManager*)pVoid;

		pThis->m_CheckForUpdatesPercent = pThis->m_pPatchManifestRequest->GetPercentDone ();

		//
		// If we're done, there will be a stream with a PatchManifest object
		// that I can write out
		//
		if (pThis->m_CheckForUpdatesPercent == 100.0f)
		{
			pThis->OnPatchManifestDownloadComplete ();
		}
	}

	void CUpdateManager::DeletePatchExe()
	{
		if (CFile::Exists (m_PatchExeFilename))
		{
			CFile::Delete (m_PatchExeFilename);
		}
	}

	void CUpdateManager::DeletePatchManifest()
	{
		if (CFile::Exists (m_PatchManifestFilename))
		{
			CFile::Delete (m_PatchManifestFilename);
		}
		m_hPatchManifest = NULL;
	}


	void CUpdateManager::DownloadUpdateCallback (const IAsyncResult* AR)
	{
		void* pVoid = AR->GetAsyncState ();
		CUpdateManager* pThis = (CUpdateManager*)pVoid;

		pThis->m_DownloadUpdatePercent = pThis->m_pPatchExeRequest->GetPercentDone ();

		//
		// If we're done, there will be a stream with a PatchManifest object
		// that I can write out
		//
		if (pThis->m_DownloadUpdatePercent == 100.0f)
		{
			pThis->OnPatchExeDownloadComplete ();
		}
	}

	EUpdateManagerState::Enum CUpdateManager::GetCurrentState()
	{
		return m_CurrentState;
	}


	CPatchManifest* CUpdateManager::GetPatchManifest()
	{
		return m_hPatchManifest;
	}


	float32 Range (const float32 Value, const float32 MinVal, const float32 MaxVal)
	{
		if (Value < MinVal) return MinVal;
		if (Value > MaxVal) return MaxVal;
		return Value;
	}


	float32 CUpdateManager::GetPercentDone ()
	{
		//TODO: should there be some sort of lock?  This call is 
		//async; if it's called when the callbacks are called it 
		//doesn't work very well.
		float32 Retval = 0.0f;
		const int State = GetCurrentState();
		switch (State)
		{
		case EUpdateManagerState::KAmDownloadingManifest:
			//assert (m_pPatchManifestRequest);
			Retval = m_pPatchManifestRequest ? m_pPatchManifestRequest->GetPercentDone () : 1.0f;
			Retval = Range (Retval, 1.0f, 99.0f);
			break;
		case EUpdateManagerState::KAmDownloadingPatch:
			//assert (m_pPatchExeRequest);
			Retval = m_pPatchExeRequest ? m_pPatchExeRequest->GetPercentDone () : 1.0f;
			Retval = Range (Retval, 1.0f, 99.0f);
			break;
		case EUpdateManagerState::KCanInstallPatch:
			Retval = 100.0f;
			break;
		default:
			Retval = 0.0f;
			break;
		}
		return Retval;
	}


	const std::string& CUpdateManager::GetVersion() const
	{
		return m_Version;
	}


	bool CUpdateManager::IsPatchAvailable () const
	{
		if (!m_hPatchManifest)
		{
			// If there's no manifest then clearly there's no patch
			return false;
		}


		const std::string& VersionTo = m_hPatchManifest->GetVersionTo();
		const bool Retval = (VersionTo != m_Version);
		return Retval;
	}


	bool CUpdateManager::IsPatchBad () const
	{
		if (!m_hPatchManifest)
		{
			// without a manifest there can be no patch
			return false;
		}
		if (!IsPatchAvailable())
		{
			return false;
		}
		const int32 ActualSize = WT::System::IO::CFile::Size (m_PatchExeFilename);
		const int32 ExpectedSize = m_hPatchManifest->GetPatchExeSize();
		if (ActualSize > ExpectedSize)
		{
			return true; 
		}
		if (ActualSize < ExpectedSize)
		{
			return false; // not done downloading yet
		}

		// Calculating the CRC is slow; only do it if needed 
		const uint32 ActualCRC = WT::System::CRC::GetFileCRC (m_PatchExeFilename);
		const uint32 ExpectedCRC = m_hPatchManifest->GetPatchExeCRC();

		if (ActualCRC != ExpectedCRC)
		{
			return true; 
		}
		return false;
	}


	bool CUpdateManager::IsPatchGood () const
	{
		if (!m_hPatchManifest)
		{
			// without a manifest there can be no patch
			return false;
		}
		if (!IsPatchAvailable())
		{
			return false;
		}
		int32 ActualSize = WT::System::IO::CFile::Size (m_PatchExeFilename);
		int32 ExpectedSize = m_hPatchManifest->GetPatchExeSize();
		if (ActualSize > ExpectedSize)
		{
			return false; 
		}
		if (ActualSize < ExpectedSize)
		{
			return false; // not done downloading yet
		}

		// Calculating the CRC is slow; only do it if needed 
		const uint32 ActualCRC = WT::System::CRC::GetFileCRC (m_PatchExeFilename);
		const uint32 ExpectedCRC = m_hPatchManifest->GetPatchExeCRC();

		if (ActualCRC != ExpectedCRC)
		{
			return false; 
		}
		return true;
	}


	bool CUpdateManager::IsPatchReady () const
	{
		if (!m_hPatchManifest)
		{
			// without a manifest there can be no patch
			return false;
		}
		if (!IsPatchAvailable())
		{
			return false;
		}

		int32 ActualSize = WT::System::IO::CFile::Size (m_PatchExeFilename);
		int32 ExpectedSize = m_hPatchManifest->GetPatchExeSize();
		if (ActualSize != ExpectedSize)
		{
			return false; 
		}

		// Calculating the CRC is slow; only do it if needed 
		uint32 ExpectedCRC = m_hPatchManifest->GetPatchExeCRC();
		uint32 ActualCRC = WT::System::CRC::GetFileCRC (m_PatchExeFilename);
		if (ActualCRC != ExpectedCRC)
		{
			return false; 
		}
		return true;
	}


	void CUpdateManager::NotifiedUserOfBadManifest()
	{
		if (m_CurrentState == EUpdateManagerState::KAmNotifyingBadManifest)
		{
			m_CurrentState = EUpdateManagerState::KCanDownloadManifest;
		}
		DeletePatchManifest();
	}


	void CUpdateManager::NotifiedUserOfBadPatch()
	{
		if (m_CurrentState == EUpdateManagerState::KAmNotifyingBadPatch)
		{
			m_CurrentState = EUpdateManagerState::KCanDownloadPatch;
		}
		DeletePatchExe();
	}


	void CUpdateManager::OnPatchManifestDownloadComplete ()
	{
		bool IsOk=false;

		CHttpWebResponse& Response = m_pPatchManifestRequest->EndGetHttpResponse (m_pPatchManifestResult);
		const HttpStatusCode::Enum Code = Response.GetStatusCode ();
		if (Code == HttpStatusCode::OK)
		{
			CStream* pStream = Response.GetResponseStream();
			//
			// Pipe stream into a file
			//
			WT::System::IO::CFileStream outf (m_PatchManifestFilename, FileMode::Create);
			outf.WriteStream (*pStream);
			pStream->Seek (0, SeekOrigin::Begin);


			//
			// And make an object
			//
			m_hPatchManifest = NULL;
			try
			{
				WT::System::Serialization::CXmlFormatter _XmlFormatter;
				CObject* pObj = _XmlFormatter.Deserialize (pStream);
				m_hPatchManifest = DynamicCast<CPatchManifest>(pObj);
				assert (m_hPatchManifest != NULL);

				IsOk = true;
			}
			catch (WT::System::CException e)
			{
			}
		}

		//
		// And update my status values
		//
		m_CurrentState = IsOk ? EUpdateManagerState::KCanDownloadPatch : EUpdateManagerState::KAmNotifyingBadManifest;
		delete m_pPatchManifestRequest;
		m_pPatchManifestRequest = 0;

		// Lastly, handle the auto-write feature
		if (IsOk)
		{
			m_hPatchManifest->UpdateFileWrite (m_StartingDirectory + "updates\\download\\");
		}
	}

	void CUpdateManager::OnPatchExeDownloadComplete ()
	{
		// Remember that we can get here even if the patch exe
		// isn't done -- it just means that the html thinks
		// that there isn't any more.  It could just be that
		// the connection failed.
		using namespace WT::System::IO;
		bool IsOk = false;

		//
		// Check the response code (if any)
		//
		if (m_pPatchExeRequest)
		{
			CHttpWebResponse& Response = m_pPatchExeRequest->EndGetHttpResponse (m_pPatchExeResult);
			const HttpStatusCode::Enum Code = Response.GetStatusCode ();
			switch (Code)
			{
			case HttpStatusCode::OK:
				{
					CStream* pStream = Response.GetResponseStream();
					//
					// Pipe stream into a file
					//
					try
					{
						CFileStream outf (m_PatchExeFilename, FileMode::Append);
						outf.WriteStream (*pStream);
						outf.Close();
					}
					catch (CIOException)
					{
					}

				}
				break;

			// Some server code are normal breakages and don't invalidate
			// the entire download

			case HttpStatusCode::ResetContent: //205
			case HttpStatusCode::PartialContent: //206
			case HttpStatusCode::BadRequest: //400
			case HttpStatusCode::Unauthorized: //401
			case HttpStatusCode::PaymentRequired: //402
			case HttpStatusCode::Forbidden: ///403
			case HttpStatusCode::NotFound: //404
			case HttpStatusCode::MethodNotAllowed:
			case HttpStatusCode::NotAcceptable:
			case HttpStatusCode::ProxyAuthenticationRequired:
			case HttpStatusCode::RequestTimeout:
				// Most of the 4xx errors and some of the 2xx errors are
				// really server misconfigurations; the user should be
				// encouraged to try again.
				break;

			default:
				//
				// By default, bad things happening means that we should 
				// delete the bad download
				//
				DeletePatchExe();
				break;
			}
			delete m_pPatchExeRequest;
			m_pPatchExeRequest = 0;
		}

		//
		// If there is no m_pPatchExeRequest, we're in an odd but not
		// totally unexpected state -- just set the new state based
		// on whether the download is done or not.
		//
		IsOk = IsPatchGood();

		//
		// And update my state
		//
		m_CurrentState = IsOk ? EUpdateManagerState::KCanInstallPatch : EUpdateManagerState::KAmNotifyingBadPatch;
	}

	void CUpdateManager::SetStartingDirectory (const std::string& Value)
	{
		m_StartingDirectory = Value;
	}

	void CUpdateManager::StartDownloadingManifest ()
	{
		if ( (m_CurrentState != EUpdateManagerState::KCanDownloadManifest)
			&& (m_CurrentState != EUpdateManagerState::KCanDownloadPatch)
			&& (m_CurrentState != EUpdateManagerState::KCanInstallPatch))
		{
			// Anyone who's not actively downloading can download this.
			return;
		}
		if (!m_hMaster)
		{
			return;
		}

		if (m_pPatchManifestRequest)
		{
			// Already started; just return
			return;
		}
		m_CurrentState = EUpdateManagerState::KAmDownloadingManifest;

		const std::string Url = m_hMaster->GetPatchManifestUrl();
		m_pPatchManifestRequest = CWebRequest::CreateHttp(Url);
		m_pPatchManifestRequest->BeginGetResponse (CAsyncCallback::Create (CheckForUpdatesCallback), (void*)this);

	}

	void CUpdateManager::StartDownloadingPatch ()
	{
		if ((m_CurrentState != EUpdateManagerState::KCanDownloadPatch)
			&& (m_CurrentState != EUpdateManagerState::KCanInstallPatch))
		{
			return;
		}

		if (m_pPatchExeRequest)
		{
			// Already started; just return
			return;
		}
		if (!IsPatchAvailable ())
		{
			// Nothing to download; just return;
			return;
		}

		m_CurrentState = EUpdateManagerState::KAmDownloadingPatch;

		const std::string Url = m_hPatchManifest->GetPatchExeUrl();
		m_pPatchExeRequest = CWebRequest::CreateHttp(Url);

		//
		// Set the range -- find the .exe file, and start from there
		//
		if (WT::System::IO::CFile::Exists (m_PatchExeFilename))
		{
			WT::System::IO::CFileStream FS (m_PatchExeFilename, WT::System::IO::FileMode::Open, FileAccess::Read);
			const int32 ActualLen = FS.GetLength();
			const int32 ExpectedLen = m_hPatchManifest->GetPatchExeSize();
			FS.Close ();

			if (ActualLen >= ExpectedLen)
			{
				// Stick a fork in us; we're done
				delete m_pPatchExeRequest;
				m_pPatchExeRequest = 0;
				const bool IsBad = IsPatchBad();
				if (IsBad)
				{
					m_CurrentState = EUpdateManagerState::KAmNotifyingBadPatch;
					DeletePatchExe();
				}
				else
				{
					m_CurrentState = EUpdateManagerState::KCanInstallPatch;
				}
			}
			else
			{
				m_pPatchExeRequest->AddRange (-ActualLen);
			}

		}

		if (m_pPatchExeRequest)
		{
			m_pPatchExeRequest->BeginGetResponse (CAsyncCallback::Create (DownloadUpdateCallback), (void*)this);
		}

	}


	void CUpdateManager::StartInstallingPatch ()
	{
		if (m_CurrentState != EUpdateManagerState::KCanInstallPatch)
		{
			return;
		}

		if (!IsPatchReady())
		{
			// nothing to run; just return.
			return;
		}
		if (!IsPatchGood ())
		{
			// Bad patch; just return.
			return;
		}
		assert (m_CurrentState == EUpdateManagerState::KCanInstallPatch);
		
		CUpdateManager::Bounce (m_WTRerunProgramUtilityFilename, m_PatchExeFilename);
		assert (false); // bounce exits the program; we should never get here.
	}

	CMasterUpdate::CMasterUpdate (const std::string& PatchManifestUrl):m_PatchManifestUrl(PatchManifestUrl)
	{
	}

	CMasterUpdate::CMasterUpdate ()
	{
	}

	CMasterUpdate::~CMasterUpdate ()
	{
	}

	void CMasterUpdate::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const
	{
		pInfo->AddValue ("PatchManifestUrl", m_PatchManifestUrl);
	}

	const std::string& CMasterUpdate::GetPatchManifestUrl() const
	{
		return m_PatchManifestUrl;
	}


	void CMasterUpdate::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext )
	{
		pInfo->GetValue ("PatchManifestUrl", m_PatchManifestUrl);
	}

	//
	//
	//
	//						CUpdateStatus clss
	//
	//
	//

	CUpdateStatus::CUpdateStatus()
	{
	}

	CUpdateStatus::~CUpdateStatus()
	{
	}

	void CUpdateStatus::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const
	{
		pInfo->AddValue ("PatchManifestUrl", m_PatchManifestUrl);
		pInfo->AddValue ("PatchManifestStatus", m_PatchManifestStatus);
		pInfo->AddValue ("PatchManifestDLStar", m_PatchManifestDLStart);
		pInfo->AddValue ("PatchManifestDLEnd", m_PatchManifestDLEnd);
		pInfo->AddValue ("PatchExeUrl", m_PatchExeUrl);
		pInfo->AddValue ("PatchExeStatus", m_PatchExeStatus);
		pInfo->AddValue ("PatchExeDLStart", m_PatchExeDLStart);
		pInfo->AddValue ("PatchExeDLEnd", m_PatchExeDLEnd);
		pInfo->AddValue ("Info", m_Info);
	}

	void CUpdateStatus::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext )
	{
		pInfo->GetValue ("PatchManifestUrl", m_PatchManifestUrl);
		pInfo->GetValue ("PatchManifestStatus", m_PatchManifestStatus);
		pInfo->GetValue ("PatchManifestDLStar", m_PatchManifestDLStart);
		pInfo->GetValue ("PatchManifestDLEnd", m_PatchManifestDLEnd);
		pInfo->GetValue ("PatchExeUrl", m_PatchExeUrl);
		pInfo->GetValue ("PatchExeStatus", m_PatchExeStatus);
		pInfo->GetValue ("PatchExeDLStart", m_PatchExeDLStart);
		pInfo->GetValue ("PatchExeDLEnd", m_PatchExeDLEnd);
		pInfo->GetValue ("Infzo", m_Info);
	}


} // namespace Net
} // namespace System
} // namespace WT
