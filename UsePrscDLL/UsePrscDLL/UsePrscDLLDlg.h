
// UsePrscDLLDlg.h : 头文件
//

#pragma once


// CUsePrscDLLDlg 对话框
class CUsePrscDLLDlg : public CDialogEx
{
// 构造
public:
	CUsePrscDLLDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~CUsePrscDLLDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_USEPRSCDLL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();

	//重写, 处理快捷键
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL  m_bSelectAll;
	CEdit  m_editOutput;
	afx_msg void OnCheckAccuracy(UINT uID);



	BOOL m_bHigh; //高精度
	BOOL m_bNormal; //普通精度


	CStatusBar m_status;

	UINT m_nLineCount;
	UINT m_nColumnCount;

	CMenu  m_menu;
	afx_msg void OnCustomBaiDuID();

public:
	CString m_strAppID;
	CString m_strApiKey;
	CString m_strSecreteKey;
	afx_msg void OnAbout();

	HANDLE m_hMutex;
	afx_msg void OnDownloadSrc();
};
