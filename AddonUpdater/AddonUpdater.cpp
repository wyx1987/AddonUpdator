
// AddonUpdater.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AddonUpdater.h"
#include "AddonUpdaterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAddonUpdaterApp

BEGIN_MESSAGE_MAP(CAddonUpdaterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAddonUpdaterApp construction

CAddonUpdaterApp::CAddonUpdaterApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CAddonUpdaterApp object

CAddonUpdaterApp theApp;


// CAddonUpdaterApp initialization

BOOL CAddonUpdaterApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	if (!GetWOWPath().IsEmpty())
	{
		CAddonUpdaterDlg dlg;
		m_pMainWnd = &dlg;
		INT_PTR nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}
	}
	else
	{
		MessageBox(NULL, _T("未找到魔兽世界安装路径。"), _T("错误"), MB_ICONEXCLAMATION | MB_OK);
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	SHFILEOPSTRUCT op = {0};
	op.wFunc = FO_DELETE;
	CString strPath = theApp.GetTempPath();
	strPath.AppendChar(_T('\0'));
	op.pFrom = strPath.GetString();
	op.fFlags = FOF_NOCONFIRMATION;
	SHFileOperation(&op);

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

CString CAddonUpdaterApp::GetApplicationPath()
{
	CString strPath;
	GetModuleFileName(NULL, strPath.GetBuffer(256), 256);
	strPath.ReleaseBuffer();
	strPath = strPath.Mid(0, strPath.ReverseFind(_T('\\')));
	return strPath;
}

CString CAddonUpdaterApp::GetApplicationName()
{
	CString strPath;
	GetModuleFileName(NULL, strPath.GetBuffer(256), 256);
	strPath.ReleaseBuffer();
	strPath = strPath.Mid(strPath.ReverseFind(_T('\\')) + 1);
	return strPath;
}

CString CAddonUpdaterApp::GetProfileFile()
{
	return GetApplicationPath() + _T("\\AddonUpdater.ini");
}

CString CAddonUpdaterApp::GetTempPath(BOOL bCreate)
{
	CString strPath;
	::GetTempPath(MAX_PATH, strPath.GetBuffer(MAX_PATH));
	strPath.ReleaseBuffer();
	strPath.Append(_T("AddonUpdater\\"));
	if (bCreate)
		SHCreateDirectoryEx(NULL, strPath, NULL);
	return strPath;
}

CString CAddonUpdaterApp::GetWOWPath()
{
	TCHAR cPath[256] = {0};
	DWORD dwSize = 256 * sizeof(TCHAR);
	HKEY hKey = NULL;
	LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Blizzard Entertainment\\World of Warcraft"), 0, KEY_QUERY_VALUE, &hKey);
	if (lResult == ERROR_SUCCESS)
	{
		RegQueryValueEx(hKey, _T("InstallPath"), NULL, NULL, (LPBYTE)cPath, &dwSize);
		RegCloseKey(hKey);
	} 
	else
	{
		return _T("");
	}

	CString strPath = cPath;
	if (!strPath.ReverseFind(_T('\\')) == strPath.GetLength() - 1)
		strPath.AppendChar(_T('\\'));

	return strPath;
}

