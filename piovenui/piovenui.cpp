// piovenui.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "piovenui.h"
#include "resource.h"

#include <piovenapi.h>

HANDLE hOven = INVALID_HANDLE_VALUE;

void GetVersion(HWND hDlg)
{
	char version[32];
	if (GetOvenVersion(hOven, version, sizeof(version)))
	{
		SetDlgItemTextA(hDlg, IDC_VERSION_EDIT, version);
	}
	else
	{
		SetDlgItemTextA(hDlg, IDC_VERSION_EDIT, "Failed to get version");
	}
}

void GetTemperature(HWND hDlg)
{
	DWORD temp = GetOvenTemperature(hOven);

	if (temp != 0xffffffff)
	{
		char text[32];
		sprintf_s(text, sizeof(text), "0x%03x", temp);
		SetDlgItemTextA(hDlg, IDC_TEMPERATURE_EDIT, text);
	}
	else
	{
		SetDlgItemTextA(hDlg, IDC_TEMPERATURE_EDIT, "Failed to get temp");
	}
}

void ManageHeater(HWND hDlg)
{
	int state = SendDlgItemMessage(hDlg, IDC_HEATER, BM_GETCHECK, 0, 0);
	if (state)
	{
		SetHeaterOn(hOven);
	}
	else
	{
		SetHeaterOff(hOven);
	}
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			GetVersion(hDlg);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hDlg, IDOK);
					break;

				case IDC_TEMPERATURE_BUTTON:
					GetTemperature(hDlg);
					break;

				case IDC_HEATER:
					ManageHeater(hDlg);
					break;

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