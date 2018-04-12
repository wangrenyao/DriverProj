#ifndef _PRIVATE_H_
#define _PRIVATE_H_

#include <ntddk.h>


typedef struct _SYSTEM_SERVICE_TABLE
{
    PULONG ServiceTableBase;
    PULONG ServiceCounterTableBase;
    ULONG NumberOfServices;
    PUCHAR ParamTableBase;
} SYSTEM_SERVICE_TABLE, *PSYSTEM_SERVICE_TABLE;

extern PSYSTEM_SERVICE_TABLE KeServiceDescriptorTable;


typedef struct  _GLOBAL_DEVICE_CONTEXT
{
    PVOID pZwCreateFile;

} GLOBAL_DEVICE_CONTEXT, *LPGLOBAL_DEVICE_CONTEXT, *PGLOBAL_DEVICE_CONTEXT;







VOID PageProtectClose();
VOID PageProtectOpen();
NTSTATUS InstallHook ( __in DEVICE_OBJECT *DeviceObject );
NTSTATUS UninstallHook ( __in DEVICE_OBJECT *DeviceObject );


#endif /* _PRIVATE_H_ */