
// AddonUpdater.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


#define WM_UPDATE_STATUS_MSG	(WM_USER + 100)
#define WM_UPDATE_MAIN_PROGRESS	(WM_USER + 101)
#define WM_UPDATE_SUB_PROGRESS	(WM_USER + 102)
#define WM_UPDATE_COMMAND_BUTTON (WM_USER + 103)

#define COMMAND_BUTTON_UPDATE	0x01
#define COMMAND_BUTTON_CANCEL	0x02
#define COMMAND_BUTTON_PLAY		0x03


// CAddonUpdaterApp:
// See AddonUpdater.cpp for the implementation of this class
//

class CAddonUpdaterApp : public CWinApp
{
public:
	CAddonUpdaterApp();

// Overrides
public:
	virtual BOOL InitInstance();
	CString GetApplicationPath();
	CString GetApplicationName();
	CString GetProfileFile();
	CString GetTempPath(BOOL bCreate = TRUE);
	CString GetWOWPath();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CAddonUpdaterApp theApp;