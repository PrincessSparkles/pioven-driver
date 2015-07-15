// piovenapi.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "piovenapi.h"


// This is an example of an exported variable
PIOVENAPI_API int npiovenapi=0;

// This is an example of an exported function.
PIOVENAPI_API int fnpiovenapi(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see piovenapi.h for the class definition
Cpiovenapi::Cpiovenapi()
{
	return;
}
