/*
 * pioven-driver.h
 *
 * This file contains the kernel-only headers for the pioven driver
 *
 * 7th July 2015
 */

#ifndef __PIOVEN_DRIVER_H
#define __PIOVEN_DRIVER_H

/* ************************************************************************* */

#include <ntddk.h>

/* ************************************************************************* */

extern "C" DRIVER_INITIALIZE DriverEntry;
DRIVER_ADD_DEVICE AddDevice;
DRIVER_UNLOAD DriverUnload;

DRIVER_DISPATCH HandleIrpMjPnp;

/* ************************************************************************* */

typedef struct _DeviceExtension 
{
	// the owning driver
	PDRIVER_OBJECT	DriverObject;

	// the physical device object
	PDEVICE_OBJECT	PhysicalDeviceObject;

	// the device to which we are attached
	PDEVICE_OBJECT	AttachedDevice;

	// UNICODE_STRING returned by IoRegisterDeviceInterface
	UNICODE_STRING	SymbolicLinkName;

	// event used to sync the completion of IRP_MN_START_DEVICE
	KEVENT	StartDeviceEvent;
} DeviceExtension;

/* ************************************************************************* */

#endif  // __PIOVEN_DRIVER_H
