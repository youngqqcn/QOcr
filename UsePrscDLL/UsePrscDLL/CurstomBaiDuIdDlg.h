#pragma once


// CCurstomBaiDuIdDlg dialog

class CCurstomBaiDuIdDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCurstomBaiDuIdDlg)

public:
	CCurstomBaiDuIdDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCurstomBaiDuIdDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnNMClickSyslinkBaiduai(NMHDR *pNMHDR, LRESULT *pResult);
};
