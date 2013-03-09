#include "StdAfx.h"
#include "DriverManager.h"
#include <winsvc.h>
#include <iostream>
using namespace std;

CDriverManager::CDriverManager(void)
{
}

CDriverManager::~CDriverManager(void)
{
}


// װ��NT��������
//
BOOL CDriverManager::LoadNTDriver(PTCHAR lpszDriverName, PTCHAR lpszDriverPath)
{
	TCHAR szDriverImagePath[256];
	GetFullPathName(lpszDriverPath, 256, szDriverImagePath, NULL);  // �õ�����������·��

	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;  // SCM�������ľ��
	SC_HANDLE hServiceDDK = NULL;  // NT��������ķ�����

	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );  // �򿪷�����ƹ�����

	if(hServiceMgr == NULL)  // OpenSCManagerʧ��  
	{
		cout<<"OpenSCManager() Faild !"<<GetLastError()<<endl;
		bRet = FALSE;
		goto BeforeLeave;
	}
	else  // OpenSCManager�ɹ�
	{
		cout<<"OpenSCManager() ok !"<<endl;
	}

	// ������������Ӧ�ķ���
	hServiceDDK = CreateService(hServiceMgr,
		lpszDriverName,  // �����������ע����е�����  
		lpszDriverName,  // ע������������ DisplayName ֵ  
		SERVICE_ALL_ACCESS,  // ������������ķ���Ȩ��  
		SERVICE_KERNEL_DRIVER,  // ��ʾ���صķ�������������  
		SERVICE_DEMAND_START,  // ע������������ Start ֵ  
		SERVICE_ERROR_IGNORE,  // ע������������ ErrorControl ֵ  
		szDriverImagePath,  // ע������������ ImagePath ֵ  
		NULL,  NULL,  NULL,  NULL,  NULL);  

	DWORD dwRtn;
	if(hServiceDDK == NULL)  // �жϷ����Ƿ�ʧ��  
	{  
		dwRtn = GetLastError();
		if(dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)  
		{  
			cout<<"CrateService() Faild !"<<dwRtn<<endl;  // ��������ԭ�򴴽�����ʧ��
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else  
		{
			cout<<"CrateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! "<<endl;  // ���񴴽�ʧ�ܣ������ڷ����Ѿ�������    
		}

		// ���������Ѿ����أ�ֻ��Ҫ��  
		hServiceDDK = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);  
		if(hServiceDDK == NULL)  // ����򿪷���Ҳʧ�ܣ�����ζ����  
		{
			dwRtn = GetLastError();  
			cout<<"OpenService() Faild !"<<dwRtn<<endl;  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else 
		{
			cout<<"OpenService() ok !"<<endl;
		}
	}  
	else  
	{
		cout<<"CrateService() ok !"<<endl;
	}

	bRet = StartService(hServiceDDK, NULL, NULL);  // �����������   
	if(!bRet)  
	{  
		DWORD dwRtn = GetLastError();  
		if(dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)  
		{  
			cout<<"StartService() Faild !"<<dwRtn<<endl;  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else  
		{  
			if(dwRtn == ERROR_IO_PENDING)   // �豸����ס  
			{  
				cout<<"StartService() Faild ERROR_IO_PENDING !"<<endl; 
				bRet = FALSE;
				goto BeforeLeave;
			}  
			else  // �����Ѿ�����  
			{  
				cout<<"StartService() Faild ERROR_SERVICE_ALREADY_RUNNING !"<<endl;
				bRet = TRUE;
				goto BeforeLeave;
			}  
		}  
	}
	bRet = TRUE;

BeforeLeave:

	// �뿪ǰ�رվ��
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}

	return bRet;
}


// ж��NT��������
//
BOOL CDriverManager::UnloadNTDriver(PTCHAR szSvrName)
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr=NULL;  // SCM�������ľ��
	SC_HANDLE hServiceDDK=NULL;  // NT��������ķ�����
	SERVICE_STATUS SvrSta;
	
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);  // ��SCM������  
	if(hServiceMgr == NULL)  // ����SCM������ʧ��  
	{
		cout<<"OpenSCManager() Faild !"<<GetLastError()<<endl;  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	else  // ����SCM�������ɹ�  
	{
		cout<<"OpenSCManager() ok !"<<endl;  
	}
	
	hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);  // ����������Ӧ�ķ���  
	if(hServiceDDK == NULL)  // ����������Ӧ�ķ���ʧ��  
	{
		cout<<"OpenService() Faild !"<<GetLastError()<<endl;  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	else  
	{  
		cout<<"OpenService() ok !"<<endl;  
	}  
	// ֹͣ�����������ֹͣʧ�ܣ�ֻ�������������ܣ��ٶ�̬���ء�  
	if(!ControlService(hServiceDDK, SERVICE_CONTROL_STOP , &SvrSta))  
	{  
		cout<<"ControlService() Faild !"<<GetLastError()<<endl;  
	}  
	else  // ����������Ӧ��ʧ��  
	{
		cout<<"ControlService() ok !"<<endl;  
	}

	// ��̬ж����������  
	if(!DeleteService(hServiceDDK))  // ж��ʧ��  
	{
		cout<<"DeleteSrevice() Faild !"<<GetLastError()<<endl;  
	}  
	else  // ж�سɹ�  
	{  
		cout<<"DelServer:eleteSrevice() ok !"<<endl;  
	}  
	bRet = TRUE;

BeforeLeave:

	// �뿪ǰ�رմ򿪵ľ��
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;	
}


