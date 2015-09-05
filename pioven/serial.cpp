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

NTSTATUS	DoSerialPortOutIOCTL(DeviceExtension *devExt, ULONG IoControlCode, PVOID OutBuffer, ULONG OutBufferSize)
{
	IO_STATUS_BLOCK ioStatusBlock;
	NTSTATUS status;

	KeResetEvent(&devExt->ComPortEvent);

	PIRP Irp = IoBuildDeviceIoControlRequest(IoControlCode, devExt->ComPortDevice, NULL, 0, OutBuffer, OutBufferSize,
		FALSE, &devExt->ComPortEvent, &ioStatusBlock);

	if (Irp == NULL)
	{
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] DoSerialPortOutIOCTL(0x%08x) failed to allocate IRP\n", IoControlCode);
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
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] DoSerialPortOutIOCTL(0x%08x) failed - 0x%08x\n", IoControlCode, status);
		}
	}

	return status;
}

/* ************************************************************************* */

NTSTATUS	ComPortRead(DeviceExtension *devExt, CHAR *c)
{
	IO_STATUS_BLOCK ioStatusBlock;
	LARGE_INTEGER	offset;
	offset.QuadPart = 0;
	NTSTATUS	status;

	KeResetEvent(&devExt->ComPortReadEvent);

	PIRP	Irp = IoBuildSynchronousFsdRequest(IRP_MJ_READ, devExt->ComPortDevice, c, sizeof(CHAR), &offset, &devExt->ComPortReadEvent, &ioStatusBlock);
	if (Irp == NULL)
	{
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] IoBuildSynchronousFsdRequest(IRP_MJ_READ, ...) failed\n");
		status = STATUS_NO_MEMORY;
	}
	else
	{
		status = IoCallDriver(devExt->ComPortDevice, Irp);
		if (status == STATUS_PENDING)
		{
			for (;;)
			{
				SERIAL_STATUS serialStatus;

				DoSerialPortOutIOCTL(devExt, IOCTL_SERIAL_GET_COMMSTATUS, &serialStatus, sizeof(SERIAL_STATUS));

				LARGE_INTEGER timeout;
				timeout.QuadPart = -(100 * 1000 * 5);

				if (KeWaitForSingleObject(&devExt->ComPortReadEvent, Executive, KernelMode, FALSE, NULL) != STATUS_TIMEOUT)
				{
					status = ioStatusBlock.Status;
					break;
				}
			}
		}

		if (NT_SUCCESS(status) == FALSE)
		{
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Failed to read response - 0x%08x\n", status);
		}
	}

	return status;
}

/* ************************************************************************* */

VOID ComPortThread(PVOID context)
{
	DeviceExtension *devExt = (DeviceExtension *)context;
	NTSTATUS status;

	CHAR	c;

	for (;;)
	{
		status = ComPortRead(devExt, &c);

		if (NT_SUCCESS(status) == FALSE)
		{
			break;
		}
		else
		{
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] read 0x%02x from ComPort\n", c);
		}
	}

	PsTerminateSystemThread(status);
}

/* ************************************************************************* */

NTSTATUS	DoSerialPortIOCTL(DeviceExtension *devExt, ULONG IoControlCode, PVOID InBuffer, ULONG InBufferSize)
{
	IO_STATUS_BLOCK ioStatusBlock;
	NTSTATUS status;

	KeResetEvent(&devExt->ComPortEvent);

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
	SERIAL_LINE_CONTROL	lineControl;
	SERIAL_CHARS	chars;
	SERIAL_HANDFLOW handFlow;

	DoSerialPortOutIOCTL(devExt, IOCTL_SERIAL_GET_BAUD_RATE, &baudRate, sizeof(SERIAL_BAUD_RATE));
	DoSerialPortOutIOCTL(devExt, IOCTL_SERIAL_GET_LINE_CONTROL, &lineControl, sizeof(SERIAL_LINE_CONTROL));
	DoSerialPortOutIOCTL(devExt, IOCTL_SERIAL_GET_CHARS, &chars, sizeof(SERIAL_CHARS));
	DoSerialPortOutIOCTL(devExt, IOCTL_SERIAL_GET_HANDFLOW, &handFlow, sizeof(SERIAL_HANDFLOW));

	DoSerialPortOutIOCTL(devExt, IOCTL_SERIAL_GET_BAUD_RATE, &baudRate, sizeof(SERIAL_BAUD_RATE));
	DoSerialPortOutIOCTL(devExt, IOCTL_SERIAL_GET_LINE_CONTROL, &lineControl, sizeof(SERIAL_LINE_CONTROL));
	DoSerialPortOutIOCTL(devExt, IOCTL_SERIAL_GET_CHARS, &chars, sizeof(SERIAL_CHARS));
	DoSerialPortOutIOCTL(devExt, IOCTL_SERIAL_GET_HANDFLOW, &handFlow, sizeof(SERIAL_HANDFLOW));

	baudRate.BaudRate = 9600;
	DoSerialPortIOCTL(devExt, IOCTL_SERIAL_SET_BAUD_RATE, &baudRate, sizeof(SERIAL_BAUD_RATE));

	DoSerialPortIOCTL(devExt, IOCTL_SERIAL_SET_RTS, NULL, 0);
	DoSerialPortIOCTL(devExt, IOCTL_SERIAL_SET_DTR, NULL, 0);

	lineControl.Parity = 0;
	lineControl.StopBits = 0;
	lineControl.WordLength = 8;
	DoSerialPortIOCTL(devExt, IOCTL_SERIAL_SET_LINE_CONTROL, &lineControl, sizeof(SERIAL_LINE_CONTROL));

	chars.EofChar = 0;
	chars.ErrorChar = 0;
	chars.BreakChar = 0;
	chars.EventChar = 0;
	chars.XonChar = 0x11;
	chars.XoffChar = 0x13;
	DoSerialPortIOCTL(devExt, IOCTL_SERIAL_SET_CHARS, &chars, sizeof(SERIAL_CHARS));

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
		KeInitializeEvent(&devExt->ComPortEvent, NotificationEvent, FALSE);

		status = ConfigureSerialPort(devExt);

		if (NT_SUCCESS(status) == FALSE)
		{
			CloseSerialPort(devExt);

			devExt->ComPortDevice = NULL;
			devExt->ComPortFile = NULL;
		}
		else
		{
			KeInitializeEvent(&devExt->ComPortReadEvent, NotificationEvent, FALSE);
			/*
			OBJECT_ATTRIBUTES objAttr;

			InitializeObjectAttributes(&objAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
			devExt->ComPortReadThread = NULL;

			status = PsCreateSystemThread(&devExt->ComPortReadThread, GENERIC_ALL, &objAttr, NULL, NULL, ComPortThread, devExt);

			if (NT_SUCCESS(status) == FALSE)
			{
				CloseSerialPort(devExt);

				devExt->ComPortDevice = NULL;
				devExt->ComPortFile = NULL;
			}
			*/
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

		if (devExt->ComPortReadThread != NULL)
		{
			ZwClose(devExt->ComPortReadThread);
			devExt->ComPortReadThread = NULL;
		}
	}

	return STATUS_SUCCESS;
}

/* ************************************************************************* */

NTSTATUS	WriteSerialCommand(DeviceExtension *devExt, CHAR cmd)
{
	NTSTATUS	status = STATUS_UNSUCCESSFUL;
	IO_STATUS_BLOCK	ioStatusBlock;
	LARGE_INTEGER	offset;
	offset.QuadPart = 0;

	while (status != STATUS_SUCCESS)
	{
		KeResetEvent(&devExt->ComPortEvent);

		PIRP	Irp = IoBuildSynchronousFsdRequest(IRP_MJ_WRITE, devExt->ComPortDevice, &cmd, sizeof(CHAR), &offset, &devExt->ComPortEvent, &ioStatusBlock);
		if (Irp == NULL)
		{
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] IoBuildSynchronousFsdRequest(IRP_MJ_WRITE, ...) failed\n");
			status = STATUS_NO_MEMORY;
			break;
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
				IoFreeIrp(Irp);
			}
		}
	}

	return status;
}

/* ************************************************************************* */

ULONG	ReadSerialResponse(DeviceExtension *devExt, CHAR *response, ULONG responseSize)
{
	NTSTATUS	status = STATUS_UNSUCCESSFUL;
	LARGE_INTEGER	offset;
	offset.QuadPart = 0;
	ULONG result;

	while (status != STATUS_SUCCESS)
	{
		KeResetEvent(&devExt->ComPortEvent);

		// gratuitous sleep
//		LARGE_INTEGER wait;
//		wait.QuadPart = -(100 * 10);
//		KeWaitForSingleObject(&devExt->ComPortEvent, Executive, KernelMode, FALSE, &wait);

		PIRP	Irp = IoBuildSynchronousFsdRequest(IRP_MJ_READ, devExt->ComPortDevice, response, responseSize, 
			&offset, &devExt->ComPortEvent, &devExt->ComPortStatus);
		if (Irp == NULL)
		{
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] IoBuildSynchronousFsdRequest(IRP_MJ_READ, ...) failed\n");
			result = 0;
			break;
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
				status = devExt->ComPortStatus.Status;
			}

			if (NT_SUCCESS(status) == FALSE)
			{
				DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Failed to read response - 0x%08x\n", status);
				result = 0;
				//IoFreeIrp(Irp);
			}
			else
			{
				result = (ULONG)devExt->ComPortStatus.Information;
				DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] ReadSerialResponse - %d bytes read\n", result);

				for (ULONG i = 0; i < result; i++)
				{
					DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "%02x ", response[i]);
				}
				DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "\n");
			}
		}
	}

	return result;
}

/* ************************************************************************* */

NTSTATUS	SendSerialCommand(DeviceExtension *devExt, CHAR cmd, CHAR *response, ULONG responseSize)
{
	CHAR *ptr = response;
	NTSTATUS	status = WriteSerialCommand(devExt, cmd);
	
	while (responseSize > 0)
	{
		ULONG amt = ReadSerialResponse(devExt, ptr, responseSize);

		if (amt == 0)
		{
			// no data read :-(
			status = STATUS_NO_DATA_DETECTED;
			break;
		}

		while (amt > 0 && ptr[amt - 1] == 0)
		{
			// strip any trailing 'zeros'
			amt--;
		}
		if (amt == 0)
		{
			// only zeros read, loop round
		}
		else if (ptr[amt - 1] == '\n')
		{
			// all responses from the pioven device are terminated
			// with a \n
			// NULL-terminate it instead
			ptr[amt - 1] = '\0';
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] %c => %s\n", cmd, response);
			break;
		}
		else
		{
			// keep reading
			ptr += amt;
			responseSize -= amt;
		}
	}

	return status;
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

