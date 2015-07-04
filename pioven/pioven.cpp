/*
 * pioven.cpp
 *
 * This file contains the main code for the pi-oven driver
 *
 * 17th June 2015
 */

/* ************************************************************************* */

#include <ntddk.h>

/* ************************************************************************* */

extern "C" DRIVER_INITIALIZE DriverEntry;

/* ************************************************************************* */
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
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT /*DriverObject*/, PUNICODE_STRING /*RegistryPath*/)
{
	DbgBreakPoint();

	// always print the startup message - debug and release
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] pioven driver starting\n");
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Driver version: 0.0.1.0\n");

	return STATUS_SUCCESS;
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
