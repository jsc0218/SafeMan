// SafeManDlg.h : ͷ�ļ�
//

#pragma once

#include "CDialogSK.h"
#include "HoverButton.h"
#include "TrayIcon.h"
#include "ProcessManager.h"
#include "SSDTManager.h"
#include "MonitorManager.h"
#include "afxwin.h"

#define WM_ICON_NOTIFY WM_USER+1  // �Զ���������Ϣ


// CSafeManDlg �Ի���
class CSafeManDlg : public CDialogSK
{
// ����
public:
	CSafeManDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CSafeManDlg();	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SAFEMAN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonProcessmanager();
	afx_msg void OnBnClickedButtonSsdtmanager();
	afx_msg void OnBnClickedMin();
	afx_msg void OnBnClickedExit();
	afx_msg void OnMenuItemSafeman();
	afx_msg void OnMenuItemQuit();
	LRESULT OnTrayNotification(WPARAM wParam,LPARAM lParam);  // ������Ϣ
	afx_msg void OnBnClickedTray();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);  // ���������ʽ

private:
	CHoverButton m_ProcessManager;
	CProcessManager* m_pProcessManager; 
	CHoverButton m_SSDTManager;
	CSSDTManager* m_pSSDTManager; 
	//
	CHoverButton m_MonitorManager;
	CMonitorManager* m_pMonitorManager;
	//
	CHoverButton m_Min;
	CHoverButton m_Exit;
	CHoverButton m_Tray;
	CTrayIcon m_TrayIcon;  // ���̱���
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButtonProcessmonitormanager();
};
