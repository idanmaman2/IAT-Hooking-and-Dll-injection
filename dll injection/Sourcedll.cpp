#include "windows.h"
#include <stdio.h>
#define DLL_EXPORT 
#include "Header.h" 
extern "C"
{
	DECLDIR void Share()
	{
		int msgboxID = MessageBoxA(NULL,
			(LPCSTR)"Hacked by IDHM\n",
			(LPCSTR)"hi i am header  ",
			MB_DEFBUTTON2);
	}
}
BOOL APIENTRY DllMain(
	HANDLE hModule, // Handle to DLL module 
	DWORD ul_reason_for_call,
	LPVOID lpReserved) // Reserved 
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// A process is loading the DLL. 
		Share();
		break;
	case DLL_THREAD_ATTACH:
		// A process is creating a new thread.
		break;
	case DLL_THREAD_DETACH:
		// A thread exits normally. 
	break; case DLL_PROCESS_DETACH:
		// A process unloads the DLL. 
		break;
	}
	return TRUE;
}