// MyDriverApp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MyDriverApp.h"
#include <winioctl.h>
#include "../MyDriver/public.h"
int APIENTRY WinMain(IN HINSTANCE hInstance, IN HINSTANCE hPrecInstance, IN LPSTR szCmdLine, IN int nCmdShow)
{
	HANDLE hDev;

	hDev = INVALID_HANDLE_VALUE;
	do 
	{
		hDev = CreateFile(GLOBAL_MYDRIVER_WIN32_NAME,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (INVALID_HANDLE_VALUE == hDev)
		{
			MessageBox(NULL, TEXT("CreateFile failed"), L"", MB_OK | MB_ICONERROR);
			break;
		}

		char buf[1024] = { 0 };
		DWORD dwBytesRetured;
		ReadFile(hDev, buf, sizeof(buf), &dwBytesRetured, NULL);

		DeviceIoControl(hDev,
			IOCTL_MYDRIVER_START_THREAD,
			NULL,
			0,
			NULL,
			0,
			&dwBytesRetured,
			NULL);
		MessageBoxA(NULL, "", "Start Thread", MB_OK | MB_ICONINFORMATION);
		DeviceIoControl(hDev,
			IOCTL_MYDRIVER_STOP_THREAD,
			NULL,
			0,
			NULL,
			0,
			&dwBytesRetured,
			NULL);
		MessageBoxA(NULL, "", "Stop Thread", MB_OK | MB_ICONINFORMATION);


	} while (FALSE);


	if (INVALID_HANDLE_VALUE != hDev)
	{
		CloseHandle(hDev);
		hDev = INVALID_HANDLE_VALUE;
	}


	MessageBox(NULL, L"Test end!", L"", MB_OK | MB_ICONINFORMATION);
	return 0;
}