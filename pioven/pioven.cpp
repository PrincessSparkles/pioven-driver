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

// read from the registry during DriverInit
UNICODE_STRING g_ComPort;

/* ************************************************************************* */
/* ************************************************************************* */

NTSTATUS InitUnicodeStringFromString(PUNICODE_STRING result, PWSTR value)
{
	NTSTATUS status;
	// copy the value into a temp buffer
	ULONG valueLen = (ULONG) wcslen(value);
	ULONG bufferSize = (valueLen + 1) * sizeof(WCHAR);
	
	PWSTR buffer = (PWSTR)ExAllocatePool(PagedPool, bufferSize);
	if (buffer == NULL)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	else
	{
		// cop the string into the buffer, and init 'result'
		RtlCopyMemory(buffer, value, bufferSize);
		RtlInitUnicodeString(result, buffer);
		
		status = STATUS_SUCCESS;
	}

	return status;
}

/* ************************************************************************* */

/*
 * GetRegistryString
 *
 * Get a string from the registry
 *
 * Parameters: keyName      - the key containing the value that we're interested in
 *             valueName    - the name of the value we're reading
 *             resultString - pointer to where to store the result
 * Returns: STATUS_xxx
 */
NTSTATUS GetRegistryString(PUNICODE_STRING keyName, PWSTR valueName, PUNICODE_STRING resultString)
{
	NTSTATUS status;

	// open the key
	OBJECT_ATTRIBUTES objAttr;
	InitializeObjectAttributes(&objAttr, keyName, 0, NULL, NULL);

	HANDLE hKey;
	status = ZwOpenKey(&hKey, KEY_READ, &objAttr);
	if (status != STATUS_SUCCESS)
	{
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Failed to open registry key %wZ - 0x%08x\n", 
			keyName, status);
		return status;
	}
	else
	{
		// read the value
		UNICODE_STRING valueNameStr;
		RtlInitUnicodeString(&valueNameStr, valueName);
		ULONG valueLen = 0;
		// get the length required
		status = ZwQueryValueKey(hKey, &valueNameStr, KeyValuePartialInformation, NULL, 0, &valueLen);

		if (valueLen == 0)
		{
			DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Failed to query value len %wZ - 0x%08x\n",
				&valueNameStr, status);
		}
		else
		{
			// allocate some memory
			PVOID pValue = ExAllocatePool(PagedPool, valueLen);
			if (pValue == NULL)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Failed to allocate %d bytes - 0x%08x\n",
					valueLen, status);
			}
			else
			{
				status = ZwQueryValueKey(hKey, &valueNameStr, KeyValuePartialInformation, pValue, valueLen, &valueLen);
				if (status != STATUS_SUCCESS)
				{
					DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Failed to read value %wZ - 0x%08x\n",
						&valueNameStr, status);
				}
				else
				{
					PKEY_VALUE_PARTIAL_INFORMATION info = (PKEY_VALUE_PARTIAL_INFORMATION)pValue;
					status = InitUnicodeStringFromString(resultString, (PWSTR)info->Data);

					if (status != STATUS_SUCCESS)
					{
						DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Failed to read value %wZ - 0x%08x\n",
							&valueNameStr, status);
					}
					else
					{
						status = STATUS_SUCCESS;
					}
				}

				ExFreePool(pValue);
			}
		}

		ZwClose(hKey);
	}

	return status;
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
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT /*DriverObject*/, PUNICODE_STRING RegistryPath)
{
	// always print the startup message - debug and release
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] pioven driver starting\n");
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "[pioven] Driver version: 0.0.1.1\n");
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_INFO_LEVEL, "[pioven] Registry Path: %wZ\n", RegistryPath);

	NTSTATUS status = GetRegistryString(RegistryPath, L"ComPort", &g_ComPort);
	if (status != STATUS_SUCCESS)
	{
		// hard code to COM3
		status = InitUnicodeStringFromString(&g_ComPort, L"COM3");
	}

	return status;
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
