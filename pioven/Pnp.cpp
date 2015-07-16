/*
 * Pnp.cpp
 *
 * This file contains the Pnp code for the pi-oven driver
 *
 * 13th July 2015
 */

/* ************************************************************************* */

#include <ntddk.h>

#include "pioven.h"
#include "pioven-driver.h"
#include "serial.h"

/* ************************************************************************* */

/* ************************************************************************* */
/* ************************************************************************* */

NTSTATUS	StartDeviceComplete(PDEVICE_OBJECT DeviceObject, PIRP /*Irp*/, PVOID /*Context*/)
{
	DeviceExtension *devExt = (DeviceExtension *)DeviceObject->DeviceExtension;

	KeSetEvent(&devExt->StartDeviceEvent, IO_NO_INCREMENT, FALSE);

	// so that the IO manager doesn't complete this Irp for us...
	return STATUS_MORE_PROCESSING_REQUIRED;
}

/* ************************************************************************* */

NTSTATUS HandleIrpMnStartDevice(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	DeviceExtension *devExt = (DeviceExtension *)DeviceObject->DeviceExtension;

	// pass the Irp down the stack, wait for it to be completed, then do
	// our initialisation
	IoCopyCurrentIrpStackLocationToNext(Irp);
	IoSetCompletionRoutine(Irp, StartDeviceComplete, NULL, TRUE, TRUE, TRUE);

	NTSTATUS status = IoCallDriver(devExt->AttachedDevice, Irp);

	if (status == STATUS_PENDING)
	{
		KeWaitForSingleObject(&devExt->StartDeviceEvent, Executive, KernelMode, FALSE, NULL);
		status = Irp->IoStatus.Status;
	}

	if (NT_SUCCESS(status))
	{
		// open the serial interface
		status = OpenSerialPort(L"\\DosDevices\\COM3", &devExt->hComPort);

		if (NT_SUCCESS(status))
		{
			status = SendSerialCommand(devExt->hComPort, 'v', devExt->PythonVersion, sizeof(devExt->PythonVersion));
		}

		// start the device
		status = IoSetDeviceInterfaceState(&devExt->SymbolicLinkName, TRUE);
	}

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

/* ************************************************************************* */

NTSTATUS HandleIrpMnRemoveDevice(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	DeviceExtension *devExt = (DeviceExtension *)DeviceObject->DeviceExtension;
	NTSTATUS status = IoSetDeviceInterfaceState(&devExt->SymbolicLinkName, FALSE);

	CloseSerialPort(devExt->hComPort);

	// pass the Irp downwards
	IoSkipCurrentIrpStackLocation(Irp);
	status = IoCallDriver(devExt->AttachedDevice, Irp);

	// we can do the rest of the cleanup here, without waiting for IoCallDriver to complete
	IoDetachDevice(devExt->AttachedDevice);
	IoDeleteDevice(DeviceObject);

	return status;
}

/* ************************************************************************* */

NTSTATUS HandleIrpMjPnp(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	PIO_STACK_LOCATION stackLoc = IoGetCurrentIrpStackLocation(Irp);
	UCHAR minor = stackLoc->MinorFunction;

	switch (minor)
	{
		case IRP_MN_START_DEVICE:
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] HandleIrpMjPnp - IRP_MN_START_DEVICE\n");
			return HandleIrpMnStartDevice(DeviceObject, Irp);

		case IRP_MN_REMOVE_DEVICE:
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] HandleIrpMjPnp - IRP_MN_REMOVE_DEVICE\n");
			return HandleIrpMnRemoveDevice(DeviceObject, Irp);

		default:
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "[pioven] HandleIrpMjPnp - 0x%02x\n", minor);
			break;
	}

	DeviceExtension *devExt = (DeviceExtension *)DeviceObject->DeviceExtension;

	// for now, just pass the Irp downwards
	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(devExt->AttachedDevice, Irp);
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
