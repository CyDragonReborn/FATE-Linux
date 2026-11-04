#ifndef WT_PAL_H_
#define WT_PAL_H_
/***
 * $Id: pal.h,v 1.15 2004/06/04 23:35:49 cvlascea Exp $
 *
 * Copyright (C) 2004 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description: Wild Tangent Publishing Abstraction Layer API
 * Author:      Cristian Vlasceanu
 * Notes:      
 *
 ***/
#define WTPAL_API_MAJOR         1
#define WTPAL_API_MINOR         0
#define WTPAL_API_REVISION      4


/**************************************************************
 WildTangent Publishing SDK game integration cheat list

 1) Add a PAL_Init() call in your initialization block
    (typically the constructor of your Application class);
    add a PAL_Done() call in your Applicator destructor, or
    near the end of your game.

 2) Install an error handler, if you need/want to have a callback
    function (of the PAL_ErrorHandler type) invoked when 
    PAL encounters an error. Hint: a callback that displays 
    the error message in a MessageBox may be useful during 
    development/integration/debugging.

 3) Call PAL_DisplayShell(kGameStartSession, NULL, title, icon)
    before entering your Windows message loop; add a call to
    PAL_DisplayShell(kGameEndSession, NULL, title, icon) after
    the message loop.

    This is where the pre and post game upsell happens -- 
    if enabled, otherwise these are just no-ops.

    NOTE For more sophisticated, seamless integration of the PAL 
    shell into your game please refer to the PAL_DisplayShell 
    documentation.

 4) Call PAL_IsUnlocked() to distinguish between a demo and a
    purchased version of the app.
 
 5) use PAL_Unlock or PAL_DisplayShell(kGameBuyOnline, ...) or
    PAL_DisplayShell(kGameUnlockOnline,...) NOTE: if you are 
    calling PAL_DisplayShell while a a message loop is already 
    active please refer to PAL_DisplayShell documentation below.

 6) If you want to support a time-based upsell, add calls to 
    PAL_QueryTimeRemaining() NOTE: if you are linking against a
    version of PAL that is not configured for a time-based
    business model, the function will return INFINITE. Preping the
    application source for different upsell models makes switching
    to another model just a matter of re-linking.

 7) Call PAL_Poll() from your rendering loop (same place you
    normally call a Render() function). This, among other things,
    enables PAL to keep track of the played time.
 */

#if defined(WIN32) && !defined(_WINDOWS_)
  #ifndef WIN32_LEAN_AND_MEAN
    /* Exclude rarely-used stuff from Windows headers */
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
#endif
#include <limits.h>

#define WTAPITYPE   __cdecl
#define WTRESULT    long
#define WTAPI       WTRESULT WTAPITYPE

#ifndef EXPORT
#if defined(DLL)
 #define EXPORT  __declspec(dllexport)
#else 
 #define EXPORT
#endif
#endif

#if defined(_MSC_VER)
 #pragma pack(push, 1)
#endif

#ifdef __cplusplus
extern "C"
{
 #define NOTHROW throw()
#else
 #define NOTHROW
#endif

/* Messages */
#define WM_PALBASEMSG       WM_USER + 42
#define WM_PALSHELLDONE     WM_PALBASEMSG + 0   /* See PAL_DisplayShell */
#define WM_PALSHELLEXIT     WM_PALBASEMSG + 1   


#ifndef INFINITE /* should already be defined in winbase.h */
/* for PAL_QueryTimeRemaining, PAL_QuerySessionsRemaining */
 #define INFINITE (WTRESULT)-1
#endif


/*
 * Type definitions
 */
typedef enum _PAL_UnlockAction
{
    kUnlockNone      = -1,
    kUnlockGame      = 0,
    kUnlockBuy       = 1,
    kUnlockSubscribe = 2,
    kUnlockAny       = INT_MAX,
} PAL_UnlockAction;


typedef struct _PAL_UnlockParam
{
    DWORD size;            /* sizeof this struct */
    PAL_UnlockAction action;
    const char* userID;     /* optional, may be NULL */
    const char* key;        /* optional, may be NULL */
    const char* contentGUID;/* if NULL, defaults to value from PAL_Init */
    const char* productGUID;
    int verifyLabel;        /* if not zero, used for extra verification */
} PAL_UnlockParam;


typedef enum _PAL_Error
{
    /* "An error occurred inside the Publishing Abstraction Layer" */
    PAL_GENERIC_ERROR           = -1,

    /* "Success" */
    PAL_SUCCESS                 = 0,

    /* DRM could not open internet resource */
    PAL_INTERNET_OPEN_FAILED    = 1,

    /* DRM could not connect to the internet */
    PAL_INTERNET_CONNECT_FAILED = 2,

    /* DRM internet request failed */
    PAL_INTERNET_REQUEST_FAILED = 3,

    /* Invalid unlock code */
    PAL_INVALID_UNLOCK_CODE     = 4,

    /* Invalid argument or null pointer. */
    PAL_INVALID_ARGUMENT        = 5,

    /* "Buffer length exceeded" */
    PAL_BUFFER_OVERFLOW         = 6,

    /* "Error writing DRM attribute (readonly?)" */
    PAL_WRITE_ATTRIBUTE_FAILED  = 7,
    
    /* Operation not supported */
    PAL_NOT_SUPPORTED           = 8,

    /* "The Publishing Abstraction Layer function is not currently implemented. */
    PAL_NOT_IMPLEMENTED         = 9,

    PAL_VALUE_NOT_FOUND         = 10,
    /* TODO: define more error codes */
} PAL_Error;


/* Event types, for PAL_DisplayShell */
typedef enum _PAL_GameEvent
{
    kGameNone           = 0,
    kGameStartSession   = 1,
    kGameStartPlay      = 2,
    kGameEndSession     = 3,
    kGameEndPlay        = 4,
    kGameLevelChange    = 5,
    kGameBuyOnline      = 6,    /* initiate e-commerce buy */
    kGameUnlockOnline   = 7,
    kGamePostScore      = 8,
    kGameViewScores     = 9,
} PAL_GameEvent;


/* Error Handler Prototype, see PAL_SetErrorHandler; various
 * functions in this API (but PAL_Init) may call a callback
 * of this type (if set), to provide error information. For
 * minimal error handling the return code should be sufficient.
 */
typedef void WTAPITYPE PAL_ErrorHandler(const char* function, 
                                        PAL_Error,
                                        const char* errorMsg, 
                                        void* userData);

/**
 * Get the version # of the bound PAL DLL.
 */
EXPORT WTAPI PAL_GetVersion(int*, int*, int*, int*) NOTHROW;

/**
 * Get the version # of the linked-in library as a string.
 * Note that this returns the linked in library version, not the bound PAL DLL version.
 */
EXPORT const char* WTAPITYPE PAL_GetVersionString() NOTHROW;

EXPORT const char* WTAPITYPE PAL_GetFlavor() NOTHROW;

/**
 * Initialize the PAL library. First to be called
 * before calling any other function in this SDK.
 * @param productGUID
 * @param appCommandLine
 * @param appPath if NULL, and the PAL library is in-process,
 * then PAL will determine it by calling GetModuleFileName
 *
 * Example
 *
 * @code
 * PAL_Init( "crystalmaze", 
 *         "3C448D22-AD49-43EC-85C5-A6020A10E823", // content GUID 
 *         "B2F422C1-3F25-4731-9CC9-1EBD63C09201", // license GUID
 *         lpCmdLine, 
 *         _AppWindow.m_AppPath );
 *
 * @endcode
 * @note: The implementation looks for a file named 
 * /scripts/prodinfo.dat under the appPath and parses it.
 * if the contentName and contentGUID are NULL, the values from
 * the prodinfo.dat files are used.
 */
EXPORT WTAPI PAL_Init(const char* contentName,
                      const char* contentGUID,
                      const char* licenseGUID,
                      const char* appCommandLine,
                      const char* appPath) NOTHROW;

/**
 * An alternate way of initializing PAL, that allows for
 * a storeName to be passed in. The content name and GUID
 * are read from the prodinfo.dat file.
 */
EXPORT WTAPI PAL_InitLicense(
                      const char* storeName, /* default is "WT" */
                      const char* licenseGUID,
                      const char* appCommandLine,
                      const char* appPath) NOTHROW;

/**
 * Uninitialize library.
 */
EXPORT void WTAPITYPE PAL_Done() NOTHROW;

EXPORT const char* WTAPITYPE PAL_GetErrorMessage(DWORD);


/**
 * This function needs to be called from the rendering loop.
 * @param elapsedMillisec time in milliseconds since last time
 * the function was called.
 * @note pass -1 to let pal update the tick count internally.
 */
EXPORT WTAPI PAL_Poll(DWORD elapsedMillisec) NOTHROW;


/**
 * Sets the error handler function and returns pointer 
 * to previous handler. 
 *
 * @example
 * @code
 * HWND hwndApp = CreateWindow(); 
 *     
 * void WTAPITTYPE DisplayError(const char* func, 
 *                              PAL_Error errCode,
 *                              const char* errMsg, 
 *                              void* userData)
 *  {
 *      HWND hwndApp = reinterpret_cast<HWND>(userData);
 *      assert(IsWindow(hwndApp));
 *
 *      // display error in a message box on top of the app window
 *      char buf[512];
 *      _snprintf(buf, sizeof(buf) - 1, 
 *               "%d: %s (in function %s)", errCode, errMsg, func);
 *      buf[sizeof(buf) - 1] = 0;
 *
 *      MessageBoxA(hwndApp, buf, "PAL Error", MB_OK | MB_ICONERROR);
 *  }
 *
 *  PAL_SetErrorHandler(DisplayError, (void*) hwndApp);
 * @endcode
 */
EXPORT PAL_ErrorHandler* WTAPITYPE 
    PAL_SetErrorHandler(PAL_ErrorHandler*, void* userData) NOTHROW;


/**
 * Attributes support applies only to some DRMs (such as WildTangent's
 * own DRM3 and above). Use PAL_AttributesSupported to determine whether
 * individual integer, float, boolean and string attributes are supported.
 * Functions for setting and querying the attributes are provided.
 */
EXPORT BOOL WTAPITYPE PAL_AttributesSupported() NOTHROW;

/*
 * Generic attribute setters
 */
EXPORT WTAPI PAL_SetInt(const char* name, int value) NOTHROW;

EXPORT WTAPI PAL_SetBool(const char* name, BOOL value) NOTHROW;

EXPORT WTAPI PAL_SetFloat(const char* name, float value) NOTHROW;

EXPORT WTAPI PAL_SetString(const char* name, const char* value) NOTHROW;


/*
 * Generic query functions
 */

/**
 * Query an integer value. If not found in the storage, the
 * defaultValue is returned instead. If errors occur, the error
 * handler is invoked.
 */
EXPORT int WTAPITYPE PAL_QueryInt(const char* name, int defaultVal) NOTHROW;


EXPORT BOOL WTAPITYPE PAL_QueryBool(const char* name, BOOL defaultVal) NOTHROW;


/**
 * Query a floating point value. If not found in the storage, the
 * defaultValue is returned instead. If errors occur, the error
 * handler is invoked.
 */
EXPORT float WTAPITYPE PAL_QueryFloat(const char*, float defaultVal) NOTHROW;


/**
 * Queries a string value, and if found, fills out buffer
 * otherwise fills in the defaultValue.
 * @return PAL_SUCCESS if ok, an error code otherwise.
 * Possible result codes: PAL_INVALID_ARGUMENT, PAL_BUFFER_OVERFLOW
 * Additionally, if some other catastrophic error occurs, the error
 * handler may be invoked.
 * @note: when the function returns PAL_BUFFER_OVERFLOW, 
 * bufLen will contain the needed size.
 */
EXPORT WTAPI PAL_QueryString(const char* name, 
                             char* buffer,
                             size_t* bufLen,
                             const char* defaultVal) NOTHROW;

/*
 *  Specialized query functions
 */

/**
 * Get remaining time, in seconds. -1 means infinite.
 * @param defaultMaxTime: in the case where the DRM does not impose a
 * max time, but rather stores the played time, the remaining time
 * is computed by subtracting the played time from defaultMaxTime.
 * If the DRM (that PAL was configured with) knows what the allowed maximum
 * time is, then defaultMaxTime is ignored. 
 */
EXPORT long WTAPITYPE PAL_QueryTimeRemaining(long defaultMaxTime) NOTHROW;

/**
 * Similar to PAL_QueryTimeRemaining only it returns number of plays left.
 */
EXPORT long WTAPITYPE PAL_QueryPlaysRemaining(long defaultMaxPlays) NOTHROW;

/**
 * Similar to PAL_QueryTimeRemaining only it returns number of sessions left.
 */
EXPORT long WTAPITYPE PAL_QuerySessionsRemaining(long defaultMaxSessions) NOTHROW;

/*
 * Shell functions
 */

/**
 * Display a "shell" window, for upsell/e-commerce purposes. Depending upon
 * the inner PAL configuration, this can be PAL's own "WIRE" shell, the Trymedia
 * or Keyhole shell (the list is open).
 * The simplest way to use the function is by passing it a NULL window handle 
 * and invoke it before and after your main event-processing loop. In this case,
 * the shell is hosted inside its own, top-level window, and runs its own message
 * pump. The integration is not that seamless, since the user may notice the separate
 * windows popping up and down.
 * @note the following applies only to PAL's own "WIRE" shell, for now.
 * Another option is to call PAL_DisplayShell from an message handler, and pass it
 * the window handle of your app window (or any other window where you would like
 * the shell to be hosted). In this case the shell relies on the application event
 * loop rather than running its own. For this use case, it is important that the
 * event loop calls PAL_TranslateAccelerator. When the shell window is destroyed,
 * the event gets translated into WM_PALSHELLDONE event, that you can use to resume
 * normal rendering.
 */
EXPORT WTAPI PAL_DisplayShell(
                              PAL_GameEvent, 
                              HWND,
                              LPTSTR title,
                              HICON hIcon
                             ) NOTHROW;


/**
 * Needs to be called from the main event loop, to ensure
 * correct functioning of TAB key and accelerators in the
 * shell, when displayed. 
 * @note: this is needed for "tight, seamless" integration 
 * only, i.e. when you pass a non-null window handle to 
 * PAL_DisplayShell.
 *
 * Example:
 * @code
 *      while( int res = GetMessage( &msg, NULL, 0, 0 ) )
 *        {
 *           if (res == -1) 
 *             {
 *               break; // oops
 *             }
 *           else if (PAL_TranslateAccelerator(&msg) == S_FALSE) 
 *             {
 *               TranslateMessage( &msg );
 *               DispatchMessage( &msg );
 *             }
 *        }
 * @endcode
 */
EXPORT WTAPI PAL_TranslateAccelerator(LPMSG) NOTHROW;

/*
 * Unlock
 */

/**
 * Unlock the application.
 * @return PAL_SUCCESS or error code (such as
 * PAL_INVALID_ARGUMENT, PAL_INVALID_UNLOCK_CODE, etc.)
 */
EXPORT WTAPI PAL_Unlock(const PAL_UnlockParam*) NOTHROW;

EXPORT BOOL WTAPITYPE PAL_IsUnlocked() NOTHROW;

EXPORT PAL_UnlockAction PAL_GetUnlockType() NOTHROW;


/* Convenience wrappers around QueryInt/SetInt, etc. 
   Also: some DRMs that we wrap may not support attributes, yet 
   keep track of played games, time, etc, in their own, specific way.
   Time is automatically incremented by PAL_Poll. */

EXPORT WTAPI PAL_IncrementGamesPlayed() NOTHROW;

EXPORT WTAPI PAL_IncrementSessionCount() NOTHROW;


EXPORT WTAPI PAL_AddHighScore(const char* name, unsigned int score) NOTHROW;

/**
 * Set a named value into a memory-based, name-value dictionary -- the
 * semantics of the stored value is left to application.
 * NAME cannot be null. If VALUE is NULL, and a value with the same
 * NAME exists, it is removed from the dictionary.
 * In other words, PAL_SetValue("foo", NULL) is equivalent to removing
 * "foo" from the dictionary.
 * @note the WIRE shell can access the dictionary via window.external.SetValue()
 * and window.external.GetValue()
 */
EXPORT WTAPI PAL_SetValue(const char* name, const char* value) NOTHROW;

/**
 * Retrieve a named value previously stored in memory by PAL_SetValue.
 * @return PAL_SUCCESS, PAL_VALUE_NOT_FOUND, PAL_BUFFER_OVERFLOW
 * length should contail the buffer size when calling the function;
 * if the buffer is not large enough to accomodate the value, the
 * function will return PAL_BUFFER_OVERFLOW and length will contain
 * the minimum needed size.
 * @note the WIRE shell can access the dictionary via window.external.SetValue()
 * and window.external.GetValue()
 */
EXPORT WTAPI PAL_GetValue(const char* name,
                          char* buffer,
                          size_t& length /* [in/out] */) NOTHROW;

/**
 * Return true if there is any DRM support built into PAL
 */
EXPORT BOOL WTAPITYPE PAL_IsDRMSupported() NOTHROW;

#ifdef __cplusplus
}
#endif

#if defined(_MSC_VER)
 #pragma pack(pop)
#endif

#endif // WT_PAL_H_
