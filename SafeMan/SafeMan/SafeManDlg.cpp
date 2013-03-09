// SafeManDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SafeMan.h"
#include "SafeManDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSafeManDlg �Ի���

CSafeManDlg::CSafeManDlg(CWnd* pParent /*=NULL*/)
	: CDialogSK(CSafeManDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pProcessManager = new CProcessManager();
	m_pSSDTManager = new CSSDTManager();
	m_pMonitorManager = new CMonitorManager();
}

CSafeManDlg::~CSafeManDlg()
{
	if(m_pProcessManager != NULL)
		delete m_pProcessManager;

	if(m_pSSDTManager != NULL)
		delete m_pSSDTManager;

	if(m_pMonitorManager != NULL)
		delete m_pMonitorManager;
}
void CSafeManDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogSK::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_PROCESSMANAGER, m_ProcessManager);
	DDX_Control(pDX, IDC_BUTTON_SSDTMANAGER, m_SSDTManager);
	DDX_Control(pDX, IDC_BUTTON_MONITORMANAGER, m_MonitorManager);
	DDX_Control(pDX, IDC_MIN, m_Min);
	DDX_Control(pDX, IDC_EXIT, m_Exit);
	DDX_Control(pDX, IDC_TRAY, m_Tray);
}

BEGIN_MESSAGE_MAP(CSafeManDlg, CDialogSK)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_PROCESSMANAGER, &CSafeManDlg::OnBnClickedButtonProcessmanager)
	ON_BN_CLICKED(IDC_BUTTON_SSDTMANAGER, &CSafeManDlg::OnBnClickedButtonSsdtmanager)
	ON_BN_CLICKED(IDC_BUTTON_MONITORMANAGER, &CSafeManDlg::OnBnClickedButtonProcessmonitormanager)
	ON_BN_CLICKED(IDC_MIN, &CSafeManDlg::OnBnClickedMin)
	ON_BN_CLICKED(IDC_EXIT, &CSafeManDlg::OnBnClickedExit)
	ON_COMMAND(ID_MENU_ITEM_SAFEMAN, &CSafeManDlg::OnMenuItemSafeman)
	ON_COMMAND(ID_MENU_ITEM_QUIT, &CSafeManDlg::OnMenuItemQuit)
	ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
	ON_BN_CLICKED(IDC_TRAY, &CSafeManDlg::OnBnClickedTray)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CSafeManDlg ��Ϣ�������
BOOL CSafeManDlg::OnInitDialog()
{
	CDialogSK::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	// ����Բ�ǶԻ���
	CRect dialogRect; 
	GetClientRect(&dialogRect); 
	CRgn rgn; 
	rgn.CreateRoundRectRgn(0,0,dialogRect.Width(),dialogRect.Height(),20,20); 
	SetWindowRgn(rgn,TRUE); 

	SetBitmap(IDB_BACKGROUND);

	m_ProcessManager.LoadBitmap(IDB_PROCESSMANAGER);  // ��ͣ��ť����λͼ
	m_SSDTManager.LoadBitmap(IDB_SSDTMANAGER);  // ��ͣ��ť����λͼ
	m_MonitorManager.LoadBitmap(IDB_MONITORMANAGER);  // ��ͣ��ť����λͼ
	m_Min.LoadBitmap(IDB_MIN);
	m_Exit.LoadBitmap(IDB_EXIT);
	m_Tray.LoadBitmap(IDB_TRAY);

	m_ProcessManager.SetState(TRUE);
	m_SSDTManager.SetState(FALSE);
	m_MonitorManager.SetState(FALSE);

	m_pProcessManager->Create(IDD_PROCESSMANAGER, this);
	m_pSSDTManager->Create(IDD_SSDTMANAGER, this);
	m_pMonitorManager->Create(IDD_MONITORMANAGER, this);

	m_pProcessManager->ShowWindow(SW_SHOW);  
	m_pSSDTManager->ShowWindow(SW_HIDE);  
	m_pMonitorManager->ShowWindow(SW_HIDE);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSafeManDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogSK::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CSafeManDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSafeManDlg::OnBnClickedButtonProcessmanager()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_ProcessManager.SetState(TRUE);
	m_SSDTManager.SetState(FALSE);
	m_MonitorManager.SetState(FALSE);
	
	m_pProcessManager->ShowWindow(SW_SHOW); 
	m_pSSDTManager->ShowWindow(SW_HIDE);  
	m_pMonitorManager->ShowWindow(SW_HIDE);
}

void CSafeManDlg::OnBnClickedButtonSsdtmanager()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_SSDTManager.SetState(TRUE);
	m_ProcessManager.SetState(FALSE);
	m_MonitorManager.SetState(FALSE);

	m_pSSDTManager->ShowWindow(SW_SHOW); 
	m_pProcessManager->ShowWindow(SW_HIDE);  
	m_pMonitorManager->ShowWindow(SW_HIDE);
}

void CSafeManDlg::OnBnClickedButtonProcessmonitormanager()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_MonitorManager.SetState(TRUE);
	m_ProcessManager.SetState(FALSE);
	m_SSDTManager.SetState(FALSE);

	m_pMonitorManager->ShowWindow(SW_SHOW); 
	m_pProcessManager->ShowWindow(SW_HIDE);  
	m_pSSDTManager->ShowWindow(SW_HIDE);
}


void CSafeManDlg::OnBnClickedMin()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ShowWindow(SW_MINIMIZE);
}

void CSafeManDlg::OnBnClickedExit()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_pProcessManager != NULL)
		delete m_pProcessManager;
	if(m_pSSDTManager != NULL)
		delete m_pSSDTManager;
	if(m_pMonitorManager != NULL)
		delete m_pMonitorManager;
	exit(0);
}

void CSafeManDlg::OnMenuItemSafeman()
{
	// TODO: �ڴ���������������

	m_TrayIcon.RemoveIcon();
	ShowWindow(SW_SHOW);
}

void CSafeManDlg::OnMenuItemQuit()
{
	// TODO: �ڴ���������������

	m_TrayIcon.RemoveIcon();
	OnCancel();
}


// ������Ϣ
//
LRESULT CSafeManDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	return m_TrayIcon.OnTrayNotification(wParam,lParam);
}


void CSafeManDlg::OnBnClickedTray()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_TrayIcon.Create(this,WM_ICON_NOTIFY,_T("SafeMan����������"),m_hIcon,IDR_MENU_TRAY);  // ����
	ShowWindow(SW_HIDE);  // ���ش���
}


// ���������ʽ
//
BOOL CSafeManDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CString strClassName;
	GetClassName(pWnd->GetSafeHwnd(), strClassName.GetBuffer(80), 80);

	if (strClassName == "Button")  // ��ť����
	{
		SetCursor( LoadCursor(NULL, IDC_HAND) );  // �����ʽΪ����
		return TRUE;
	}
	return CDialogSK::OnSetCursor(pWnd, nHitTest, message);
}
