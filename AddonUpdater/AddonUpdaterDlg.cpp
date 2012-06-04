
// AddonUpdaterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AddonUpdater.h"
#include "AddonUpdaterDlg.h"
#include "afxdialogex.h"
#include "UpdateThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAddonUpdaterDlg dialog




CAddonUpdaterDlg::CAddonUpdaterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAddonUpdaterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pUpdateThread = NULL;
	m_bClose = TRUE;
}

void CAddonUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOG, m_txtLog);
	DDX_Control(pDX, IDC_SUB_PROGRESS, m_prgSubProgressBar);
	DDX_Control(pDX, IDC_MAIN_PROGRESS, m_prgMainProgressBar);
}

BEGIN_MESSAGE_MAP(CAddonUpdaterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CAddonUpdaterDlg::OnBnClickedOk)
	ON_MESSAGE(WM_UPDATE_STATUS_MSG, &CAddonUpdaterDlg::OnUpdateStatusText)
	ON_MESSAGE(WM_UPDATE_MAIN_PROGRESS, &CAddonUpdaterDlg::OnUpdateMainProgress)
	ON_MESSAGE(WM_UPDATE_SUB_PROGRESS, &CAddonUpdaterDlg::OnUpdateSubProgress)
	ON_MESSAGE(WM_UPDATE_COMMAND_BUTTON, &CAddonUpdaterDlg::OnUpdateCommandButton)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CAddonUpdaterDlg message handlers

BOOL CAddonUpdaterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_txtLog.SetBackColor(0x00ffffff);
	m_prgMainProgressBar.SetRange32(0, 100);
	m_prgSubProgressBar.SetRange32(0, 100);
	m_nCommandAction = COMMAND_BUTTON_UPDATE;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAddonUpdaterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAddonUpdaterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAddonUpdaterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAddonUpdaterDlg::OnBnClickedOk()
{
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	switch (m_nCommandAction)
	{
	case COMMAND_BUTTON_UPDATE:
		m_pUpdateThread = AfxBeginThread(RUNTIME_CLASS(CUpdateThread), 0, 0, CREATE_SUSPENDED);
		((CUpdateThread *) m_pUpdateThread)->SetHWnd(m_hWnd);
		m_pUpdateThread->ResumeThread();
		break;
	case COMMAND_BUTTON_CANCEL:
		((CUpdateThread *) m_pUpdateThread)->Cancel();
		m_pUpdateThread = NULL;
		break;
	case COMMAND_BUTTON_PLAY:
		m_pUpdateThread = NULL;
		ShellExecute(NULL, NULL, theApp.GetWOWPath() + _T("Launcher.exe"), NULL, NULL, SW_NORMAL);
		OnOK();
		break;
	}
}

LPARAM CAddonUpdaterDlg::OnUpdateStatusText( WPARAM wParam, LPARAM lParam )
{
	int nLen = m_txtLog.GetWindowTextLength();
	if (nLen > 10000) 
	{
		CString rTxt;
		m_txtLog.GetWindowText(rTxt);
		m_txtLog.SetWindowText(rTxt.Mid(rTxt.Find(_T("\r\n"), 1000)));
		m_txtLog.SetSel(m_txtLog.GetWindowTextLength(), m_txtLog.GetWindowTextLength());
	}
	m_txtLog.SetSel(nLen, nLen);
	CString strTxt((LPCTSTR)lParam);
	strTxt.Append(_T("\r\n"));
	m_txtLog.ReplaceSel(strTxt);
	return 0;
}

LRESULT CAddonUpdaterDlg::OnUpdateMainProgress( WPARAM wParam, LPARAM lParam )
{
	m_prgMainProgressBar.SetPos(lParam);
	return 0;
}

LRESULT CAddonUpdaterDlg::OnUpdateSubProgress( WPARAM wParam, LPARAM lParam )
{
	m_prgSubProgressBar.SetPos(lParam);
	return 0;
}

LRESULT CAddonUpdaterDlg::OnUpdateCommandButton( WPARAM wParam, LPARAM lParam )
{
	switch (lParam)
	{
	case COMMAND_BUTTON_UPDATE:
		if (m_bClose)
		{
			OnOK();
			return 0;
		}
		GetDlgItem(IDOK)->SetWindowText(_T("更新"));
		break;
	case COMMAND_BUTTON_CANCEL:
		GetDlgItem(IDOK)->SetWindowText(_T("取消"));
		break;
	case COMMAND_BUTTON_PLAY:
		m_pUpdateThread = NULL;
		GetDlgItem(IDOK)->SetWindowText(_T("启动游戏"));
		break;
	default:
		ASSERT(FALSE);
	}
	m_nCommandAction = lParam;
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	return 0;
}


void CAddonUpdaterDlg::OnClose()
{
	if (m_pUpdateThread)
	{
		m_bClose = TRUE;
		((CUpdateThread *) m_pUpdateThread)->Cancel();
	} 
	else if (m_bClose)
	{
		CDialogEx::OnClose();
	}
	else
	{
		OnUpdateStatusText(NULL, (LPARAM) _T("取消中，请稍候……"));
	}
}
