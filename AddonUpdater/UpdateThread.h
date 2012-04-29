#pragma once
#include <afxinet.h>
#include "AddonFile.h"


// CUpdateThread

class CUpdateThread : public CWinThread
{
	DECLARE_DYNCREATE(CUpdateThread)

protected:
	CUpdateThread();           // protected constructor used by dynamic creation
	virtual ~CUpdateThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	void SetHWnd(HWND hWnd);
	void Cancel();

protected:

	void Update();

	DECLARE_MESSAGE_MAP()

private:
	BOOL Download(LPCTSTR lpszUrl, LPCTSTR lpszSavePath, ULONG uSize = 0);
	BOOL GetFileList(LPCTSTR lpszUrl, CMap<CString, LPCTSTR, AddonFile, AddonFile &> &fileList);
	void LoadOldList(CArray<CString, LPCTSTR> &oldList);
	void RemoveFolderIfEmpty(CString &strWOWPath, CString &strPath);
	void UpdateStatusText(LPCTSTR lpszText);
	void UpdateMainProgress(int percent);
	void UpdateSubProgress(int percent);
	void UpdateCommandButton(int action);
	BOOL CheckCancel();
	CInternetSession m_Session;
	HWND m_hWnd;
	BOOL m_bCancel;
	BOOL m_bChecked;
};


