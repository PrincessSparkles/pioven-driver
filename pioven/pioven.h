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
#define PIOVEN_DEVICE_TYPE		0xd7f3

/* ************************************************************************* */

#endif  // __PIOVEN_H
