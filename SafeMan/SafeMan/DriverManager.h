#pragma once

class CDriverManager
{
public:
	CDriverManager(void);
	~CDriverManager(void);
	static BOOL LoadNTDriver(PTCHAR lpszDriverName, PTCHAR lpszDriverPath);  // װ��NT��������
	static BOOL UnloadNTDriver(PTCHAR szSvrName);  // ж��NT��������    
};
