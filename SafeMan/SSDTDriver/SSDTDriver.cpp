#include "SSDTDriver.h"


/*----------------------------------------------------------------    ��     ��  -------------------------------------------------------------------------*/

NTSTATUS SSDTDriverDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	KdPrint(("Enter SSDTDriverDispatchRoutine\n"));
	NTSTATUS status = STATUS_SUCCESS;
	// ���IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;  // bytes xfered
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint(("Leave SSDTDriverDispatchRoutine\n"));

	return status;
}


NTSTATUS SSDTDriverDispatchDeviceControl(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	KdPrint(("Enter SSDTDriverDeviceIOControl\n"));

	NTSTATUS status = STATUS_SUCCESS;

	PVOID ioBuf = pIrp->AssociatedIrp.SystemBuffer;  // ϵͳ�ṩ�Ļ�����

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);  // �õ���ǰ��ջ
	ULONG inBufLength = stack->Parameters.DeviceIoControl.InputBufferLength;  // �õ����뻺������С
	ULONG outBufLength = stack->Parameters.DeviceIoControl.OutputBufferLength;  // �õ������������С
	ULONG ioControlCode = stack->Parameters.DeviceIoControl.IoControlCode;  // �õ�IOCTL��

	switch (ioControlCode)
	{
	case IOCTL_SSDTDRIVER_GETSSDT:
		{
			RtlCopyMemory(ioBuf, KeServiceDescriptorTable, sizeof(SSDT));
			pIrp->IoStatus.Information = sizeof(SSDT);
			KdPrint(("[KeServiceDescriptorTable]:%x", KeServiceDescriptorTable));
			break;
		}
	case IOCTL_SSDTDRIVER_GETFUNADDRBYINDEX:
		{
			ULONG ulIndex = *(PULONG)ioBuf;
			if (KeServiceDescriptorTable->ulNumberOfServices <= ulIndex)
			{
				status = STATUS_INVALID_PARAMETER;
				pIrp->IoStatus.Information = 0;
				break;
			}
			PULONG pBase  = (PULONG)KeServiceDescriptorTable->pvSSDTBase;
			RtlCopyMemory(ioBuf, pBase + ulIndex, sizeof(ULONG));
			pIrp->IoStatus.Information = sizeof(ULONG);
			KdPrint(("[FuntionAddress]:%x", *((PULONG)ioBuf)));
			break;
		}
	case IOCTL_SSDTDRIVER_RESTOREFUNADDR:
		{
			ULONG ulIndex = ((PRESTORE_INFO)ioBuf)->ulIndex;
			if (KeServiceDescriptorTable->ulNumberOfServices <= ulIndex)
			{
				status = STATUS_INVALID_PARAMETER;
				pIrp->IoStatus.Information = 0;
				break;
			}
			ULONG ulAddr = ((PRESTORE_INFO)ioBuf)->ulAddr;

			*((PULONG)KeServiceDescriptorTable->pvSSDTBase + ulIndex) = ulAddr;
			pIrp->IoStatus.Information = 0;

			break;
		}
	default:
		status = STATUS_INVALID_PARAMETER;
		pIrp->IoStatus.Information = 0;
		KdPrint(("[SSDTDriver] unknown IRP_MJ_DEVICE_CONTROL \n"));
	}

	// ���IRP
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	KdPrint(("Leave SSDTDriverDeviceIOControl\n"));

	return status;
}


VOID SSDTDriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("Enter DriverUnload\n"));

	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\SSDTDriver");

	IoDeleteSymbolicLink(&symLinkName);  // ɾ����������
	IoDeleteDevice(pDriverObject->DeviceObject);  // ɾ���豸
}


NTSTATUS CreateDevice(IN PDRIVER_OBJECT	pDriverObject) 
{
	// �����豸����
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\SSDTDriver");

	// �����豸
	PDEVICE_OBJECT pDevObj;
	NTSTATUS status = IoCreateDevice(pDriverObject,
		sizeof(DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		0, 
		TRUE,
		&pDevObj);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

	//������������
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\SSDTDriver");
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status)) 
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	return STATUS_SUCCESS;
}


extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{    
	KdPrint(("Enter DriverEntry\n"));

	// ע�������������ú������
	pDriverObject->DriverUnload = SSDTDriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = SSDTDriverDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = SSDTDriverDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SSDTDriverDispatchDeviceControl;

	NTSTATUS status = CreateDevice(pDriverObject);  // ���������豸����

	KdPrint(("DriverEntry end\n"));
	return status;
}
