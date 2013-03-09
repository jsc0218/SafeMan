// SSDTManager.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SafeMan.h"
#include "SSDTManager.h"
#include "DriverManager.h"
#include <winioctl.h>
#include "Ioctls.h"

// CSSDTManager �Ի���

IMPLEMENT_DYNAMIC(CSSDTManager, CDialog)

CSSDTManager::CSSDTManager(CWnd* pParent /*=NULL*/)
	: CDialog(CSSDTManager::IDD, pParent)
{
	if( CDriverManager::LoadNTDriver(_T("SSDTDriver"), _T("..\\SSDTDriver\\objchk\\i386\\SSDTDriver.sys")) == FALSE)
	//if( CDriverManager::LoadNTDriver(_T("SSDTDriver"), _T("SSDTDriver.sys")) == FALSE)
	{
		MessageBox(_T("��������ʧ�ܣ�"));
		CDriverManager::UnloadNTDriver(_T("SSDTDriver"));  // ж������ 
	}
}

CSSDTManager::~CSSDTManager()
{
	CDriverManager::UnloadNTDriver(_T("SSDTDriver"));  // ж������ 
}

void CSSDTManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SSDT, m_ListSSDT);
	DDX_Control(pDX, IDC_BUTTON_SSDT_REPAIR_SELECTED, m_ButRepairSel);
}


BEGIN_MESSAGE_MAP(CSSDTManager, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_SSDT_REPAIR_SELECTED, &CSSDTManager::OnBnClickedButtonSsdtRepairSelected)
	ON_STN_CLICKED(IDC_STATIC_CHECK, &CSSDTManager::OnStnClickedStaticCheck)
	ON_STN_CLICKED(IDC_STATIC_UNCHECK, &CSSDTManager::OnStnClickedStaticUncheck)
	ON_WM_SETCURSOR()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_SSDT, &CSSDTManager::OnNMCustomdrawListSsdt)
	ON_BN_CLICKED(IDC_BUTTON_SSDT_REFRESH, &CSSDTManager::OnBnClickedButtonSsdtRefresh)
END_MESSAGE_MAP()


// CSSDTManager ��Ϣ�������


// ��ʼ���Ի���
//
BOOL CSSDTManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	// SSDT�б�������
	m_ListSSDT.InsertColumn(0, _T("�����"), LVCFMT_LEFT, 80);
	m_ListSSDT.InsertColumn(1, _T("��ǰ��ַ"), LVCFMT_LEFT, 80);
	m_ListSSDT.InsertColumn(2, _T("ԭʼ��ַ"), LVCFMT_LEFT, 80);
	m_ListSSDT.InsertColumn(3, _T("��������"), LVCFMT_LEFT, 200);
	m_ListSSDT.InsertColumn(4, _T("ģ����"), LVCFMT_LEFT, 100);

	// SSDT�б��ÿ��ȫѡ�Ҵ������ߺ�ѡ���
	m_ListSSDT.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);  

	m_brush.CreateSolidBrush(RGB(255,255,255));  // ������ɫ��ˢ

	EnumSSDT();  // ����SSDT

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


// �ı�Ի���Ϳؼ�����ɫ
//
HBRUSH CSSDTManager::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	//return hbr;
	if(nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(RGB(255,255,255));  // ���ÿؼ�����ɫ����ɫ��

		if (pWnd->GetDlgCtrlID() == IDC_STATIC_CHECK || pWnd->GetDlgCtrlID() == IDC_STATIC_UNCHECK)
		{
			pDC->SetTextColor(RGB(165,205,246));
		}
		return m_brush;  // �����Զ���Ļ�ˢ���
	}
	return hbr;  // ����Ļ�����ϵͳĬ�ϵĻ�ˢ���
}


// ����SSDT
//
VOID CSSDTManager::EnumSSDT()
{
	m_ListSSDT.DeleteAllItems();  // ɾ���б�������

	// ���豸
	HANDLE hDevice = CreateFile(
		_T("\\\\.\\SSDTDriver"), 
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

	HMODULE hNtDll = GetModuleHandle( _T("ntdll.dll"));  // ���ģ����

	DWORD dwOffset = (DWORD)hNtDll;

	dwOffset += ( (PIMAGE_DOS_HEADER)hNtDll )->e_lfanew + sizeof(DWORD);  // Image_File_Header
	dwOffset += sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER) - 
		IMAGE_NUMBEROF_DIRECTORY_ENTRIES * sizeof(IMAGE_DATA_DIRECTORY);  // ��һ������Ŀ¼
	dwOffset = (DWORD)hNtDll + ( (PIMAGE_DATA_DIRECTORY)dwOffset )->VirtualAddress;  // ������
	PIMAGE_EXPORT_DIRECTORY pExpDir = (PIMAGE_EXPORT_DIRECTORY)dwOffset;

	DWORD numOfNames = pExpDir->NumberOfNames;  // ��������������

	PDWORD pNameArray = (PDWORD)( (DWORD)hNtDll + pExpDir->AddressOfNames );  // FNT  

	PSYSTEM_MODULE_INFORMATION pSysModInfo = GetModuleList(hNtDll);  // �õ�ϵͳģ��

	CString funName;  // ������
	USHORT rowCount = 0;  // �б������
	for (UINT i = 0; i < numOfNames; i++)
	{
		funName = CString((PCSTR)( pNameArray[i] + (DWORD)hNtDll ));  // ANSI???

		if ( funName.GetAt(0) == _T('N') && funName.GetAt(1) == _T('t') )  // ��������ͷ�����ַ���Nt
		{
			// FOT
			PWORD pOrdNameArray = (PWORD)( (DWORD)hNtDll + pExpDir->AddressOfNameOrdinals );
			// FAT
			PDWORD pFunArray = (PDWORD)( (DWORD)hNtDll + pExpDir->AddressOfFunctions );

			PVOID pFun = (PVOID)( (DWORD)hNtDll + pFunArray[ pOrdNameArray[i] ] );

			SSDTENTRY entry;
			CopyMemory( &entry, pFun, sizeof(SSDTENTRY) );
			if (entry.byMov == MOV)
			{
				ULONG ulAddr;
				GetFunAddrByIndex(hDevice, entry.dwIndex, &ulAddr);  // ͨ�������õ�������ַ

				CString strModule = GetModuleByAddr(ulAddr, pSysModInfo);  // ͨ����ַ�ҵ�����ģ��

				CString strIndex;
				strIndex.Format(_T("0x%04X"), entry.dwIndex);  // DWORDתCString

				CString strAddr;
				strAddr.Format(_T("0x%08X"), ulAddr);  // ULONGתCString

				int iRow = m_ListSSDT.InsertItem(entry.dwIndex, strIndex);  
				m_ListSSDT.SetItemText(iRow, 1, strAddr);  
				m_ListSSDT.SetItemText(iRow, 3, funName);
				m_ListSSDT.SetItemText(iRow, 4, strModule);

				rowCount++;
			}
		}
	}

	GetOrigFunAddr();  // �õ���������ԭʼ��ַ

	delete pSysModInfo;  // ɾ��ģ���б�
	CloseHandle(hDevice);  // �ر��豸
}


// �õ�SSDT
//
BOOL CSSDTManager::GetSSDT(IN HANDLE hDevice, OUT PSSDT pSSDT)
{
	DWORD dwDeviceRet = 0;

	BOOL bRet = DeviceIoControl(
		hDevice, 
		IOCTL_SSDTDRIVER_GETSSDT, 
		NULL, 
		0, 
		pSSDT,
		sizeof(SSDT), 
		&dwDeviceRet, 
		0);

	return bRet;
}


// ͨ�������õ�������ַ
//
BOOL CSSDTManager::GetFunAddrByIndex(IN HANDLE hDevice, IN ULONG ulIndex, OUT PULONG pFunAddr)
{
    DWORD dwDeviceRet = 0;

    BOOL bRet = DeviceIoControl( 
		hDevice, 
		IOCTL_SSDTDRIVER_GETFUNADDRBYINDEX, 
		&ulIndex,
        sizeof(ULONG), 
		pFunAddr, 
		sizeof(ULONG), 
		&dwDeviceRet, 
		0);

    return bRet;
}


// �õ�ϵͳģ��
//
PSYSTEM_MODULE_INFORMATION CSSDTManager::GetModuleList(IN HMODULE hNtDll)
{
	// ����ָ�����Ͷ���
	typedef DWORD (WINAPI * NTQUERYSYSTEMINFORMATION)(
		SYSTEM_INFORMATION_CLASS, 
		LPVOID, 
		DWORD,
		PULONG);

	 // ����ָ���Ķ�̬���ӿ�(DLL)�е�����⺯����ַ
	NTQUERYSYSTEMINFORMATION NtQuerySystemInformation =
		(NTQUERYSYSTEMINFORMATION)GetProcAddress(hNtDll, "NtQuerySystemInformation"); 

	ULONG size;

	// ��̽һ�������ڴ��С��������Ҫsize��С
	NtQuerySystemInformation(SystemModuleInformation, &size, 0, &size); 

	// ����size���ֽڵĴ�С
	PSYSTEM_MODULE_INFORMATION pSysModInfo = (PSYSTEM_MODULE_INFORMATION)new BYTE[size];
	
	// ������ȥȡ��Ϣ
	NtQuerySystemInformation(SystemModuleInformation, pSysModInfo, size, 0); 

	return pSysModInfo;
} 


// ͨ����ַ�ҵ�����ģ��
//
CString CSSDTManager::GetModuleByAddr(IN ULONG ulAddr, IN PSYSTEM_MODULE_INFORMATION pSysModInfo)
{
	for (UINT i = 0; i < pSysModInfo->Count; i++)
	{
		ULONG ulBase = (ULONG)pSysModInfo->Module[i].Base;
		ULONG ulMax  = ulBase + pSysModInfo->Module[i].Size;
		if (ulBase <= ulAddr && ulAddr < ulMax)
		{
			return CString(pSysModInfo->Module[i].ImageName + pSysModInfo->Module[i].PathLength);
		}
	}
	return NULL;
}


// �õ���������ԭʼ��ַ
//
VOID CSSDTManager::GetOrigFunAddr()
{
	PSYSTEM_MODULE_INFORMATION pSysModInfo =   
		GetModuleList(GetModuleHandle( _T("ntdll.dll")));  // �õ�ϵͳģ��  

	// ���¼����ں��ļ�(ntkrnlpa.exe)���������������ģ��
	HMODULE hNtDll = LoadLibraryExA(
		pSysModInfo->Module[0].ImageName + pSysModInfo->Module[0].PathLength,   // ϵͳģ�����һ�������ں��ļ�
		NULL, 
		DONT_RESOLVE_DLL_REFERENCES);
	if (hNtDll == NULL)
	{
		return;
	}

	DWORD dwKSDT = (DWORD)GetProcAddress(hNtDll, "KeServiceDescriptorTable");  // ���ں��ļ��в���KeServiceDescriptorTable������ַ 
	if (dwKSDT == 0)
	{
		return;
	}
 
	dwKSDT -= (DWORD)hNtDll;  // ��ȡKeServiceDescriptorTable������RVA 

	DWORD dwKiServiceTable = GetKiServiceTable(hNtDll, dwKSDT);  // �õ�KiServiceTable��RVA
	if (dwKiServiceTable == 0)
	{
		return;
	}

	PIMAGE_FILE_HEADER pfh = NULL;
	PIMAGE_OPTIONAL_HEADER poh = NULL;
	PIMAGE_SECTION_HEADER psh = NULL;
	GetPEHeaders(hNtDll, pfh, poh, psh);  // �õ�PE�ļ���ĸ���ͷ

	DWORD dwKernelBase = (DWORD)pSysModInfo->Module[0].Base;  // ntkrnlpa.exe���ػ�ַ

	ULONG ulAddr = 0;
	CString strAddr;
	DWORD dwIndex = 0;
	for (PDWORD pService = (PDWORD)( (DWORD)hNtDll + dwKiServiceTable ); 
		*pService - poh->ImageBase < poh->SizeOfImage; 
		pService++, dwIndex++)
	{
		ulAddr = *pService - poh->ImageBase + dwKernelBase;  // ��������ԭʼ��ַ
		strAddr.Format(_T("0x%08X"), ulAddr);  // ULONGתCString
		m_ListSSDT.SetItemText(dwIndex, 2, strAddr);
	}

	FreeLibrary(hNtDll);  // ��LoadLibraryExA��Ӧ
	delete pSysModInfo;
}


// �õ�KiServiceTable��RVA
//
DWORD CSSDTManager::GetKiServiceTable(HMODULE hModule, DWORD dwKSDT) 
{ 
	PIMAGE_FILE_HEADER pfh = NULL; 
	PIMAGE_OPTIONAL_HEADER poh = NULL; 
	PIMAGE_SECTION_HEADER psh = NULL; 
	
	GetPEHeaders(hModule, pfh, poh, psh);  // �õ�PE�ļ���ĸ���ͷ 

	if ( (poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) && 
		!((pfh->Characteristics)&IMAGE_FILE_RELOCS_STRIPPED) )  // �ж��Ƿ�����ض�λ��Ϣ
	{ 
		// ��һ��IMAGE_BASE_RELOCATION
		PIMAGE_BASE_RELOCATION pbr = 
			(PIMAGE_BASE_RELOCATION)(
			(DWORD)(poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) + (DWORD)hModule); 

		BOOL bFirstChunk = TRUE; 
		PIMAGE_FIXUP_ENTRY pfe = NULL; 
		// ����IMAGE_BASE_RELOCATION��ntoskrnl�е�һ��IMAGE_BASE_RELOCATION.VirtualAddress��0
		while (bFirstChunk || pbr->VirtualAddress) 
		{
			bFirstChunk = FALSE; 

			pfe = (PIMAGE_FIXUP_ENTRY)( (DWORD)pbr + sizeof(IMAGE_BASE_RELOCATION) );  // IMAGE_BASE_RELOCATION.TypeOffset[i](ÿ��2�ֽ�) 

			// ����IMAGE_BASE_RELOCATION.TypeOffset[i]
			for (DWORD i = 0; i < ( pbr->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION) )>>1; i++, pfe++)  
			{ 
				if (pfe->type == IMAGE_REL_BASED_HIGHLOW) 
				{ 
					DWORD dwPointerRva = pbr->VirtualAddress + pfe->offset;  // ָ����Ҫ�޸ĵĵ�ַ���� 
					DWORD dwPointsToRva = *(PDWORD)( (DWORD)hModule + dwPointerRva ) - (DWORD)poh->ImageBase; 

					if (dwPointsToRva == dwKSDT)  // 1.�ж��Ƿ�ΪKeServiceDescriptorTable
					{ 
						// "mov ds:_KeServiceDescriptorTable.Base, offset _KiServiceTable" from the KiInitSystem.
						if ( *(PWORD)( (DWORD)hModule + dwPointerRva - 2 ) == 0x05C7 )  // 2.�ж��Ƿ�ΪMOVָ��
						{ 
							// ���������KiServiceTable��RVA
							DWORD dwKiServiceTable = *(PDWORD)( (DWORD)hModule + dwPointerRva + 4 ) - poh->ImageBase; 
							return dwKiServiceTable; 
						} 
					} 
				}  
			} 

			pbr = (PIMAGE_BASE_RELOCATION)( (DWORD)pbr + pbr->SizeOfBlock );  // ��һ��IMAGE_BASE_RELOCATION
		} 
	}     

	return 0; 
} 


// �õ�PE�ļ���ĸ���ͷ
//
BOOL CSSDTManager::GetPEHeaders(HMODULE hModule, PIMAGE_FILE_HEADER& pfh,
				 PIMAGE_OPTIONAL_HEADER& poh, PIMAGE_SECTION_HEADER& psh)
{
	PIMAGE_DOS_HEADER pdh = (PIMAGE_DOS_HEADER)hModule;
	if (pdh->e_magic != IMAGE_DOS_SIGNATURE)  // ��־λ 
	{
		return FALSE;
	}

	PIMAGE_NT_HEADERS pnh = (PIMAGE_NT_HEADERS)( (DWORD)hModule + pdh->e_lfanew );
	if (pnh->Signature != IMAGE_NT_SIGNATURE)  // ��־λ  
	{
		return FALSE;
	}

	pfh = (PIMAGE_FILE_HEADER)( &(pnh->FileHeader) );

	poh = (PIMAGE_OPTIONAL_HEADER)( &(pnh->OptionalHeader) );
	if (poh->Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)  // ��־λ 
	{
		return FALSE;
	}

	psh = (PIMAGE_SECTION_HEADER)( (PBYTE)poh + sizeof(IMAGE_OPTIONAL_HEADER) );

	return TRUE;
}


// SSDT�޸�ѡ���ť�¼�
//
void CSSDTManager::OnBnClickedButtonSsdtRepairSelected()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	// �鿴�û���û��ѡ��һ��
	for (int i = 0; i < m_ListSSDT.GetItemCount(); i++)
	{
		if (m_ListSSDT.GetCheck(i) == TRUE)
		{
			break;
		}
		if (i == (m_ListSSDT.GetItemCount() - 1) && m_ListSSDT.GetCheck(i) == FALSE)
		{
			MessageBox(_T("����ѡ��һ������޸���"));
			return;
		}
	}

	m_ButRepairSel.EnableWindow(FALSE);  // �޸�ѡ���ť��Ч

	// ���豸
	HANDLE hDevice = CreateFile(
		_T("\\\\.\\SSDTDriver"), 
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

	// �����б�ÿһ��
	for (int i = 0; i < m_ListSSDT.GetItemCount(); i++)
	{
		// ѡ���ѡ�в���ǰ���ַ��һ��
		if ( m_ListSSDT.GetCheck(i) == TRUE && 
			m_ListSSDT.GetItemText(i, 1) != m_ListSSDT.GetItemText(i, 2) )
		{
			if (RestoreSSDTItem(hDevice, i) == FALSE)
			{
				MessageBox(_T("�޸�ʧ�ܣ�"));
			}
		}
	}

	CloseHandle(hDevice);  // �ر��豸

	EnumSSDT();  // ����SSDT

	m_ButRepairSel.EnableWindow(TRUE);  // �޸�ѡ���ť��Ч
}


// ����SSDT��
//
BOOL CSSDTManager::RestoreSSDTItem(IN HANDLE hDevice, IN ULONG ulIndex)
{
	CString strAddr = m_ListSSDT.GetItemText(ulIndex, 2);
	DWORD dwAddr;
	_stscanf_s(strAddr.Mid(2), _T("%x"), &dwAddr);  // CStringתDWORD

	RESTORE_INFO restoreInfo;
	restoreInfo.ulIndex = ulIndex;  // �����
	restoreInfo.ulAddr = dwAddr;  // ����������ʵ��ַ

	// �ָ�SSDT��
	DWORD dwDeviceRet = 0;
	BOOL bRet = DeviceIoControl(
		hDevice,
		IOCTL_SSDTDRIVER_RESTOREFUNADDR, 
		&restoreInfo, 
		sizeof(restoreInfo), 
		NULL, 
		0, 
		&dwDeviceRet, 
		NULL);

	return bRet;
}


// ȫѡ��ť�¼�(Notify����Ҫѡ��)
//
void CSSDTManager::OnStnClickedStaticCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	for (int i = 0; i < m_ListSSDT.GetItemCount(); i++)
	{
		m_ListSSDT.SetCheck(i);  // ѡ����
	}
}


// ȫ��ѡ��ť�¼�(Notify����Ҫѡ��)
//
void CSSDTManager::OnStnClickedStaticUncheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	for (int i = 0; i < m_ListSSDT.GetItemCount(); i++)
	{
		m_ListSSDT.SetCheck(i, FALSE);  // ѡ��򲻴�
	}
}


// ���������ʽ
//
BOOL CSSDTManager::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CString strClassName;
	GetClassName(pWnd->GetSafeHwnd(), strClassName.GetBuffer(80), 80);

	if (strClassName == "Static" || strClassName == "Button")  // ��̬�ؼ���ť����
	{
		SetCursor( LoadCursor(NULL, IDC_HAND) );  // �����ʽΪ����
		return TRUE;
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}


// �����б�
//
void CSSDTManager::OnNMCustomdrawListSsdt(NMHDR *pNMHDR, LRESULT *pResult)
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
		COLORREF clrNewTextColor, clrNewBkColor;

		int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);

		// �����ǰSSDT��ַ��ԭʼ��ַ��һ��,����ʾ����Ϊ��ɫ,����Ϊ��ɫ
		if ( m_ListSSDT.GetItemText(nItem, 1) != m_ListSSDT.GetItemText(nItem, 2) )		
		{
			clrNewTextColor = RGB(255, 0, 0);  // ����Ϊ��ɫ
		}
		else		
		{
			clrNewTextColor = RGB(0, 0, 0);  // ����Ϊ��ɫ
		}

		// ���ñ���ɫ
		if (nItem%2 ==0)
		{
			clrNewBkColor = RGB(240, 240, 240);	// ż���б���ɫΪ��ɫ
		}
		else
		{
			clrNewBkColor = RGB(255, 255, 255);	// �����б���ɫΪ��ɫ
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}
}


// ˢ�°�ť�¼�
//
void CSSDTManager::OnBnClickedButtonSsdtRefresh()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	EnumSSDT();  // ����SSDT
}
