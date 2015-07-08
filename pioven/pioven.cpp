/*
 * pioven.cpp
 *
 * This file contains the main code for the pi-oven driver
 *
 * 17th June 2015
 */

/* ************************************************************************* */

#include <ntddk.h>

#include "pioven.h"
#include "pioven-driver.h"

/* ************************************************************************* */

// {E6F826CF-8E71-4390-A2F0-76B6C6076FE1}
static const GUID PIOVEN_GUID =
{ 0xe6f826cf, 0x8e71, 0x4390, { 0xa2, 0xf0, 0x76, 0xb6, 0xc6, 0x7, 0x6f, 0xe1 } };

/* ************************************************************************* */

NTSTATUS HandleIrpMjCreate(PDEVICE_OBJECT /*DeviceObject*/, PIRP Irp)
{
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] HandleIrpMjCreate\n");

	Irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_DEVICE_NOT_READY;
}

/* ************************************************************************* */

NTSTATUS HandleIrpMjClose(PDEVICE_OBJECT /*DeviceObject*/, PIRP Irp)
{
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] HandleIrpMjClose\n");

	Irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_DEVICE_NOT_READY;
}

/* ************************************************************************* */

NTSTATUS HandleIrpMjRead(PDEVICE_OBJECT /*DeviceObject*/, PIRP Irp)
{
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] HandleIrpMjRead\n");

	Irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_DEVICE_NOT_READY;
}

/* ************************************************************************* */

NTSTATUS HandleIrpMjWrite(PDEVICE_OBJECT /*DeviceObject*/, PIRP Irp)
{
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] HandleIrpMjWrite\n");

	Irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_DEVICE_NOT_READY;
}

/* ************************************************************************* */

NTSTATUS HandleIrpMjDeviceControl(PDEVICE_OBJECT /*DeviceObject*/, PIRP Irp)
{
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] HandleIrpMjDeviceControl\n");

	Irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_DEVICE_NOT_READY;
}

/* ************************************************************************* */

NTSTATUS HandleIrpMjPower(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] HandleIrpMjPower\n");

	DeviceExtension *devExt = (DeviceExtension *)DeviceObject->DeviceExtension;

	// for now, just pass the Irp downwards
	IoSkipCurrentIrpStackLocation(Irp);
	return PoCallDriver(devExt->AttachedDevice, Irp);
}

/* ************************************************************************* */

NTSTATUS HandleIrpMjPnp(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	PIO_STACK_LOCATION stackLoc = IoGetCurrentIrpStackLocation(Irp);
	UCHAR minor = stackLoc->MinorFunction;

	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] HandleIrpMjPnp - 0x%02x\n", minor);

	DeviceExtension *devExt = (DeviceExtension *)DeviceObject->DeviceExtension;

	// for now, just pass the Irp downwards
	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(devExt->AttachedDevice, Irp);
}

/* ************************************************************************* */

NTSTATUS HandleIrpMjSystemControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] HandleIrpMjSystemControl\n");

	DeviceExtension *devExt = (DeviceExtension *)DeviceObject->DeviceExtension;

	// for now, just pass the Irp downwards
	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(devExt->AttachedDevice, Irp);
}

/* ************************************************************************* */
/* ************************************************************************* */

NTSTATUS AddDevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT PhysicalDeviceObject)
{
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] AddDevice\n");

	DbgBreakPoint();

	PDEVICE_OBJECT deviceObject;
	NTSTATUS status = IoCreateDevice(DriverObject, sizeof(DeviceExtension), NULL, 
		PIOVEN_DEVICE_TYPE, FILE_DEVICE_SECURE_OPEN, FALSE, &deviceObject);

	if (NT_SUCCESS(status))
	{
		DeviceExtension *devExt = (DeviceExtension *)deviceObject->DeviceExtension;

		status = IoRegisterDeviceInterface(PhysicalDeviceObject, &PIOVEN_GUID, NULL, &devExt->SymbolicLinkName);
		if (NT_SUCCESS(status))
		{
			devExt->DriverObject = DriverObject;
			devExt->PhysicalDeviceObject = PhysicalDeviceObject;

			deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
			deviceObject->Flags |= DO_BUFFERED_IO;

			devExt->AttachedDevice = IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);

			if (devExt->AttachedDevice != NULL)
			{
				return STATUS_SUCCESS;
			}
			else
			{
				status = STATUS_INVALID_DEVICE_STATE;
			}
		}

		IoDeleteDevice(deviceObject);
	}

	return status;
}

/* ************************************************************************* */

VOID DriverUnload(PDRIVER_OBJECT /*DriverObject*/)
{
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] DriverUnload\n");
}

/* ************************************************************************* */

/*
 * DriverEntry
 *
 * The main entry point into the driver
 *
 * Parameters: DriverObject - DRIVER_OBJECT that needs to be initialised
 *             RegistryPath - Registry path holding configuration information
 * Returns: status
 * Called at: PASSIVE_LEVEL
 *
 * Standard initialisation code
 *
 * extern "C" to allow the linker to find it, without C++ name-mangling
 *
 * _Use_decl_annotations_ lets static analysis tools know to use the 
 * annotations from when the function was declared - i.e. that the DriverEntry 
 * is initialisation code
 */
_Use_decl_annotations_
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING /*RegistryPath*/)
{
	// always print the startup message - debug and release
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] pioven driver starting\n");
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Driver version: 0.1.0.2\n");

	DriverObject->MajorFunction[IRP_MJ_CREATE] = HandleIrpMjCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = HandleIrpMjClose;
	DriverObject->MajorFunction[IRP_MJ_READ] = HandleIrpMjRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = HandleIrpMjWrite;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleIrpMjDeviceControl;
	DriverObject->MajorFunction[IRP_MJ_POWER] = HandleIrpMjPower;
	DriverObject->MajorFunction[IRP_MJ_PNP] = HandleIrpMjPnp;
	DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = HandleIrpMjSystemControl;

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->DriverExtension->AddDevice = AddDevice;

	return STATUS_SUCCESS;
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
