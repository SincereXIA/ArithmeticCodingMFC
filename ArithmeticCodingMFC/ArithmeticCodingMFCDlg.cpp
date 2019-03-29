
// ArithmeticCodingMFCDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "ArithmeticCodingMFC.h"
#include "ArithmeticCodingMFCDlg.h"
#include "afxdialogex.h"
#include "ACC.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CArithmeticCodingMFCDlg 对话框



CArithmeticCodingMFCDlg::CArithmeticCodingMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ARITHMETICCODINGMFC_DIALOG, pParent)
	, strinput(_T(""))
	, m_Status_Info(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CArithmeticCodingMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STRInput, strinput);
	DDX_Text(pDX, IDC_Status_Info, m_Status_Info);
}

BEGIN_MESSAGE_MAP(CArithmeticCodingMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_encode_button, &CArithmeticCodingMFCDlg::OnBnClickedencodebutton)
	ON_BN_CLICKED(IDC_decode_BUTTON, &CArithmeticCodingMFCDlg::OnBnClickeddecodeButton)
END_MESSAGE_MAP()


// CArithmeticCodingMFCDlg 消息处理程序

BOOL CArithmeticCodingMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CArithmeticCodingMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CArithmeticCodingMFCDlg::OnPaint()
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

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CArithmeticCodingMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CString byte_to_hex(const char* str, int len) //transfer string to hex-string
{
	std::string result = "";
	std::string tmp;
	std::stringstream ss;
	for (int i = 0; i <len; i++)
	{
		ss << std::setfill('0')<< std::setw(2) << std::hex << (unsigned int)(unsigned char)str[i] << std::endl;
		ss >> tmp;
		result += tmp;
		result += " ";
	}
	CString cstr(result.c_str());
	return cstr;
}



void CArithmeticCodingMFCDlg::OnBnClickedencodebutton()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	
	CString text;
	GetDlgItemText(IDC_STRInput, text);
	int strLength = text.GetLength()+1;
	char *str = new char[strLength];
	strncpy_s(str, strLength, text, strLength);
	encode((BYTE *)str, strLength);
	char *destr = new char[strLength];
	CString show_code = byte_to_hex(code, code_index);
	//AfxMessageBox(show_code);
	CString msg;
	msg.Format(_T("原始长度：%d\n编码长度：%d"), strLength, code_index);
	AfxMessageBox(msg);
	
	SetDlgItemText(IDC_STRoutput, show_code);
}


void CArithmeticCodingMFCDlg::OnBnClickeddecodeButton()
{
	// TODO: 在此添加控件通知处理程序代码
	decode();
	ShowStatusMsg("解码完成！");
	CString res = CString((char *)DecodeRS);
	SetDlgItemText(IDC_STRoutput, res);
}

void CArithmeticCodingMFCDlg::ShowStatusMsg(CString msg) {
	SetDlgItemText(IDC_Status_Info, msg);
}
