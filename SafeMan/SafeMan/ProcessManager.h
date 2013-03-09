#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// ��¼������Ϣ�Ľṹ��
typedef struct tagProcessInfo     
{
	ULONG address;  // ���̵�ַ           
	LONG pid;  // ����ID
	UCHAR name[16];  // ������
	struct tagProcessInfo *next;  // ��������ָ��
}ProcessInfo, *PProcessInfo;


// CProcessManager �Ի���

class CProcessManager : public CDialog
{
	DECLARE_DYNAMIC(CProcessManager)

public:
	CProcessManager(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CProcessManager();

// �Ի�������
	enum { IDD = IDD_PROCESSMANAGER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();  // ��ʼ���Ի���
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);  // �ı�Ի���Ϳؼ�����ɫ
	afx_msg void OnBnClickedButtonListprocessRing3();  // Ring3�оٽ��̰�ť�¼�
	afx_msg void OnBnClickedButtonPspcidtable();  // ͨ��Pspcidtable�оٽ���
	afx_msg void OnNMRclickListProcess(NMHDR *pNMHDR, LRESULT *pResult);  // �б�򵥻��Ҽ��¼�
	afx_msg void OnMenuTerminateProcessRing0();  // Ring0��������
	afx_msg void OnNMCustomdrawListProcess(NMHDR *pNMHDR, LRESULT *pResult);  // �����б�
	afx_msg void OnBnClickedButtonActiveProcessLinks();  // ͨ��ActiveProcessLinks�оٽ���
	afx_msg void OnBnClickedButtonObjectTable();  // ͨ��ObjectTable�оٽ���
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);  // ���������ʽ
private:
	BOOL AdjustPurview();  // ����Ȩ�ޣ���Ҫ��Ϊ��OpenProcess�ɹ�����Handle

private:
	CBrush m_brush;  // �Զ��廭ˢ
	CListCtrl m_ListProcess;  // �����о��б��	
};
