
// UsePrscDLL.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CUsePrscDLLApp: 
// �йش����ʵ�֣������ UsePrscDLL.cpp
//

class CUsePrscDLLApp : public CWinApp
{
public:
	CUsePrscDLLApp();

// ��д
public:
	virtual BOOL InitInstance();

	//ʹ��GDI
#if defined(USE_GDI)
	ULONG_PTR        m_gdiplusToken;
#endif

// ʵ��

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CUsePrscDLLApp theApp;