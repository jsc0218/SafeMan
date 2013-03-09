#include "ProcessDriver.h"

// ͨ������PsLookupProcessByProcessId��������ȡPspCidTable���ȫ�ֱ����ĵ�ַ��
// �ٷ���PspCidTable���ȫ�ֱ�����ֵ����_HANDLE_TABLE�ĵ�ַ
ULONG GetTableAddress()
{
	UNICODE_STRING functionName;  // ��������	
	PUCHAR pfunction;  // ָ������ʼ����ָ��
	ULONG tableAddress;  // _HANDLE_TABLE��ַ
	PUCHAR p;  // ��ַ����ָ��

	RtlInitUnicodeString(&functionName, L"PsLookupProcessByProcessId");	
	pfunction = (PUCHAR)MmGetSystemRoutineAddress(&functionName);  // ͨ�����������ָ������ʼ��ַ��ָ��
	for (p = pfunction; p < pfunction + PAGE_SIZE; p++)  // PAGE_SIZEΪ4KB
	{
		if((*(PUSHORT)p == 0x35ff) && (*(p + 6) == 0xe8))  // ������
		{
			tableAddress = *(PULONG)(*(PULONG)(p + 2));  // ȫ�ֱ���PspCidTable��ֵ����_HANDLE_TABLE�ĵ�ַ
			return tableAddress;
		}
	}

	return 0;
}


// ͨ����ǰ���̻�ȡ���̶��������
ULONG GetProcessType()
{
	ULONG objectHeaderAddress;  // ����ͷ��ַ
	ULONG type;  // ��������

	objectHeaderAddress = (ULONG)PsGetCurrentProcess();  // ����ָ��ǰ���̵�EPROCESSָ�룬��תΪ��ַ(������)
	objectHeaderAddress = (ULONG)OBJECT_TO_OBJECT_HEADER(objectHeaderAddress);  // �Ӷ������ַת������ͷ��ַ
	type = *(PULONG)(objectHeaderAddress + TYPE);  // ����

	return type;
}


// ʹ�õ��������¼���̶������ַ
VOID RecordProcessAddress(ULONG objectAddress)
{
	PProcessInfo pProcessInfo = NULL;  // ������Ϣ�ṹָ�����
	if(head == NULL)
	{
		head = (PProcessInfo)ExAllocatePool(NonPagedPool, sizeof(ProcessInfo));  // �ڶ�������ָ����С�Ƿ�ҳ�ڴ�
		if(head == NULL)
		{
			return;
		}

		head->address = objectAddress;
		head->next = NULL;
		pCurrent = head;
	}
	else
	{
		pProcessInfo = (PProcessInfo)ExAllocatePool(NonPagedPool, sizeof(ProcessInfo));  // �ڶ�������ָ����С�Ƿ�ҳ�ڴ�
		if(pProcessInfo == NULL)
		{
			return;
		}
		
		pCurrent->next = pProcessInfo;
		pProcessInfo->address = objectAddress;
		pProcessInfo->next = NULL;
		pCurrent = pProcessInfo;
	}
}


// ��1����ʼ����
ULONG BrowseTableL1(ULONG tableAddr)
{
	ULONG objBodyAddr = 0;  // �������ַ����
	ULONG ItemCount = 512;  // һ�����������������
	ULONG objHeaderAddr = 0;  // ����ͷ��ַ
	ULONG flags;

	do 
	{
		objBodyAddr = *(PULONG)tableAddr;  // �õ��������ַ
		objBodyAddr &= 0xFFFFFFF8;  // �������ַ�����λ��0
        tableAddr += 8;  // �Ƶ���һ���ÿ������8�ֽ� 

		if (objBodyAddr == 0)
		{
			continue;
		}

		objHeaderAddr = (ULONG)OBJECT_TO_OBJECT_HEADER(objBodyAddr);  // �������ַת����ͷ��ַ
		if ( GetProcessType() == *(PULONG)(objHeaderAddr + TYPE) )  // �ж϶��������Ƿ�Ϊ����
		{
			flags = *(PULONG)(objBodyAddr + FLAGS);  // ���̶���Ķ��������EPROCESS
			if( (flags & 0x0000000C) != 0x0000000C)  // �жϽ����Ƿ�Ϊ�����ڡ���
			{
				RecordProcessAddress(objBodyAddr);  // ʹ�õ��������¼���̶������ַ	
			}
		}    
	} while (--ItemCount > 0);

	return 0;
}


// ��2����ʼ����
ULONG BrowseTableL2(ULONG tableAddr)
{
	do 
	{
		BrowseTableL1(*(PULONG)tableAddr);  // ת��������
		tableAddr += 4;  // ����2�������һ�����ÿ��4���ֽ�
	} while ((*(PULONG)tableAddr) != 0);

	return 0;
}


// ��3����ʼ����
ULONG BrowseTableL3(ULONG tableAddr)
{
	do 
	{
		BrowseTableL2(*(PULONG)tableAddr);  // ת��2����
		tableAddr += 4;  // ����3�������һ�����ÿ��4���ֽ�
	} while ((*(PULONG)tableAddr) != 0);

	return 0;
}


// ����PspCidTable�����ľ����
VOID EnumPspCidTable()
{
	ULONG tableAddress = GetTableAddress();  // �õ�������ַ
	ULONG tableCode = *(PULONG)tableAddress;  // �õ�tableCode 
	ULONG flag = tableCode & 3;  // �õ���־�������жϼ����
	tableCode &= 0xFFFFFFFC;  // tableCode����λ��0  

	switch (flag)
	{
	case 0:  // һ���
		BrowseTableL1(tableCode);
		break;
	case 1:  // �����
		BrowseTableL2(tableCode);
		break;
	case 2:  // �����
		BrowseTableL3(tableCode);
		break;    
	}
}


// �õ���ǰ����̵�����
int GetProcessNumber()
{
	int count = 0;  // ��������
	// �������̵�����
	for(pCurrent = head; pCurrent != NULL; pCurrent = pCurrent->next)
	{
		count++;
	}

	return count;
}


// �ͷŽ�������
VOID FreeProcessList()
{
	PProcessInfo  p = NULL;
	pCurrent = head;
	p = pCurrent->next;
	while(p != NULL)
	{
		ExFreePool(pCurrent);  // ���ն��ڴ�
		pCurrent = p;
		p = pCurrent->next;
	}
	ExFreePool(pCurrent);  // ���ն��ڴ�
	head = NULL;
	pCurrent = NULL;
}


// ͨ�����̶���ĵ�ַ�õ����̶����ID������
VOID GetProcessInfoByAddr(PProcessInfo buffer)
{
	int i = 0;

	for(pCurrent = head; pCurrent != NULL; pCurrent = pCurrent->next)
	{
		// PID
		if (pCurrent->address == 0)
		{
			pCurrent->pid = 0;
		}
		else if (pCurrent->address == 4)
		{
			pCurrent->pid = 4;
		}
		else
		{
			pCurrent->pid = *(int *)(pCurrent->address + UNIQUEPROCESSID);  
			if (pCurrent->pid < 0)
			{
				pCurrent->pid = 0;
			}
		}
    	
		// ImageFileName
		if (pCurrent->pid == 0)
		{
			PCHAR pStr = "System Idle";
			memcpy(pCurrent->name, pStr, sizeof(pCurrent->name));
		}
		else if (pCurrent->pid == 4)
		{
			PCHAR pStr = "System";
			memcpy(pCurrent->name, pStr, sizeof(pCurrent->name));
		}
		else
		{
			strcpy((PCHAR)pCurrent->name, (PCHAR)(pCurrent->address + IMAGEFILENAME));  
		}

		memcpy(buffer+i, pCurrent, sizeof(ProcessInfo));
		i++;
	}

	FreeProcessList();  // �ͷŽ�������
}


// ����ZwQuerySystemInformation����SystemModuleInformation��ȡntoskrnl.exe��ַ�;����С��Ȼ��������������λ
ULONG GetPspTerminateThreadByPointer()
{
	NTSTATUS status;
	PVOID buffer;
	ULONG code1 = 0x8B55FF8B, code2 = 0x0CEC83EC, code3 = 0xFFF84D83, code4 = 0x7D8B5756;  // PspTerminateThreadByPointer������SP3
	ULONG address;  // PspTerminateThreadByPointer��ַ
	ULONG ntoskrnlBase;  // ntoskrnl.exe���ػ�ַ
	ULONG ntoskrnlEndAddress;  // ntoskrnl.exe������ַ
	ULONG i;  // ����
	PSYSTEM_MODULE_INFORMATION_ENTRY pModule;
	ULONG size;  // ��̬�����С

	// ��̽һ�������ڴ��С��������Ҫsize��С
	ZwQuerySystemInformation(SystemModuleInformation, &size, 0, &size);  
	KdPrint(("[PspTerminateThreadByPointer] size:0x%x\n", size));

	// ������ڴ�
	buffer = ExAllocatePool(NonPagedPool, size);  
	if(buffer == NULL)
	{
		KdPrint(("[PspTerminateThreadByPointer] malloc memory failed\n"));
		return 0;
	}

	// ������ȥȡ��Ϣ
	status = ZwQuerySystemInformation(SystemModuleInformation, buffer, size, 0);
	if(!NT_SUCCESS(status))
	{
		KdPrint(("[PspTerminateThreadByPointer] query failed\n"));
		return status;
	}

	pModule = (PSYSTEM_MODULE_INFORMATION_ENTRY)((PULONG)buffer + 1);
	ntoskrnlBase = (ULONG)pModule->Base;  // �õ�ntoskrnl.exe���ػ�ַ
	ntoskrnlEndAddress = (ULONG)pModule->Base + (ULONG)pModule->Size;  // �õ�ntoskrnl.exe������ַ
	KdPrint(("[PspTerminateThreadByPointer] Size :0x%x\n", (ULONG)pModule->Size));  
	KdPrint(("[PspTerminateThreadByPointer] NtoskrnlBase :0x%x\n", ntoskrnlBase));  
	KdPrint(("[PspTerminateThreadByPointer] NtoskrnlEndAddress :0x%x\n", ntoskrnlEndAddress));

	ExFreePool(buffer);  // �ͷŶ��ڴ�

	// ����PspTerminateThreadByPointer������
	for(i = ntoskrnlBase; i <= ntoskrnlEndAddress; i++)
	{
		if(MmIsAddressValid((PULONG)i) && MmIsAddressValid((PULONG)(i+4)) 
			&& MmIsAddressValid((PULONG)(i+8)) && MmIsAddressValid((PULONG)(i+12)))  // �ж��Ƿ����ȱҳ�ж�
		{
			if((*(PULONG)i == code1) && (*(PULONG)(i+4) == code2) && (*(PULONG)(i+8) == code3) && (*(PULONG)(i+12) == code4))
			{
				address = (ULONG)i;
				KdPrint(("[PspTerminateThreadByPointer] address :0x%x\n", address));
				return address;
			}
		}
	}

	return 0;
}


// ���ָ�����������һ���߳�
PETHREAD GetNextProcessThread(IN PEPROCESS Process, IN PETHREAD Thread OPTIONAL)  
{  
	PETHREAD FoundThread = NULL;  
	PLIST_ENTRY ListHead, Entry;  
	PAGED_CODE();  

	if (Thread)  
	{  
		  //Entry = Thread->ThreadListEntry.Flink;//   +0x22c ThreadListEntry  : _LIST_ENTRY  
		Entry = (PLIST_ENTRY)((ULONG)(Thread) + THREADLISTENTRY);  
		Entry = Entry->Flink;  
	}  
	else 
	{  
		Entry = (PLIST_ENTRY)((ULONG)(Process) + THREADLISTHEAD);//+0x190 ThreadListHead   : _LIST_ENTRY  
		Entry = Entry->Flink;  
	}  
	// ListHead = &rocess->ThreadListHead;  
	ListHead = (PLIST_ENTRY)((ULONG)Process + THREADLISTHEAD);  
	while (ListHead != Entry)  
	{  
		//   FoundThread = CONTAINING_RECORD(Entry, ETHREAD, ThreadListEntry);  
		FoundThread = (PETHREAD)((ULONG)Entry - THREADLISTENTRY);  
		if ( ObReferenceObject(FoundThread) ) 
		{
			break;
		}
		FoundThread = NULL;  
		Entry = Entry->Flink;  
	}  
	if (Thread) 
	{
		ObDereferenceObject(Thread);  
	}

	return FoundThread;  
}  


// ����PspTerminateThreadByPointer��������
NTSTATUS KillProcessWithPsp(ULONG pid)  
{
	PEPROCESS pEProcess;
	PsLookupProcessByProcessId(pid, &pEProcess);  // ���ݽ���PID�õ�PEPROCESS 

	typedef NTSTATUS (*PSPTERMINATETHREADBYPOINTER) (PETHREAD, NTSTATUS);  // ����ָ�����Ͷ���  
	PSPTERMINATETHREADBYPOINTER myPspTerminateThreadByPointer;  // ����ָ�� 

	ULONG address = GetPspTerminateThreadByPointer();  // PspTerminateThreadByPointer�ĵ�ַ
	myPspTerminateThreadByPointer = (PSPTERMINATETHREADBYPOINTER)address;  // ��λPspTerminateThreadByPointer

	NTSTATUS status = STATUS_SUCCESS;  
	PETHREAD pEThread;  

	_try
	{
		// ����ָ��������������߳�
		for ( pEThread = GetNextProcessThread(pEProcess, NULL); pEThread != NULL; pEThread = GetNextProcessThread(pEProcess, pEThread) )  
		{  
			status = (*myPspTerminateThreadByPointer) (pEThread, 0);  // ����ָ����������߳�  
		}
	}
	_except(EXCEPTION_EXECUTE_HANDLER)
	{
		status = GetExceptionCode();
	}

	return status;  
}  


// ͨ��EPROCESS�е�ActiveProcessLinks���������ؽ�����
//
ULONG EnumActiveProcessLinks(PProcessInfo buffer)
{
	KdPrint(("EnumActiveProcessLinks:"));

	ULONG eproc = (ULONG)PsGetCurrentProcess();  // ���̶������ַ
	LONG startPID = *( PLONG(eproc + UNIQUEPROCESSID) );  // ��ʼPID
	LONG currentPID = startPID;  // ��ǰPID
	ULONG count = 0;  // ������
	while(1)
	{
		if((count >= 1)&&(startPID == currentPID))
		{
			break;
		}
		else
		{
			PLIST_ENTRY pListActiveProcs = (PLIST_ENTRY)(eproc + ACTIVEPROCESSLINKS);
			eproc = (ULONG)pListActiveProcs->Flink;
			eproc -= ACTIVEPROCESSLINKS;  // ��һ�����̶������ַ
			RecordProcessAddress(eproc);  // ʹ�õ��������¼���̶������ַ

			currentPID = *( PLONG(eproc + UNIQUEPROCESSID) );
			if (currentPID < 0)
			{
				currentPID = 0;
			}
			KdPrint(("process id %4d,address %8x,image %s", currentPID, eproc, PCHAR(eproc+IMAGEFILENAME) ));

			count++;
		}
	}
	KdPrint(("Total number is %d", count));

	if (buffer != NULL)  // ����̽
	{
		GetProcessInfoByAddr(buffer);  // �õ����̶�����Ϣ������ϵͳ�ṩ�Ļ�����
	}
	else  // ��̽
	{
		FreeProcessList();  // �ͷŽ�������
	}

	return count;
}


// ͨ��EPROCESS�е�ObjectTable���������ؽ�����
//
ULONG EnumObjectTable(PProcessInfo buffer)
{
	KdPrint(("EnumObjectTable:"));
	ULONG eproc = (ULONG)PsGetCurrentProcess();
	PLIST_ENTRY pHandleTableList = (PLIST_ENTRY)(*(PULONG)( (ULONG)eproc + OBJECTTABLE ) + HANDLETABLELIST);
	PLIST_ENTRY pStartList = pHandleTableList;
	ULONG addr;
	int count = 0;
	do 
	{
		int pid = *(PULONG)( (ULONG)pHandleTableList + UNIQUE_PROCESSID - HANDLETABLELIST );  // �õ�PID
		addr = *(PULONG)( (ULONG)pHandleTableList + QUOTAPROCESS - HANDLETABLELIST );  // �õ����̶������ַ
		KdPrint(("process id %4d,address %8x", pid, addr));

		// PIDΪ0��4��ϵͳ�����޷��ɴ��ַ����õ��������ַ
		if (pid == 0)  
		{
			RecordProcessAddress(0);
		}
		else if (pid == 4)
		{
			RecordProcessAddress(4);
		}
		else
		{
			RecordProcessAddress(addr);
		}

		count++;
		pHandleTableList = pHandleTableList->Flink;

	} while(pStartList != pHandleTableList);

	KdPrint(("Total number is %d", count));

	if (buffer != NULL)  // ����̽
	{
		GetProcessInfoByAddr(buffer);  // �õ����̶�����Ϣ������ϵͳ�ṩ�Ļ�����
	}
	else  // ��̽
	{
		FreeProcessList();  // �ͷŽ�������
	}

	return count;
}


/*----------------------------------------------------------------    ��     ��  -------------------------------------------------------------------------*/

NTSTATUS ProcessDriverDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	KdPrint(("Enter ProcessDriverDispatchRoutine\n"));
	NTSTATUS status = STATUS_SUCCESS;
	// ���IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;  // bytes xfered
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint(("Leave ProcessDriverDispatchRoutine\n"));

	return status;
}


NTSTATUS ProcessDriverDispatchDeviceControl(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	KdPrint(("Enter ProcessDriverDeviceIOControl\n"));

	NTSTATUS status = STATUS_SUCCESS;
   
    PVOID ioBuf = pIrp->AssociatedIrp.SystemBuffer;  // ϵͳ�ṩ�Ļ�����

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);  // �õ���ǰ��ջ
    ULONG inBufLength = stack->Parameters.DeviceIoControl.InputBufferLength;  // �õ����뻺������С
    ULONG outBufLength = stack->Parameters.DeviceIoControl.OutputBufferLength;  // �õ������������С
    ULONG ioControlCode = stack->Parameters.DeviceIoControl.IoControlCode;  // �õ�IOCTL��

    switch (ioControlCode)
    {
	case IOCTL_PROCESSDRIVER_PSPCIDTABLE_GETPROCESSNUMBER:
		{
			EnumPspCidTable();
			*(int *)ioBuf = GetProcessNumber();  // �õ�������Ŀ����ֵ����ϵͳ�ṩ�Ļ�����
			pIrp->IoStatus.Information = sizeof(int);
            break;
		}
	case IOCTL_PROCESSDRIVER_PSPCIDTABLE_GETPROCESSINFO:
		{
			GetProcessInfoByAddr((PProcessInfo)ioBuf);  // �õ����̶�����Ϣ������ϵͳ�ṩ�Ļ�����
			pIrp->IoStatus.Information = inBufLength;
            break;
		}
	case IOCTL_PROCESSDRIVER_TERMINATEPROCESS:
		{
			KillProcessWithPsp(*(PULONG)ioBuf);
			pIrp->IoStatus.Information = 0;
            break;
		}
	case IOCTL_PROCESSDRIVER_ACTIVEPROCESSLINKS:
		{	
			pIrp->IoStatus.Information = EnumActiveProcessLinks((PProcessInfo)ioBuf) * sizeof(ProcessInfo);  // ���������ڴ�
			break;
		}
	case IOCTL_PROCESSDRIVER_OBJECTTABLE:
		{
			pIrp->IoStatus.Information = EnumObjectTable((PProcessInfo)ioBuf) * sizeof(ProcessInfo);
			break;
		}
    default:
        status = STATUS_INVALID_PARAMETER;
		pIrp->IoStatus.Information = 0;
        KdPrint(("[ProcessDriver] unknown IRP_MJ_DEVICE_CONTROL \n"));
	}

	// ���IRP
    pIrp->IoStatus.Status = status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	KdPrint(("Leave ProcessDriverDeviceIOControl\n"));

    return status;
}


VOID ProcessDriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("Enter DriverUnload\n"));

    UNICODE_STRING symLinkName;
    RtlInitUnicodeString(&symLinkName, L"\\??\\ProcessDriver");

    IoDeleteSymbolicLink(&symLinkName);  // ɾ����������
    IoDeleteDevice(pDriverObject->DeviceObject);  // ɾ���豸
}


NTSTATUS CreateDevice(IN PDRIVER_OBJECT	pDriverObject) 
{
	// �����豸����
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\ProcessDriver");
	
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
	RtlInitUnicodeString(&symLinkName, L"\\??\\ProcessDriver");
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
	pDriverObject->DriverUnload = ProcessDriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = ProcessDriverDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = ProcessDriverDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProcessDriverDispatchDeviceControl;

	NTSTATUS status = CreateDevice(pDriverObject);  // ���������豸����

	KdPrint(("DriverEntry end\n"));
	return status;
}
