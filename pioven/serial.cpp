/*
 * serial.cpp
 *
 * This file contains the main code for the pi-oven driver
 *
 * 15th July 2015
 */

/* ************************************************************************* */

#include <ntddk.h>
#include <ntddser.h>

#include "pioven.h"
#include "pioven-driver.h"

/* ************************************************************************* */

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

NTSTATUS DoSerialPortOutIOCTL(HANDLE hComPort, ULONG ioctl, PVOID outBuffer, ULONG outBufferSize)
{
	IO_STATUS_BLOCK ioStatusBlock;

	NTSTATUS status = ZwDeviceIoControlFile(hComPort, NULL, NULL, NULL, &ioStatusBlock, ioctl, NULL, 0, outBuffer, outBufferSize);

	return status;
}

/* ************************************************************************* */

NTSTATUS DoSerialPortIOCTL(HANDLE hComPort, ULONG ioctl, PVOID inBuffer, ULONG inBufferSize)
{
	IO_STATUS_BLOCK ioStatusBlock;

	NTSTATUS status = ZwDeviceIoControlFile(hComPort, NULL, NULL, NULL, &ioStatusBlock, ioctl, inBuffer, inBufferSize, NULL, 0);

	return status;
}

/* ************************************************************************* */

NTSTATUS	ConfigureSerialPort(HANDLE hComPort)
{
	//#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
	//    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
	//)
	//	ULONG foo = IOCTL_SERIAL_SET_BAUD_RATE;
	//	// 9600
	//
	//	foo = IOCTL_SERIAL_SET_LINE_CONTROL;
	//	// 00 00 08
	//
	//	foo = IOCTL_SERIAL_SET_CHARS;
	//	// 00 00 00 00 11 13
	//
	//	foo = IOCTL_SERIAL_SET_HANDFLOW;
	//	// 01 00 00 00  43 00 00 00  00 08 00 00  00 02 00 00
	//
	//	foo = IOCTL_SERIAL_SET_TIMEOUTS;
	//	// 01 00 .. 00

	// these values were taken from running PUTTY and capturing
	// NtDeviceIoControlFile with API Monitor
	SERIAL_BAUD_RATE baudRate;
	SERIAL_LINE_CONTROL	lineControl;
	SERIAL_CHARS	chars;
	SERIAL_HANDFLOW handFlow;

	DoSerialPortOutIOCTL(hComPort, IOCTL_SERIAL_GET_BAUD_RATE, &baudRate, sizeof(SERIAL_BAUD_RATE));
	DoSerialPortOutIOCTL(hComPort, IOCTL_SERIAL_GET_LINE_CONTROL, &lineControl, sizeof(SERIAL_LINE_CONTROL));
	DoSerialPortOutIOCTL(hComPort, IOCTL_SERIAL_GET_CHARS, &chars, sizeof(SERIAL_CHARS));
	DoSerialPortOutIOCTL(hComPort, IOCTL_SERIAL_GET_HANDFLOW, &handFlow, sizeof(SERIAL_HANDFLOW));

	DoSerialPortOutIOCTL(hComPort, IOCTL_SERIAL_GET_BAUD_RATE, &baudRate, sizeof(SERIAL_BAUD_RATE));
	DoSerialPortOutIOCTL(hComPort, IOCTL_SERIAL_GET_LINE_CONTROL, &lineControl, sizeof(SERIAL_LINE_CONTROL));
	DoSerialPortOutIOCTL(hComPort, IOCTL_SERIAL_GET_CHARS, &chars, sizeof(SERIAL_CHARS));
	DoSerialPortOutIOCTL(hComPort, IOCTL_SERIAL_GET_HANDFLOW, &handFlow, sizeof(SERIAL_HANDFLOW));

	baudRate.BaudRate = 9600;
	DoSerialPortIOCTL(hComPort, IOCTL_SERIAL_SET_BAUD_RATE, &baudRate, sizeof(SERIAL_BAUD_RATE));

	DoSerialPortIOCTL(hComPort, IOCTL_SERIAL_SET_RTS, NULL, 0);
	DoSerialPortIOCTL(hComPort, IOCTL_SERIAL_SET_DTR, NULL, 0);

	lineControl.Parity = 0;
	lineControl.StopBits = 0;
	lineControl.WordLength = 8;
	DoSerialPortIOCTL(hComPort, IOCTL_SERIAL_SET_LINE_CONTROL, &lineControl, sizeof(SERIAL_LINE_CONTROL));

	chars.EofChar = 0;
	chars.ErrorChar = 0;
	chars.BreakChar = 0;
	chars.EventChar = 0;
	chars.XonChar = 0x11;
	chars.XoffChar = 0x13;
	DoSerialPortIOCTL(hComPort, IOCTL_SERIAL_SET_CHARS, &chars, sizeof(SERIAL_CHARS));

	handFlow.ControlHandShake = SERIAL_DTR_CONTROL;
	handFlow.FlowReplace = SERIAL_AUTO_TRANSMIT | SERIAL_AUTO_RECEIVE | SERIAL_RTS_CONTROL;
	handFlow.XonLimit = 0x800;
	handFlow.XoffLimit = 0x200;

	SERIAL_TIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = 2000;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	DoSerialPortIOCTL(hComPort, IOCTL_SERIAL_SET_TIMEOUTS, &timeouts, sizeof(SERIAL_TIMEOUTS));

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

			while (responseSize > 0)
			{
				status = ZwReadFile(hComPort, NULL, NULL, NULL, &ioStatusBlock, &response[totalReceived],
					1, &offset, NULL);

				if (NT_SUCCESS(status) == FALSE)
				{
					DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Error reading from com port - 0x%08x\n", status);
					break;
				}
				else
				{
					totalReceived += (ULONG) ioStatusBlock.Information;
					responseSize -= (ULONG)ioStatusBlock.Information;

					if (response[totalReceived - 1] == '\n')
					{
						break;
					}
				}
			}

			response[totalReceived - 1] = 0;
		}
	}

	return status;
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

