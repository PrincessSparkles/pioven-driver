// piovenapi.cpp : Defines the exported functions for the DLL application.
//

/* ************************************************************************* */

#include "stdafx.h"
#include "piovenapi.h"

#include <pioven.h>

/* ************************************************************************* */

/* ************************************************************************* */
/* ************************************************************************* */

PIOVENAPI_API HANDLE OpenOven()
{
	// GUID specifies a device interface (rather than a setup class) and
	// we only want those devices that are plugged in
	DWORD flags = DIGCF_DEVICEINTERFACE | DIGCF_PRESENT;
	HDEVINFO hDevInfo = SetupDiGetClassDevs(&PIOVEN_GUID, NULL, NULL, flags);

	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		return INVALID_HANDLE_VALUE;
	}

	SP_DEVINFO_DATA devInfoData;
	devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	if (SetupDiEnumDeviceInfo(hDevInfo, 0, &devInfoData) == FALSE)
	{
		return INVALID_HANDLE_VALUE;
	}

	SP_DEVICE_INTERFACE_DATA devInterfaceData;
	devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	if (SetupDiEnumDeviceInterfaces(hDevInfo, &devInfoData, &PIOVEN_GUID, 0, &devInterfaceData) == FALSE)
	{
		return INVALID_HANDLE_VALUE;
	}

	DWORD required;
	unsigned char devInterfaceDetailBuffer[1024];
	PSP_DEVICE_INTERFACE_DETAIL_DATA devInterfaceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)devInterfaceDetailBuffer;
	devInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInterfaceData, devInterfaceDetail, sizeof(devInterfaceDetailBuffer), &required, NULL) == FALSE)
	{
		return INVALID_HANDLE_VALUE;
	}

	return CreateFile(devInterfaceDetail->DevicePath, GENERIC_ALL, 0, NULL, OPEN_EXISTING, 0, NULL);
}

/* ************************************************************************* */

PIOVENAPI_API void CloseOven(HANDLE hOven)
{
	CloseHandle(hOven);
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
