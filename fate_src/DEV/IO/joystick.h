/*
	joystick.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

*/

#ifndef _JOYSTICK_H
#define _JOYSTICK_H

#define DIRECTINPUT_VERSION 0x0800
#include <D3D8.h>
#include <dinput.h>
#include <string>
#include "../UTILITIES/constants.h"


enum EJoystickAxis
{
	KXAxis,
	KYAxis,
	KZAxis,
	KXAxisRotation,
	KYAxisRotation,
	KZAxisRotation,
	KThrottle1,
	KThrottle2,
	KJoystickAxes
};

class CEffect
{
public:
	LPDIRECTINPUTEFFECT	pDIEffect;	// directinput effect pointer
};

class CJoystick
{
public:
							CJoystick( HWND hWnd );			// this windows hWnd

							~CJoystick( void );

	void					FreeDirectInput( void );
	void					EmptyEffectList( void );

	void					Focus( void );
	void					UnFocus( void );

	HRESULT					InitDirectInput( HWND hWnd );	// hwnd of the window

	static BOOL CALLBACK	EnumerateJoysticksCallback( LPCDIDEVICEINSTANCE pDIDeviceInstance,	// directinput device 
														LPVOID pClassPointer );								// pointer to the parent class

	static BOOL CALLBACK	EnumerateObjectsCallback( LPCDIDEVICEOBJECTINSTANCE pDIDeviceObjectInstance,	// directinput device object
													  LPVOID pClassPointer );							// pointer to the parent class

	HRESULT					LoadFFEffects( std::string FileName );	// FileName to force feedback effects file (.ffe)

	static BOOL CALLBACK	EnumerateAndCreateEffectsCallback( LPCDIFILEEFFECT pDIFileEffect,	// pointer to dinput file effect
															   LPVOID pClassPointer );			// pointer to the parent class

	void					Update( void );

	float32					GetAxis( uint32 Axis );		// axis to retrieve the value for
	float32					GetAxisDelta( uint32 Axis ); // axis to retrieve the value for

	bool					GetButtonHeld( uint32 Button );		// button to get the state of
	bool					GetButtonPressed( uint32 Button );	// button to get the state of

	void					PlayEffect( uint32 Effect,	// index of effect to play
										DWORD Repeat );	// number of times to repeat it - INFINITE (a dinput constant) for infinite

	void					StopEffect( uint32 Effect );	// index of effect to stop
	void					SetEffectGain( uint32 Effect,	// index of the Effect
										   DWORD Gain );	// gain of the effect

	void					StopAllEffects( void );


	bool					HasJoystick( void )	{	return m_HasJoystick;	};

	bool					HasFF( void )		{	return m_HasFF;			};

	bool					IsWheel( void )		{	return m_IsWheel;		};

	bool					HasDualAxis( void )	{	return m_HasDualAxis;	};

private:

	HWND					m_hWnd;

	LPDIRECTINPUT8			m_pDirectInput;   	  
	LPDIRECTINPUTDEVICE8	m_pJoystick;	

	uint32					m_Effects;

	CEffect*				m_pEffects[100];

	float32					m_AxisPosition[KJoystickAxes];
	float32					m_LastAxisPosition[KJoystickAxes];

	bool					m_ButtonsHeld[128];	
	bool					m_ButtonsPressed[128];	

	bool					m_HasJoystick;

	bool					m_HasFF;
	bool					m_UseFF;

	bool					m_IsWheel;

	bool					m_HasDualAxis;
	bool					m_UseDualAxis;
};


#endif