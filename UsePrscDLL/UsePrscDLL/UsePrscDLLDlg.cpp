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

#include "CurstomBaiDuIdDlg.h"

#include <curl/curl.h>
#include <json/json.h>
#include "ocr.h"  //文字识别
using namespace Gdiplus;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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




/*  libcurl write callback function */
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}


/*
Function:   libcurl connection initialization  download  file
Parameters:   (const char* url, const char outfilename[FILENAME_MAX])
url:   要下载文件的url地址
outfilename:   下载文件指定的文件名
*/
//不支持  https 下载
int DOWNLOAD_FILE(const char* url, const char outfilename[FILENAME_MAX]) {
	CURL *curl;
	FILE *fp;
	CURLcode res;
	/*   调用curl_global_init()初始化libcurl  */
	res = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != res)
	{
		printf("init libcurl failed.");
		curl_global_cleanup();
		return -1;
	}
	/*  调用curl_easy_init()函数得到 easy interface型指针  */
	curl = curl_easy_init();
	if (curl) {

		fopen_s(&fp, outfilename, "wb");

		/*  调用curl_easy_setopt()设置传输选项 */
		res = curl_easy_setopt(curl, CURLOPT_URL, url);
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}
		/*  根据curl_easy_setopt()设置的传输选项，实现回调函数以完成用户特定任务  */
		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}
		/*  根据curl_easy_setopt()设置的传输选项，实现回调函数以完成用户特定任务  */
		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}

		res = curl_easy_perform(curl);                               // 调用curl_easy_perform()函数完成传输任务  
		fclose(fp);
		/* Check for errors */
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			curl_easy_cleanup(curl);
			return -1;
		}

		/* always cleanup */
		curl_easy_cleanup(curl);                                     // 调用curl_easy_cleanup()释放内存   

	}
	curl_global_cleanup();
	return 0;
}





static UINT BASED_CODE indicators[] =
{
	IDS_STRING_ROWS,
	IDS_STRING_PROB
};

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

//保存剪切板中的位图 为 图片文件
BOOL SaveBitmapToFile(HBITMAP   hBitmap, CString szfilename)
{
	HDC     hDC;
	//当前分辨率下每象素所占字节数          
	int     iBits;
	//位图中每象素所占字节数          
	WORD     wBitCount;
	//定义调色板大小，     位图中像素字节大小     ，位图文件大小     ，     写入文件字节数              
	DWORD     dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	//位图属性结构              
	BITMAP     Bitmap;
	//位图文件头结构          
	BITMAPFILEHEADER     bmfHdr;
	//位图信息头结构              
	BITMAPINFOHEADER     bi;
	//指向位图信息头结构                  
	LPBITMAPINFOHEADER     lpbi;
	//定义文件，分配内存句柄，调色板句柄              
	HANDLE     fh, hDib, hPal, hOldPal = NULL;

	//计算位图文件每个像素所占字节数              
	hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL)     *     GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else  if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else
		wBitCount = 24;

	GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;

	dwBmBitsSize = ((Bitmap.bmWidth *wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	//为位图内容分配内存              
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	//     处理调色板                  
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	//     获取该调色板下新的像素值              
	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
		(BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	//恢复调色板                  
	if (hOldPal)
	{
		::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	//创建位图文件                  
	fh = CreateFile(szfilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)         return     FALSE;

	//     设置位图文件头              
	bmfHdr.bfType = 0x4D42;     //     "BM"              
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
	//     写入位图文件头              
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	//     写入位图文件其余内容              
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	//清除                  
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);

	return     TRUE;

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
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUsePrscDLLDlg 对话框



CUsePrscDLLDlg::CUsePrscDLLDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_USEPRSCDLL_DIALOG, pParent)
{
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
END_MESSAGE_MAP()


// CUsePrscDLLDlg 消息处理程序

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
	SetDlgItemText(IDC_EDIT_OUTPUT, _T("==============================================\r\n使用说明:\r\n\t1.请保持网络连接正常;\r\n\t2.请关闭\"某安全卫士\"或\"某毒霸\";\r\n\t3.请关闭防火墙.\r\n\r\n\r\n温馨提示:\r\n\t本软件基于百度人工智能平台,识别结果可能存在误差.\r\n\r\n==============================================\r\n"));


	TCHAR szDocPath[_MAX_PATH];
	SHGetSpecialFolderPath(this->GetSafeHwnd(), szDocPath, CSIDL_MYDOCUMENTS, 0);
	CString strCfgFilePath = szDocPath + CString(_T("\\__ocr_config"));
	CStdioFile  file;
	if (FALSE == file.Open(strCfgFilePath,  CStdioFile::shareCompat|CStdioFile::modeRead)) //使用默认账号
	{
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
	BOOL bIsWindowsXPorLater;
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


void CUsePrscDLLDlg::OnBnClickedOk()
{
#if 0
	Gdiplus::Bitmap newbitmap(L"test.bmp");
	const wchar_t *pFileName = L"new.jpg";
	Bitmap2Jpg(&newbitmap, pFileName);

	return;
#endif



	CRect rectDlg;
	GetWindowRect(rectDlg);//获得窗体的大小

	TCHAR szTempDir[MAX_PATH];
	memset(szTempDir, 0, sizeof(szTempDir));
	GetTempPath(sizeof(szTempDir), szTempDir);//获取临时文件夹路径
	CString strBmpName = szTempDir + CString(_T("__ocr_temp.bmp"));
	CString strJpgName = szTempDir + CString(_T("__ocr_temp.jpg"));
	BOOL bNewImageFlag = FALSE;
	HINSTANCE dll_handle = LoadLibrary("PrScrn.dll");
	if (dll_handle != NULL)
	{
		
		MoveWindow(rectDlg.TopLeft().x, rectDlg.TopLeft().y, 0, 0, TRUE);


		//清除剪切板中的数据
		if (OpenClipboard())
		{
			BOOL bClear = EmptyClipboard();
			while (!CloseClipboard()) {};
		}

		EntryPoint my_fun = (EntryPoint)GetProcAddress(dll_handle, _T("PrScrn"));
		if (my_fun != NULL)
		{
			my_fun((HWND)this->m_hWnd, dll_handle, "PrScrn", 0);

			if (OpenClipboard())
			{
				//从剪切板获取位图数据
				HBITMAP hBmNew = (HBITMAP)GetClipboardData(CF_BITMAP);
				if (hBmNew)
				{
					//保存为图片文件
					//SaveBitmapToFile(hBmNew, strBmpName);

#if defined(USE_GDI)
					//CString strMajor; strMajor.Format("%d", GetWindowMajorVerNo());
					//MessageBox(strMajor, _T("window版本号"), MB_OK);
					//if (GetWindowMajorVerNo() == 10) //win10
					{
						//Gdiplus::Bitmap bitmap(strBmpName.AllocSysString());
						Gdiplus::Bitmap bitmap(hBmNew, NULL);  //直接获取内存中的bmp数据
						Bitmap2Jpg(&bitmap, strJpgName.AllocSysString());  //将bmp数据转为 jpg, jpg比bmp小很多
						//DeleteFile(strBmpName);
						strBmpName = strJpgName;
					}
#endif

					//隐藏文件
					CFileStatus fs;
					CFile::GetStatus(strBmpName, fs);
					fs.m_attribute = CFile::hidden; 
					CFile::SetStatus(strBmpName, fs);

					//关闭剪切板
					while (!CloseClipboard()) {};
					if (hBmNew)
					{
						DeleteObject(hBmNew);
						hBmNew = NULL;
					}
					bNewImageFlag = TRUE;
				}
			}
		}
		else
		{
			MessageBox(_T(" 未找到 PrScrn函数！ 请联系开发者!"), _T("严重错误"));
			return;
		}

		FreeLibrary(dll_handle); //释放动态库
	}
	else
	{
		MessageBox(_T("加载！PrScrn.dll失败"), _T("提示"));
	}


#if 1
	
	MoveWindow(rectDlg, TRUE);  //重新显示对话框
	UpdateWindow(); //立即显示出来
	if (bNewImageFlag)
	{
		SetDlgItemText(IDC_EDIT_OUTPUT, _T("识别中......"));
		UpdateWindow(); //立即显示出来

		// 设置APPID/AK/SK
		std::string app_id;
		std::string api_key;
		std::string secret_key;

		//app_id = "11176125";
		//api_key = "nUKlV0kDnZTBzNDBsONDhCXu";
		//secret_key = "5riESvRvtMLHhe9SM3sSMCt87E4bCapM";
		app_id = m_strAppID;
		api_key = m_strApiKey;
		secret_key = m_strSecreteKey;


		aip::Ocr client(app_id, api_key, secret_key);

		Json::Value result;

		std::string image;
		aip::get_file_content(strBmpName.GetBuffer(), &image);

		// 如果有可选参数
		std::map<std::string, std::string> options;
		options["language_type"] = "CHN_ENG";
		options["detect_direction"] = "true";
		options["detect_language"] = "true";
		options["probability"] = "true";

		int result_num = 0;
		for (int iTry = 0; iTry < 5; iTry++)  //尝试5次, 如果5次都失败, 没办法了
		{
			if (m_bHigh)
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

		double probabilityAvr = (0 == result["words_result"].size() ) ? (0) : (probabilitySum / (double)result["words_result"].size());
		CString strProbOutput;
		strProbOutput.Format("%.1f%%", probabilityAvr * 100);
		m_status.SetPaneText(1, _T("平均置信度: ") + strProbOutput);

		int iRows = result["words_result"].size();
		CString strRows; 
		strRows.Format("总行数: %d", iRows);
		m_status.SetPaneText(0, strRows);
		

		SetDlgItemText(IDC_EDIT_OUTPUT, _T(""));
		SetDlgItemText(IDC_EDIT_OUTPUT, strOutput);

		int nLine = ((CEdit*)GetDlgItem(IDC_EDIT_OUTPUT))->GetLineCount();
		
		if (nLine > m_nLineCount)
			GetDlgItem(IDC_EDIT_OUTPUT)->ShowScrollBar(SB_VERT, TRUE);
		else
			GetDlgItem(IDC_EDIT_OUTPUT)->ShowScrollBar(SB_VERT, FALSE);

		if(iMaxCharCount > m_nColumnCount)
			GetDlgItem(IDC_EDIT_OUTPUT)->ShowScrollBar(SB_HORZ, TRUE);
		else
			GetDlgItem(IDC_EDIT_OUTPUT)->ShowScrollBar(SB_HORZ, FALSE);

		UpdateWindow();
		DeleteFile(strBmpName);
		DeleteFile(strJpgName);
	}

#endif
	
}

BOOL CUsePrscDLLDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		BOOL bCtrl = ::GetKeyState(VK_CONTROL) & 0x8000;
		BOOL bAlt = ::GetKeyState(VK_MENU) & 0x8000;


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
	customDlg.DoModal();
}

void CUsePrscDLLDlg::OnAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}



void CUsePrscDLLDlg::OnDownloadSrc()
{
	if (0 == DOWNLOAD_FILE("http://192.168.2.55/Other/ocr/ocr.7z", "截图文字识别_源代码.7z")) //本地局域网下载
	{
		MessageBox(_T("下载成功, 保存在程序所在文件夹, 文件名为: 截图文字识别_源代码.7z "), _T("提示"));
	}
	else //访问github
	{
		ShellExecute(NULL, _T("open"), _T("https://github.com/youngqqcn/QOcr"), NULL, NULL, SW_SHOW);
	}
	
}
