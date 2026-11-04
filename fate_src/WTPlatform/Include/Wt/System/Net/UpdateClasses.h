// All of the update classes

#pragma once

#include "WT/System/IAsyncResult.h"
#include "WT/System/DateTime.h"
#include "WT/System/Net/WebRequestEnum.h"
#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/System/Net/Net.h"
#include "WT/System/Serialization/ISerializable.h"
namespace WT
{
namespace System
{
namespace Net
{
#ifdef _DEBUG
	int WTCall test_Updates();
#endif

namespace UnicodeInputMethod
{
	// KNoUnicode = string does not have encoded unicode characters
	// KUPlus = string has unicode with format U+2040
	enum Enum { KNoUnicode, KUPlus };
}

	/// The Basic underlying class for CStringI18NSet -- contains a Value and the language that
	/// the value is in.  Is Serializable so that it can be used in other classes
	/// easily.
	///
	/// Note: this class is a candidate for MOVING to the correct spot
	typedef SmartPointer<class CStringI18N> HStringI18N;
	class CStringI18N : public CObject, public Serialization::ISerializable
	{
	public:
		WT_BEGIN_CLASS( CStringI18N, CObject )
			WT_INHERITS_INTERFACE( Serialization::ISerializable );
		WT_END_CLASS();
		CStringI18N();
		CStringI18N(const std::string& Language, const std::wstring& Value);
		~CStringI18N();

		// Gets the charset associated with the string
		const int32 GetCharset() const;

		// Gets the fontname for the value
		const std::string& GetFontname() const;

		// Gets the height associated with the string
		const int32 GetHeight() const;

		// Gets the language for the value,
		const std::string& GetLanguage() const;

		// Gets the value; it's in the language that's specified.
		const std::wstring& Get() const;

		// Gets the value, converting to narrow format through the
		// very yucky process of simply truncating the upper bits.
		// If we need a conversion that uses something fancier 
		// (like switching to a particular encoding) the right
		// way is through our encoder classes.
		std::string GetA() const;

		// Sets the charset associated with the string
		void SetCharset(const int32 );

		// Sets the fontname for the value
		void SetFontname(const std::string& );

		// Sets the height associated with the string
		void SetHeight(const int32 );

		// Sets the language for the value,
		void SetLanguage(const std::string& );

		// Sets the value; it's in the language that's specified.
		void SetValue(const std::wstring&);

		// Sets the value from a narrow string.  The narrow string is
		// simply expanded out without thought to proper unicde; if your
		// string is in fact an encoded string you must run it through
		// one of the platform encoding functions.
		void SetValueA (const std::string&, const UnicodeInputMethod::Enum InputMethod);

		//
		// ISerializable interface
		//
		void GetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext ) const;

		void SetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext );

	private:
		std::string m_Language;
		std::wstring m_Value;
		std::string m_Fontname;
		int32 m_Charset;
		int32 m_Height;

	};


	/// An extension to std::string.  Contains a bunch of strings, each tagged with 
	/// the language that the string is in.  The simple "Get" interface will get
	/// the most appropriate language.
	///
	/// Language codes follow the Microsoft system in which each language is
	/// <language>-<region>.<code page> and is what you get back from setlocale (LC_ALL,0)
	///
	/// The rules are:  exact match is best, then the first two-char match, then
	/// the first item.
	///
	///
	/// Note: this class is a candidate for MOVING to the correct spot
	typedef SmartPointer<class CStringI18NSet> HStringI18NSet;
	class CStringI18NSet : public CObject, public Serialization::ISerializable
	{
	public:
		WT_BEGIN_CLASS( CStringI18NSet, CObject );
			WT_INHERITS_INTERFACE( Serialization::ISerializable );
		WT_END_CLASS();

		CStringI18NSet();
		CStringI18NSet(const std::wstring& Value);
		virtual ~CStringI18NSet();

		// Overload for the most popular of all of string's methods: c_str
		// Does a Get() to get the default CStringI18N and then a c_str()
		// on that to get the 'Value' out
		const wchar_t* const c_str() const;

		// Adds the given language/value pair into the object.
		void Add (const CStringI18N& Value);

		// Gets the best match.  This is by far the most common method called.
		const CStringI18N& Get() const; 

		// Gets the language part given an index; anything out of rang
		// results in an exception (and an assert in Debug mode)
		const string& GetLanguage (const int32 Index) const;

		// Returns the overall prefered language
		static const string GetPreferredLanguage();

		// Gets the value part given the index; anything out of range 
		// results in an exception (and an assert in Debug mode)
		const CStringI18N& GetValue (const int32 Index) const;

		// Gets the number of language/value pairs; you can iterates 0..Length-1
		// of these and pass the integer into GetLanguage and GetValue.
		int32 Length() const;


		//
		// ISerializable interface
		//
		void GetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext ) const;

		void SetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext );

		private:
			std::vector< HStringI18N > m_hStrings; 
	};


	// Simple class that wraps the idea of a name=value pair where
	// the value is a CStringI18NSet
	typedef SmartPointer<class CNameValueI18N> HNameValueI18N;
	class CNameValueI18N : public CObject, public Serialization::ISerializable
	{
	public:
		WT_BEGIN_CLASS( CNameValueI18N, CObject );
			WT_INHERITS_INTERFACE( Serialization::ISerializable );
		WT_END_CLASS();

		CNameValueI18N();
		CNameValueI18N(const string& Name, const CStringI18NSet& Value);
		virtual ~CNameValueI18N();


		// Gets the name part of the name=value pair
		const std::string& GetName() const;

		// Gets the most likely value part of the internationalized value
		const std::wstring& GetValue() const;

		// Gets the raw StringI18N from the name=value pair
		// Also available in non-const
		const CStringI18NSet& GetValueI18N () const;

		// Gets the raw StringI18N from the name=value pair
		// Also available in const
		CStringI18NSet& GetValueI18N ();

		// Sets the name of the name=value pair
		void SetName (const std::string& Name);

		// Adds an internationalized string component.
		void AddValue (const CStringI18N& Value);

		//
		// ISerializable interface
		//
		void GetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext ) const;

		void SetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext );
	private:
		std::string m_Name;
		CStringI18NSet m_Value;
	};


	bool operator==(const CNameValueI18N& Left, const std::string& Name);
	bool operator==(const CNameValueI18N& Left, const CNameValueI18N& Right);



//?
	/*
	The CPatchManifest class wraps the singlepatch.xml file that games
	download from the server.  The URL of this file is gotten from the 
	CMasterUpdate class (which wraps the MasterUpdate.xml file).  

	The primary use of this class (for external users) is to get data
	about the patch to display to end users.  For that, there are a series
	of standard fields (AboutShort, AboutLong, etc) plus a set of 
	expansion fields.

	To download the actual patch exe, use the UpdateMgr::DownloadPatchExe()
	method.
	 */
	typedef SmartPointer<class CPatchManifest> HPatchManifest;
	class CPatchManifest : public CObject, public Serialization::ISerializable 
	{
	public:
		WT_BEGIN_CLASS( CPatchManifest, CObject );
			WT_INHERITS_INTERFACE( Serialization::ISerializable );
		WT_END_CLASS();

		CPatchManifest()
			: m_hAboutShort( new CStringI18NSet() )
			, m_hAboutLong( new CStringI18NSet() )
			, m_hExpansionType( new CStringI18NSet() )
		{
		}

		CPatchManifest(const std::string& PatchExeUrl, const std::string& VersionTo, const CStringI18NSet& AboutShort, const CStringI18NSet& AboutLong)
			: m_PatchExeUrl(PatchExeUrl)
			, m_VersionTo(VersionTo)
			, m_hAboutShort( new CStringI18NSet( AboutShort ) )
			, m_hAboutLong( new CStringI18NSet( AboutLong ) )
			, m_hExpansionType( new CStringI18NSet() )
			, m_PatchExeFilename("")
			, m_Size(-1)
			, m_CRC(0)
		{
		}


		// Adds a language/value pair to the about long
		const void AddAboutLong (const CStringI18N& Value); 

		// Adds a language/value pair to the about short
		const void AddAboutShort (const CStringI18N& Value); 

		// Adds a pair of extension strings in a name/value format
		const void AddExpansionString(const std::string& Name, const CStringI18N& Value);

		const void AddExpansionString(const std::string& Name, const CStringI18NSet& Value);

		// Deletes an expansion by name
		const void DeleteExpansion (const std::string& Name);

		// Returns true if the given name exists in the expansion array.
		bool ExpansionExists (const std::string& Name) const;

		// Gets a "long" description of the patch
		const CStringI18N& GetAboutLong() const;

		// Gets a "short" description of the patch
		const CStringI18N& GetAboutShort() const;

		// Gets the number of expansion strings
		const int32 GetExpansionLength() const;

		// Gets the name of an expansion string.  If the
		// index is out of range will throw an exception "CIndexOutOfRangeException"
		const std::string& GetExpansionName(const int32 Index) const;

		// Gets a single expansion string by name.  If the name isn't in the 
		// expansion array, throws 'CNoSuchData' (use ExpansionExists() to test
		// for existance).
		// Is also in a non-const form
		const CStringI18NSet& GetExpansionString(const std::string& Name) const;

		// Gets a single expansion string by name.  If the name isn't in the 
		// expansion array, throws 'CNoSuchData' (use ExpansionExists() to test
		// for existance)
		// Is also in a const form
		CStringI18NSet& GetExpansionString(const std::string& Name);

		// Gets a single expansion string by index.  If the
		// index is out of range will throw an exception "CIndexOutOfRangeException"
		// Call .Get() on the CStringI18NSet to get the proper string value
		// for your locale.
		// Also in a non-const form
		const CStringI18NSet& GetExpansionValue(const int32 Index) const;

		// Gets a single expansion string by index.  If the
		// index is out of range will throw an exception "CIndexOutOfRangeException"
		// Call .Get() on the CStringI18NSet to get the proper string value
		// for your locale.
		// Also in a const form
		CStringI18NSet& GetExpansionValue(const int32 Index);

		// Gets the original exe filename.  This is used in the
		// UpdateFileInfo callto update the basic file info (size
		// and crc)
		const std::string& GetPatchExeFilename () const;


		// Gets the URL associated with this update.  The URL must
		// point to the .EXE that will be run when we want to update
		// the game.
		const std::string& GetPatchExeUrl() const;

		// Gets the CRC of the patch exe (as determined by UpdateFileInfo)
		const int32 GetPatchExeCRC() const;

		// Gets the size of the patch exe (as determined by UpdateFileInfo)
		const uint32 GetPatchExeSize() const;


		// Gets the version that this update, when applied, will bump
		// our game to.  That is, if we were version apple, and this
		// returns jack, then running this update will bump us from
		// apple to jack.
		//
		// Problem: should this just be a number?  Because then I can 
		// compare them; otherwise we might get into nasty versioning
		// issues.
		const std::string& GetVersionTo() const;

		// Sets a "long" description of the patch
		void SetAboutLong(const CStringI18NSet& Value);

		// Sets a "short" description of the patch
		void SetAboutShort(const CStringI18NSet& Value);

		// Sets the 'original' exe filename.  This is used in
		// UpdateFileInfo to set the size and CRC of the patch.
		void SetPatchExeFilename (const std::string& Value);

		// Sets the URL associated with this update.  The URL must
		// point to the .EXE that will be run when we want to update
		// the game.
		void SetPatchExeUrl(const std::string& Value);

		// Sets the version that this update, when applied, will bump
		// our game to.  That is, if we were version apple, and this
		// returns jack, then running this update will bump us from
		// apple to jack.
		//
		void SetVersionTo(const std::string& Value);

		// Updates the file into (size and CRC) and the file-based
		// expansion information based on the OriginalExeFilename
		// or the [name]_autoread value
		void UpdateFileInfo();

		// Loops through all of the expansions looking for
		// [name]_autowrite and writes them out into 
		// directory.  Is normally automatically called as part
		// of the 'get patch manifest' phase
		void UpdateFileWrite(const std::string& Directory);


		//
		// ISerializable interface
		//

		void GetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext ) const;

		void SetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext );

		static HPatchManifest ReadFromFile (const std::string& Filename);
		static HPatchManifest ReadFromFileNoThrow (const std::string& Filename);

		private:
			std::string m_PatchExeUrl;
			std::string m_VersionTo;

			//
			// Information that all patches need to have
			//
			HStringI18NSet m_hAboutShort;
			HStringI18NSet m_hAboutLong;

			//
			// File Info
			//
			std::string m_PatchExeFilename;
			int32 m_Size;
			uint32 m_CRC;

			//
			// Expansion info
			//
			HStringI18NSet m_hExpansionType;
			std::vector< CNameValueI18N > m_Expansion;
		};


//?
	/*
	CUpdateStatus tells you integrated status of a single update.  This does not control
	an update: you can't, for example, set the PatchManifest URL and call 
	SetStartPatchManifestDownload() and have anything actually happen.  This is
	the job of either CUpdate (from a low-level standpoint) or CUpdateMgr (from 
	the normal high-level standpoint).

	One of the important jobs of CUpdateStatus is to provide the details of the
	UpdateHistory.xml file.
	 */
	typedef SmartPointer<class CUpdateStatus> HUpdateStatus;
	class CUpdateStatus : public CObject, public Serialization::ISerializable 
	{
	public:
		WT_BEGIN_CLASS( CUpdateStatus, CObject );
			WT_INHERITS_INTERFACE( Serialization::ISerializable );
		WT_END_CLASS();
		CUpdateStatus();
		~CUpdateStatus();

		// Returns true iff we're as done as we're going to get
		// Note that we can be in virtually any state and be
		// done.
		bool IsDone() const;

		// Returns true iff we're as done as we're going to get
		// and there were no errors.
		bool IsDoneOK() const;

		void SetPatchManifestUrl(const string& Value);
		void SetStartPatchManifestDownload();
		void SetEndPatchManifestDownload(HttpStatusCode::Enum);

		void SetPatchExeUrl(const string& Value);
		void SetStartPatchExeDownload();
		void SetEndPatchExeDownload(HttpStatusCode::Enum);

		// Adds non-structured information about an update.
		void AddInfo (const string& Value);


		//
		// ISerializable interface
		//

		void GetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext ) const;

		void SetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext );

		private:
			std::string m_PatchManifestUrl;
			/*HttpStatusCode::Enum*/ int32 m_PatchManifestStatus;
			CDateTime m_PatchManifestDLStart;
			CDateTime m_PatchManifestDLEnd;
			std::string m_PatchExeUrl;
			/*HttpStatusCode::Enum*/ int32 m_PatchExeStatus;
			CDateTime m_PatchExeDLStart;
			CDateTime m_PatchExeDLEnd;

			std::vector< std::string > m_Info;
		};

//?
	/* 
	The CUpdateHistory class wraps the UpdateHistory.xml file.  That file
	contains our complete update history and functions as a sort of log file
	for debugging and lets us save, between program runs, what we're actually
	doing -- whether we've got a new patch manifest file, whether we've applied
	the update, what the results were, etc.

	CUpdateHistory contains a series of CUpdateStatus items.
	This is used by the UpdateMgr class to manage the updates as whole
	*/
	typedef SmartPointer<class CUpdateHistory> HUpdateHistory;
	class CUpdateHistory : public CObject, public Serialization::ISerializable 
	{
	public:
		WT_BEGIN_CLASS( CUpdateHistory, CObject );
			WT_INHERITS_INTERFACE( Serialization::ISerializable );
		WT_END_CLASS();

		CUpdateHistory();
		virtual ~CUpdateHistory();

		// Adds an update status to the list of status; only up
		// to <n> are kept where <n> is normally 32.
		void Add (CUpdateStatus& Value);

		// Returns the update status of the index.  If the index is
		// out of range, will throw TODO:
		CUpdateStatus& Get(const int32 Index);

		// Returns the newest status; if there are non will throw TODO:
		// Is just a simple way of call Get(Length()-1).
		CUpdateStatus& GetNewest();

		// Gets the maximum number of update items
		int32 GetMaxData() const;

		// Returns the amount of history we're keeping.
		int32 Length() const;

		// Sets the maximum number of update items
		void SetMaxData(const int32 Value);



		//
		// ISerializable interface
		//

		void GetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext ) const;

		void SetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext );

		private:
			int32 m_MaxData;
			std::vector< CUpdateStatus > m_Data;
		};

//?
	/*
	The CMasterUpdate class wraps the MasterUpdate.xml file that declares,
	for each game, what URL has the most up-to-date PatchManifest.xml file.
	 */
	typedef SmartPointer<class CMasterUpdate> HMasterUpdate;
	class CMasterUpdate : public CObject, public Serialization::ISerializable 
	{
	public:
		WT_BEGIN_CLASS( CMasterUpdate, CObject );
			WT_INHERITS_INTERFACE( Serialization::ISerializable );
		WT_END_CLASS();

		CMasterUpdate();
		CMasterUpdate(const std::string& PatchManifestUrl);
		virtual ~CMasterUpdate();

		const std::string& GetPatchManifestUrl() const;

		//
		// ISerializable interface
		//

		void GetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext ) const;

		void SetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext );

		private:
			std::string m_PatchManifestUrl;
	};



//?
	namespace EUpdateManagerState
	{
		enum Enum
		{
			KCanDownloadManifest,
			KAmDownloadingManifest,
			KAmNotifyingBadManifest,
			KCanDownloadPatch,
			KAmDownloadingPatch,
			KAmNotifyingBadPatch,
			KCanInstallPatch,
		};
	}

	/* 
	The CUpdateManager class is the main class used by applications to
	handle updates for a single application. 
	 */
	typedef SmartPointer<class CUpdateManager> HUpdateManager;
	class CUpdateManager : public CObject //, public Serialization::ISerializable 
	{
	public:
		WT_BEGIN_CLASS( CUpdateManager, CObject );
			//WT_INHERITS_INTERFACE( Serialization::ISerializable );
		WT_END_CLASS();

		CUpdateManager();
		CUpdateManager(const std::string& StartingDirectory, const std::string& Version);
		virtual ~CUpdateManager();

		// Will do a bounce -- that is, will
		// [a] set up a the RerunProgram external program to
		//     1. run the given Patch program
		//     2. rerun the client program
		// [b] exit
		// while avoiding a shutdown/startup race by requiring
		// that the 'wtrerunprogramutility' program implement
		// a 'W<pid> <maxtime>' command.
		static void Bounce(const std::string& RerunProgram, const std::string& PatchProgram);


		// Sets the starting directory.  When this is called the MasterUpdate.xml
		// and UpdateHistory.xml files are read in and the class is initialized.
		// This is automatically called by the constructor in all cases.
		void SetStartingDirectory(const std::string& Value);

		// Utility routine: deletes the old patch.exe
		// This is not normally called from the game application,
		// but is being made public just in case
		void DeletePatchExe ();

		// Utility routine: deletes the patch manifest
		// This is not normally called from the game application,
		// but is being made public just in case.
		// Will also get rid of the current internal 'CUpdateManifest'
		void DeletePatchManifest();

		EUpdateManagerState::Enum GetCurrentState();

		bool IsPatchAvailable() const;
		bool IsPatchBad() const;
		bool IsPatchGood() const;
		bool IsPatchReady() const; 



		// Gets the most recent patch manifest
		CPatchManifest* GetPatchManifest();

		// First of five actions: start downloading the manifest
		// This should be exposed to the user as 'Check for new updates'
		// State must be: KCanDownloadManifest OR KCanDownloadPatch OR KCanInstallPatch
		void StartDownloadingManifest();

		// First of two interupts: cancel a manifest download.
		void CancelManifestDownload();

		// Second of five actions: the manifest download has failed,
		// and the user has been told -- which means that they can
		// try to download again.
		// State must be: KAmNotifyingBadManifest
		void NotifiedUserOfBadManifest();

		// Third of five actions: the manifest download worked, and
		// there's a new patch -- starts downloading it on a new thread.
		// State must be: KCanDownloadPatch
		void StartDownloadingPatch();

		// Second of two interupts: cancel a manifest download.
		void CancelPatchDownload();

		// Fourth of five actions: the patch downloaded, but it's 
		// bad somehow -- it has a bad checksum, etc -- and the user
		// has been told.  Once this is called, the user can try to
		// download again.
		// State must be: KAmNotifyingBadPatch
		void NotifiedUserOfBadPatch();

		// Fifth of five actions: the patch is ready to go.  This will
		// normally NOT RETURN -- the application EXITS.
		// State must be: KCanInstallPatch
		void StartInstallingPatch();


		// Returns a number from 0 to 100 (inclusive) where:
		//			MEANING OF PERCENT DONE
		//  STATE
		//		0	 Always	  0
		//		1			  1..99
		//		2	 Always	  0
		//		3	 Always	  0
		//		4			  1..99
		//		5	 Always	  0
		//		6	 Always	100
		//
		float32 GetPercentDone();

		// Gets the current version that the UpdateManager thinks
		// that the application is set to.  This is set in the 
		// constructor.
		const std::string& GetVersion() const;


		/*
		//
		// ISerializable interface
		//

		void GetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext ) const;

		void SetObjectData( Serialization::CSerializationInfo* pInfo, 
							Serialization::CSerializationContext* pContext );
		*/
	private:
		std::string	m_StartingDirectory;
		std::string m_Version; // the application version
		HMasterUpdate m_hMaster;
		CUpdateHistory* m_pHistory;
		HPatchManifest m_hPatchManifest;
		EUpdateManagerState::Enum m_CurrentState;



		std::string m_PatchExeFilename;
		std::string m_PatchManifestFilename;
		std::string m_UpdateHistoryFilename;
		std::string m_MasterUpdateFilename;
		std::string m_WTRerunProgramUtilityFilename;

		//
		// Temporary objects for downloading
		//
		CHttpWebRequest* m_pPatchExeRequest;
		IAsyncResult* m_pPatchExeResult;
		static void WTCall DownloadUpdateCallback (const IAsyncResult* AR);
		WT::float32 m_DownloadUpdatePercent;

		CHttpWebRequest* m_pPatchManifestRequest;
		IAsyncResult* m_pPatchManifestResult;
		static void WTCall CheckForUpdatesCallback (const IAsyncResult* AR);
		WT::float32 m_CheckForUpdatesPercent;

		//
		// Helper methods
		//
		void OnPatchManifestDownloadComplete();
		void OnPatchExeDownloadComplete();

		};


} // namespace Net
} // namespace System
} // namespace WT