#include "private.h"
#include "public.h"

NTSTATUS MyDriverDispatchIoCreate(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp)
{
	NTSTATUS			status;
	IO_STACK_LOCATION	*IoStackLocation;

	UNREFERENCED_PARAMETER(DeviceObject);
	status = STATUS_SUCCESS;
	IoStackLocation = NULL;

	MyDriver_DbgPrint(LOG_INFO, ("Enter MyDriverDispatchIoCreate."));
	IoStackLocation = IoGetCurrentIrpStackLocation(Irp);

	MyDriver_DbgPrint(LOG_INFO, ("FileName: %wZ.", &IoStackLocation->FileObject->FileName));

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	MyDriver_DbgPrint(LOG_INFO, ("Exit MyDriverDispatchIoCreate."));
	return STATUS_SUCCESS;
}



NTSTATUS MyDriverDispatchIoCleanup(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp)
{
	NTSTATUS			status;
	IO_STACK_LOCATION	*IoStackLocation;
	UNREFERENCED_PARAMETER(DeviceObject);
	status = STATUS_SUCCESS;
	IoStackLocation = NULL;

	MyDriver_DbgPrint(LOG_INFO, ("Enter MyDriverDispatchIoCleanup."));
	IoStackLocation = IoGetCurrentIrpStackLocation(Irp);

	MyDriver_DbgPrint(LOG_INFO, ("FileName: %wZ.", &IoStackLocation->FileObject->FileName));

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	MyDriver_DbgPrint(LOG_INFO, ("Exit MyDriverDispatchIoCleanup."));
	return STATUS_SUCCESS;
}


NTSTATUS MyDriverDispatchIoClose(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp)
{
	NTSTATUS			status;
	IO_STACK_LOCATION	*IoStackLocation;
	UNREFERENCED_PARAMETER(DeviceObject);
	status = STATUS_SUCCESS;
	IoStackLocation = NULL;

	MyDriver_DbgPrint(LOG_INFO, ("Enter MyDriverDispatchIoClose."));
	IoStackLocation = IoGetCurrentIrpStackLocation(Irp);

	MyDriver_DbgPrint(LOG_INFO, ("FileName: %wZ.", &IoStackLocation->FileObject->FileName));

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	MyDriver_DbgPrint(LOG_INFO, ("Exit MyDriverDispatchIoClose."));
	return STATUS_SUCCESS;
}


NTSTATUS MyDriverDispatchIoDeviceControl(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp)
{
	NTSTATUS			status;
	IO_STACK_LOCATION	*IoStackLocation;
	GLOBAL_DEVICE_CONTEXT *pDeviceContext;

	UNREFERENCED_PARAMETER(DeviceObject);
	status = STATUS_SUCCESS;
	IoStackLocation = NULL;

	MyDriver_DbgPrint(LOG_INFO, ("Enter MyDriverDispatchIoDeviceControl."));
	IoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	pDeviceContext = (GLOBAL_DEVICE_CONTEXT*)DeviceObject->DeviceExtension;

	MyDriver_DbgPrint(LOG_INFO, ("FileName: %wZ.", &IoStackLocation->FileObject->FileName));


	switch (IoStackLocation->Parameters.DeviceIoControl.IoControlCode)
	{
		case IOCTL_MYDRIVER_START_THREAD:
		{
			OBJECT_ATTRIBUTES objAttributs;
			InitializeObjectAttributes(&objAttributs, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

			pDeviceContext->bRunThread = TRUE;
			PsCreateSystemThread(&pDeviceContext->hMainThread,
				SYNCHRONIZE,
				&objAttributs,
				NtCurrentProcess(),
				NULL,
				KThreadRoutine,
				pDeviceContext);
			break;
		}

		case IOCTL_MYDRIVER_STOP_THREAD:
		{
			PETHREAD ThreadObj;

			pDeviceContext->bRunThread = FALSE;
			ObReferenceObjectByHandle(pDeviceContext->hMainThread,
				SYNCHRONIZE,
				NULL,
				KernelMode,
				&ThreadObj,
				NULL);
			KeWaitForSingleObject(ThreadObj, Executive, KernelMode, FALSE, NULL);
			ObDereferenceObject(ThreadObj);

			ZwClose(pDeviceContext->hMainThread);

			break;
		}

		default:
			break;
	}





	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT );

	MyDriver_DbgPrint(LOG_INFO, ("Exit MyDriverDispatchIoDeviceControl."));
	return STATUS_SUCCESS;
}