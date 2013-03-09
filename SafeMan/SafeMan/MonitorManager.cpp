// MonitorManager.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SafeMan.h"
#include "MonitorManager.h"
#include "DriverManager.h"
#include <winioctl.h>
#include "Ioctls.h"

// CMonitorManager �Ի���
char static chInteractionBuffer[264];			//�䵱���ں˽��н����Ĺ��û�����
DWORD static dwControlBuffer[2];	//dwControlBuffer[0]�д��ZwCreateProcessEx�����ĵ�ַ
												//dwControlBuffer[1]�д��chInteractionBuffer�������ʼ��ַ
void static ThreadMonitor(LPVOID lpParam);

struct ThreadInfo
{
	CStatic *p_Count;
};

int iCount = 0;	//����

IMPLEMENT_DYNAMIC(CMonitorManager, CDialog)

CMonitorManager::CMonitorManager(CWnd* pParent /*=NULL*/)
	: CDialog(CMonitorManager::IDD, pParent)
{
	if( CDriverManager::LoadNTDriver(_T("MONITORDriver"), _T("..\\MonitorDriver\\objchk\\i386\\MonitorDriver.sys")) == FALSE)
	//if( CDriverManager::LoadNTDriver(_T("MONITORDriver"), _T("MonitorDriver.sys")) == FALSE)
	{
		MessageBox(_T("��������ʧ�ܣ�"));
		CDriverManager::UnloadNTDriver(_T("MONITORDriver"));  // ж������ 
	}

	//��ʼ��
	m_pProcessMonitor = 0;
	DWORD RealZwCreateProcessAddress = GetZwCreateProcessAddress();		//�õ�ʵ�ʵ�ַ
	ZeroMemory(dwControlBuffer,2);
	dwControlBuffer[0] = RealZwCreateProcessAddress;		//�����dwControlBuffer[0]
	ZeroMemory(chInteractionBuffer,264);		//���
	dwControlBuffer[1] = (DWORD)&chInteractionBuffer[0];	//���׵�ַ�����dwControlBuffer[1]��

	ThreadInfo *m_ThreadInfo = new ThreadInfo();
	m_ThreadInfo->p_Count = &m_Count;

	DWORD dwThreadId;
	//���������߳�
	::CreateThread(	0,
							0,
							(LPTHREAD_START_ROUTINE)ThreadMonitor,
							m_ThreadInfo,
							0,
							&dwThreadId);
}

CMonitorManager::~CMonitorManager()
{
	CDriverManager::UnloadNTDriver(_T("MONITORDriver"));  // ж������
}

void CMonitorManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROCESSMONITORCONTROL, m_ButtonProcessMonitor);
	DDX_Control(pDX, IDC_ACCUMULATION, m_Count);
	DDX_Control(pDX, IDC_STATUS, m_Status);
}


BEGIN_MESSAGE_MAP(CMonitorManager, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_PROCESSMONITORCONTROL, &CMonitorManager::OnBnClickedProcessmonitorcontrol)
END_MESSAGE_MAP()


// CMonitorManager ��Ϣ�������

HBRUSH CMonitorManager::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	if(nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(RGB(255,255,255));  // ���ÿؼ�����ɫ����ɫ��

		if (pWnd->GetDlgCtrlID() == IDC_ACCUMULATION)
		{
			pDC->SetTextColor(RGB(255,0,0));
		}

		return m_brush;  // �����Զ���Ļ�ˢ���
	}
	return hbr;
}

BOOL CMonitorManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	m_brush.CreateSolidBrush(RGB(255,255,255));  // ������ɫ��ˢ

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CMonitorManager::OnBnClickedProcessmonitorcontrol()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	// ���豸
	HANDLE hDevice = CreateFile(
		_T("\\\\.\\MONITORDriver"), 
		GENERIC_WRITE | GENERIC_READ, 
		0, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL);  
	if (hDevice == INVALID_HANDLE_VALUE)  
	{
		MessageBox(_T("���豸ʧ�ܣ�"));  
		return;
	}

	DWORD dwDeviceRet = 0;
	//�������̼��
	if (m_pProcessMonitor == 0)
	{
		ULONG input;
		
		DWORD RealZwCreateProcessAddress = GetZwCreateProcessAddress();
		BOOL bRet = DeviceIoControl(
			hDevice,
			IOCTL_PROCESS_MONITORDRIVER_ON, 
			dwControlBuffer, 
			8, 
			dwControlBuffer, 
			8, 
			&dwDeviceRet, 
			NULL);
		m_pProcessMonitor = 1;
		CString close("�ر�");
		m_ButtonProcessMonitor.SetWindowText(close);
		CString text("�ѿ�����");
		m_Status.SetWindowText(text);

		//LOGFONT lf;                        // Used to create the CFont.
		//memset(&lf, 0, sizeof(LOGFONT));   // Clear out structure.
		//lf.lfHeight = 20;   // Request a 20-pixel-high font
		//strcpy(lf.lfFaceName, "Arial");    //    with face name "Arial".
		//CFont m_font;
		//m_font.CreateFontIndirect(&lf);    // Create the font.

		//m_Status.SetFont(&m_font);
	} 
	else
	{
		ULONG input;
		DWORD RealZwCreateProcessAddress = GetZwCreateProcessAddress();
		BOOL bRet = DeviceIoControl(
			hDevice,
			IOCTL_PROCESS_MONITORDRIVER_OFF, 
			&dwControlBuffer,
			sizeof(dwControlBuffer), 
			&dwControlBuffer, 
			sizeof(dwControlBuffer), 
			&dwDeviceRet, 
			NULL);
		m_pProcessMonitor = 0;
		CString open("����");
		m_ButtonProcessMonitor.SetWindowText(open);
		CString text("��δ���������鿪����");
		m_Status.SetWindowText(text);

	}
	
	CloseHandle(hDevice);  // �ر��豸
}

// ��ȡZwCreateProcessEx����ʵ��ַ
DWORD CMonitorManager::GetZwCreateProcessAddress(void)
{
	HMODULE hNtDll = GetModuleHandle( _T("ntdll.dll"));  // ���ģ����

	DWORD dwOffset = (DWORD)hNtDll;
	
	dwOffset += ( (PIMAGE_DOS_HEADER)hNtDll )->e_lfanew + sizeof(DWORD);  // Image_File_Header
	dwOffset += sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER) - 
		IMAGE_NUMBEROF_DIRECTORY_ENTRIES * sizeof(IMAGE_DATA_DIRECTORY);  // ��һ������Ŀ¼
	dwOffset = (DWORD)hNtDll + ( (PIMAGE_DATA_DIRECTORY)dwOffset )->VirtualAddress;  // ������
	PIMAGE_EXPORT_DIRECTORY pExpDir = (PIMAGE_EXPORT_DIRECTORY)dwOffset;

	DWORD pNumberOfNames = pExpDir->NumberOfNames;  // ��������������

	PDWORD pAddressOfNames = (PDWORD)( (DWORD)hNtDll + pExpDir->AddressOfNames );  // FNT  

	// FOT
	PWORD arrayOfFunctionOrdinals = (PWORD)( (DWORD)hNtDll + pExpDir->AddressOfNameOrdinals );
	// FAT
	PDWORD arrayOfFunctionAddresses = (PDWORD)( (DWORD)hNtDll + pExpDir->AddressOfFunctions );

	CString funName;
	CString functionName("ZwCreateProcessEx");
	DWORD Base = pExpDir->Base;

	for (UINT i = 0; i < pNumberOfNames; i++)
	{
		funName = CString((PCSTR)( pAddressOfNames[i] + (DWORD)hNtDll ));

		if (funName.Compare(functionName) == 0)
		{
			WORD functionOrdinal = arrayOfFunctionOrdinals[i] + Base - 1;
			DWORD functionAddress = (DWORD)( (BYTE*)hNtDll + arrayOfFunctionAddresses[functionOrdinal]);
			
			return functionAddress;
		}
	}
	return 0;
}

// �û�̬���ں�̬֮����߳̽���
void static ThreadMonitor(LPVOID lpParam)
{
	DWORD dwProcessMonitor = 0;		//���̼�ؿ������
	DWORD dwGoOrNot = 0;				//�Ƿ��������ִ��
	char chMessageBuffer[512] = {0};

	while(1)
	{
		memmove(&dwProcessMonitor, &chInteractionBuffer[0],4);

		if (!dwProcessMonitor)		//���̼�عر�
		{
			Sleep(10);		//��λ������
			continue;		//����ȥ����ִ��ѭ��
		}
		
		char *message = (char *)&chInteractionBuffer[8];		//��ȡ���������Ϣ
		char *positon = strstr(message, "##");						//����message�С�##����һ�γ��ֵ�λ��
		if (positon != NULL)
		{
			int iIntdex = positon - message;		//��ȡ����
			strcpy(chMessageBuffer, "�Ƿ�����");
			strncat(chMessageBuffer, &chInteractionBuffer[8], iIntdex);		//strncat�����޶�����
			strcat(chMessageBuffer, "����");
			strcat(chMessageBuffer, &chInteractionBuffer[iIntdex+10]);    //????+10
		}

		ThreadInfo *m_ThreadInfo = (ThreadInfo *)lpParam;
		
		if(IDYES == MessageBoxA(0, chMessageBuffer, "WARNING", MB_YESNO|MB_ICONQUESTION|0x00200000L))
		{
			dwGoOrNot = 1; //�������ִ��
		}
		else 
		{
			dwGoOrNot = 0;//���������ִ��
			//�������ؽ���
			iCount++;
			CString csCount;
			csCount.Format(TEXT("%d"), iCount);
			m_ThreadInfo->p_Count->SetWindowText(csCount);
		}

		memmove(&chInteractionBuffer[4], &dwGoOrNot, 4);
		
		dwProcessMonitor = 0;	//��֪���������û�ִ̬����ϣ�����
		
		memmove(&chInteractionBuffer[0], &dwProcessMonitor, 4);
	}
}
