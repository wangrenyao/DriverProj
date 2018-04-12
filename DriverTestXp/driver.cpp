
#include "private.h"


NTSTATUS DriverEntry(IN DRIVER_OBJECT *DriverObject, IN UNICODE_STRING *RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    KdPrint(("Enter DriverEntry.\n"));

    return STATUS_UNSUCCESSFUL;
}