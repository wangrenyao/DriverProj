
#include "private.h"
#include "public.h"

#pragma  alloc_gtext(PAGE, DriverUnload)

ULONG g_ulLogLevel = LOG_INFO;


VOID DriverUnload(IN DRIVER_OBJECT *DriverObject)
{
	UNICODE_STRING SymbolicLink;

	MyDriver_DbgPrint(LOG_INFO, ("Enter DriverUnload."));

	while (DriverObject->DeviceObject)
	{
		IoDeleteDevice(DriverObject->DeviceObject);
	}

	RtlInitUnicodeString(&SymbolicLink, GLOBAL_MYDRIVR_SYMBOLICLINK);

	IoDeleteSymbolicLink(&SymbolicLink);

	MyDriver_DbgPrint(LOG_INFO, ("Exit DriverUnload."));
}

NTSTATUS DriverEntry(IN DRIVER_OBJECT *DriverObject, IN UNICODE_STRING *RegistryPath)
{
	NTSTATUS status;

	PAGED_CODE();

	status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(RegistryPath);
	MyDriver_DbgPrint(LOG_INFO, ("Enter DriverEntry."));
	
	do 
	{
		status = CreateGlobalDevice(DriverObject);
		if (!NT_SUCCESS(status))
		{
			MyDriver_DbgPrint(LOG_INFO, ("CreateGlobalDevice failed, status=0x%08x.", status));
			break;
		}

		DriverObject->DriverUnload = DriverUnload;
		DriverObject->MajorFunction[IRP_MJ_CREATE] = MyDriverDispatchIoCreate;
		DriverObject->MajorFunction[IRP_MJ_CLEANUP] = MyDriverDispatchIoCleanup;
		DriverObject->MajorFunction[IRP_MJ_CLOSE] = MyDriverDispatchIoClose;
		DriverObject->MajorFunction[IRP_MJ_READ] = MyDriverDispatchIoRead;
		DriverObject->MajorFunction[IRP_MJ_WRITE] = MyDriverDispatchIoWrite;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MyDriverDispatchIoDeviceControl;

	} while (FALSE);
	
	MyDriver_DbgPrint(LOG_INFO, ("Exit DriverEntry, status=0x%08x.", status));
	return status;
}


MmGetSystemAddressForMdlSafe()