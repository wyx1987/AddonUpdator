
// AddonUpdaterDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "readonlyedit.h"
#include "afxcmn.h"


// CAddonUpdaterDlg dialog
class CAddonUpdaterDlg : public CDialogEx
{
// Construction
public:
	CAddonUpdaterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ADDONUPDATER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	BOOL m_bClose;
	CWinThread *m_pUpdateThread;
	int m_nCommandAction;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnUpdateStatusText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateMainProgress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateSubProgress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateCommandButton(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CReadOnlyEdit m_txtLog;
	CProgressCtrl m_prgSubProgressBar;
	CProgressCtrl m_prgMainProgressBar;
	afx_msg void OnClose();
};
