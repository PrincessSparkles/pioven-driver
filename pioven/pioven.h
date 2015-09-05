/*
 * pioven.h
 *
 * This file contains the shared headers for the pioven driver
 * (shared between user-mode and kernel-mode)
 *
 * 7th July 2015
 */

#ifndef __PIOVEN_H
#define __PIOVEN_H

/* ************************************************************************* */

// {E6F826CF-8E71-4390-A2F0-76B6C6076FE1}
static const GUID PIOVEN_GUID =
{ 0xe6f826cf, 0x8e71, 0x4390, { 0xa2, 0xf0, 0x76, 0xb6, 0xc6, 0x7, 0x6f, 0xe1 } };

/* ************************************************************************* */

// not a system defined type of device, so use a number between 0x8000 and 0xffff
#define FILE_DEVICE_PIOVEN		0xd7f3

/* ************************************************************************* */

// max size of the string returned by python code in response to a version request
#define PIOVEN_VERSION_SIZE		32

/* ************************************************************************* */

// no input, PIOVEN_VERSION_SIZE bytes for output
#define IOCTL_PIOVEN_GET_VERSION      CTL_CODE(FILE_DEVICE_PIOVEN,0,METHOD_BUFFERED,FILE_ANY_ACCESS)

// no input, DWORD output
#define IOCTL_PIOVEN_GET_TEMPERATURE  CTL_CODE(FILE_DEVICE_PIOVEN,1,METHOD_BUFFERED,FILE_ANY_ACCESS)

// no input, no output
#define IOCTL_PIOVEN_HEATER_ON        CTL_CODE(FILE_DEVICE_PIOVEN,2,METHOD_BUFFERED,FILE_ANY_ACCESS)

// no input, no output
#define IOCTL_PIOVEN_HEATER_OFF       CTL_CODE(FILE_DEVICE_PIOVEN,3,METHOD_BUFFERED,FILE_ANY_ACCESS)

/* ************************************************************************* */

#endif  // __PIOVEN_H
