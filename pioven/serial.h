/*
 * serial.h
 *
 * This file contains the Serial I/O headers
 *
 * 15th July 2015
 */

#ifndef __SERIAL_H
#define __SERIAL_H

/* ************************************************************************* */

#include <ntddk.h>

/* ************************************************************************* */

NTSTATUS	OpenSerialPort(PCWSTR comPort, DeviceExtension *devExt);
NTSTATUS	CloseSerialPort(DeviceExtension *devExt);
NTSTATUS	SendSerialCommand(DeviceExtension *devExt, CHAR cmd, CHAR *response, ULONG responseSize);

/* ************************************************************************* */

#endif  // __SERIAL_H
