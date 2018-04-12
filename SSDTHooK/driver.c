
#include "private.h"


VOID DriverUnload ( __in DRIVER_OBJECT *DriverObject )
{
    KdPrint ( ( "DriverUnload start..." ) );

    UninstallHook ( NULL );

    while ( DriverObject->DeviceObject )
    {
        IoDeleteDevice ( DriverObject->DeviceObject );
    }

    KdPrint ( ( "DriverUnload exit..." ) );
}


NTSTATUS DriverEntry ( _In_ DRIVER_OBJECT *DriverObject,
                       _In_ UNICODE_STRING *RegistryPath )
{
    UNREFERENCED_PARAMETER ( RegistryPath );

    NTSTATUS	status;
    DEVICE_OBJECT *globalDeviceObj;

    KdPrint ( ( "DriverEntry start..." ) );

    DriverObject->DriverUnload = DriverUnload;


    status = IoCreateDevice ( DriverObject,
                              sizeof ( GLOBAL_DEVICE_CONTEXT ),
                              NULL,
                              FILE_DEVICE_UNKNOWN,
                              0,
                              FALSE,
                              &globalDeviceObj );

    if ( !NT_SUCCESS ( status ) )
    {
        KdPrint ( ( "IoCreateDevice failed, status=0x%08x...", status ) );
        goto EXIT;
    }


    status = InstallHook ( globalDeviceObj );


    globalDeviceObj->Flags &= ( ~DO_DEVICE_INITIALIZING );

EXIT:

    KdPrint ( ( "DriverEntry exit, status=0x%08x.", status ) );
    return status;
}