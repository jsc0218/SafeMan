#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include <ntifs.h>
#ifdef __cplusplus
}
#endif 

// IOCTLs

// �������̼��
#define IOCTL_PROCESS_MONITORDRIVER_ON CTL_CODE(\
			FILE_DEVICE_UNKNOWN, \
			0x820, \
			METHOD_BUFFERED, \
			FILE_ANY_ACCESS)

// �رս��̼��
#define IOCTL_PROCESS_MONITORDRIVER_OFF CTL_CODE(\
	       FILE_DEVICE_UNKNOWN, \
	       0x821, \
	       METHOD_BUFFERED, \
	       FILE_ANY_ACCESS)

// �豸��չ
typedef struct _DEVICE_EXTENSION {
	ULONG stateVariable;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// SSDT�ṹ
typedef struct _tagSSDT {
	PVOID pvSSDTBase;  // SSDT����ַ
	PVOID pvServiceCounterTable;  // ָ����һ�����ñ������ÿ�����������õĴ���
	ULONG ulNumberOfServices;  // ��ǰϵͳ��֧�ֵķ������
	PVOID pvParamTableBase;  // ָ��ϵͳ����������ñ������ÿ����������Ĳ����ֽ���
} SSDT, *PSSDT;

extern "C" PSSDT KeServiceDescriptorTable;  // KeServiceDescriptorTableȫ�ֱ�������Ԯ

BOOLEAN pProcessMonitor= FALSE;   //�Ƿ������̼�أ�Ĭ��Ϊδ����
char *chInteractionBuffer;	//�������û��Ľ���������
ULONG ulIndex;	//ZwCreateProcessEx��SSDT�е�����ֵ
KEVENT event ; 
ULONG ulProcessNameOffset;
ULONG ulProcessManager = 0;

#define BOOL unsigned long
#define PROCNAMELEN     50
#define MAXPATHLEN 256
#define NT_PROCNAMELEN  50

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
							  );

typedef NTSTATUS (*ZWCREATEPROCESSEX)(
									OUT PHANDLE ProcessHandle,
									IN ACCESS_MASK DesiredAccess,
									IN POBJECT_ATTRIBUTES ObjectAttributes,
									IN HANDLE InheritFromProcessHandle,
									IN BOOLEAN InheritHandles,
									IN HANDLE SectionHandle OPTIONAL,
									IN HANDLE DebugPort OPTIONAL,
									IN HANDLE ExceptionPort OPTIONAL,
									IN HANDLE Unknown 
									);

ZWCREATEPROCESSEX RealZwCreateProcess;