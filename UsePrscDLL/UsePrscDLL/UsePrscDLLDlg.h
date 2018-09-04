
// UsePrscDLLDlg.h : ͷ�ļ�
//

#pragma once


// CUsePrscDLLDlg �Ի���
class CUsePrscDLLDlg : public CDialogEx
{
// ����
public:
	CUsePrscDLLDlg(CWnd* pParent = NULL);	// ��׼���캯��
	virtual ~CUsePrscDLLDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_USEPRSCDLL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();

	//��д, �����ݼ�
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL  m_bSelectAll;
	CEdit  m_editOutput;
	afx_msg void OnCheckAccuracy(UINT uID);



	BOOL m_bHigh; //�߾���
	BOOL m_bNormal; //��ͨ����


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
