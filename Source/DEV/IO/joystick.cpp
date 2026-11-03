/*
	joystick.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

*/
#define STRICT

#include <windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <D3D8.h>
#include <dinput.h>

#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "joystick.h"

CJoystick::CJoystick( HWND hWnd ) :				// this windows hWnd
					  m_hWnd( hWnd ),
					  m_pDirectInput( NULL ),
					  m_pJoystick( NULL ),
					  m_HasJoystick( kFalse ),
					  m_HasDualAxis( kFalse ),
					  m_UseDualAxis( kFalse ),
					  m_IsWheel( kFalse ),
					  m_HasFF( kFalse ),
					  m_UseFF( kFalse ),
					  m_Effects( 0 )
					

{
	// clear out button states
	for ( uint32 i = 0; i < 128; i++ )
	{
		m_ButtonsPressed[i] = kFalse;
		m_ButtonsHeld[i] = kFalse;
	}

	// clear out axis positions
	for ( i = 0; i < KJoystickAxes; i++ )
	{
		m_AxisPosition[i] = 0;
		m_LastAxisPosition[i] = 0;
	}



	// try to initialize our DInput devices
	if ( FAILED( InitDirectInput( hWnd ) ) )
	{
		m_HasJoystick = kFalse;
	}
	else
	{
		m_HasJoystick = kTrue;
	}

	// no jostick, we sure don't have force feedback
	if ( !m_HasJoystick )
	{
		m_HasFF = kFalse;
	}
	// automatically use dual axis if available
	if ( m_HasDualAxis )
	{
		m_UseDualAxis = kTrue;
	}
	
} // CJoystick::CJoystick()

CJoystick::~CJoystick( void )
{
	// release all the dinput devices
	FreeDirectInput();
} // CJoystick::~CJoystick())

// release any direct input objects
void CJoystick::FreeDirectInput( void )
{
	m_HasJoystick = kFalse;
	m_HasFF = kFalse;
	// Unacquire the device one last time just in case 
	// the app tried to exit while the device is still acquired.
	EmptyEffectList();

	if ( m_pJoystick )
	{
		m_pJoystick->SetCooperativeLevel( m_hWnd, DISCL_NONEXCLUSIVE );
		m_pJoystick->Unacquire();
		Sleep( 100 );
	}
	m_hWnd = NULL;

	// Release any DirectInput objects.
	RELEASE_SAFELY( m_pJoystick );
	Sleep( 100 );
	RELEASE_SAFELY( m_pDirectInput );
} // CJoystick::FreeDirectInput()

// remove any exisiting effects from the list
void CJoystick::EmptyEffectList( void )
{
	for ( uint32 i = 0; i < m_Effects; i++ )
	{
		RELEASE_SAFELY( m_pEffects[i]->pDIEffect );
		DELETE_SAFELY( m_pEffects[i] );
	}
	m_Effects = 0;
} // CJoystick::EmptyEffectList()

// reaquire our joystickwhen the window is re-focused
void CJoystick::Focus( void )
{
	if ( m_HasJoystick )
	{
		m_pJoystick->Acquire();
	}
} // CJoystick::Focus()

// unaquire our joystick when the window is de-focused
void CJoystick::UnFocus()
{
	if ( m_HasJoystick )
	{
		m_pJoystick->Unacquire();
	}
} // CJoystick::UnFocus()

HRESULT CJoystick::InitDirectInput( HWND hWnd )	// hwnd of the window
{
	HRESULT	hr;

	// Register with the DirectInput subsystem and get a pointer
	// to a IDirectInput interface we can use.
	
	// Create a DInput object
	if ( FAILED( hr = DirectInput8Create( GetModuleHandle( NULL ),
										  DIRECTINPUT_VERSION,
										  IID_IDirectInput8,
										  ( VOID * * ) &m_pDirectInput,
										  NULL ) ) )
	{
		// failed - no joysticks available
		return hr;
	}

	// now that we have a device, try to enumerate force feedback joysticks
	if ( FAILED( hr = m_pDirectInput->EnumDevices( DI8DEVCLASS_GAMECTRL,
										  EnumerateJoysticksCallback,
										  this,
										  DIEDFL_ATTACHEDONLY |
										  DIEDFL_FORCEFEEDBACK ) ) || m_pJoystick == NULL )
	{
		// since we failed,we KNOW we don't have force feedback
		m_HasFF = kFalse;
		// so try to enumerate joysticks without it
		if ( FAILED( hr = m_pDirectInput->EnumDevices( DI8DEVCLASS_GAMECTRL,
											  EnumerateJoysticksCallback,
											  this,
											  DIEDFL_ATTACHEDONLY ) ) )
		{
			// nope, no joysticks at all
			return hr;
		}
	}
	else	// if we succeeded in the first enumeration, we know we have FF
	{
		m_HasFF = kTrue;
	}

	// Make sure we got a joystick, and didn't end up with a null pointer
	if ( NULL == m_pJoystick )
	{
		return E_FAIL;
	}

	// Set the data format to "simple joystick" - a predefined data format 
	//
	// A data format specifies which controls on a device we are interested in,
	// and how they should be reported. This tells DInput that we will be
	// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
	if ( FAILED( hr = m_pJoystick->SetDataFormat( &c_dfDIJoystick2 ) ) )
	{
		return hr;
	}

	// Set the cooperative level to let DInput know how this device should
	// interact with the system and with other DInput applications.
	// we want exclusive control when the window is in the foreground
	if ( FAILED( hr = m_pJoystick->SetCooperativeLevel( hWnd,
														DISCL_EXCLUSIVE |
														DISCL_FOREGROUND ) ) )
	{
		return hr;
	}

	// Enumerate the joystick objects - axes, buttons, throttles
	if ( FAILED( hr = m_pJoystick->EnumObjects( EnumerateObjectsCallback,
												reinterpret_cast<void*>( this ),
												DIDFT_ALL ) ) )
	{
		return hr;
	}
	
	// all went well - we have a joystick!
	return S_OK;
} // CJoystick::InitDirectInput()

// enumerate all the joysticks and pick one.
// right now, we're just picking the first valid one we come to, but if you need to allow
// the user to pick from multiple joysticks some additions will have to be made
BOOL CJoystick::EnumerateJoysticksCallback( LPCDIDEVICEINSTANCE pDIDeviceInstance,	// directinput device 
											LPVOID pClassPointer )								// pointer to the parent class
{
	HRESULT	hr;
	CJoystick*	pThis	= reinterpret_cast<CJoystick*>( pClassPointer );

	// Obtain an interface to the enumerated joystick.
	hr = pThis->m_pDirectInput->CreateDevice( pDIDeviceInstance->guidInstance,
											  &pThis->m_pJoystick,
											  NULL );

	// If it failed, then we can't use this joystick. (Maybe the user unplugged
	// it while we were in the middle of enumerating it.)
	if ( FAILED( hr ) )
	{
		pThis = NULL;
		return DIENUM_CONTINUE;	// we want to keep enumerating though-
	}

	// it's a wheel!
	if ( GET_DIDEVICE_TYPE( pDIDeviceInstance->dwDevType ) ==
		 DI8DEVTYPE_DRIVING )
	{
		pThis->m_IsWheel = kTrue;
		// does it have dual axis pedals?
		if ( GET_DIDEVICE_SUBTYPE( pDIDeviceInstance->dwDevType ) ==
			 DI8DEVTYPEDRIVING_DUALPEDALS )
		{
			pThis->m_HasDualAxis = kTrue;
		}
		// three axis counts too
		if ( GET_DIDEVICE_SUBTYPE( pDIDeviceInstance->dwDevType ) ==
			 DI8DEVTYPEDRIVING_THREEPEDALS )
		{
			pThis->m_HasDualAxis = kTrue;
		}
	}

	// Stop enumeration. Note: we're just taking the first joystick we get. You
	// could store all the enumerated joysticks and let the user pick.
	pThis = NULL;

	return DIENUM_STOP;
} // EnumJoysticksCallback()

// enumerate objects on the joystick (axes, buttons, throttle )
BOOL CJoystick::EnumerateObjectsCallback( LPCDIDEVICEOBJECTINSTANCE pDIDeviceObjectInstance,	// directinput device object
										  LPVOID pClassPointer )								// pointer to the parent class
{
	CJoystick*		pThis		= reinterpret_cast<CJoystick*>( pClassPointer );
	static int32	SliderCount	= 0;	// Number of returned slider controls
	static int32	POVCount		= 0;	 // Number of returned POV controls

	// For axes that are returned, set the DIPROP_RANGE property for the
	// enumerated axis in order to scale min/max values. we range betwee -1000 and 1000
	if ( pDIDeviceObjectInstance->dwType & DIDFT_AXIS )
	{
		DIPROPRANGE	PropertyRange; 
		PropertyRange.diph.dwSize		= sizeof( DIPROPRANGE ); 
		PropertyRange.diph.dwHeaderSize = sizeof( DIPROPHEADER ); 
		PropertyRange.diph.dwHow		= DIPH_BYID; 
		PropertyRange.diph.dwObj		= pDIDeviceObjectInstance->dwType; // Specify the enumerated axis
		PropertyRange.lMin				= -1000; 
		PropertyRange.lMax				= +1000; 

		// Set the range for the axis
		if ( FAILED( pThis->m_pJoystick->SetProperty( DIPROP_RANGE,
													  &PropertyRange.diph ) ) )
		{
			pThis = NULL;
			return DIENUM_STOP;
		}


		DIPROPDWORD	Properties; 
		Properties.diph.dwSize			= sizeof( DIPROPDWORD ); 
		Properties.diph.dwHeaderSize	= sizeof( DIPROPHEADER ); 
		Properties.diph.dwObj			= DIJOFS_X; 
		Properties.diph.dwHow			= DIPH_BYOFFSET; 
		Properties.dwData				= 100; 


		// if we couldn't set the properties, we're done and we failed
		if ( FAILED( pThis->m_pJoystick->SetProperty( DIPROP_DEADZONE,
													  &Properties.diph ) ) )
		{
			pThis = NULL;
			return DIENUM_STOP;
		}
	}

	pThis = NULL;
	return DIENUM_CONTINUE;	// keep on enumerating
} // CJoystick::EnumObjectsCallback()

// load an FFE file and store the effects to play them later
HRESULT CJoystick::LoadFFEffects( std::string FileName )	// FileName to force feedback effects file (.ffe)
{
	if( !m_HasFF )
	{
		return E_FAIL;
	}

	HRESULT	hr;

	// clear out the bank of effects we may currently have loaded
	EmptyEffectList();

	// Enumerate the effects in the file selected, and create them in the callback
	if ( FAILED( hr = m_pJoystick->EnumEffectsInFile( FileName.c_str(),
													  EnumerateAndCreateEffectsCallback,
													  this,
													  DIFEF_MODIFYIFNEEDED ) ) )
	{
		return hr;
	}
	m_UseFF = kTrue;

	return S_OK;
} // CJoystick::LoadFFEffects()


BOOL CJoystick::EnumerateAndCreateEffectsCallback( LPCDIFILEEFFECT pDIFileEffect,	// pointer to dinput file effect
												   LPVOID pClassPointer )			// pointer to the parent class
{
	HRESULT				hr;
	LPDIRECTINPUTEFFECT	pDIEffect	= NULL;

	CJoystick*				pThis	= reinterpret_cast<CJoystick*>( pClassPointer );


	// Create the file Effect
	if ( FAILED( hr = pThis->m_pJoystick->CreateEffect( pDIFileEffect->GuidEffect,
														   pDIFileEffect->lpDiEffect,
														   &pDIEffect,
														   NULL ) ) )
	{
		pThis = NULL;
		return DIENUM_CONTINUE;
	}

	// if we have over 99 effects, just stop
	if ( pThis->m_Effects >= 99 )
	{
		pThis = NULL;
		return DIENUM_STOP;
	}

	// Create a new Effect node
	pThis->m_pEffects[pThis->m_Effects] = new CEffect();
	if ( NULL == pThis->m_pEffects[pThis->m_Effects] )
	{
		pThis = NULL;
		return DIENUM_STOP;
	}

	// Fill the pEffectNode up
	ZeroMemory( pThis->m_pEffects[pThis->m_Effects],
				sizeof( CEffect ) );
	pThis->m_pEffects[pThis->m_Effects]->pDIEffect			= pDIEffect;


	pThis->m_Effects++;

	pThis = NULL;

	return DIENUM_CONTINUE;
} // CJoystick::EnumerateAndCreateEffectsCallback()


// update the current state of the axes and buttons
void CJoystick::Update( void )
{
	if ( !m_HasJoystick )
	{
	return;
	}

	HRESULT		hr;
	DIJOYSTATE2	JoystickState; 		  // DInput joystick state 


	// Poll the device to read the current state
	hr = m_pJoystick->Poll(); 
	if ( FAILED( hr ) )
	{
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.
		hr = m_pJoystick->Acquire();
		while ( hr == DIERR_INPUTLOST )
		{
			hr = m_pJoystick->Acquire();
		}

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 
		return;
	}

	// Get the input's device state
	if ( FAILED( hr = m_pJoystick->GetDeviceState( sizeof( DIJOYSTATE2 ), &JoystickState ) ) )
	{
		return;
	} // The device should have been acquired during the Poll()

	// store our last axis position
	for( uint32 i = 0; i < KJoystickAxes; i ++ )
	{
		m_LastAxisPosition[i] = m_AxisPosition[i];
	}
	// all are divided by 1000 to give us a range of -1.0f to 1.0f
	m_AxisPosition[KXAxis] = JoystickState.lX / 1000.0f;
	m_AxisPosition[KYAxis] = JoystickState.lY / 1000.0f;
	m_AxisPosition[KZAxis] = JoystickState.lZ / 1000.0f;

	m_AxisPosition[KXAxisRotation] = JoystickState.lRx / 1000.0f;
	m_AxisPosition[KYAxisRotation] = JoystickState.lRy / 1000.0f;
	m_AxisPosition[KZAxisRotation] = JoystickState.lRz / 1000.0f;

	m_AxisPosition[KThrottle1] = JoystickState.rglSlider[0] / 1000.0f;
	m_AxisPosition[KThrottle2] = JoystickState.rglSlider[1] / 1000.0f;

	// track the button states
	for ( i = 0; i < 128; i++ )
	{
		if ( JoystickState.rgbButtons[i] & 0x80 )
		{
			if ( !m_ButtonsHeld[i] )
			{
				m_ButtonsPressed[i] = kTrue;
			}
			else
			{
				m_ButtonsPressed[i] = kFalse;
			}
			m_ButtonsHeld[i] = kTrue;
		}
		else
		{
			m_ButtonsHeld[i] = kFalse;
			m_ButtonsPressed[i] = kFalse;
		}
	}

	return;
} // CJoystick::Update()

// get the position of a given axis
float32 CJoystick::GetAxis( uint32 Axis )	// axis to retrieve the value for
{
	if( Axis >= KJoystickAxes )
	{
		return 0;
	}

	return m_AxisPosition[Axis];
}

// get the change in a given axis' position since its last event
float32 CJoystick::GetAxisDelta( uint32 Axis )	// axis to retrieve the value for
{
	if( Axis >= KJoystickAxes )
	{
		return 0;
	}

	return m_AxisPosition[Axis] - m_LastAxisPosition[Axis];
}
// check to see if a specific button is being held down
bool CJoystick::GetButtonHeld( uint32 Button )		// button to get the state of
{
	if ( Button > 127 )
	{
		return kFalse;
	}
	return m_ButtonsHeld[Button];
} // CJoystick::GetButtonHeld()

// check to see if a specific button has just been pressed
bool CJoystick::GetButtonPressed( uint32 Button )	// button to get the state of
{
	if ( Button> 127 )
	{
		return kFalse;
	}
	return m_ButtonsPressed[Button];
} // CJoystick::GetButtonPressed()


// start a force-feedback effect
void CJoystick::PlayEffect( uint32 Effect,	// index of effect to play
							DWORD Repeat )	// number of times to repeat it - INFINITE (a dinput constant) for infinite
{
	if ( m_HasJoystick && m_HasFF && m_UseFF && Effect < m_Effects )
	{
		m_pEffects[Effect]->pDIEffect->Start( Repeat, 0 );
	}
} // CJoystick::PlayEffect()

// stop a specific effect
void CJoystick::StopEffect( uint32 Effect )	// index of effect to stop
{
	// if we actually have a joystick and FF support, and we're using it,
	// and the effect is in range of the ones we loaded, set its gain
	if ( m_HasJoystick && m_HasFF && m_UseFF && Effect < m_Effects )
	{
		m_pEffects[Effect]->pDIEffect->Stop();
	}
} // CJoystick::StopEffect()

// set the gain of an Effect
void CJoystick::SetEffectGain( uint32 Effect,	// index of the Effect
							   DWORD Gain )		// gain of the effect
{
	// if we actually have a joystick and FF support, and we're using it,
	// and the effect is in range of the ones we loaded, set its gain
	if ( m_HasJoystick && m_HasFF && m_UseFF && Effect < m_Effects )
	{
		DIEFFECT	SelectedEffect	=
		{
			sizeof( DIEFFECT )
		};
		SelectedEffect.dwGain = Gain;
		m_pEffects[Effect]->pDIEffect->SetParameters( &SelectedEffect, DIEP_GAIN );
	}
} // CJoystick::SetEffectGain()

// halt all currently playing effects
void CJoystick::StopAllEffects( void )
{
	// if we have a joystick and are using FF< stop all effects
	if ( m_HasJoystick && m_HasFF && m_UseFF )
	{
		for ( uint32 i = 0; i < m_Effects; i++ )
		{
			m_pEffects[i]->pDIEffect->Stop();
		}
	}
} // CJoystick::StopAllEffects()
