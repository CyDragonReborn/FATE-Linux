// ----------------------------------------------------------------------------
//
// Ad.h
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
#ifdef USE_DIRECTX_7
#include "d3dx.h"
#elif defined USE_DIRECTX_8
#include "d3dx8.h"
#else
#include "d3dx9.h"
#endif

namespace WT
{
namespace HOT
{

class CAdManager;
class CAdImpl;
class CAdManagerImpl;

// This namespace wraps the EAdPolicy enumeration

namespace EAdPolicy
{
	//This enumeration is used to determine how updating and rendering of the
	//ad texture would behave.

	enum AdPolicy
	{
		Never,					// Never update or render the ad texture.

		AsFastAsPossible,		// Update or render the ad texture as fast as
								// possible

		Interval,				// Update or render the ad texture based on a
								// given set interval in seconds

		BasedOnCameraLocation,	// Update or render the ad texture based on the
								// given camera location. As the camera gets
								// closer to the ad, updating and rendering
								// will be faster, as the camera moves away,
								// updating and rendering will be slower.

		Custom					// Use a custom callback to calculate when to
								// update and/or render the ad texture.
	};
}

// This namespace wraps the EAdMetric enumeration

namespace EAdMetric
{
	// Enumeration used to enquire about several ad metrics.
	// The various metrics combined provide indication about how viewable is
	// the ad to the user, where 0 means the ad is completely not viewable,
	// and 1 means completely viewable.
	// All metrics are approximated values and are based on several factors
	// given by the developer to the system, amongst them are the size of the
	// ad, its position, its visiblity, etc. if these parameters are not
	// provided, the system has no idea about where and how the ads are
	// actually renderer (it can be wrapped on a sphere for all that it
	// cares...), in that case the only calculations will be done base on the
	// visiblity flag.

	enum AdMetric
	{
		Alignment,				// Metric that defines if the ad is facing
								// the user. 1 means fully facing, 0 means
								// facing away.

		Roll,					// Metric that defines if the ad is in the
								// right viewing angle to the user. 1 means
								// parallel to the camera, 0 means up-side
								// down.

		ViewArea,				// Metric that defines if the ad is within
								// the viewing frustum of the camera. 1 means
								// it is fully within the frustum, 0 means it
								// is fully clipped away.

		ScreenSpace,			// Metric that defines how much real screen
								// space the ad occupies. 1 means that ad takes
								// 100% if its texture size on screen, 0 means
								// it takes 0 pixels or occupies too much screen
								// space

		View,					// Returns a combined value from 0 to 1 of all
								// other metrics.

		Last					// Metric to indicate last value in the enum.
	};
}

// This struct is used to define the ad appearance in the 3d scene. This
// will describe a rectangular plane in 3d space facing the direction
// of the normal. Using this info (if set on the ad), the ad manager can
// calculate optimized display behavior, as well as compute tracking
// information about the ad - is it being viewed, optimize render intervals
// and more. Note that, if one of Width or Height (but not both) are negative, 
// then it is understood that the ad is facing away from the normal.

struct CAd3DInfo
{
	float32		Width;			// The width (in real world units) of the ad
	float32		Height;			// The height (in real world units) of the ad
	D3DXVECTOR3	Position;		// Postion (in 3d space) of the center of the ad
	D3DXVECTOR3	Normal;			// Normal (normalized form) of the vector the ad
								// is facing.
};

//CAdUserData is used to attach user-defined data in ads. You can define
//a derived class and attach it to an ad by calling CAd::SetUserData.
class CAdUserData
{
public:
	virtual ~CAdUserData();
};

// This class represents a HOT Ad, showing HTML on a surface provided by the
// developer in 3d world, that is being displayed and managed by the AdManager 
// class itself.

class CAd
{
public:

	//Destructor to be called automatically be CAdManager

	virtual ~CAd();

	// Set the surface this ad will be displayed on. This surface has to be of the format X8R8G8B8 
	// in order to be compatible with the HOT rendering logic.

#ifdef USE_DIRECTX_7
	void SetSurface ( LPDIRECTDRAWSURFACE7 pSurface );
#elif defined USE_DIRECTX_8
	void SetSurface ( LPDIRECT3DSURFACE8 pSurface );
#else
	void SetSurface ( LPDIRECT3DSURFACE9 pSurface );
#endif

	// Return the surface this ad is being displayed on.

#ifdef USE_DIRECTX_7
	LPDIRECTDRAWSURFACE7 GetSurface () const;
#elif defined USE_DIRECTX_8
	LPDIRECT3DSURFACE8 GetSurface () const;
#else
	LPDIRECT3DSURFACE9 GetSurface () const;
#endif

	// Set the ID associated with this ID. This will identify the ad for the
	// ad server (if online) or for local html page if offline. In any case,
	// the offline URL - the AdId plus a ".html" extension - must exist in the 
	// LocalHTML directory under the current working directory, or a
	// std::runtime_error exception will be thrown.

	void SetID( const std::string& ID );

	// Get the ID associated with this ID. This ID identifies the ad for the
	// ad server (if online) or for local html page if offline

	std::string const& GetID() const;

	// Set the custom params associated with this ID. These params have to be
	// in the form of name-value pairs concatinated with & (URL query string
	// style, e.g. "User=1&Score=203".
	// These parameters will be attached to the url calculated from the ID,
	// and product code name and DP code name from the ad manager to form the 
	// actual URL used to retrieve the AD.

	void SetCustomParams ( const std::string& CustomParams = "" );

	// Get the custom params associated with this ID.

	std::string const& GetCustomParams() const;

	// Get the current URL that the ad is currently using. This may change 
	// depending on which URL's are found to be available.

	std::string GetCurrentURL() const;

	// Return the actual online URL associated with this ad - it is based on the
	// ProductCodeName, DPCodeName, and AdID associated with the ad

	std::string GetOnlineURL() const;

	// Returns the actual offline (backup) URL associated with this ad. It is
	// based on the ProductCodeName, DPCodeName, and AdID associated with the ad.

	std::string GetOfflineURL() const;

	// Set the width and height for this ad.

	void SetSize ( uint32 Width , uint32 Height );

	// Set the width in pixels of the ad.

	void SetWidth ( uint32 Width );

	// Set the height in pixels of the ad.

	void SetHeight ( uint32 Height );

	// Get the width in pixels of the ad.

	uint32 GetWidth () const;
	
	// Get the height in pixels of the ad.

	uint32 GetHeight () const;

	// Set the destination rect the ad will be drawn into. The ad will be
	// rendered into this rectangular area on the surface its attached
	// to. Pass NULL to indicate that the ad is to be drawn without any 
	// scaling using its original width and height.

	void SetDestRect ( const RECT* DestRect = NULL );

	// Get the destination rect the ad will be drawn into.
	// See SetDestRect for the value. if no rectange is stored, the function
	// will return false, otherwise true will be returned into the DestRect
	// parameter.

	bool GetDestRect ( RECT* const DestRect ) const;

	// Set the ad policy for updating (copying of the html rendered page from
	// the background store to the front buffer.

	void SetUpdatePolicy ( EAdPolicy::AdPolicy Policy );

	// Returns the policy for updaing the ad (copying of the html rendered page
	// from the background store to the front buffer.

	EAdPolicy::AdPolicy GetUpdatePolicy() const;

	// Set the ad policy for rendering (copying of the html rendered page from
	// the front buffer to the actual surface provided by the user.

	void SetRenderPolicy ( EAdPolicy::AdPolicy Policy );

	// Return the policy for rendering the ad (copying of the html rendered
	// page from the front buffer to the actual surface provided by the user.

	EAdPolicy::AdPolicy GetRenderPolicy() const;

	// Set the update interval for the ad. If the ad is using
	// AdPolicy::Interval, this value is used to control how often to update
	// the ad. This value is in seconds.

	void SetUpdateInterval ( float32 Interval );

	// Get the update interval for the ad. If the ad is using
	// AdPolicy::Interval, this value is used to control how often to update
	// the ad. This value is in seconds.

	float32 GetUpdateInterval () const;

	// Set the render interval for the ad. If the ad is using
	// AdPolicy::Interval, this value is used to control how often to render
	// the ad. This value is in seconds.

	void SetRenderInterval ( float32 Interval );

	// Get the update interval for the ad. If the ad is using
	// AdPolicy::Interval, this value is used to control how often to render
	// the ad. This value is in seconds.

	float32 GetRenderInterval () const;

	// Calling this method will cause the html data behind the ad to be
	// refreshed - in case of an online page, the page will get another hit
	// (another impression).

	void Refresh();

	// Set the 3d information related to this ad. This will define world
	// appearance for the ad, like position, size and orientation - which
	// is needed to get acturate view tracking data. Specify NULL to remove 
	// any 3d information from the ad. Note that this function will copy the 
	// structure you specify; it will not retain your pointer.

	void Set3DInfo ( const CAd3DInfo* Info = NULL );

	// Get the 3d information related to this ad.
	// This information defines world appearance for the ad, like position,
	// size and orientation - which are needed to get acturate view tracking
	// data.
	// If the ad doesn't have 3d information associated with it, this method
	// will return false, otherwise it will return true and copy the values
	// into the Info structure given.

	bool Get3DInfo ( CAd3DInfo* const Info ) const;

	// Set to true to consider this ad visible, false otherwise.

	void SetVisible ( bool Visible );

	// Return true if the ad is visible and is being updated by the ad manager
	// (rendering the HTML content onto the surface).

	bool IsVisible () const;

	// Set the time that passed since the renderer first rendered this ad into
	// its attached surface (in seconds)

	void SetTimeSinceFirstRender ( float32 TimeSinceFirstRender );

	// Get the time that passed since the renderer first rendered this ad into its
	// attached surface (in seconds).
	float32 GetTimeSinceFirstRender () const;

	// Set the time that passed since the renderer last rendered this ad into its
	// attached surface (in seconds)

	void SetTimeSinceLastRender ( float32 TimeSinceLastRender );

	// Get the time that passed since the renderer last rendered this ad into its
	// attached surface (in seconds)

	float32 GetTimeSinceLastRender () const;

	// Get the total time this ad has been actually viewed on screen.
	// This is based on all other metrics being above a certain value
	// and is useful for keeping track of how much time the user actually spend
	// looking at the ad.

	float64 GetActualTimeViewed() const;

	// Reset the total time this ad has been actually viewed on screen.
	// This is based on all other metrics being above a certain value
	// and is useful for keeping track of how much time the user actually spend
	// looking at the ad.

	void ResetActualTimeViewed();

	// Get the total time this ad has been shown so far. This counter is useful
	// for tracking and will accumulate time whenever the ad, or part of it is
	// showing on screen based on the visible flag.

	float64 GetTotalTimeShown() const;

	// Reset the tracked total time this ad has been shown so far.

	void ResetTotalTimeShown();

	// Get how many times this ad has been viewed - displayed or refreshed -
	// number of impressions.

	uint32 GetViewCount() const;

	// Reset how many times this ad has been viewed - displayed or refreshed -
	// number of impressions.

	void ResetViewCount();

	// Get a value corresponding to the given metric. This value represents
	// a certain ad metric and will be in the range of 0.0f to 1.0f. The value
	// will be valid only after the ad has been rendered, as it requires
	// inspecting the device for certain properties, like view matrix, etc.
	// See EAdMetric::AdMetric for the values that are available to get.

	float32 GetMetric( EAdMetric::AdMetric Metric ) const;

	// Get the number of time this ad has been actually rendered into a
	// surface. You can use the ResetRenderCount to reset this count 
	// back to zero.

	uint32 GetRenderCount() const;

	// Reset the render counter back to zero.

	void ResetRenderCount();

	// Returns true when the navigation to the ad is complete.

	bool IsComplete() const;

	// Returns true if navigation to the ad ended up with an error (like 404 - page not found).

	bool IsError() const;

	///Turn on transparency processing (using the color key) on the rendering.
	///In order for this to work, the rendering has to be done into an ARGB surface
	///and alpha blending has to be turned on as well.
	void SetTransparent ( bool Transparent );

	///Return true if transparency is turned on.
	bool GetTransparent () const;

	///Set the color key for the transparency processing. Use the form 0xffrrggbb.
	void SetColorKey ( ULONG ColorKey );

	///Get the color key used for the transparency processing.
	ULONG GetColorKey () const;

	///Set whether the ad is static (i.e. unchanging).
	void SetStatic( bool Static );

	///Check whether the ad is static (i.e. unchanging).
	bool GetStatic() const;

	//Set the user data for an ad.
	void SetUserData( CAdUserData* pData );

	//Get the user data for an ad.
	CAdUserData* GetUserData();

	//Get the user data for an ad.
	CAdUserData* GetUserData() const;

private:

	CAd(CAdManager* pManager);
	CAdImpl*					m_pImpl;

	friend CAdManager;
	friend CAdManagerImpl;
};

} //namespace HOT
} //namespace WT
