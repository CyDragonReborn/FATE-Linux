// Patch.cpp : Defines the entry point for the application.
//
#include <windows.h>
#include <stdio.h>
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	FILE* f = fopen ("message.txt", "w");
	if (f)
	{
	fprintf (f, "Patch 1.0");
	fclose (f);
	}
	else
	{
		MessageBox (0, "Unable to write message.txt file", "Patch Error", MB_OK);
	}
	return 0;
}

