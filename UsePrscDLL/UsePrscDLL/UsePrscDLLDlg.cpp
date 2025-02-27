/*
//作者 :  yqq 
//邮箱 : youngqqcn@qq.com
//我的github主页:   https://github.com/youngqqcn
//每一片云,都有它自己的颜色. 2018-09-04 yqq 留
*/

#include "stdafx.h"
#include "UsePrscDLL.h"
#include "UsePrscDLLDlg.h"
#include "afxdialogex.h"

#include <iphlpapi.h>
#include <curl/curl.h>
#include <json/json.h>
#include "ocr.h"  //文字识别

#include "CurstomBaiDuIdDlg.h"
using namespace Gdiplus;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Iphlpapi.lib") 
#pragma comment(lib, "../lib/libcurl.lib")
#pragma comment(lib, "../lib/libeay32.lib")
#pragma comment(lib, "../lib/ssleay32.lib")
#pragma comment(lib, "../lib/GdiPlus.lib")  //2018-09-21 保存图片为jpg格式, 解决win10下图片过大导致失败的问题 
#pragma comment(linker, "/subsystem:windows")   

typedef void(*EntryPoint)(
	HWND hwnd,        // handle to owner window   
	HINSTANCE hinst,  // instance handle for the DLL   
	LPTSTR lpCmdLine, // string the DLL will parse   
	int nCmdShow      // show state   
	);

HANDLE   g_hOcrMutex = NULL;
HANDLE   g_hCheckIpMutex = NULL;
HANDLE   g_hExitEvent = NULL; //线程退出事件

static UINT BASED_CODE indicators[] =
{
	IDS_STRING_ROWS,
	IDS_STRING_PROB
};


BOOL CheckIPReachable(LPCTSTR strIPAddress)
{
	ASSERT(strIPAddress);
	char strIP[100];

	strcpy_s(strIP, strIPAddress);

	IPAddr ipaddr = inet_addr(strIP);
	ULONG ulHopCount, ulRTT;

	
	::WaitForSingleObject(g_hCheckIpMutex, 100);
	BOOL  bRet = (BOOL)GetRTTAndHopCount(ipaddr, &ulHopCount, 30/*最大hop数, 可自行设置*/, &ulRTT); //相当于  ping 
	::ReleaseMutex(g_hCheckIpMutex);
	return bRet;
}


//GB2312到UTF-8的转换
static int GB2312ToUtf8(const char* gb2312, char* utf8)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return len;
}


//UTF-8到GB2312的转换
static int Utf8ToGB2312(const char* utf8, char* gb2312)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, gb2312, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return len;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;                     // number of image encoders   
	UINT size = 0;                   // size of the image encoder array in bytes   
	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;     //   Failure   

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;     //   Failure   

	GetImageEncoders(num, size, pImageCodecInfo);
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;     //   Success   
		}
	}
	free(pImageCodecInfo);
	return -1;     //   Failure   
}

void Bitmap2Jpg(Gdiplus::Bitmap* pImage, const wchar_t* pFileName)//
{
	DeleteFile(CString(pFileName));

	Gdiplus::EncoderParameters encoderParameters;
	CLSID jpgClsid;
	GetEncoderClsid(L"image/jpeg", &jpgClsid);
	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;

	// Save the image as a JPEG with quality level 100.
	ULONG             quality;
	quality = 100;
	encoderParameters.Parameter[0].Value = &quality;
	Status status = pImage->Save(pFileName, &jpgClsid, &encoderParameters);
	if (status != Ok)
	{
		wprintf(L"%d Attempt to save %s failed./n", status, pFileName);
	}
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}



BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE); //设置小图标 
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUsePrscDLLDlg 对话框
CUsePrscDLLDlg::CUsePrscDLLDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_USEPRSCDLL_DIALOG, pParent)
{
	m_bBtnOkStatus = TRUE;
	m_bNetOk = TRUE; //假设网络可用
	m_hMutex = NULL;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bSelectAll = FALSE;
	m_bHigh = TRUE;  //默认使用高精度, 500次/天
	m_bNormal = FALSE; //5000次/天
}

CUsePrscDLLDlg::~CUsePrscDLLDlg()
{
	if (m_hMutex)
	{
		ReleaseMutex(m_hMutex);
		CloseHandle(m_hMutex);
	}
	
}

void CUsePrscDLLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_editOutput);
}

BEGIN_MESSAGE_MAP(CUsePrscDLLDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CUsePrscDLLDlg::OnBnClickedOk)
	ON_COMMAND_RANGE(ID_NORMAL, ID_HIGH,  &CUsePrscDLLDlg::OnCheckAccuracy)
	ON_COMMAND(ID_32771, &CUsePrscDLLDlg::OnCustomBaiDuID)
	ON_COMMAND(ID_32794, &CUsePrscDLLDlg::OnAbout)
	ON_COMMAND(ID_32795, &CUsePrscDLLDlg::OnDownloadSrc)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


DWORD  WINAPI  NetCheckProc(LPVOID lpParam)
{
	ResetEvent(g_hExitEvent); 
	while (TRUE)
	{
		if (FALSE == CheckIPReachable("114.114.114.114"))
			((CUsePrscDLLDlg *)lpParam)->m_bNetOk = FALSE;
		else 
			((CUsePrscDLLDlg *)lpParam)->m_bNetOk = TRUE;

		DWORD  dwWartRet = ::WaitForSingleObject(g_hExitEvent, 1);
		if (WAIT_OBJECT_0 == dwWartRet)
		{
			OutputDebugString("线程退出咯\n");
			break;
		}
	}
	return 0;
}


BOOL CUsePrscDLLDlg::OnInitDialog()
{
	//单例进程模式
	m_hMutex = ::CreateMutex(NULL, TRUE, _T("CUsePrscDLLDlg"));
	if (m_hMutex != NULL)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			MessageBox(_T("程序已经运行!"), _T("提示"));
			CloseHandle(m_hMutex);
			CDialogEx::OnOK();
		}
	}


	g_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hNetCheckThread = CreateThread(NULL, 0, NetCheckProc, this, NULL, NULL);


	g_hCheckIpMutex	= CreateMutex(NULL, FALSE, NULL);
	CDialogEx::OnInitDialog();


	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//加载菜单
	//CMenu menu;
	m_menu.LoadMenu(IDR_MAIN);   //menu应该保存起来 
	SetMenu(&m_menu);
	GetMenu()->GetSubMenu(0)->GetSubMenu(0)->CheckMenuRadioItem(0, 1, 0, MF_BYPOSITION  ); //改为ID_menuSkin2为选中项
	SetDlgItemText(IDC_EDIT_OUTPUT, _T("==============================================\r\n使用说明:\r\n\t1.请保持网络连接正常;\r\n\t2.请关闭360卫士 \t3.请关闭防火墙.\r\n\r\n\r\n温馨提示:\r\n\t本软件基于百度人工智能平台,识别结果可能存在误差.\r\n\r\n==============================================\r\n  \r\n\r\n 百度取消了免费api,  默认的API key已经失效， 需要自己付费购买 \r\n https://ai.baidu.com/ai-doc/OCR/skibizxzo  2025-02-27"));


	TCHAR szDocPath[_MAX_PATH];
	SHGetSpecialFolderPath(this->GetSafeHwnd(), szDocPath, CSIDL_MYDOCUMENTS, 0);
	CString strCfgFilePath = szDocPath + CString(_T("\\__ocr_config"));
	CStdioFile  file;
	AfxMessageBox(_T("百度取消了免费api， 需要自己付费购买 "));
	if (FALSE == file.Open(strCfgFilePath,  CStdioFile::shareCompat|CStdioFile::modeRead)) //使用默认账号
	{
		// 已经失效， 百度取消了免费api， 需要自己付费购买 2025-02-27 
		// https://ai.baidu.com/ai-doc/OCR/skibizxzo
		m_strAppID = _T("11176125");
		m_strApiKey = _T("nUKlV0kDnZTBzNDBsONDhCXu");
		m_strSecreteKey = _T("5riESvRvtMLHhe9SM3sSMCt87E4bCapM");
	}
	else
	{
		CString strTmp;

		BOOL   bAPPID = FALSE, bAPIKEY = FALSE, bSK = FALSE;
		while (file.ReadString(strTmp))
		{
			if (strTmp.Find("APPID") >= 0)
			{
				strTmp = strTmp.Mid(strTmp.Find("=") + 1);
				strTmp.Trim();
				if (8 == strTmp.GetLength())
				{
					m_strAppID = strTmp;
					bAPPID = TRUE;
				}
				else
				{
					AfxMessageBox("配置文件appid非法, 请检查");
					break;
				}
			}
			else if (strTmp.Find("APIKEY") >= 0)
			{
				strTmp = strTmp.Mid(strTmp.Find("=") + 1);
				strTmp.Trim();
				if (24 == strTmp.GetLength())
				{
					m_strApiKey = strTmp;
					bAPIKEY = TRUE;
				}
				else
				{
					AfxMessageBox("配置文件apikey非法, 请检查");
					break;
				}
			}
			else if (strTmp.Find("SECRETEKEY") >= 0)
			{
				strTmp = strTmp.Mid(strTmp.Find("=") + 1);
				strTmp.Trim();
				if (32 == strTmp.GetLength())
				{
					m_strSecreteKey = strTmp;
					bSK = TRUE;
				}
				else
				{
					AfxMessageBox("配置文件secretekey非法, 请检查");
					break;
				}
			}
		}
		file.Close();

		if (!(bAPPID && bAPIKEY && bSK)) //如果__config被损坏,则使用默认账号
		{
			m_strAppID = _T("11176125");
			m_strApiKey = _T("nUKlV0kDnZTBzNDBsONDhCXu");
			m_strSecreteKey = _T("5riESvRvtMLHhe9SM3sSMCt87E4bCapM");

			MessageBox(_T("由于账号配置文件被损坏,已自动切换到默认账号,可以继续使用.\r\n 如需自定义账号,您可以重新在\"设置\"中输入自定义账号信息."));
		}
	}


	//添加状态栏
	m_status.Create(this);
	m_status.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
	CRect rect;
	GetClientRect(&rect);
	m_status.SetPaneInfo(0, IDS_STRING_ROWS, SBPS_NORMAL, 120);
	m_status.SetPaneInfo(1, IDS_STRING_PROB, SBPS_NORMAL, rect.Width() - 100); //设置各栏长度
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, IDS_STRING_ROWS);//自动填充用户区域的窗口
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, IDS_STRING_PROB);//自动填充用户区域的窗口
	//m_status.GetStatusBarCtrl().SetBkColor(RGB(255, 100, 100));//设置状态栏颜色


	if(TRUE)
	{
		TEXTMETRIC tm;
		CRect rect;
		GetDlgItem(IDC_EDIT_OUTPUT)->GetClientRect(&rect);
		CDC* pdc = GetDlgItem(IDC_EDIT_OUTPUT)->GetDC();
		::GetTextMetrics(pdc->m_hDC, &tm);
		GetDlgItem(IDC_EDIT_OUTPUT)->ReleaseDC(pdc);
		m_nLineCount = rect.bottom / (tm.tmHeight - 1.5); //获取edit能够容纳的行数
		m_nColumnCount = rect.right/ (tm.tmMaxCharWidth - 1.5); //获取edit一行能够容纳的字符数
	}

	OnCheckAccuracy(ID_HIGH); //默认选用高精度, 就是这么任性!

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUsePrscDLLDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUsePrscDLLDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

INT  GetWindowMajorVerNo()
{
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

	return osvi.dwMajorVersion;  //win7:6    win10: 10
}



//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CUsePrscDLLDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




DWORD  WINAPI  OCRThreadProc(LPVOID  lpParam)
{
	CUsePrscDLLDlg  *pDlg = static_cast<CUsePrscDLLDlg *>(lpParam);

	// 设置APPID/AK/SK
	std::string app_id;
	std::string api_key;
	std::string secret_key;

	//app_id = "11176125";
	//api_key = "nUKlV0kDnZTBzNDBsONDhCXu";
	//secret_key = "5riESvRvtMLHhe9SM3sSMCt87E4bCapM";
	app_id = pDlg->m_strAppID;
	api_key = pDlg->m_strApiKey;
	secret_key = pDlg->m_strSecreteKey;


	aip::Ocr client(app_id, api_key, secret_key);

	Json::Value result;

	std::string image;
	if (-1 == aip::get_file_content(pDlg->m_strJpgName.GetBuffer(), &image))
	{
		pDlg->MessageBox(_T("获取图片内容失败,可能是截取区域太小或太大"), _T("提示"), MB_OK);
		pDlg->UpdateWindow();
		DeleteFile(pDlg->m_strJpgName);

		pDlg->GetDlgItem(IDOK)->EnableWindow(TRUE);
		pDlg->GetSystemMenu(FALSE)->EnableMenuItem(SC_CLOSE, MF_ENABLED);
		pDlg->m_bBtnOkStatus = TRUE;
		return 0;
	}

	// 如果有可选参数
	std::map<std::string, std::string> options;
	options["language_type"] = "CHN_ENG";
	options["detect_direction"] = "true";
	options["detect_language"] = "true";
	options["probability"] = "true";

	int result_num = 0;
	for (int iTry = 0; iTry < 5; iTry++)  //尝试5次, 如果5次都失败, 没办法了
	{
		if (pDlg->m_bHigh)
			result = client.accurate_basic(image, options); //高精度
		else
			result = client.general_basic(image, options); //普通精度
		result_num = result["words_result_num"].asInt();
		if (result_num > 0)
			break;
	}

	double probabilitySum = 0.0;
	int  iMaxCharCount = 0; //识别结果中一行包含的 最大字符数
	CString strOutput = _T("");
	if (result_num == 0)
	{
		std::string  err_msg = result["error_msg"].asString();
		std::string  err_code = result["error_code"].asString();
		strOutput = CString(_T("错误码:")) + CString(err_code.c_str()) + CString("\r\n") + _T("错误描述:") + CString(err_msg.c_str()) + _T("\r\n");
		strOutput += CString(_T("\r\n本次识别未成功,很抱歉,请您再试试. 祝您好运! ^_^"));

		Json::StyledWriter sw;
		std::ofstream os;
		os.open("tmp.json");
		os << sw.write(result);
		os.close();
	}
	else
	{
		for (int i = 0; i < result["words_result"].size(); i++)
		{
			probabilitySum += result["words_result"][i]["probability"]["average"].asDouble(); //识别的精确度
			std::string tmpStr = result["words_result"][i]["words"].asCString();
			char *pgb2312 = new char[tmpStr.length() * 2];
			memset(pgb2312, 0, tmpStr.length() * 2);
			Utf8ToGB2312(tmpStr.c_str(), pgb2312);
			strOutput += pgb2312 + CString("\r\n");

			CString strTmp; strTmp = pgb2312;
			if (strTmp.GetLength() > iMaxCharCount /*字符数,不是字节数*/)
				iMaxCharCount = strTmp.GetLength();

			if (pgb2312)
			{
				delete[] pgb2312;
				pgb2312 = NULL;
			}
		}
	}

	double probabilityAvr = (0 == result["words_result"].size()) ? (0) : (probabilitySum / (double)result["words_result"].size());
	CString strProbOutput;
	strProbOutput.Format("%.1f%%", probabilityAvr * 100);
	pDlg->m_status.SetPaneText(1, _T("平均准确率: ") + strProbOutput);

	int iRows = result["words_result"].size();
	CString strRows;
	strRows.Format("总行数: %d", iRows);
	pDlg->m_status.SetPaneText(0, strRows);


	pDlg->SetDlgItemText(IDC_EDIT_OUTPUT, _T(""));
	pDlg->SetDlgItemText(IDC_EDIT_OUTPUT, strOutput);

	int nLine = ((CEdit*)(pDlg->GetDlgItem(IDC_EDIT_OUTPUT)))->GetLineCount();

	if (nLine > pDlg->m_nLineCount)
		pDlg->GetDlgItem(IDC_EDIT_OUTPUT)->ShowScrollBar(SB_VERT, TRUE);
	else
		pDlg->GetDlgItem(IDC_EDIT_OUTPUT)->ShowScrollBar(SB_VERT, FALSE);

	if (iMaxCharCount > pDlg->m_nColumnCount)
		pDlg->GetDlgItem(IDC_EDIT_OUTPUT)->ShowScrollBar(SB_HORZ, TRUE);
	else
		pDlg->GetDlgItem(IDC_EDIT_OUTPUT)->ShowScrollBar(SB_HORZ, FALSE);

	pDlg->UpdateWindow();
	DeleteFile(pDlg->m_strJpgName);


	pDlg->GetDlgItem(IDOK)->EnableWindow(TRUE);
	pDlg->GetSystemMenu(FALSE)->EnableMenuItem(SC_CLOSE, MF_ENABLED);
	pDlg->m_bBtnOkStatus = TRUE;

	return 0;
}



void CUsePrscDLLDlg::OnBnClickedOk()
{
	CRect rectDlg;
	GetWindowRect(rectDlg);//获得窗体的大小

	TCHAR szTempDir[MAX_PATH];
	memset(szTempDir, 0, sizeof(szTempDir));
	GetTempPath(sizeof(szTempDir), szTempDir);//获取临时文件夹路径

	CString strJpgName = szTempDir + CString(_T("__ocr_temp.jpg"));
	m_strJpgName = strJpgName;

	BOOL bNewImageFlag = FALSE;
	HINSTANCE hPrScrnDLL = LoadLibrary("PrScrn.dll");
	if (hPrScrnDLL != NULL)
	{
		MoveWindow(rectDlg.TopLeft().x, rectDlg.TopLeft().y, 0, 0, TRUE);


		//清除剪切板中的数据
		if (OpenClipboard())
		{
			BOOL bClear = EmptyClipboard();
			while (!CloseClipboard()) {};
		}

		//从PrScrn.dll 获取截图入口地址
		EntryPoint pfnScreenShot = (EntryPoint)GetProcAddress(hPrScrnDLL, _T("PrScrn"));
		if (pfnScreenShot != NULL)
		{
			//执行截图
			pfnScreenShot((HWND)this->m_hWnd, hPrScrnDLL, "PrScrn", 0);

			if (OpenClipboard())
			{
				//从剪切板获取位图数据
				HBITMAP hBmNew = (HBITMAP)GetClipboardData(CF_BITMAP);
				if (hBmNew)
				{
					Gdiplus::Bitmap bitmap(hBmNew, NULL);  //直接获取内存中的bmp数据
					Bitmap2Jpg(&bitmap, strJpgName.AllocSysString());  //将bmp数据转为 jpg, jpg比bmp小很多
					//隐藏文件
					CFileStatus fs;
					CFile::GetStatus(strJpgName, fs);
					fs.m_attribute = CFile::hidden; 
					CFile::SetStatus(strJpgName, fs);
					
					if (hBmNew)
					{
						DeleteObject(hBmNew);
						hBmNew = NULL;
					}
					bNewImageFlag = TRUE;
				}

				//关闭剪切板
				while (!CloseClipboard()) {};
			}
			
		}
		else
		{
			MessageBox(_T(" 未找到 PrScrn函数！ 请联系开发者!"), _T("严重错误"));
			return;
		}

		FreeLibrary(hPrScrnDLL); //释放动态库
	}
	else
	{
		MessageBox(_T("加载！PrScrn.dll失败,请重试."), _T("提示"));
		return;
	}
	MoveWindow(rectDlg, TRUE);  //重新显示对话框
	UpdateWindow(); //立即显示出来

	if (bNewImageFlag)
	{
		if (FALSE == m_bNetOk)
		{
			MessageBox(_T("网络不可用, 请检查网络"), _T("提示"), MB_OK);
			return;
		}

		SetDlgItemText(IDC_EDIT_OUTPUT, _T("识别中......"));
		UpdateWindow(); //立即显示出来
		GetSystemMenu(FALSE)->EnableMenuItem(SC_CLOSE, MF_DISABLED);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		//创建线程
		m_bBtnOkStatus = FALSE;
		CreateThread(NULL, 0, OCRThreadProc, this, 0, NULL);
	}
}

BOOL CUsePrscDLLDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		BOOL bCtrl = ::GetKeyState(VK_CONTROL) & 0x8000;
		BOOL bAlt = ::GetKeyState(VK_MENU) & 0x8000;
		BOOL bShift = ::GetKeyState(VK_SHIFT) & 0x8000;


		switch (pMsg->wParam)
		{
		case 'A': //ctrl + a   全选
			if (bCtrl)
			{
				CString txt;
				int start, end;
				m_editOutput.GetWindowText(txt);
				m_editOutput.GetSel(start, end);

				if(txt.GetLength() == end - start) //当前是全选状态,则取消全选
				{
					m_editOutput.SetSel(-1);
				}
				else 
				{
					m_editOutput.SetSel(0, -1);
				}
				m_bSelectAll = !m_bSelectAll;
			}
			break;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CUsePrscDLLDlg::OnCheckAccuracy(UINT uID)
{
	CMenu* submenu = m_menu.GetSubMenu(0);
	if (TRUE == submenu->CheckMenuRadioItem(ID_NORMAL, ID_HIGH, uID, MF_BYCOMMAND))
	{
		switch (uID)
		{
		case ID_NORMAL:
			m_bNormal = TRUE;
			m_bHigh = FALSE;
			break;
		case ID_HIGH:
			m_bHigh = TRUE;
			m_bNormal = FALSE;
			break;
		default:
			break;
		}
	}
	
}

void CUsePrscDLLDlg::OnCustomBaiDuID()
{
	CCurstomBaiDuIdDlg customDlg;
	//customDlg.SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
	customDlg.DoModal();
}

void CUsePrscDLLDlg::OnAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}




void CUsePrscDLLDlg::OnDownloadSrc()
{
	char szFileName[] = "青狗-截图文字识别_源代码.7z";
	if (CheckIPReachable(_T("192.168.2.55"))/* && 0 == DownloadSrcFile("http://192.168.2.55/Other/ocr/ocr.7z", szFileName)*/) //本地局域网下载
	{
		ShellExecute(NULL, _T("open"), _T("http://192.168.2.55/Other/ocr"), NULL, NULL, SW_SHOW);
		//MessageBox(_T("下载成功, 保存在程序所在文件夹, 文件名为: 青狗-截图文字识别_源代码.7z "), _T("提示"));
		return;
	}
	else //访问github
	{
		//::DeleteFile(szFileName); //删除下载失败的空文件
		ShellExecute(NULL, _T("open"), _T("https://github.com/youngqqcn/QOcr"), NULL, NULL, SW_SHOW);
		return;
	}
	
}



void CUsePrscDLLDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	MoveWindow(0, 0, 0, 0, TRUE); //隐藏窗口
	ShowWindow(SW_HIDE); //隐藏任务栏图片

	//等待 网络检测线程 退出
	SetEvent(g_hExitEvent);
	::WaitForSingleObject(m_hNetCheckThread, 1000);
	CDialogEx::OnClose();
}



