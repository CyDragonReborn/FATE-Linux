#include "stdafx.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	(hInstance);
	(hPrevInstance);
	(lpCmdLine);
	(nCmdShow);

	try
	{
		CRuntimeAttribute RuntimeAttributes;

		TCHAR SKUNumber[_MAX_PATH];
		_tcscpy( SKUNumber, _T("\0") );

		TCHAR TempString[_MAX_PATH];
		_tcscpy( TempString, _T("\0") );


		RuntimeAttributes.GetSKU ( SKUNumber, _MAX_PATH );
		::MessageBox ( 0, SKUNumber, _T("SKUNumber"), 0 );

		RuntimeAttributes.GetString ( _T("ProductDisplayName"), TempString, _MAX_PATH );
		::MessageBox ( 0, TempString, _T("ProductDisplayName"), 0 );

		int testInt = RuntimeAttributes.GetInt ( _T("TestInt") );
		_stprintf ( TempString, _T("Test Int: %d"), testInt );
		::MessageBox ( 0, TempString, _T("TestInt"), 0 );

		float testFloat = RuntimeAttributes.GetFloat ( _T("TestFloat") );
		_stprintf ( TempString, _T("Test Float: %f"), testFloat );
		::MessageBox ( 0, TempString, _T("TestFloat"), 0 );


		if ( RuntimeAttributes.GetBool( _T("TestBoolTrue") ) == true) 
		{
			::MessageBox ( 0, _T("TestBoolTrue is true"), _T("TestBoolTrue"), 0 );
		}
		else
		{
			::MessageBox ( 0, _T("TestBoolTrue is false"), _T("TestBoolTrue"), 0 );
		}

		if ( RuntimeAttributes.GetBool( _T("TestBoolFalse") ) == true) 
		{
			::MessageBox ( 0, _T("TestBoolFalse is true"), _T("TestBoolFalse"), 0 );
		}
		else
		{
			::MessageBox ( 0, _T("TestBoolFalse is false"), _T("TestBoolFalse"), 0 );
		}
	}
	catch ( const TCHAR * Exception )
	{
		::MessageBox ( 0, Exception, _T("Exception"), MB_ICONERROR );
	}

	return 0;
}