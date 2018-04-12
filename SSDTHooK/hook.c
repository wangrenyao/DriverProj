#include "private.h"


VOID PageProtectClose()
{
    __asm
    {
        cli
        mov eax, cr0
        and eax, not 10000h
        mov cr0, eax
    }
}

//启用页面保护
VOID PageProtectOpen()
{
    __asm
    {
        mov eax, cr0
        or eax, 10000h
        mov cr0, eax
        sti
    }
}

#define SYSTEMCALL_INDEX(ServiceFunction) (*(PULONG)((PUCHAR)ServiceFunction + 1))


typedef NTSTATUS ( NTAPI *pZwCreateFile ) (
    PHANDLE            FileHandle,
    ACCESS_MASK        DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK   IoStatusBlock,
    PLARGE_INTEGER     AllocationSize,
    ULONG              FileAttributes,
    ULONG              ShareAccess,
    ULONG              CreateDisposition,
    ULONG              CreateOptions,
    PVOID              EaBuffer,
    ULONG              EaLength
);

pZwCreateFile pOldZwCreateFile = NULL;

NTSTATUS NTAPI ZwCreateFile_Hook (
    PHANDLE            FileHandle,
    ACCESS_MASK        DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK   IoStatusBlock,
    PLARGE_INTEGER     AllocationSize,
    ULONG              FileAttributes,
    ULONG              ShareAccess,
    ULONG              CreateDisposition,
    ULONG              CreateOptions,
    PVOID              EaBuffer,
    ULONG              EaLength
)
{


    NTSTATUS status;

    status = pOldZwCreateFile ( FileHandle,
                                DesiredAccess,
                                ObjectAttributes,
                                IoStatusBlock,
                                AllocationSize,
                                FileAttributes,
                                ShareAccess,
                                CreateDisposition,
                                CreateOptions,
                                EaBuffer,
                                EaLength );

    KdPrint ( ( "[SSDTHOOK] Post ZwCreateFile, FileName=%ws, status=0x%08x\n",
                ObjectAttributes->ObjectName->Buffer, status ) );


    KdPrint ( ( "PID: %u TID: %u, ImagePath: %s (%ws).\n",
                PsGetCurrentProcessId(),
                PsGetCurrentThreadId(),
                ( PUCHAR ) PsGetCurrentProcess() + 0x170,
                ( ( OBJECT_NAME_INFORMATION* ) ( * ( PULONG ) ( ( PUCHAR ) PsGetCurrentProcess()
                        + 0x184 ) ) )->Name.Buffer ) );

    return status;
}

NTSTATUS InstallHook ( __in DEVICE_OBJECT *DeviceObject )
{
    PageProtectClose();

    pOldZwCreateFile = ( pZwCreateFile )
                       KeServiceDescriptorTable->ServiceTableBase[SYSTEMCALL_INDEX (
                                                    ZwCreateFile )];
    InterlockedExchange (
        &KeServiceDescriptorTable->ServiceTableBase[SYSTEMCALL_INDEX (
                                     ZwCreateFile )], ( ULONG ) &ZwCreateFile_Hook );
    PageProtectOpen();

    return STATUS_SUCCESS;
}


NTSTATUS UninstallHook ( __in DEVICE_OBJECT *DeviceObject )
{
    PageProtectClose();

    InterlockedExchange (
        &KeServiceDescriptorTable->ServiceTableBase[SYSTEMCALL_INDEX (
                                     ZwCreateFile )], ( ULONG ) pOldZwCreateFile );

    PageProtectOpen();

    return STATUS_SUCCESS;
}