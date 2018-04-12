#include "private.h"

NTSTATUS MyDriverDispatchIoRead(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp)
{
	NTSTATUS			status;
	ULONG				ulLength;
	IO_STACK_LOCATION *IoStackLocation;
	PVOID	pBuf;
	char *szStr;

	UNREFERENCED_PARAMETER(DeviceObject);

	status = STATUS_SUCCESS;
	ulLength = 0;
	IoStackLocation = NULL;
	pBuf = NULL;
	szStr = NULL;

	MyDriver_DbgPrint(LOG_INFO, ("Enter MyDriverDispatchIoRead."));
	szStr = "This is a test string!";
	pBuf = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
	IoStackLocation = IoGetCurrentIrpStackLocation(Irp);

	ulLength = IoStackLocation->Parameters.Read.Length;
	if (ulLength < strlen(szStr) + 1)
	{
		Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
		Irp->IoStatus.Information = 0;
	}
	else
	{
		RtlCopyMemory(pBuf, szStr, strlen(szStr) + 1);
		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = strlen(szStr) + 1;
	}
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	MyDriver_DbgPrint(LOG_INFO, ("Exit MyDriverDispatchIoRead."));
	return STATUS_SUCCESS;
}

NTSTATUS MyDriverDispatchIoWrite(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp)
{
	NTSTATUS			status;
	ULONG				ulLength;
	IO_STACK_LOCATION *IoStackLocation;
	PVOID	pBuf;

	UNREFERENCED_PARAMETER(DeviceObject);
	status = STATUS_SUCCESS;
	ulLength = 0;
	IoStackLocation = NULL;
	pBuf = NULL;

	MyDriver_DbgPrint(LOG_INFO, ("Enter MyDriverDispatchIoWrite."));
	pBuf = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
	IoStackLocation = IoGetCurrentIrpStackLocation(Irp);

	ulLength = IoStackLocation->Parameters.Read.Length;

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = ulLength;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	MyDriver_DbgPrint(LOG_INFO, ("Exit MyDriverDispatchIoWrite."));
	return STATUS_SUCCESS;
}