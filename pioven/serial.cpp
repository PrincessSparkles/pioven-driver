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
#include "serial.h"

/* ************************************************************************* */

/* ************************************************************************* */
/* ************************************************************************* */

NTSTATUS	DoSerialPortIOCTL(DeviceExtension *devExt, ULONG IoControlCode, PVOID InBuffer, ULONG InBufferSize)
{
	IO_STATUS_BLOCK ioStatusBlock;
	NTSTATUS status;

	PIRP Irp = IoBuildDeviceIoControlRequest(IoControlCode, devExt->ComPortDevice, InBuffer, InBufferSize, NULL, 0, 
		FALSE, &devExt->ComPortEvent, &ioStatusBlock);

	if (Irp == NULL)
	{
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] DoSerialPortIOCTL(0x%08x) failed to allocate IRP\n", IoControlCode);
		status = STATUS_NO_MEMORY;
	}
	else
	{
		status = IoCallDriver(devExt->ComPortDevice, Irp);
		if (status == STATUS_PENDING)
		{
			KeWaitForSingleObject(&devExt->ComPortEvent, Executive, KernelMode, FALSE, NULL);
			status = ioStatusBlock.Status;
		}

		if (NT_SUCCESS(status) == FALSE)
		{
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] DoSerialPortIOCTL(0x%08x) failed - 0x%08x\n", IoControlCode, status);
		}
	}

	return status;
}

/* ************************************************************************* */

NTSTATUS	ConfigureSerialPort(DeviceExtension *devExt)
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
	baudRate.BaudRate = 9600;
	DoSerialPortIOCTL(devExt, IOCTL_SERIAL_SET_BAUD_RATE, &baudRate, sizeof(SERIAL_BAUD_RATE));

	SERIAL_LINE_CONTROL	lineControl;
	lineControl.Parity = 0;
	lineControl.StopBits = 0;
	lineControl.WordLength = 8;
	DoSerialPortIOCTL(devExt, IOCTL_SERIAL_SET_LINE_CONTROL, &lineControl, sizeof(SERIAL_LINE_CONTROL));

	SERIAL_CHARS	chars;
	chars.EofChar = 0;
	chars.ErrorChar = 0;
	chars.BreakChar = 0;
	chars.EventChar = 0;
	chars.XonChar = 0x11;
	chars.XoffChar = 0x13;
	DoSerialPortIOCTL(devExt, IOCTL_SERIAL_SET_CHARS, &chars, sizeof(SERIAL_CHARS));

	SERIAL_HANDFLOW handFlow;
	handFlow.ControlHandShake = SERIAL_DTR_CONTROL;
	handFlow.FlowReplace = SERIAL_AUTO_TRANSMIT | SERIAL_AUTO_RECEIVE | SERIAL_RTS_CONTROL;
	handFlow.XonLimit = 0x800;
	handFlow.XoffLimit = 0x200;
	DoSerialPortIOCTL(devExt, IOCTL_SERIAL_SET_HANDFLOW, &handFlow, sizeof(SERIAL_HANDFLOW));

	SERIAL_TIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = 100;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	DoSerialPortIOCTL(devExt, IOCTL_SERIAL_SET_TIMEOUTS, &timeouts, sizeof(SERIAL_TIMEOUTS));

	return STATUS_SUCCESS;
}

/* ************************************************************************* */

NTSTATUS	OpenSerialPort(PCWSTR comPort, DeviceExtension *devExt)
{
	NTSTATUS	status;
	UNICODE_STRING comPortName;
	RtlInitUnicodeString(&comPortName, comPort);

	status = IoGetDeviceObjectPointer(&comPortName, GENERIC_ALL, &devExt->ComPortFile, &devExt->ComPortDevice);

	if (NT_SUCCESS(status) == FALSE)
	{
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Failed to open serial port %wZ - 0x%08x\n", &comPortName, status);

		devExt->ComPortDevice = NULL;
		devExt->ComPortFile = NULL;
	}
	else
	{
		KeInitializeEvent(&devExt->ComPortEvent, SynchronizationEvent, FALSE);

		status = ConfigureSerialPort(devExt);

		if (NT_SUCCESS(status) == FALSE)
		{
			CloseSerialPort(devExt);

			devExt->ComPortDevice = NULL;
			devExt->ComPortFile = NULL;
		}
	}

	return status;
}

/* ************************************************************************* */

NTSTATUS	CloseSerialPort(DeviceExtension *devExt)
{
	if (devExt->ComPortDevice != NULL)
	{
		// according to the docs for IoGetDeviceObjectPointer, we have to add a reference
		// to the device object, before dereferencing the file object, because we're
		// not in the driver unload routine
		ObReferenceObject(devExt->ComPortDevice);

		// now dereference the file to close it
		ObDereferenceObject(devExt->ComPortFile);
	}

	return STATUS_SUCCESS;
}

/* ************************************************************************* */

NTSTATUS	WriteSerialCommand(DeviceExtension *devExt, CHAR cmd)
{
	NTSTATUS	status = STATUS_SUCCESS;
	IO_STATUS_BLOCK	ioStatusBlock;
	LARGE_INTEGER	offset;
	offset.QuadPart = 0;

	PIRP	Irp = IoBuildSynchronousFsdRequest(IRP_MJ_WRITE, devExt->ComPortDevice, &cmd, sizeof(CHAR), &offset, &devExt->ComPortEvent, &ioStatusBlock);
	if (Irp == NULL)
	{
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] IoBuildSynchronousFsdRequest(IRP_MJ_WRITE, ...) failed\n");
		status = STATUS_NO_MEMORY;
	}
	else
	{
		status = IoCallDriver(devExt->ComPortDevice, Irp);
		if (status == STATUS_PENDING)
		{
			KeWaitForSingleObject(&devExt->ComPortEvent, Executive, KernelMode, FALSE, NULL);
			status = ioStatusBlock.Status;
		}

		if (NT_SUCCESS(status) == FALSE)
		{
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Failed to send cmd '%c' - 0x%08x\n", cmd, status);
		}
	}

	return status;
}

/* ************************************************************************* */

ULONG	ReadSerialResponse(DeviceExtension *devExt, CHAR *response, ULONG responseSize)
{
	NTSTATUS	status = STATUS_SUCCESS;
	IO_STATUS_BLOCK	ioStatusBlock;
	LARGE_INTEGER	offset;
	offset.QuadPart = 0;
	ULONG result;

	PIRP	Irp = IoBuildSynchronousFsdRequest(IRP_MJ_READ, devExt->ComPortDevice, response, responseSize, &offset, &devExt->ComPortEvent, &ioStatusBlock);
	if (Irp == NULL)
	{
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] IoBuildSynchronousFsdRequest(IRP_MJ_READ, ...) failed\n");
		result = 0;
	}
	else
	{
		status = IoCallDriver(devExt->ComPortDevice, Irp);
		if (status == STATUS_PENDING)
		{
			LARGE_INTEGER timeout;
			// 1000 miliseconds, * 10 to get into 100-nanosecond intervals
			// negative => relative to now
			timeout.QuadPart = -(1000 * 10);

			KeWaitForSingleObject(&devExt->ComPortEvent, Executive, KernelMode, FALSE, &timeout);
			status = ioStatusBlock.Status;
		}

		if (NT_SUCCESS(status) == FALSE)
		{
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Failed to read response - 0x%08x\n", status);
			result = 0;
		}
		else
		{
			result = (ULONG) ioStatusBlock.Information;
		}
	}

	return result;
}

/* ************************************************************************* */

NTSTATUS	SendSerialCommand(DeviceExtension *devExt, CHAR cmd, CHAR *response, ULONG responseSize)
{
	NTSTATUS	status = WriteSerialCommand(devExt, cmd);

	while (responseSize > 0)
	{
		ULONG amt = ReadSerialResponse(devExt, response, responseSize);

		if (amt == 0)
		{
			status = STATUS_NO_DATA_DETECTED;
			break;
		}
		else if (response[amt - 1] == '\n')
		{
			// all responses from the pioven device are terminated
			// with a \n
			// NULL-terminate it instead
			response[amt - 1] = '\0';
			break;
		}
		else
		{
			// only managed a partial read
			response += amt;
			responseSize -= amt;
		}
	}

	return status;
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

