// piovenui.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "piovenui.h"

#include <piovenapi.h>

HANDLE hOven = INVALID_HANDLE_VALUE;

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hDlg, IDOK);

				default:
					break;
			}
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hOven = OpenOven();
	if (hOven == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, L"Failed to open oven", L"PiOven", MB_OK | MB_ICONERROR | MB_TOPMOST);
		return -1;
	}

	int result = DialogBox(hInstance, MAKEINTRESOURCE(IDD_PIOVENUI), NULL, DlgProc);

	CloseOven(hOven);
	return result;
}