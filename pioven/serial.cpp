/*
 * serial.cpp
 *
 * This file contains the main code for the pi-oven driver
 *
 * 15th July 2015
 */

/* ************************************************************************* */

#include <ntddk.h>

#include "pioven.h"
#include "pioven-driver.h"

/* ************************************************************************* */

/* ************************************************************************* */
/* ************************************************************************* */

NTSTATUS	ConfigureSerialPort(HANDLE /*hComPort*/)
{
	return STATUS_SUCCESS;
}

/* ************************************************************************* */

NTSTATUS	OpenSerialPort(PCWSTR comPort, PHANDLE hComPort)
{
	UNICODE_STRING comPortName;
	RtlInitUnicodeString(&comPortName, comPort);

	OBJECT_ATTRIBUTES objAttr;
	InitializeObjectAttributes(&objAttr, &comPortName, OBJ_KERNEL_HANDLE, NULL, NULL);

	IO_STATUS_BLOCK ioStatusBlock;

	NTSTATUS status = ZwCreateFile(hComPort, GENERIC_ALL, &objAttr, &ioStatusBlock, 
		NULL, FILE_ATTRIBUTE_DEVICE, 0, FILE_OPEN, 0, NULL, 0);

	if (NT_SUCCESS(status) == FALSE)
	{
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Failed to open %wZ - 0x%08x\n", comPortName, status);
	}
	else
	{
		status = ConfigureSerialPort(*hComPort);

		if (NT_SUCCESS(status) == FALSE)
		{
			ZwClose(*hComPort);
		}
	}

	return status;
}

/* ************************************************************************* */

NTSTATUS	CloseSerialPort(HANDLE hComPort)
{
	ZwClose(hComPort);

	return STATUS_SUCCESS;
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

