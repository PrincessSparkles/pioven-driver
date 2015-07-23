// ComTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

BOOL quit = FALSE;

int _tmain(int argc, _TCHAR* argv[])
{
	printf("v0.0.1\n");
	HANDLE	hCom = CreateFile(L"\\\\.\\COM3", MAXIMUM_ALLOWED, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hCom == INVALID_HANDLE_VALUE)
	{
		printf("Open Error: 0x%08x\n", GetLastError());
		return 1;
	}

	DCB	dcb;
	GetCommState(hCom, &dcb);

	// Assign user parameter.
	dcb.BaudRate = 9600;  // Specify buad rate of communicaiton.
	dcb.StopBits = 0;  // Specify stopbit of communication.
	dcb.Parity = 0;      // Specify parity of communication.
	dcb.ByteSize = 8;  // Specify  byte of size of communication.

	SetCommState(hCom, &dcb);

	// instance an object of COMMTIMEOUTS.
	COMMTIMEOUTS comTimeOut;
	// Specify time-out between charactor for receiving.
	comTimeOut.ReadIntervalTimeout = 0;
	// Specify value that is multiplied 
	// by the requested number of bytes to be read. 
	comTimeOut.ReadTotalTimeoutMultiplier = 0;
	// Specify value is added to the product of the 
	// ReadTotalTimeoutMultiplier member
	comTimeOut.ReadTotalTimeoutConstant = 0;
	// Specify value that is multiplied 
	// by the requested number of bytes to be sent. 
	comTimeOut.WriteTotalTimeoutMultiplier = 0;
	// Specify value is added to the product of the 
	// WriteTotalTimeoutMultiplier member
	comTimeOut.WriteTotalTimeoutConstant = 0;
	// set the time-out parameter into device control.
	SetCommTimeouts(hCom, &comTimeOut);

	while (quit == FALSE)
	{
		char c = 0;
		DWORD num;
		if (ReadFile(hCom, &c, sizeof(char), &num, NULL) == FALSE)
		{
			printf("Read Error: 0x%08x\n", GetLastError());
			break;
		}

		if (num != 0)
		{
			printf("0x%02x %d\n", c, num);
		}
	}

	CloseHandle(hCom);

	return 0;
}

