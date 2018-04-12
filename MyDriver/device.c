#include <ntddk.h>

#include "private.h"
#include "public.h"

NTSTATUS CreateGlobalDevice(IN DRIVER_OBJECT *DriverObject)
{
	NTSTATUS			status;
	UNICODE_STRING		DeviceName;
	UNICODE_STRING		SymbolicLink;
	DEVICE_OBJECT		*DeviceObject;

	MyDriver_DbgPrint(LOG_INFO, ("Enter CreateGlobalDevice."));

	status = STATUS_SUCCESS;
	DeviceObject = NULL;

	do
	{
		if (!DriverObject)
		{
			status = STATUS_INVALID_PARAMETER;
			MyDriver_DbgPrint(LOG_INFO, ("DriverObject is NULL!"));
			break;
		}

		RtlInitUnicodeString(&DeviceName, GLOBAL_MYDRIVER_DEVICE_NAME);
		RtlInitUnicodeString(&SymbolicLink, GLOBAL_MYDRIVR_SYMBOLICLINK);

		status = IoCreateDevice(DriverObject,
			sizeof(GLOBAL_DEVICE_CONTEXT),
			&DeviceName,
			FILE_DEVICE_UNKNOWN,
			FILE_DEVICE_SECURE_OPEN,
			FALSE,
			&DeviceObject);
		if (!NT_SUCCESS(status))
		{
			MyDriver_DbgPrint(LOG_ERROR, ("IoCreateDevice failed, DeviceName=%wZ, status=0x%08x.", &DeviceName, status));
			break;
		}

		status = IoCreateSymbolicLink(&SymbolicLink, &DeviceName);
		if (!NT_SUCCESS(status))
		{
			MyDriver_DbgPrint(LOG_ERROR, ("IoCreateSymbolicLink failed, SymbolicLink=%wZ, status=0x%08x.", &SymbolicLink, status));
			break;
		}

		ObReferenceObject(DeviceObject);
		DeviceObject->Flags |= DO_DIRECT_IO;
		DeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);
		

		TestSynchorization(DeviceObject);

		ObDereferenceObject(DeviceObject);

	} while (FALSE);

	if (!NT_SUCCESS(status))
	{
		if (DeviceObject)
		{
			IoDeleteDevice(DeviceObject);
		}
	}

	MyDriver_DbgPrint(LOG_INFO, ("Exit CreateGlobalDevice, status=0x%08x.", status));
	return status;
}



VOID KThreadRoutine(IN PVOID ThreadContext)
{
	PGLOBAL_DEVICE_CONTEXT pDeviceContext = (PGLOBAL_DEVICE_CONTEXT)ThreadContext;
	LARGE_INTEGER llWaitTime;
	llWaitTime.QuadPart = -10 * 1000 * 100;
	while (pDeviceContext->bRunThread)
	{
		MyDriver_DbgPrint(LOG_INFO, ("This is KThreadRoutine: %d, %d\n", PsGetCurrentProcessId(), PsGetCurrentThreadId()));
		KeDelayExecutionThread(KernelMode, FALSE, &llWaitTime);
	}

	//PsTerminateSystemThread(STATUS_SUCCESS);
}

VOID KWorkItemRoutine(IN DEVICE_OBJECT *DeviceObject, IN PVOID Context)
{
	UNREFERENCED_PARAMETER(DeviceObject);


	PIO_WORKITEM pIoWorkItem;

	pIoWorkItem = (PIO_WORKITEM)Context;

	IoFreeWorkItem(pIoWorkItem);

	MyDriver_DbgPrint(LOG_INFO, ("[%d][%d] KWorkItemRoutine running.", 
		PsGetCurrentProcessId(), 
		PsGetCurrentThreadId()));
}

VOID KIoTimerRoutine(_In_ DEVICE_OBJECT *DeviceObject, _In_opt_ PVOID Context)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Context);

	MyDriver_DbgPrint(LOG_INFO, ("[%d][%d] KIoTimerRoutine running.",
		PsGetCurrentProcessId(),
		PsGetCurrentThreadId()));
}

VOID KCallbackRoutine(IN PVOID CallbackContext, IN PVOID Argument1, IN PVOID Argument2)
{
	UNREFERENCED_PARAMETER(CallbackContext);
	
	MyDriver_DbgPrint(LOG_INFO, ("Argument1: %d, Argument2: %d", Argument1, Argument2));
	MyDriver_DbgPrint(LOG_INFO, 
		("[%d][%d] KWorkItemKCallbackRoutineRoutine running, Current IRQL: %d.", 
		PsGetCurrentProcessId(), 
		PsGetCurrentThreadId(), 
		KeGetCurrentIrql()));
}

VOID TestSynchorization(IN DEVICE_OBJECT *DeviceObject)
{
	LARGE_INTEGER llTimeOut;
	llTimeOut.QuadPart = -10 * 1000 * 1000;

	// Timer Object
	{
		KTIMER timerObj;
		LARGE_INTEGER llDueTime;
		

		KeInitializeTimer(&timerObj);
		llDueTime.QuadPart = -10 * 1000 * 500;
		KeSetTimer(&timerObj, llDueTime, NULL);
		KeWaitForSingleObject(&timerObj, Executive, KernelMode, FALSE, &llTimeOut);

		//PEX_TIMER extimerObj;
	}

	// Event Object
	{
		KEVENT eventObj;
		LONG status;

		KeInitializeEvent(&eventObj, SynchronizationEvent, FALSE);
		KeClearEvent(&eventObj);
		status = KeResetEvent(&eventObj);
		status = KeReadStateEvent(&eventObj);
		status = KePulseEvent(&eventObj, 0, FALSE);
		status = KeSetEvent(&eventObj, 0, FALSE);

		KeWaitForSingleObject(&eventObj, Executive, KernelMode, FALSE, &llTimeOut);
	}

	// Mutex object
	{
		KMUTEX mutexObj;
		LONG status;

		KeInitializeMutex(&mutexObj, 0);

		status = KeReadStateMutex(&mutexObj);
		KeWaitForMutexObject(&mutexObj, Executive, KernelMode, FALSE, &llTimeOut);
		KeWaitForSingleObject(&mutexObj, Executive, KernelMode, FALSE, &llTimeOut);
		
		KIRQL irql = KeGetCurrentIrql();
	
		irql = irql;
		KeReleaseMutex(&mutexObj, FALSE);
		KeReleaseMutex(&mutexObj, FALSE);
	}

	// Semaphore object
	{
		KSEMAPHORE semaphoreObj;
		LONG status;

		KeInitializeSemaphore(&semaphoreObj, 1, 2);

		status = KeReadStateSemaphore(&semaphoreObj);
		KeReleaseSemaphore(&semaphoreObj, 0, 1, FALSE);
		KeWaitForSingleObject(&semaphoreObj, Executive, KernelMode, FALSE, &llTimeOut);
		KeWaitForSingleObject(&semaphoreObj, Executive, KernelMode, FALSE, &llTimeOut);
	}

	// Thread Object
	{
		KeSetBasePriorityThread(KeGetCurrentThread(), 1);
		PIO_WORKITEM pWorkItem;
		// Work item
		pWorkItem = IoAllocateWorkItem(DeviceObject);
		IoQueueWorkItem(pWorkItem, KWorkItemRoutine, DelayedWorkQueue, pWorkItem);
	}

	// Callback object
	{
		OBJECT_ATTRIBUTES objAttributs;
		UNICODE_STRING objName;
		PCALLBACK_OBJECT pCallbackObj;

		RtlInitUnicodeString(&objName, L"\\Callback\\MyDriverCallback");
		InitializeObjectAttributes(&objAttributs, &objName, OBJ_CASE_INSENSITIVE, NULL, NULL);

		ExCreateCallback(&pCallbackObj, &objAttributs, TRUE, TRUE);
		PVOID CallbackReg = ExRegisterCallback(pCallbackObj, KCallbackRoutine, NULL);
		ExNotifyCallback(pCallbackObj, (PVOID)1, (PVOID)2);
		ExUnregisterCallback(CallbackReg);
		ObDereferenceObject(pCallbackObj);
	}


	// spin lock
	{
		KSPIN_LOCK spinLock;
		KIRQL OldIrql;
		KIRQL NewIrql;

		KeInitializeSpinLock(&spinLock);
		///////////////////////////////////
		KeAcquireSpinLock(&spinLock, &OldIrql);
		NewIrql = KeGetCurrentIrql();	//DPC_LEVEL
		KeReleaseSpinLock(&spinLock, OldIrql);

		///////////////////////////////////
		KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
		KeAcquireSpinLockAtDpcLevel(&spinLock);
		KeReleaseSpinLockFromDpcLevel(&spinLock);
		KeLowerIrql(OldIrql);

		///////////////////////////////////
		KLOCK_QUEUE_HANDLE lockHandle;

		KeAcquireInStackQueuedSpinLock(&spinLock, &lockHandle);
		KeReleaseInStackQueuedSpinLock(&lockHandle);

		///////////////////////////////////
		ULONG ulCount = 0;
		ExInterlockedAddUlong(&ulCount, 1, &spinLock);	//can run any IRQL

		///////////////////////////////////
		//KeSynchronizeExecution();

		EX_SPIN_LOCK exSpinLock;

		exSpinLock = 0;
		OldIrql = ExAcquireSpinLockExclusive(&exSpinLock);
		NewIrql = KeGetCurrentIrql();//DISPATCH_LEVEL
		ExReleaseSpinLockExclusive(&exSpinLock, OldIrql);

		OldIrql = ExAcquireSpinLockShared(&exSpinLock);
		NewIrql = KeGetCurrentIrql();//DISPATCH_LEVEL
		ExReleaseSpinLockShared(&exSpinLock, OldIrql);

	}

	// Fast mutex and guarded mutex
	{
		KIRQL OldIrql;
		KIRQL NewIrql;
		FAST_MUTEX fastMutex;

		ExInitializeFastMutex(&fastMutex);
		ExAcquireFastMutex(&fastMutex);
		NewIrql = KeGetCurrentIrql();
		ExReleaseFastMutex(&fastMutex);

		KeRaiseIrql(APC_LEVEL, &OldIrql);
		ExAcquireFastMutexUnsafe(&fastMutex);
		NewIrql = KeGetCurrentIrql();
		ExReleaseFastMutexUnsafe(&fastMutex);
		KeLowerIrql(OldIrql);

		KGUARDED_MUTEX guardedMutex;

		KeInitializeGuardedMutex(&guardedMutex);

		KeAcquireGuardedMutex(&guardedMutex);
		NewIrql = KeGetCurrentIrql();
		KeReleaseGuardedMutex(&guardedMutex);
	}

	// ERESOURCE
	{
		ERESOURCE res;
		KIRQL NewIrql;

		ExInitializeResourceLite(&res);

		ExAcquireResourceExclusiveLite(&res, TRUE);
		NewIrql = KeGetCurrentIrql();
		ExReleaseResourceLite(&res);

		ExAcquireResourceSharedLite(&res, TRUE);
		NewIrql = KeGetCurrentIrql();
		ExReleaseResourceLite(&res);
	}

	// IO timer
	{
		IoInitializeTimer(DeviceObject, KIoTimerRoutine, NULL);
		IoStartTimer(DeviceObject);

		LARGE_INTEGER llWaitTime;
		llWaitTime.QuadPart = -10000 * 1000 * 2;
		KeDelayExecutionThread(KernelMode, FALSE, &llWaitTime);
		IoStopTimer(DeviceObject);
	}

	//APC
	{
		KIRQL NewIrql;
		KeEnterCriticalRegion();
		NewIrql = KeGetCurrentIrql();
		KeLeaveCriticalRegion();

		KeEnterGuardedRegion();
		NewIrql = KeGetCurrentIrql();
		KeLeaveGuardedRegion();
	}

	// Run Down
	{
		EX_RUNDOWN_REF runDownRef;

		ExInitializeRundownProtection(&runDownRef);

		ExAcquireRundownProtection(&runDownRef);
		ExReleaseRundownProtection(&runDownRef);

		ExWaitForRundownProtectionRelease(&runDownRef);

	}
}



typedef struct _MYDRIVER_LIST
{
	LIST_ENTRY	Entry;
	ULONG		ulNumber;
} MYDRIVER_LIST, *PMYDRIVER_LIST;



VOID BugCheckCallback(_In_  PVOID Buffer, _In_  ULONG Length)
{
	UNREFERENCED_PARAMETER(Buffer);
	UNREFERENCED_PARAMETER(Length);
}

VOID BugCheckAddPagesCallback(
	_In_    KBUGCHECK_CALLBACK_REASON                Reason,
	_In_    struct _KBUGCHECK_REASON_CALLBACK_RECORD *Record,
	_Inout_ PVOID                                    ReasonSpecificData,
	_In_    ULONG                                    ReasonSpecificDataLength
)
{
	UNREFERENCED_PARAMETER(Reason);
	UNREFERENCED_PARAMETER(Record);
	UNREFERENCED_PARAMETER(ReasonSpecificData);
	UNREFERENCED_PARAMETER(ReasonSpecificDataLength);

}



extern PULONG InitSafeBootMode;

__declspec(noinline) ULONG DoFloatingPointCalculation(_In_ ULONG ulCount)
{
	double nTotal = 100.1;
	
	return (ULONG)(nTotal / ulCount);
}

VOID TestMisc(IN DEVICE_OBJECT *DeviceObject)
{

	UNREFERENCED_PARAMETER(DeviceObject);
	//NTSTATUS
	{
		NT_SUCCESS(STATUS_SUCCESS);
		NT_ERROR(STATUS_ERROR_PROCESS_NOT_IN_JOB);
		NT_INFORMATION(STATUS_INFO_LENGTH_MISMATCH);
		NT_WARNING(STATUS_SUCCESS);
	}

	// Exception
	{
		__try
		{
			ProbeForWrite(NULL, 4, 4);
			ExRaiseDatatypeMisalignment();
			//ExRaiseAccessViolation();
			//ExRaiseStatus(STATUS_SUCCESS);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{

		}
	}

	// List
	{
		//PushEntryList()
		//PopEntryList();

		LIST_ENTRY listHead;

		InitializeListHead(&listHead);

		InsertHeadList(&listHead, &listHead);
	}

	// Bug check callback
	{
		KBUGCHECK_CALLBACK_RECORD bgChCbr;

		KeInitializeCallbackRecord(&bgChCbr);
		KeRegisterBugCheckCallback(&bgChCbr, BugCheckCallback, NULL, 0, (PUCHAR)"MyDriver");

		KeDeregisterBugCheckCallback(&bgChCbr);
		//KeRegisterBugCheckReasonCallback()

		KBUGCHECK_REASON_CALLBACK_RECORD bgchrcbr;

		KeInitializeCallbackRecord(&bgchrcbr);

		KeRegisterBugCheckReasonCallback(&bgchrcbr, BugCheckAddPagesCallback, KbCallbackAddPages, (PUCHAR)"MyDriver");
	}


	{
		switch (*InitSafeBootMode)
		{
		case 0:
		{
			MyDriver_DbgPrint(LOG_INFO, ("The operating system is not in Safe Mode."));
			break;
		}
		case 1:
		{
			MyDriver_DbgPrint(LOG_INFO, ("The operating system is in SAFEBOOT_MINIMAL."));
			break;
		}
		case 2:
		{
			MyDriver_DbgPrint(LOG_INFO, ("The operating system is in SAFEBOOT_NETWORK."));
			break;
		}
		default:
			break;
		}
	}

	// Floating calculation
	{
		KFLOATING_SAVE floatSave;
		KeSaveFloatingPointState(&floatSave);

		__try
		{
			DoFloatingPointCalculation(1);
		}
		__finally
		{
			KeRestoreFloatingPointState(&floatSave);
		}
	}

	//
	{
		HANDLE hFile = NULL;
		OBJECT_ATTRIBUTES objAttr;
		UNICODE_STRING FileName;
		IO_STATUS_BLOCK IoStatusBlock;

		RtlInitUnicodeString(&FileName, L"\\DosDevices\\C:\\Windows\\example.txt");
		InitializeObjectAttributes(&objAttr, &FileName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

		ZwCreateFile(&hFile,
			GENERIC_WRITE,
			&objAttr,
			&IoStatusBlock,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			0,
			FILE_OPEN_IF,
			0,
			NULL,
			0);

		char *pBuf = "1234.";
		size_t len = 0;
		RtlStringCbLengthA(pBuf, 100, &len);

		ZwWriteFile(hFile,
			NULL,
			NULL,
			NULL,
			&IoStatusBlock,
			pBuf,
			len,
			NULL,
			NULL);

		ZwClose(hFile);

	}

	{
		//ZwCurrentProcess
		//PsInitialSystemProcess
		//COMPUTE_PAGES_SPANNED
	}
	
}
