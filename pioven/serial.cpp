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

	NTSTATUS status = ZwCreateFile(hComPort, GENERIC_ALL | SYNCHRONIZE, &objAttr, &ioStatusBlock, 
		NULL, FILE_ATTRIBUTE_DEVICE, 0, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

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

NTSTATUS	SendSerialCommand(HANDLE hComPort, CHAR cmd, CHAR *response, ULONG responseSize)
{
	IO_STATUS_BLOCK ioStatusBlock;
	LARGE_INTEGER offset;
	offset.QuadPart = 0;

	NTSTATUS status = ZwWriteFile(hComPort, NULL, NULL, NULL, &ioStatusBlock, 
		&cmd, sizeof(CHAR), &offset, NULL);

	if (NT_SUCCESS(status) == FALSE)
	{
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Error writing to com port - 0x%08x\n", status);
	}
	else
	{
		if (responseSize != 0 && response != NULL)
		{
			*response = 0;
			ULONG totalReceived = 0;

			while (response[totalReceived] != '\n')
			{
				status = ZwReadFile(hComPort, NULL, NULL, NULL, &ioStatusBlock, &response[totalReceived],
					responseSize - totalReceived, &offset, NULL);

				if (NT_SUCCESS(status) == FALSE)
				{
					DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Error reading from com port - 0x%08x\n", status);
					break;
				}
				else
				{
					totalReceived += (ULONG) ioStatusBlock.Information;
				}
			}
		}
	}

	return status;
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

