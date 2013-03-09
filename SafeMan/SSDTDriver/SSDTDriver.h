#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include <NTDDK.h>
#ifdef __cplusplus
}
#endif 


// IOCTLs

// �õ�SSDT
#define IOCTL_SSDTDRIVER_GETSSDT CTL_CODE(\
			FILE_DEVICE_UNKNOWN, \
			0x810, \
			METHOD_BUFFERED, \
			FILE_ANY_ACCESS)

// ͨ�������õ�������ַ
#define IOCTL_SSDTDRIVER_GETFUNADDRBYINDEX CTL_CODE(\
	       FILE_DEVICE_UNKNOWN, \
	       0x811, \
	       METHOD_BUFFERED, \
	       FILE_ANY_ACCESS)

// �޸�SSDT
#define IOCTL_SSDTDRIVER_RESTOREFUNADDR CTL_CODE(\
	       FILE_DEVICE_UNKNOWN, \
	       0x812, \
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


// ����SSDT��Ϣ
typedef struct _RESTORE_INFO {
	ULONG ulIndex;  // �����
	ULONG ulAddr;  // ����������ʵ��ַ
} RESTORE_INFO, *PRESTORE_INFO;


extern "C" PSSDT KeServiceDescriptorTable;  // KeServiceDescriptorTableȫ�ֱ�������Ԯ