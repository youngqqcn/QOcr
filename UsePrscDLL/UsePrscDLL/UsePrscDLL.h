
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

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CUsePrscDLLApp theApp;