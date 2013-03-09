#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// SSDT�ṹ
typedef struct _tagSSDT {
	PVOID pvSSDTBase;  // SSDT����ַ
	PVOID pvServiceCounterTable;  // ָ����һ�����ñ������ÿ�����������õĴ���
	ULONG ulNumberOfServices;  // ��ǰϵͳ��֧�ֵķ������
	PVOID pvParamTableBase;  // ָ��ϵͳ����������ñ������ÿ����������Ĳ����ֽ���
} SSDT, *PSSDT;


// SYSTEM_INFORMATION_CLASS��SystemModuleInformation�Ľṹ����
typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY {
	HANDLE Section;
	PVOID MappedBase;
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT PathLength;  // ���һ��б�ܵ�λ��
	CHAR ImageName[256];  // ������·��
} SYSTEM_MODULE_INFORMATION_ENTRY, *PSYSTEM_MODULE_INFORMATION_ENTRY;


typedef struct _SYSTEM_MODULE_INFORMATION {
	ULONG Count;
	SYSTEM_MODULE_INFORMATION_ENTRY Module[1];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;


typedef enum _SYSTEM_INFORMATION_CLASS     //    Q S
{
	SystemBasicInformation,                // 00 Y N
	SystemProcessorInformation,            // 01 Y N
	SystemPerformanceInformation,          // 02 Y N
	SystemTimeOfDayInformation,            // 03 Y N
	SystemNotImplemented1,                 // 04 Y N
	SystemProcessesAndThreadsInformation,  // 05 Y N
	SystemCallCounts,                      // 06 Y N
	SystemConfigurationInformation,        // 07 Y N
	SystemProcessorTimes,                  // 08 Y N
	SystemGlobalFlag,                      // 09 Y Y
	SystemNotImplemented2,                 // 10 Y N
	SystemModuleInformation,               // 11 Y N
	SystemLockInformation,                 // 12 Y N
	SystemNotImplemented3,                 // 13 Y N
	SystemNotImplemented4,                 // 14 Y N
	SystemNotImplemented5,                 // 15 Y N
	SystemHandleInformation,               // 16 Y N
	SystemObjectInformation,               // 17 Y N
	SystemPagefileInformation,             // 18 Y N
	SystemInstructionEmulationCounts,      // 19 Y N
	SystemInvalidInfoClass1,               // 20
	SystemCacheInformation,                // 21 Y Y
	SystemPoolTagInformation,              // 22 Y N
	SystemProcessorStatistics,             // 23 Y N
	SystemDpcInformation,                  // 24 Y Y
	SystemNotImplemented6,                 // 25 Y N
	SystemLoadImage,                       // 26 N Y
	SystemUnloadImage,                     // 27 N Y
	SystemTimeAdjustment,                  // 28 Y Y
	SystemNotImplemented7,                 // 29 Y N
	SystemNotImplemented8,                 // 30 Y N
	SystemNotImplemented9,                 // 31 Y N
	SystemCrashDumpInformation,            // 32 Y N
	SystemExceptionInformation,            // 33 Y N
	SystemCrashDumpStateInformation,       // 34 Y Y/N
	SystemKernelDebuggerInformation,       // 35 Y N
	SystemContextSwitchInformation,        // 36 Y N
	SystemRegistryQuotaInformation,        // 37 Y Y
	SystemLoadAndCallImage,                // 38 N Y
	SystemPrioritySeparation,              // 39 N Y
	SystemNotImplemented10,                // 40 Y N
	SystemNotImplemented11,                // 41 Y N
	SystemInvalidInfoClass2,               // 42
	SystemInvalidInfoClass3,               // 43
	SystemTimeZoneInformation,             // 44 Y N
	SystemLookasideInformation,            // 45 Y N
	SystemSetTimeSlipEvent,                // 46 N Y
	SystemCreateSession,                   // 47 N Y
	SystemDeleteSession,                   // 48 N Y
	SystemInvalidInfoClass4,               // 49
	SystemRangeStartInformation,           // 50 Y N
	SystemVerifierInformation,             // 51 Y Y
	SystemAddVerifier,                     // 52 N Y
	SystemSessionProcessesInformation      // 53 Y N
} SYSTEM_INFORMATION_CLASS;


// mov eax, <ServiceIndex> 
// B8 <ServiceIndex> 
#pragma pack (push, 1)  // �������뷽ʽΪ1�ֽ�
typedef struct _tagSSDTEntry {
	BYTE byMov;  // 0xB8
	DWORD dwIndex;
} SSDTENTRY;
#pragma pack (pop)


#define MOV 0xB8


// IMAGE_BASE_RELOCATION.TypeOffset�ṹ
typedef struct { 
	WORD offset:12;
	WORD type:4;
} IMAGE_FIXUP_ENTRY, *PIMAGE_FIXUP_ENTRY;


// ����SSDT��Ϣ
typedef struct _RESTORE_INFO {
	ULONG ulIndex;  // �����
	ULONG ulAddr;  // ����������ʵ��ַ
} RESTORE_INFO, *PRESTORE_INFO;



// CSSDTManager �Ի���

class CSSDTManager : public CDialog
{
	DECLARE_DYNAMIC(CSSDTManager)

public:
	CSSDTManager(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSSDTManager();

// �Ի�������
	enum { IDD = IDD_SSDTMANAGER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();  // ��ʼ���Ի���
	// �ı�Ի���Ϳؼ�����ɫ
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonSsdtRepairSelected();  // SSDT�޸�ѡ���ť�¼�
	afx_msg void OnStnClickedStaticCheck();  // ȫѡ��ť�¼�
	afx_msg void OnStnClickedStaticUncheck();  // ȫ��ѡ��ť�¼�
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);  // ���������ʽ
	afx_msg void OnNMCustomdrawListSsdt(NMHDR *pNMHDR, LRESULT *pResult);  // �����б�
	afx_msg void OnBnClickedButtonSsdtRefresh();  // ˢ�°�ť�¼�
private:
	VOID EnumSSDT();  // ����SSDT
	BOOL GetSSDT(IN HANDLE hDevice, OUT PSSDT pSSDT);  // �õ�SSDT
	BOOL GetFunAddrByIndex(IN HANDLE hDevice, IN ULONG ulIndex, OUT PULONG pFunAddr);  // ͨ�������õ�������ַ
	PSYSTEM_MODULE_INFORMATION GetModuleList(IN HMODULE hNtDll);  // �õ�ϵͳģ��
	CString GetModuleByAddr(IN ULONG ulAddr, IN PSYSTEM_MODULE_INFORMATION pList);  // ͨ����ַ�ҵ�����ģ��
	VOID GetOrigFunAddr();  // �õ���������ԭʼ��ַ
	DWORD GetKiServiceTable(HMODULE hModule, DWORD dwKSDT);  // �õ�KiServiceTable��RVA  
	BOOL GetPEHeaders(HMODULE hModule, PIMAGE_FILE_HEADER& pfh,
		PIMAGE_OPTIONAL_HEADER& poh, PIMAGE_SECTION_HEADER& psh);  // �õ�PE�ļ���ĸ���ͷ
	BOOL RestoreSSDTItem(IN HANDLE hDevice, IN ULONG ulIndex);  // ����SSDT��
	
private:
	CBrush m_brush;  // �Զ��廭ˢ
	CListCtrl m_ListSSDT;  // SSDT�б��
	CButton m_ButRepairSel;  //�޸�ѡ���ť
};
