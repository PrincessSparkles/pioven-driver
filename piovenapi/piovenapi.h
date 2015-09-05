
#ifndef __PIOVENAPI_H
#define __PIOVENAPI_H

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PIOVENAPI_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PIOVENAPI_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PIOVENAPI_EXPORTS
#define PIOVENAPI_API __declspec(dllexport)
#else
#define PIOVENAPI_API __declspec(dllimport)
#endif

// for now, we only handle one oven. If this were a 'proper' project
// we would have a function that enumerated the ovens, and returned a list
// to the user to choose - and then we'd open that one
PIOVENAPI_API HANDLE OpenOven(void);

PIOVENAPI_API void CloseOven(HANDLE hOven);

PIOVENAPI_API BOOL GetOvenVersion(HANDLE hOven, char *buf, DWORD bufSize);
PIOVENAPI_API DWORD GetOvenTemperature(HANDLE hOven);
PIOVENAPI_API BOOL SetHeaterOn(HANDLE hOven);
PIOVENAPI_API BOOL SetHeaterOff(HANDLE hOven);

#endif
