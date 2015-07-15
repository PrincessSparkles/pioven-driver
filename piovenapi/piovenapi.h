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

// This class is exported from the piovenapi.dll
class PIOVENAPI_API Cpiovenapi {
public:
	Cpiovenapi(void);
	// TODO: add your methods here.
};

extern PIOVENAPI_API int npiovenapi;

PIOVENAPI_API int fnpiovenapi(void);
