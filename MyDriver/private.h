#ifndef _PRIVATE_H_
#define _PRIVATE_H_

#pragma once

#include <wdm.h>
#include <ntstrsafe.h>

extern ULONG g_ulLogLevel;

#define LOG_FATAL		0x00
#define LOG_ERROR		0x01
#define LOG_WARNING		0x02
#define LOG_INFO		0x03
#define LOG_DEBUG		0x04

#if DBG

#define MyDriver_DbgPrint(loglevel, _x_)		\
		do										\
		{										\
			if (loglevel <= g_ulLogLevel)		\
			{									\
				DbgPrint("MyDriver: ");			\
				DbgPrint _x_ ;					\
				DbgPrint("\n");					\
			}									\
		} while (0);

#else

#define MyDriver_DbgPrint(loglevel, _x_)

#endif



typedef struct  _GLOBAL_DEVICE_CONTEXT
{
	ULONG nCount;

	BOOLEAN bRunThread;
	HANDLE hMainThread;
} GLOBAL_DEVICE_CONTEXT, *LPGLOBAL_DEVICE_CONTEXT, *PGLOBAL_DEVICE_CONTEXT;





NTSTATUS MyDriverDispatchIoCreate(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp);
NTSTATUS MyDriverDispatchIoCleanup(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp);
NTSTATUS MyDriverDispatchIoClose(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp);
NTSTATUS MyDriverDispatchIoRead(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp);
NTSTATUS MyDriverDispatchIoWrite(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp);
NTSTATUS MyDriverDispatchIoDeviceControl(IN DEVICE_OBJECT *DeviceObject, IN IRP *Irp);

NTSTATUS CreateGlobalDevice(IN DRIVER_OBJECT *DriverObject);
VOID KThreadRoutine(IN PVOID ThreadContext);
VOID TestSynchorization();
#endif