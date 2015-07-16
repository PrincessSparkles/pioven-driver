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

NTSTATUS	OpenSerialPort(PCWSTR comPort, PHANDLE hComPort);
NTSTATUS	CloseSerialPort(HANDLE hComPort);
NTSTATUS	SendSerialCommand(HANDLE hComPort, CHAR cmd, CHAR *response, ULONG responseSize);

/* ************************************************************************* */

#endif  // __SERIAL_H
