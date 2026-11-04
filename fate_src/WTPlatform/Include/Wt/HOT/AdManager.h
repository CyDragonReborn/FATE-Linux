// ----------------------------------------------------------------------------
//
// AdManager.h
//
// Copyright (C) 2006 WildTangent, Inc. 
// All Rights Reserved
//
// ----------------------------------------------------------------------------

#pragma once

// ----------------------------------------------------------------------------
// Include Files
// ----------------------------------------------------------------------------

#include "WT/System/Platform.h"
#include <string>
#include "WT/HOT/Ad.h"
#include "WT/System/Callback.h"
#ifdef USE_DIRECTX_7
#include "d3dx.h"
#elif defined USE_DIRECTX_8
#include "d3dx8.h"
#else
#include "d3dx9.h"
#endif

namespace WT
{

// WildTangent HOT (HTML on Textures) namespace is the namespace for all 
// HOT-related classes and functionality.

namespace HOT
{

class CAdManagerImpl;

// HOT (HTML On Texture) Ad Manager is the high level ad manager for
// placing ads on different textures in a game.

class CAdManager
{
public:

	// Type definition for the Ad update callback - this defines
	// a member or a static method that accepts a CAd pointer as
	// a parameter and returns a float32 as return value.

	typedef System::CCallback1r<CAd*, float32>	UpdateCallback;

	// Type definition for the update event

	typedef UpdateCallback::Event				UpdateEvent;

	// Type definition for the render callback - this defines a
	// member or a static method the accepts a CAd pointer as a
	// parameter and returns a bool as a return value

	typedef System::CCallback1r<CAd*, bool>		RenderCallback;

	// type definition for the render event

	typedef RenderCallback::Event				RenderEvent;

	// Construct a new instance of the CAdManager class. The product code names
	// will be determined automatically according to game settings registered per
	// WildTangent conventions (the runtime attributes).
	CAdManager(
#ifdef USE_DIRECTX_7
		IDirect3DDevice7* pDevice,
#elif defined USE_DIRECTX_8
		IDirect3DDevice8* pDevice,
#else
		IDirect3DDevice9* pDevice,	// The Direct3DDevice9 this manager will
									// use to render the ads. This device
									// has be valid and allocated before
									// creating the manager
#endif

		HWND hParentWindow			// Parent window for the manager - usually
									// the application main window (see
									// SetParentWindow)
	);

	// Construct a new instance of the CAdManager class with explicitly specified
	// product code names. If either prodcut code name is empty, then
	// a std::runtime_error exception will be thrown.
	CAdManager(
#ifdef USE_DIRECTX_7
		IDirect3DDevice7* pDevice,
#elif defined USE_DIRECTX_8
		IDirect3DDevice8* pDevice,
#else
		IDirect3DDevice9* pDevice,	// The Direct3DDevice9 this manager will
									// use to render the ads. This device
									// has be valid and allocated before
									// creating the manager
#endif

		HWND hParentWindow,			// Parent window for the manager - usually
									// the application main window (see
									// SetParentWindow)

		const std::string& ProductCodeName,	// The ProductCodeName given to the developer. This
											// affects which ads will be displayed. The ProductCodeName
											// must not be empty.
		const std::string& DPCodeName //The DPCodeName (distribution partnter code name) given to
									  //the developer. This affects which ads will be displayed. The
									  //DPCodeName must not be empty.
	);

	// Destructor for the CAdManager class

	virtual ~CAdManager();

	// Set the parent window for the manager. This will usually be the main
	// application window.

	void SetParentWindow( HWND hParentWindow );

	// Get the parent window for the manager.

	HWND GetParentWindow() const;

	// Set the ProductCodeName this manager will use in order to display the ads. The
	// ProductCodeName along with the DPCodeName and each ad's ID is used to choose
	// the ad. Attempting to use an empty ProductCodeName will result in a std::runtime_error
    // exception.
	void SetProductCodeName( std::string const& ProductCodeName );

	// Get the ProductCodeName this manager is using in order to display the ads. The
	// ProductCodeName along with the DPCodeName and each ad's ID is used to choose the ad.
	std::string const& GetProductCodeName() const;

	// Set the DPCodeName this manager will use in order to display the ads. The
	// DPCodeName along with the ProductCodeName and each ad's ID is used to choose
	// the ad. Attempting to use an empty DPCodeName will result in a std::runtime_error
    // exception.
	void SetDPCodeName( std::string const& DPCodeName );

	// Get the DPCodeName this manager is using in order to display the ads. The
	// DPCodeName along with the ProductCodeName and each ad's ID is used to choose the ad.
	std::string const& GetDPCodeName() const;

	// Check if the user is currently online.
	// This method will check if the user is currently connected to the
	// internet, and is useful for making decisions on which ads to show.
	// Note: be aware that this is a lengthly operation and it is blocking - so
	// it is not recommended to check for it every frame - only in the
	// beginning of the game.
	static bool WTCall IsOnline();

	// Create a new ad to be attached to a DirectX surface and managed by 
	// the manager. The return value is the newly created ad. The manager retains
	// ownership over the ad. You can call RemoveAd to remove and delete the ad.
	// The offline URL - the AdId plus a ".html" extension - must exist in the 
	// LocalHTML directory under the current working directory, or 
	// std::runtime_error will be thrown.

	CAd* AddAd(
#ifdef USE_DIRECTX_7
		LPDIRECTDRAWSURFACE7 pSurface ,
#elif defined USE_DIRECTX_8
		LPDIRECT3DSURFACE8 pSurface ,
#else
		LPDIRECT3DSURFACE9 pSurface ,			// The DirectX surface this ad
												// will be rendered into
#endif

		const std::string& AdID ,				// The Ad Id to be passed to
												// the server

		uint32 Width ,							// The width of the ad (in
												// pixels)

		uint32 Height ,							// The height of the ad (in
												// pixels)

		std::string const& ImageName,			//An image file name used to initialize the ad
												//surface. The image must be in D3DX-compatible
												//format. In DX7, the ad destination surface must 
												//be in one of these supported formats: D3DX_SF_A8R8G8B8,
												//D3DX_SF_X8R8G8B8, or D3DX_SF_R8G8B8.

	    RECT* DestRect = NULL ,					// The destination rect the
												// ad will be drawn into.
												// passing NULL will render the
												// ad using its original width
												// and height

		const std::string& CustomParams = "" ,	// Custom parameters that will
												// be send to the server. These
												// needs to be in the form of a
												// name-value pairs

		bool VisibleImmediately = true			// Is this ad going to be
												// visible immediately (true)
												// or not (false).
	);

	// Remove a specific ad from the ad manager's managed ads.
	// Note: this will actually delete the specified ad.

	void RemoveAd( CAd* pAd );

	// Remove all managed ads from the manager and delete them.

	void RemoveAllAds();

	// Return an ad pointer based on the specified zero-based AdIndex.

	CAd* GetAd( uint32 AdIndex ) const;

	// Get the number of ads currently in the manager

	uint32 GetAdCount() const;

	// OnFrameMove method that needs to be called on the frame move of the
	// game.
	// You need to call this method on every frame move, this will allow the
	// manager to update various ad related properties. You can pass an
	// optional camera location parameter to assist with optimizing
	// updating performance.

	void OnFrameMove(
		float ElapsedTime,						 // Time passed since last frame
												 // move call (in seconds )

        D3DXVECTOR3 const* CameraPosition = NULL // Optional 3d vector for the
												 // camera position)
	);

	// OnFrameRender method that needs to be called on the frame render of the
	// game.
	// You need to call this method on every frame render, this will allow the
	// manager to update the actual ad textures. This method can be called
	// inside the beingscene/endscene or before it. You can pass an optional
	// camera location parameter to assist with optimizing updating
	// performance.

	void OnFrameRender(
		float ElapsedTime,						 // Time passed since last frame
												 // move call (in seconds )

		D3DXVECTOR3 const* CameraPosition = NULL // Optional 3d vector for the
												 // camera position)
	);

	// Set the maximum number of HTML renderers to be used by the manager.
	// Each renderer takes a certain amount of resources, so using many
	// renderers will decrease performance and use more memory. Using less
	// renderers is preferable, however, this will cause ads to update less
	// often.
	// If you have more visible ads than renderers, then the manager will
	// try to cycle the usage of the available renderers amongst all visible
	// ads (doing that in a defined interval - see SetOptimizationInterval)
	// The default value is 5 renderers.

	void SetMaxRenderers( uint32 Value );

	// Get the maximum number of HTML renderers to be used by the manager.
	// Each renderer takes a certain amount of resources, so using many
	// renderers will decrease performance and use more memory. Using less
	// renderers is preferable, however, this will cause ads to update less
	// often.
	// If you have more visible ads than renderers, then the manager will
	// try to recycle the usage of the available renderers amongst all visible
	// ads (doing that in a defined interval - see SetOptimizationInterval)
	// The default value is 5 renderers.

	uint32 GetMaxRenderers() const;

	// Set the background render thread priority. This will impact how much
	// time is given to the renderers to update the background image for each
	// ad. See the SetThreadPriority windows call for values.

	void SetRenderThreadPriority( int Priority );

	// Get the background render thread priority. This will impact how much
	// time is given to the renderers to update the background image for each
	// ad. See the SetThreadPriority windows call for values.

	int GetRenderThreadPriority() const;

	// Set the optimization interval for renderers recycling (in seconds).
	// See SetMaxRenderers about renderers recycling.The default value
	// is one second.

	void SetOptimizationInterval( float32 Interval );

	// Get the optimization interval for renderers recycling (in seconds).
	// See SetMaxRenderers about renderers recycling.
	// The default value is one second.

	float32 GetOptimizationInterval( ) const;

	// Track usage statistics for the ads in the system. Pass a specific ad to
	// track only for this ad, or NULL to track for all currently managed ads.
	// This call will use the tracking system to keep a record of the various
	// metrics for the ads in the system, maintain an offline queue (in case
	// the user is not online) and upload this data to the servers as long as
	// the data is being generated. Calling this method will result in the
	// generation of one or more log records for the ad/ads.

	void TrackUsage( CAd* pAd = NULL );

	// The update event for the ads

	UpdateEvent OnUpdateAd;

	// The render event for the ads

	RenderEvent OnRenderAd;

	//Check if ads should be displayed. Returns true if ads should be displayed, 
	//false otherwise.

	static bool WTCall ShouldAdsBeDisplayed();

private:
	CAdManagerImpl* m_pImpl;

	friend CAd;
	friend CAdImpl;
};

} //namespace HOT
} //namespace WT
