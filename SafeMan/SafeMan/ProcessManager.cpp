// ProcessManager.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SafeMan.h"
#include "ProcessManager.h"
#include "tlhelp32.h"
#include "shlwapi.h"
#include "psapi.h"
#pragma comment(lib, "Psapi.Lib")
#include "DriverManager.h"
#include <winioctl.h>
#include "Ioctls.h"

// CProcessManager �Ի���

IMPLEMENT_DYNAMIC(CProcessManager, CDialog)

CProcessManager::CProcessManager(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessManager::IDD, pParent)
{
	 if( CDriverManager::LoadNTDriver(_T("ProcessDriver"), _T("..\\ProcessDriver\\objchk\\i386\\ProcessDriver.sys")) == FALSE)
	 //if( CDriverManager::LoadNTDriver(_T("ProcessDriver"), _T("ProcessDriver.sys")) == FALSE)
	 {
		 MessageBox(_T("��������ʧ�ܣ�"));
		 CDriverManager::UnloadNTDriver(_T("ProcessDriver"));  // ж������ 
	 }
}

CProcessManager::~CProcessManager()
{
	CDriverManager::UnloadNTDriver(_T("ProcessDriver"));  // ж������   
}

void CProcessManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_ListProcess);
	//DDX_Control(pDX, IDC_BUTTON_LISTPROCESS_RING3, m_ListProcessByRing3);
	//DDX_Control(pDX, IDC_BUTTON_LISTPROCESS_RING0, m_ListProcessByRing0);
}


BEGIN_MESSAGE_MAP(CProcessManager, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_LISTPROCESS_RING3, &CProcessManager::OnBnClickedButtonListprocessRing3)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS, &CProcessManager::OnNMRclickListProcess)
	ON_BN_CLICKED(IDC_BUTTON_LISTPROCESS_RING0, &CProcessManager::OnBnClickedButtonPspcidtable)
	ON_COMMAND(ID_MENU_TERMINATE_PROCESS_RING0, &CProcessManager::OnMenuTerminateProcessRing0)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_PROCESS, &CProcessManager::OnNMCustomdrawListProcess)
	ON_BN_CLICKED(IDC_BUTTON_ACTIVE_PROCESS_LINKS, &CProcessManager::OnBnClickedButtonActiveProcessLinks)
	ON_BN_CLICKED(IDC_BUTTON_OBJECT_TABLE, &CProcessManager::OnBnClickedButtonObjectTable)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CProcessManager ��Ϣ�������


// ��ʼ���Ի���
//
BOOL CProcessManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	
	// �����б�������
	m_ListProcess.InsertColumn(0, _T("������"), LVCFMT_LEFT, 155);
	m_ListProcess.InsertColumn(1, _T("PID"), LVCFMT_LEFT, 80);
	m_ListProcess.InsertColumn(2, _T("����·��"), LVCFMT_LEFT, 315);

	m_ListProcess.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);  // �����б��ÿ��ȫѡ�Ҵ�������  

	m_brush.CreateSolidBrush(RGB(255,255,255));  // ������ɫ��ˢ

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


// �ı�Ի���Ϳؼ�����ɫ
//
HBRUSH CProcessManager::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	//return hbr;
	if(nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(RGB(255,255,255));  // ���ÿؼ�����ɫ����ɫ��
		return m_brush;  // �����Զ���Ļ�ˢ���
	}
	return hbr;  // ����Ļ�����ϵͳĬ�ϵĻ�ˢ���
}


// Ring3�оٽ��̰�ť�¼�
//
void CProcessManager::OnBnClickedButtonListprocessRing3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_ListProcess.DeleteAllItems();  // ɾ���б�������

	HANDLE hSnapshotProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // �������գ���ȡ���н���
	if (hSnapshotProcess == INVALID_HANDLE_VALUE)
	{
		return;
	}

	PROCESSENTRY32 processInfo;  // ������Ϣ�ṹ��
	processInfo.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hProcess;  // ���̾��
	CString path;  // ��������·��
	CString processID;  // PID
	DWORD dwRet;  // ����ֵ

	INT i = 0;
	BOOL bRet = FALSE;
	// ���������е����н���
	for (bRet = Process32First(hSnapshotProcess, &processInfo); bRet; bRet = Process32Next(hSnapshotProcess, &processInfo), i++)
	{
		AdjustPurview();  // ����Ȩ�ޣ���Ҫ��Ϊ��OpenProcess�ɹ�����Handle
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processInfo.th32ProcessID);  // ���ݽ���ID�򿪽��̣����ؽ��̾��

		dwRet =GetModuleFileNameEx(hProcess, NULL, path.GetBuffer(MAX_PATH), MAX_PATH);  // ͨ�����̾���������·��
		if(dwRet == 0)
		{
			path = _T("·����ȡʧ��");
		}

		int iRow = m_ListProcess.InsertItem(i, processInfo.szExeFile);  // ���̵Ŀ�ִ���ļ�����
		processID.Format(_T("%u"), processInfo.th32ProcessID);  // DWORDתCString
		m_ListProcess.SetItemText(iRow, 1, processID);  // PID 
		m_ListProcess.SetItemText(iRow, 2, path);  // ��������·�� 

		CloseHandle(hProcess);  // �رս��̾��
	}
}


// // ͨ��Pspcidtable�оٽ���
//
void CProcessManager::OnBnClickedButtonPspcidtable()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_ListProcess.DeleteAllItems();  // ɾ���б�������

	// ���豸
	HANDLE hDevice = CreateFile(
		_T("\\\\.\\ProcessDriver"), 
		GENERIC_WRITE | GENERIC_READ, 
		0, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL);  
	if(hDevice == INVALID_HANDLE_VALUE)  
	{
		MessageBox(_T("���豸ʧ�ܣ�"));  
		return;
	}

	// ��ȡ������Ŀ
	DWORD dwProcessNumber = 0;
	DWORD dwDeviceRet = 0;
	BOOL bRet = DeviceIoControl(
		hDevice,       
		IOCTL_PROCESSDRIVER_PSPCIDTABLE_GETPROCESSNUMBER, 
		&dwProcessNumber,
		sizeof(DWORD),        
		&dwProcessNumber,                  
		sizeof(DWORD),                       
		&dwDeviceRet,  
		0);
	if(bRet == FALSE)
	{
		MessageBox(_T("��ȡ������Ŀʧ�ܣ�"));
		CloseHandle(hDevice);  // �ر��豸
		return;
	}

	// ��ȡ������Ϣ
	PVOID pProcessInfo = (PVOID)new BYTE[dwProcessNumber * sizeof(ProcessInfo)];  // ����ָ����С�������ڴ���ÿһ�����̶�����Ϣ
	memset(pProcessInfo, 0, dwProcessNumber * sizeof(ProcessInfo));  // ��ʼ���ڴ�
	bRet = DeviceIoControl(
		hDevice,       
		IOCTL_PROCESSDRIVER_PSPCIDTABLE_GETPROCESSINFO,               
		pProcessInfo,           
		dwProcessNumber*sizeof(ProcessInfo),        
		pProcessInfo,                         
		dwProcessNumber*sizeof(ProcessInfo),                         
		&dwDeviceRet,               
		0);
	if(bRet == FALSE)
	{
		MessageBox(_T("��ȡ������Ϣʧ�ܣ�"));
		CloseHandle(hDevice);  // �ر��豸
		return;
	}
                         
	CloseHandle(hDevice);  // �ر��豸


	HANDLE hProcess;  // ���̾��
	CString path;  // ��������·��
	CString processID;  // PID
	DWORD dwRet;  // ����ֵ
	ProcessInfo processInfo;  // ������Ϣ�ṹ��	
	for (DWORD i = 0; i < dwProcessNumber; i++)
	{
		processInfo = *((PProcessInfo)(pProcessInfo) + i);  // ���������ڴ�������ȡ��ÿ�����̶���
		AdjustPurview();  // ����Ȩ�ޣ���Ҫ��Ϊ��OpenProcess�ɹ�����Handle
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processInfo.pid);  // ���ݽ���ID�򿪽��̣����ؽ��̾��

		dwRet = GetModuleFileNameEx(hProcess, NULL, path.GetBuffer(MAX_PATH), MAX_PATH);  // ͨ�����̾���������·��
		if(dwRet == 0)
		{
			path = _T("·����ȡʧ��");
		}

		CString exeName(processInfo.name);  // ���̵Ŀ�ִ���ļ�����
		int iRow = m_ListProcess.InsertItem(i, exeName);
		processID.Format(_T("%u"), processInfo.pid);  // ULONGתCString
		m_ListProcess.SetItemText(iRow, 1, processID);  // PID  
		m_ListProcess.SetItemText(iRow, 2, path);  // ��������·�� 

		CloseHandle(hProcess);  // �رս��̾��
	}

	delete pProcessInfo;  // �ͷ��ڴ�
}


// �б�򵥻��Ҽ��¼�
//
void CProcessManager::OnNMRclickListProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1 && pNMListView->iSubItem != -1)
	{ 
		CPoint pt;
		GetCursorPos(&pt);
		CMenu menu;
		menu.LoadMenu(IDR_MENU_PROCESS);
		CMenu* pMenu = menu.GetSubMenu(0);
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
	}

	*pResult = 0;
}


// Ring0��������
//
void CProcessManager::OnMenuTerminateProcessRing0()
{
	// TODO: �ڴ���������������

	int index = m_ListProcess.GetNextItem(-1, LVNI_SELECTED);  // �õ�ѡ�������к�
	CString strProcessID = m_ListProcess.GetItemText(index, 1);  // �����кź��к�ȡ��PID
	DWORD dwProcessID = _ttol(strProcessID);  // CStringתDWORD 


	// ���豸
	HANDLE hDevice = CreateFile(
		_T("\\\\.\\ProcessDriver"), 
		GENERIC_WRITE | GENERIC_READ, 
		0, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL);  
	if(hDevice == INVALID_HANDLE_VALUE)  
	{
		MessageBox(_T("���豸ʧ�ܣ�"));
		return;
	}

	// ��������
	DWORD dwDeviceRet = 0;
	BOOL bRet = DeviceIoControl(
		hDevice,      
		IOCTL_PROCESSDRIVER_TERMINATEPROCESS,         
		&dwProcessID,          
		sizeof(DWORD),         
		NULL,                          
		0,                             
		&dwDeviceRet,                
		0); 
	if(bRet == FALSE)
	{
		MessageBox(_T("��������ʧ�ܣ�"));
	}

	CloseHandle(hDevice);  // �ر��豸	
}


// ����Ȩ�ޣ���Ҫ��Ϊ��OpenProcess�ɹ�����Handle
//
BOOL CProcessManager::AdjustPurview()
{
	TOKEN_PRIVILEGES TokenPrivileges;
	bool bRet;
	HANDLE hToken;

	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &TokenPrivileges.Privileges[0].Luid);   
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);

	TokenPrivileges.PrivilegeCount = 1;   
	TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	bRet = !!AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, 0, NULL, NULL);

	CloseHandle(hToken);
	return bRet ;
}


// �����б�
//
void CProcessManager::OnNMCustomdrawListProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{
		COLORREF clrNewBkColor;

		int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);

		// ���ñ���ɫ
		if (nItem%2 ==0)
		{
			clrNewBkColor = RGB(240, 240, 240);	// ż���б���ɫΪ��ɫ
		}
		else
		{
			clrNewBkColor = RGB(255, 255, 255);	// �����б���ɫΪ��ɫ
		}

		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}


// ͨ��ActiveProcessLinks�оٽ���
//
void CProcessManager::OnBnClickedButtonActiveProcessLinks()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_ListProcess.DeleteAllItems();  // ɾ���б�������

	// ���豸
	HANDLE hDevice = CreateFile(
		_T("\\\\.\\ProcessDriver"), 
		GENERIC_WRITE | GENERIC_READ, 
		0, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL);  
	if(hDevice == INVALID_HANDLE_VALUE)  
	{
		MessageBox(_T("���豸ʧ�ܣ�"));  
		return;
	}

	// ��̽��������ڴ��С
	DWORD dwDeviceRet = 0;
	BOOL bRet = DeviceIoControl(
		hDevice,       
		IOCTL_PROCESSDRIVER_ACTIVEPROCESSLINKS,               
		NULL,           
		0,        
		NULL,                         
		0,                         
		&dwDeviceRet,               
		0);
	if (bRet == FALSE)
	{
		MessageBox(_T("��ȡ������Ϣʧ�ܣ�"));
		CloseHandle(hDevice);  // �ر��豸
		return;
	}

	PVOID pProcessInfo = (PVOID)new BYTE[dwDeviceRet];  // ����ָ����С�������ڴ���ÿһ�����̶�����Ϣ
	memset(pProcessInfo, 0, dwDeviceRet);  // ��ʼ���ڴ�
	
	// ������ȥȡ������Ϣ
	bRet = DeviceIoControl(
		hDevice,       
		IOCTL_PROCESSDRIVER_ACTIVEPROCESSLINKS,               
		pProcessInfo,           
		dwDeviceRet,        
		pProcessInfo,                         
		dwDeviceRet,                         
		&dwDeviceRet,               
		0);
	if (bRet == FALSE)
	{
		MessageBox(_T("��ȡ������Ϣʧ�ܣ�"));
		CloseHandle(hDevice);  // �ر��豸
		return;
	}

	CloseHandle(hDevice);  // �ر��豸

	HANDLE hProcess;  // ���̾��
	CString path;  // ��������·��
	CString processID;  // PID
	DWORD dwRet;  // ����ֵ
	ProcessInfo processInfo;  // ������Ϣ�ṹ��	
	for (DWORD i = 0; i < dwDeviceRet/sizeof(ProcessInfo); i++)
	{
		processInfo = *((PProcessInfo)(pProcessInfo) + i);  // ���������ڴ�������ȡ��ÿ�����̶���
		AdjustPurview();  // ����Ȩ�ޣ���Ҫ��Ϊ��OpenProcess�ɹ�����Handle
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processInfo.pid);  // ���ݽ���ID�򿪽��̣����ؽ��̾��

		dwRet = GetModuleFileNameEx(hProcess, NULL, path.GetBuffer(MAX_PATH), MAX_PATH);  // ͨ�����̾���������·��
		if(dwRet == 0)
		{
			path = _T("·����ȡʧ��");
		}

		CString exeName(processInfo.name);  // ���̵Ŀ�ִ���ļ�����
		int iRow = m_ListProcess.InsertItem(i, exeName);
		processID.Format(_T("%u"), processInfo.pid);  // ULONGתCString
		m_ListProcess.SetItemText(iRow, 1, processID);  // PID  
		m_ListProcess.SetItemText(iRow, 2, path);  // ��������·�� 

		CloseHandle(hProcess);  // �رս��̾��
	}

	delete pProcessInfo;  // �ͷ��ڴ�
}


// ͨ��ObjectTable�оٽ���
//
void CProcessManager::OnBnClickedButtonObjectTable()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_ListProcess.DeleteAllItems();  // ɾ���б�������

	// ���豸
	HANDLE hDevice = CreateFile(
		_T("\\\\.\\ProcessDriver"), 
		GENERIC_WRITE | GENERIC_READ, 
		0, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL);  
	if(hDevice == INVALID_HANDLE_VALUE)  
	{
		MessageBox(_T("���豸ʧ�ܣ�"));  
		return;
	}

	// ��̽��������ڴ��С
	DWORD dwDeviceRet = 0;
	BOOL bRet = DeviceIoControl(
		hDevice,       
		IOCTL_PROCESSDRIVER_OBJECTTABLE,               
		NULL,           
		0,        
		NULL,                         
		0,                         
		&dwDeviceRet,               
		0);
	if (bRet == FALSE)
	{
		MessageBox(_T("��ȡ������Ϣʧ�ܣ�"));
		CloseHandle(hDevice);  // �ر��豸
		return;
	}

	PVOID pProcessInfo = (PVOID)new BYTE[dwDeviceRet];  // ����ָ����С�������ڴ���ÿһ�����̶�����Ϣ
	memset(pProcessInfo, 0, dwDeviceRet);  // ��ʼ���ڴ�

	// ������ȥȡ������Ϣ
	bRet = DeviceIoControl(
		hDevice,       
		IOCTL_PROCESSDRIVER_OBJECTTABLE,               
		pProcessInfo,           
		dwDeviceRet,        
		pProcessInfo,                         
		dwDeviceRet,                         
		&dwDeviceRet,               
		0);
	if (bRet == FALSE)
	{
		MessageBox(_T("��ȡ������Ϣʧ�ܣ�"));
		CloseHandle(hDevice);  // �ر��豸
		return;
	}

	CloseHandle(hDevice);  // �ر��豸

	HANDLE hProcess;  // ���̾��
	CString path;  // ��������·��
	CString processID;  // PID
	DWORD dwRet;  // ����ֵ
	ProcessInfo processInfo;  // ������Ϣ�ṹ��	
	for (DWORD i = 0; i < dwDeviceRet/sizeof(ProcessInfo); i++)
	{
		processInfo = *((PProcessInfo)(pProcessInfo) + i);  // ���������ڴ�������ȡ��ÿ�����̶���
		AdjustPurview();  // ����Ȩ�ޣ���Ҫ��Ϊ��OpenProcess�ɹ�����Handle
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processInfo.pid);  // ���ݽ���ID�򿪽��̣����ؽ��̾��

		dwRet = GetModuleFileNameEx(hProcess, NULL, path.GetBuffer(MAX_PATH), MAX_PATH);  // ͨ�����̾���������·��
		if(dwRet == 0)
		{
			path = _T("·����ȡʧ��");
		}

		CString exeName(processInfo.name);  // ���̵Ŀ�ִ���ļ�����
		int iRow = m_ListProcess.InsertItem(i, exeName);
		processID.Format(_T("%u"), processInfo.pid);  // ULONGתCString
		m_ListProcess.SetItemText(iRow, 1, processID);  // PID  
		m_ListProcess.SetItemText(iRow, 2, path);  // ��������·�� 

		CloseHandle(hProcess);  // �رս��̾��
	}

	delete pProcessInfo;  // �ͷ��ڴ�
}


// ���������ʽ
//
BOOL CProcessManager::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CString strClassName;
	GetClassName(pWnd->GetSafeHwnd(), strClassName.GetBuffer(80), 80);

	if (strClassName == "Button")  // ��ť����
	{
		SetCursor( LoadCursor(NULL, IDC_HAND) );  // �����ʽΪ����
		return TRUE;
	}
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}
