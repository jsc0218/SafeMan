#include "MonitorDriver.h"

int GetProcessNameOffset()
{
	PEPROCESS pCurrrentProcess = PsGetCurrentProcess();

	for( int i = 0; i < 3*PAGE_SIZE; i++ )
	{
		if( !strncmp( "System", (PCHAR) pCurrrentProcess + i, strlen("System") ))
		{
			ulProcessNameOffset = i;
		}
	}
	return 0;
}

BOOL GetProcessName( PCHAR theName)
{
	char            *nameptr;
	ULONG           i;
	KIRQL           oldirql;

	if( ulProcessNameOffset )
	{
		PEPROCESS pCurrrentProcess = PsGetCurrentProcess();
		nameptr   = (PCHAR) pCurrrentProcess + ulProcessNameOffset;
		strncpy( theName, nameptr, 30 );
		theName[30] = 0; /* NULL at end */
		return TRUE;
	}
	return FALSE;
}

NTSTATUS   GetFullPath(HANDLE keyHandle, char *chFullPathName)   
{
	NTSTATUS ns;
	PVOID pKey = NULL, pFile = NULL;   
	UNICODE_STRING fullUniName;   
	ANSI_STRING akeyname;   
	ULONG actualLen;   
	UNICODE_STRING dosName;   

	fullUniName.Buffer=NULL;   
	fullUniName.Length=0;   
	chFullPathName[0]=0x00;   
	ns = ObReferenceObjectByHandle( keyHandle,  0, NULL, KernelMode, &pKey, NULL );  

	if( !NT_SUCCESS(ns) )   
		return ns;   

	fullUniName.Buffer = (PWSTR)ExAllocatePool(   PagedPool,   MAXPATHLEN*2);//1024*2   
	fullUniName.MaximumLength = MAXPATHLEN*2;   

	__try   
	{   
		pFile=(PVOID)*(ULONG *)((char *)pKey+20);   
		pFile=(PVOID)*(ULONG *)((char *)pFile);   
		pFile=(PVOID)*(ULONG *)((char *)pFile+36);   

		ObReferenceObjectByPointer(pFile,   0,   NULL,   KernelMode);   
		RtlVolumeDeviceToDosName(((PFILE_OBJECT)pFile)->DeviceObject,&dosName);   
		RtlCopyUnicodeString(&fullUniName,   &dosName);   
		RtlAppendUnicodeStringToString(&fullUniName,&((PFILE_OBJECT)pFile)->FileName);   

		ObDereferenceObject(pFile);   
		ObDereferenceObject(pKey   );   

		RtlUnicodeStringToAnsiString(   &akeyname,   &fullUniName,   TRUE   );   
		if(akeyname.Length<MAXPATHLEN)     
		{   
			memcpy(chFullPathName,akeyname.Buffer,akeyname.Length);   
			chFullPathName[akeyname.Length]=0x00;   
		}   
		else   
		{   
			memcpy(chFullPathName,akeyname.Buffer,MAXPATHLEN);   
			chFullPathName[MAXPATHLEN-1]=0x00;   
		}   

		RtlFreeAnsiString(   &akeyname   );   
		ExFreePool(dosName.Buffer);   
		ExFreePool(   fullUniName.Buffer   );   

		return   STATUS_SUCCESS;   
	}   
	__except(1)   
	{   
		if(fullUniName.Buffer)   ExFreePool(   fullUniName.Buffer     );   
		if(pKey)   ObDereferenceObject(pKey   );   
		return   STATUS_SUCCESS;   

	}   
}   

BOOL GoOrNot(char *chProcessName,char *chFullPathName)
{
	char chMessageBuffer[256] = {0};
	ULONG dwProcessMonitor = 1;		//���̼�ؿ������
	ULONG dwGoOrNot = 0;				//�Ƿ��������ִ��

	LARGE_INTEGER large_int;
	large_int.QuadPart = -10000;
	KeWaitForSingleObject( &event, Executive, KernelMode, 0, 0);	//����ǰ�߳�����Ϊ�ȴ�״̬���ȴ���׽�ں˶����������¼�

	strcpy(chMessageBuffer, chProcessName);
	strcat(chMessageBuffer, chFullPathName);

	strncpy( &chInteractionBuffer[8], chMessageBuffer, sizeof(chMessageBuffer));

	memmove(&chInteractionBuffer[0], &dwProcessMonitor, 4);

	while (1)
	{
		KeDelayExecutionThread(KernelMode, 0, &large_int);	//�����û��߳�
		memmove(&dwProcessMonitor, &chInteractionBuffer[0], 4);
		if (!dwProcessMonitor)		//�û��жϽ�������dwProcessMonitorΪ0
		{
			break;
		}
	}

	memmove(&dwGoOrNot, &chInteractionBuffer[4], 4);

	KdPrint(("[dwGoOrNot]  %x \n", dwGoOrNot));

	KeSetEvent(&event, 0, 0);

	return dwGoOrNot;
}

NTSTATUS FakedZwCreateProcess(
							  OUT PHANDLE ProcessHandle,
							  IN ACCESS_MASK DesiredAccess,
							  IN POBJECT_ATTRIBUTES ObjectAttributes,
							  IN HANDLE InheritFromProcessHandle,
							  IN BOOLEAN InheritHandles,
							  IN HANDLE SectionHandle OPTIONAL,
							  IN HANDLE DebugPort OPTIONAL,
							  IN HANDLE ExceptionPort OPTIONAL,
							  IN HANDLE Unknown 
							  )
{
	char chProcessName[PROCNAMELEN];
	char chFullPathName[MAXPATHLEN];   //Ӧ�ó���·��

	GetFullPath( SectionHandle,chFullPathName);
	GetProcessName(chProcessName);

	KdPrint(("ZwCreateProcess is called by %s\n",chProcessName));
	KdPrint(("The name is %s\n",chFullPathName));

	strcat(chProcessName,"##");

	if ( GoOrNot( chProcessName, chFullPathName))
	{
		KdPrint(("Enter FakedZwCreateProcess!!!!!\n"));
		return RealZwCreateProcess(
				ProcessHandle,
				DesiredAccess,
				ObjectAttributes,
				InheritFromProcessHandle,
				InheritHandles,
				SectionHandle,
				DebugPort,
				ExceptionPort,
				Unknown);
	}
	else
	{ 
		KdPrint(("Enter RealZwCreateProcess!!!!!\n"));
		//ProcessHandle = NULL;
		return STATUS_SUCCESS;
	}
}

//�������̼��
void ProcessMonitorOn()
{
	KdPrint(("Enter ProcessMonitorOn() function!!!\n"));
	RealZwCreateProcess = (ZWCREATEPROCESSEX)(*((PULONG)KeServiceDescriptorTable->pvSSDTBase + ulIndex));

	//�ü�ð�ĵ�ַ����SSDT������ʵ�ĺ�����ַ
	*(PULONG)((PULONG)KeServiceDescriptorTable->pvSSDTBase + ulIndex) = (ULONG)FakedZwCreateProcess;

}

void ProcessMonitorOff()
{
	KdPrint(("Enter ProcessMonitorOff() function!!!\n"));
	*(PULONG)((PULONG)KeServiceDescriptorTable->pvSSDTBase + ulIndex) = (ULONG)RealZwCreateProcess;
}
/*----------------------------------------------------------------    ��     ��  -------------------------------------------------------------------------*/

NTSTATUS MONITORDRIVERDispatchRoutine(
    IN PDEVICE_OBJECT		DeviceObject,
    IN PIRP					Irp
    )
{
	KdPrint(("Enter MONITORDRIVER_DispatchCreateClose\n"));
    NTSTATUS status = STATUS_SUCCESS;
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
	KdPrint(("Leave MONITORDRIVER_DispatchCreateClose\n"));
    return status;
}

NTSTATUS MONITORDRIVER_DispatchDeviceControl(
    IN PDEVICE_OBJECT		DeviceObject,
    IN PIRP					Irp
    )
{
	KdPrint(("Enter MONITORDriverDeviceIOControl\n"));

    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);// �õ���ǰ��ջ

	//�û�̬��dwControlBuffer������ʼ��ַ
	ULONG *inputBuffer = (PULONG)Irp->AssociatedIrp.SystemBuffer;
	ULONG ulFunctionAddress;
	memmove(&ulFunctionAddress, &inputBuffer[0], 4);
	ulIndex  = *(PLONG)(ulFunctionAddress+1);

	KdPrint(("[MONITORDriver] ulIndex %x \n", ulFunctionAddress));

    switch(irpStack->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_PROCESS_MONITORDRIVER_ON:
		{
				ulProcessManager = 1;
				ProcessMonitorOn();
				ULONG ulInteractionBufferAddress;
				memmove(&ulInteractionBufferAddress,&inputBuffer[1],4);
				//�õ�����������׵�ַ
				chInteractionBuffer = (char *)MmMapIoSpace( MmGetPhysicalAddress( (PVOID)ulInteractionBufferAddress), 256, MmNonCached);
		}
        break;
	case IOCTL_PROCESS_MONITORDRIVER_OFF:
		{
				ulProcessManager = 0;
				ProcessMonitorOff();
		}
        break;
    default:
		status = STATUS_INVALID_PARAMETER;
		Irp->IoStatus.Information = 0;
		KdPrint(("[MONITORDriver] unknown IRP_MJ_DEVICE_CONTROL \n"));
        break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
	KdPrint(("Leave MONITORDriverDeviceIOControl\n"));
    return status;
}

VOID MONITORDRIVER_DriverUnload(
    IN PDRIVER_OBJECT		pDriverObject
    )
{
	KdPrint(("Enter MONITORDriverUnload\n"));

	if (ulProcessManager == 1)
	{
		ProcessMonitorOff();
	}
	
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\MONITORDriver");

	IoDeleteSymbolicLink(&symLinkName);  // ɾ����������
	IoDeleteDevice(pDriverObject->DeviceObject);  // ɾ���豸
}

NTSTATUS CreateDevice(IN PDRIVER_OBJECT	pDriverObject) 
{
	// �����豸����
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MONITORDriver");

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
	RtlInitUnicodeString(&symLinkName, L"\\??\\MONITORDriver");
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status)) 
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	return STATUS_SUCCESS;
}

extern "C" NTSTATUS DriverEntry(
					IN OUT PDRIVER_OBJECT   DriverObject,
					IN PUNICODE_STRING      RegistryPath)
{
	KdPrint(("Enter MONITORDriverEntry\n"));

	DriverObject->DriverUnload = MONITORDRIVER_DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = MONITORDRIVERDispatchRoutine;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = MONITORDRIVERDispatchRoutine;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MONITORDRIVER_DispatchDeviceControl;
    

	NTSTATUS status = CreateDevice(DriverObject);  // ���������豸����

	KeInitializeEvent( &event, SynchronizationEvent, 1);	//��ʼ��һ���¼�����(ͬ���¼���signaled)

	GetProcessNameOffset();

	KdPrint(("MONITORDriverEntry end\n"));
	return status;
}

