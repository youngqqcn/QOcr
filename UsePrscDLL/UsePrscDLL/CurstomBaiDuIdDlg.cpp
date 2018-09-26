// CurstomBaiDuIdDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UsePrscDLL.h"
#include "CurstomBaiDuIdDlg.h"
#include "afxdialogex.h"
#include "resource.h"


// CCurstomBaiDuIdDlg dialog

IMPLEMENT_DYNAMIC(CCurstomBaiDuIdDlg, CDialogEx)

CCurstomBaiDuIdDlg::CCurstomBaiDuIdDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_CUSTOM_BAIDUID, pParent)
{
	//SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);
}

CCurstomBaiDuIdDlg::~CCurstomBaiDuIdDlg()
{
}

void CCurstomBaiDuIdDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCurstomBaiDuIdDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CCurstomBaiDuIdDlg::OnBnClickedOk)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_BAIDUAI, &CCurstomBaiDuIdDlg::OnNMClickSyslinkBaiduai)
END_MESSAGE_MAP()


// CCurstomBaiDuIdDlg message handlers


void CCurstomBaiDuIdDlg::OnBnClickedOk()
{
	CString strAppID;
	CString strApiKey;
	CString strSecretKey;
	GetDlgItemText(IDC_EDIT_APPID, strAppID);
	GetDlgItemText(IDC_EDIT_APIKEY, strApiKey);
	GetDlgItemText(IDC_EDIT_SECRETKEY, strSecretKey);


	if (8 == strAppID.GetLength() && 24 == strApiKey.GetLength() && 32 == strSecretKey.GetLength())
	{
		TCHAR szDocPath[_MAX_PATH];
		SHGetSpecialFolderPath(this->GetSafeHwnd(), szDocPath, CSIDL_MYDOCUMENTS, 0);
		CString strCfgFilePath = szDocPath + CString(_T("\\__ocr_config"));

		CStdioFile file;
		CFileException fe;
		DeleteFile(strCfgFilePath);
		if (FALSE == file.Open(strCfgFilePath, CStdioFile::shareCompat |CStdioFile::modeCreate | CStdioFile::modeWrite, &fe))
		{
			CHAR szError[1024] = { 0 };
			memset(szError, 0, sizeof(szError));
			fe.GetErrorMessage(szError, 0xff);
			AfxMessageBox(CString(_T("错误: ")) +  szError);
			return;
		}
		file.WriteString(_T("APPID=") + strAppID + _T("\n"));
		file.WriteString(_T("APIKEY=") + strApiKey + _T("\n"));
		file.WriteString(_T("SECRETEKEY=") + strSecretKey + _T("\n"));
		file.Close();


		//隐藏文件
		CFileStatus fs;
		CFile::GetStatus(strCfgFilePath, fs);
		fs.m_attribute = CFile::hidden; 
		CFile::SetStatus(strCfgFilePath, fs);

		CDialogEx::OnOK();
	}
	else
	{
		MessageBox(_T("账号不合法, 请检查并重新输入."), _T("提示"));
		return;
	}
}



void CCurstomBaiDuIdDlg::OnNMClickSyslinkBaiduai(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(NULL, _T("open"), _T("https://login.bce.baidu.com/"), NULL, NULL, SW_SHOW);
}


BOOL CCurstomBaiDuIdDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE); //设置小图标 

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
