/*
//���� :  yqq 
//���� : youngqqcn@qq.com
//�ҵ�github��ҳ:   https://github.com/youngqqcn
//ÿһƬ��,�������Լ�����ɫ. 2018-09-04 yqq ��
*/

#include "stdafx.h"
#include "UsePrscDLL.h"
#include "UsePrscDLLDlg.h"
#include "afxdialogex.h"

#include "CurstomBaiDuIdDlg.h"

#include <curl/curl.h>
#include <json/json.h>
#include "ocr.h"  //����ʶ��
using namespace Gdiplus;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "../lib/libcurl.lib")
#pragma comment(lib, "../lib/libeay32.lib")
#pragma comment(lib, "../lib/ssleay32.lib")
#pragma comment(lib, "../lib/GdiPlus.lib")  //2018-09-21 ����ͼƬΪjpg��ʽ, ���win10��ͼƬ������ʧ�ܵ����� 


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
url:   Ҫ�����ļ���url��ַ
outfilename:   �����ļ�ָ�����ļ���
*/
//��֧��  https ����
int DOWNLOAD_FILE(const char* url, const char outfilename[FILENAME_MAX]) {
	CURL *curl;
	FILE *fp;
	CURLcode res;
	/*   ����curl_global_init()��ʼ��libcurl  */
	res = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != res)
	{
		printf("init libcurl failed.");
		curl_global_cleanup();
		return -1;
	}
	/*  ����curl_easy_init()�����õ� easy interface��ָ��  */
	curl = curl_easy_init();
	if (curl) {

		fopen_s(&fp, outfilename, "wb");

		/*  ����curl_easy_setopt()���ô���ѡ�� */
		res = curl_easy_setopt(curl, CURLOPT_URL, url);
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}
		/*  ����curl_easy_setopt()���õĴ���ѡ�ʵ�ֻص�����������û��ض�����  */
		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}
		/*  ����curl_easy_setopt()���õĴ���ѡ�ʵ�ֻص�����������û��ض�����  */
		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(curl);
			return -1;
		}

		res = curl_easy_perform(curl);                               // ����curl_easy_perform()������ɴ�������  
		fclose(fp);
		/* Check for errors */
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			curl_easy_cleanup(curl);
			return -1;
		}

		/* always cleanup */
		curl_easy_cleanup(curl);                                     // ����curl_easy_cleanup()�ͷ��ڴ�   

	}
	curl_global_cleanup();
	return 0;
}





static UINT BASED_CODE indicators[] =
{
	IDS_STRING_ROWS,
	IDS_STRING_PROB
};

//GB2312��UTF-8��ת��
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


//UTF-8��GB2312��ת��
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

//������а��е�λͼ Ϊ ͼƬ�ļ�
BOOL SaveBitmapToFile(HBITMAP   hBitmap, CString szfilename)
{
	HDC     hDC;
	//��ǰ�ֱ�����ÿ������ռ�ֽ���          
	int     iBits;
	//λͼ��ÿ������ռ�ֽ���          
	WORD     wBitCount;
	//�����ɫ���С��     λͼ�������ֽڴ�С     ��λͼ�ļ���С     ��     д���ļ��ֽ���              
	DWORD     dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	//λͼ���Խṹ              
	BITMAP     Bitmap;
	//λͼ�ļ�ͷ�ṹ          
	BITMAPFILEHEADER     bmfHdr;
	//λͼ��Ϣͷ�ṹ              
	BITMAPINFOHEADER     bi;
	//ָ��λͼ��Ϣͷ�ṹ                  
	LPBITMAPINFOHEADER     lpbi;
	//�����ļ��������ڴ�������ɫ����              
	HANDLE     fh, hDib, hPal, hOldPal = NULL;

	//����λͼ�ļ�ÿ��������ռ�ֽ���              
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

	//Ϊλͼ���ݷ����ڴ�              
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	//     �����ɫ��                  
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	//     ��ȡ�õ�ɫ�����µ�����ֵ              
	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
		(BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	//�ָ���ɫ��                  
	if (hOldPal)
	{
		::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	//����λͼ�ļ�                  
	fh = CreateFile(szfilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)         return     FALSE;

	//     ����λͼ�ļ�ͷ              
	bmfHdr.bfType = 0x4D42;     //     "BM"              
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
	//     д��λͼ�ļ�ͷ              
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	//     д��λͼ�ļ���������              
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	//���                  
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


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CUsePrscDLLDlg �Ի���



CUsePrscDLLDlg::CUsePrscDLLDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_USEPRSCDLL_DIALOG, pParent)
{
	m_hMutex = NULL;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bSelectAll = FALSE;
	m_bHigh = TRUE;  //Ĭ��ʹ�ø߾���, 500��/��
	m_bNormal = FALSE; //5000��/��
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


// CUsePrscDLLDlg ��Ϣ�������

BOOL CUsePrscDLLDlg::OnInitDialog()
{
	//��������ģʽ
	m_hMutex = ::CreateMutex(NULL, TRUE, _T("CUsePrscDLLDlg"));
	if (m_hMutex != NULL)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			MessageBox(_T("�����Ѿ�����!"), _T("��ʾ"));
			CloseHandle(m_hMutex);
			CDialogEx::OnOK();
		}
	}
	



	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//���ز˵�
	//CMenu menu;
	m_menu.LoadMenu(IDR_MAIN);   //menuӦ�ñ������� 
	SetMenu(&m_menu);
	GetMenu()->GetSubMenu(0)->GetSubMenu(0)->CheckMenuRadioItem(0, 1, 0, MF_BYPOSITION  ); //��ΪID_menuSkin2Ϊѡ����
	SetDlgItemText(IDC_EDIT_OUTPUT, _T("==============================================\r\nʹ��˵��:\r\n\t1.�뱣��������������;\r\n\t2.��ر�\"ĳ��ȫ��ʿ\"��\"ĳ����\";\r\n\t3.��رշ���ǽ.\r\n\r\n\r\n��ܰ��ʾ:\r\n\t��������ڰٶ��˹�����ƽ̨,ʶ�������ܴ������.\r\n\r\n==============================================\r\n"));


	TCHAR szDocPath[_MAX_PATH];
	SHGetSpecialFolderPath(this->GetSafeHwnd(), szDocPath, CSIDL_MYDOCUMENTS, 0);
	CString strCfgFilePath = szDocPath + CString(_T("\\__ocr_config"));
	CStdioFile  file;
	if (FALSE == file.Open(strCfgFilePath,  CStdioFile::shareCompat|CStdioFile::modeRead)) //ʹ��Ĭ���˺�
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
					AfxMessageBox("�����ļ�appid�Ƿ�, ����");
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
					AfxMessageBox("�����ļ�apikey�Ƿ�, ����");
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
					AfxMessageBox("�����ļ�secretekey�Ƿ�, ����");
					break;
				}
			}
		}
		file.Close();

		if (!(bAPPID && bAPIKEY && bSK)) //���__config����,��ʹ��Ĭ���˺�
		{
			m_strAppID = _T("11176125");
			m_strApiKey = _T("nUKlV0kDnZTBzNDBsONDhCXu");
			m_strSecreteKey = _T("5riESvRvtMLHhe9SM3sSMCt87E4bCapM");

			MessageBox(_T("�����˺������ļ�����,���Զ��л���Ĭ���˺�,���Լ���ʹ��.\r\n �����Զ����˺�,������������\"����\"�������Զ����˺���Ϣ."));
		}
	}


	//���״̬��
	m_status.Create(this);
	m_status.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
	CRect rect;
	GetClientRect(&rect);
	m_status.SetPaneInfo(0, IDS_STRING_ROWS, SBPS_NORMAL, 120);
	m_status.SetPaneInfo(1, IDS_STRING_PROB, SBPS_NORMAL, rect.Width() - 100); //���ø�������
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, IDS_STRING_ROWS);//�Զ�����û�����Ĵ���
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, IDS_STRING_PROB);//�Զ�����û�����Ĵ���
	//m_status.GetStatusBarCtrl().SetBkColor(RGB(255, 100, 100));//����״̬����ɫ


	if(TRUE)
	{
		TEXTMETRIC tm;
		CRect rect;
		GetDlgItem(IDC_EDIT_OUTPUT)->GetClientRect(&rect);
		CDC* pdc = GetDlgItem(IDC_EDIT_OUTPUT)->GetDC();
		::GetTextMetrics(pdc->m_hDC, &tm);
		GetDlgItem(IDC_EDIT_OUTPUT)->ReleaseDC(pdc);
		m_nLineCount = rect.bottom / (tm.tmHeight - 1.5); //��ȡedit�ܹ����ɵ�����
		m_nColumnCount = rect.right/ (tm.tmMaxCharWidth - 1.5); //��ȡeditһ���ܹ����ɵ��ַ���
	}

	OnCheckAccuracy(ID_HIGH); //Ĭ��ѡ�ø߾���, ������ô����!

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CUsePrscDLLDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
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



//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
	GetWindowRect(rectDlg);//��ô���Ĵ�С

	TCHAR szTempDir[MAX_PATH];
	memset(szTempDir, 0, sizeof(szTempDir));
	GetTempPath(sizeof(szTempDir), szTempDir);//��ȡ��ʱ�ļ���·��
	CString strBmpName = szTempDir + CString(_T("__ocr_temp.bmp"));
	CString strJpgName = szTempDir + CString(_T("__ocr_temp.jpg"));
	BOOL bNewImageFlag = FALSE;
	HINSTANCE dll_handle = LoadLibrary("PrScrn.dll");
	if (dll_handle != NULL)
	{
		
		MoveWindow(rectDlg.TopLeft().x, rectDlg.TopLeft().y, 0, 0, TRUE);


		//������а��е�����
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
				//�Ӽ��а��ȡλͼ����
				HBITMAP hBmNew = (HBITMAP)GetClipboardData(CF_BITMAP);
				if (hBmNew)
				{
					//����ΪͼƬ�ļ�
					//SaveBitmapToFile(hBmNew, strBmpName);

#if defined(USE_GDI)
					//CString strMajor; strMajor.Format("%d", GetWindowMajorVerNo());
					//MessageBox(strMajor, _T("window�汾��"), MB_OK);
					//if (GetWindowMajorVerNo() == 10) //win10
					{
						//Gdiplus::Bitmap bitmap(strBmpName.AllocSysString());
						Gdiplus::Bitmap bitmap(hBmNew, NULL);  //ֱ�ӻ�ȡ�ڴ��е�bmp����
						Bitmap2Jpg(&bitmap, strJpgName.AllocSysString());  //��bmp����תΪ jpg, jpg��bmpС�ܶ�
						//DeleteFile(strBmpName);
						strBmpName = strJpgName;
					}
#endif

					//�����ļ�
					CFileStatus fs;
					CFile::GetStatus(strBmpName, fs);
					fs.m_attribute = CFile::hidden; 
					CFile::SetStatus(strBmpName, fs);

					//�رռ��а�
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
			MessageBox(_T(" δ�ҵ� PrScrn������ ����ϵ������!"), _T("���ش���"));
			return;
		}

		FreeLibrary(dll_handle); //�ͷŶ�̬��
	}
	else
	{
		MessageBox(_T("���أ�PrScrn.dllʧ��"), _T("��ʾ"));
	}


#if 1
	
	MoveWindow(rectDlg, TRUE);  //������ʾ�Ի���
	UpdateWindow(); //������ʾ����
	if (bNewImageFlag)
	{
		SetDlgItemText(IDC_EDIT_OUTPUT, _T("ʶ����......"));
		UpdateWindow(); //������ʾ����

		// ����APPID/AK/SK
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

		// ����п�ѡ����
		std::map<std::string, std::string> options;
		options["language_type"] = "CHN_ENG";
		options["detect_direction"] = "true";
		options["detect_language"] = "true";
		options["probability"] = "true";

		int result_num = 0;
		for (int iTry = 0; iTry < 5; iTry++)  //����5��, ���5�ζ�ʧ��, û�취��
		{
			if (m_bHigh)
				result = client.accurate_basic(image, options); //�߾���
			else
				result = client.general_basic(image, options); //��ͨ����
			result_num = result["words_result_num"].asInt();
			if (result_num > 0)
				break;
		}

		double probabilitySum = 0.0;
		int  iMaxCharCount = 0; //ʶ������һ�а����� ����ַ���
		CString strOutput = _T("");
		if (result_num == 0)
		{
			std::string  err_msg = result["error_msg"].asString();
			std::string  err_code = result["error_code"].asString();
			strOutput = CString(_T("������:")) + CString(err_code.c_str()) + CString("\r\n") + _T("��������:") + CString(err_msg.c_str()) + _T("\r\n");
			strOutput += CString(_T("\r\n����ʶ��δ�ɹ�,�ܱ�Ǹ,����������. ף������! ^_^"));

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
				probabilitySum += result["words_result"][i]["probability"]["average"].asDouble(); //ʶ��ľ�ȷ��
				std::string tmpStr = result["words_result"][i]["words"].asCString();
				char *pgb2312 = new char[tmpStr.length() * 2];
				memset(pgb2312, 0, tmpStr.length() * 2);
				Utf8ToGB2312(tmpStr.c_str(), pgb2312);
				strOutput += pgb2312 + CString("\r\n");

				CString strTmp; strTmp = pgb2312;
				if (strTmp.GetLength() > iMaxCharCount /*�ַ���,�����ֽ���*/)
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
		m_status.SetPaneText(1, _T("ƽ�����Ŷ�: ") + strProbOutput);

		int iRows = result["words_result"].size();
		CString strRows; 
		strRows.Format("������: %d", iRows);
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
		case 'A': //ctrl + a   ȫѡ
			if (bCtrl)
			{
				CString txt;
				int start, end;
				m_editOutput.GetWindowText(txt);
				m_editOutput.GetSel(start, end);

				if(txt.GetLength() == end - start) //��ǰ��ȫѡ״̬,��ȡ��ȫѡ
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
	if (0 == DOWNLOAD_FILE("http://192.168.2.55/Other/ocr/ocr.7z", "��ͼ����ʶ��_Դ����.7z")) //���ؾ���������
	{
		MessageBox(_T("���سɹ�, �����ڳ��������ļ���, �ļ���Ϊ: ��ͼ����ʶ��_Դ����.7z "), _T("��ʾ"));
	}
	else //����github
	{
		ShellExecute(NULL, _T("open"), _T("https://github.com/youngqqcn/QOcr"), NULL, NULL, SW_SHOW);
	}
	
}
