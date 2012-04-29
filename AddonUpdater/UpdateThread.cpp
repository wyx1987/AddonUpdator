// UpdateThread.cpp : implementation file
//

#include "stdafx.h"
#include "AddonUpdater.h"
#include "UpdateThread.h"
#include "../tinyxml/tinyxml.h"
#include "md5.h"
#include "../zlib/zlib.h"

LPCTSTR WATCH_FOLDERS[] = {_T(""), _T("Interface"), _T("Interface\\Addons")};


// CUpdateThread

IMPLEMENT_DYNCREATE(CUpdateThread, CWinThread)

CUpdateThread::CUpdateThread()
{
	m_bCancel = FALSE;
	m_bChecked = FALSE;
}

CUpdateThread::~CUpdateThread()
{
}

BOOL CUpdateThread::InitInstance()
{
	Update();
	return FALSE;
}

int CUpdateThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CUpdateThread, CWinThread)
END_MESSAGE_MAP()

void CUpdateThread::Update()
{
	// 读取本地配置
	UpdateCommandButton(COMMAND_BUTTON_CANCEL);
	UpdateMainProgress(0);
	UpdateSubProgress(0);

	UpdateStatusText(_T("读取站点列表……"));
	CString strIniPath = theApp.GetProfileFile();
	CString strSites;
	DWORD dwSize = 0;
	do 
	{
		dwSize += 4096;
	} while(GetPrivateProfileSection(_T("Sites"), strSites.GetBuffer(dwSize), dwSize, strIniPath.GetString()) == dwSize - 2);
	UpdateSubProgress(100);

	CArray<CString> sites;
	LPCTSTR lpszSite = strSites.GetBuffer();
	while (lpszSite[0]) 
	{
		sites.Add(lpszSite);
		lpszSite += _tcslen(lpszSite) + 1;
	}
	strSites.ReleaseBuffer();

	UpdateMainProgress(2);

	CMap<CString, LPCTSTR, AddonFile, AddonFile&> files;
	// 下载文件列表
	double step = 6.0 / sites.GetSize();
	for (int i = 0; i < sites.GetSize() && CheckCancel(); ++i) 
	{
		CString &strSite = sites.GetAt(i);
		if (!GetFileList(strSite, files))
		{
			UpdateSubProgress(100);
			UpdateMainProgress(100);
			UpdateStatusText(_T("无法下载文件列表。"));
			return;
		}
		UpdateMainProgress(2 + (int)(step * i + 0.5));
	}
	if (!CheckCancel())
		return;
	UpdateMainProgress(8);
		
	UpdateStatusText(_T("正在检测需要更新的文件……"));

	CString strWOWPath = theApp.GetWOWPath();
	CString strTempPath = theApp.GetTempPath();
	CArray <AddonFile *> aDownloadList;
	// 需要下载的文件
	CMap<CString, LPCTSTR, AddonFile, AddonFile&>::CPair *pair = files.PGetFirstAssoc();
	while (pair && CheckCancel())
	{
		CString strMD5;
		try 
		{
			CString strFilePath;
			strFilePath.Append(strWOWPath);
			strFilePath.Append(pair->value.m_strPath);
			md5_state_t md5;
			md5_init(&md5);
			md5_byte_t digest[16] = {0};
			CFile file(strFilePath, CFile::shareDenyRead | CFile::modeRead);
			char buf[4096];
			UINT nCount;
			while ((nCount = file.Read(buf, 4096)) > 0)
			{
				md5_append(&md5, buf, nCount);	
			}
			file.Close();
			md5_finish(&md5, digest);
			for (int i = 0; i < 16; ++i)
			{
				strMD5.AppendFormat(_T("%02x"), digest[i]);
			}
		}
		catch (CFileException *e)
		{
			e->Delete();
		}
		if (strMD5.Compare(pair->value.m_strMD5) != 0)
		{
			aDownloadList.Add(&pair->value);
		}

		pair = files.PGetNextAssoc(pair);
	}

	if (!CheckCancel())
		return;

	ULONG uTotalSize = 0;
	for (int i = 0; i < aDownloadList.GetSize(); ++i)
	{
		uTotalSize += aDownloadList.GetAt(i)->m_uCompressedSize;
	}

	if (!CheckCancel())
		return;

	CString strStatus;
	strStatus.AppendFormat(_T("共有%u个文件需要更新,%.2fMB。"), aDownloadList.GetSize(), uTotalSize * 1.0f / 1024 / 1024);
	UpdateStatusText(strStatus);
	UpdateMainProgress(10);

	step = 88.0 / uTotalSize;
	ULONG uDownload = 0;
	for (int i = 0; i < aDownloadList.GetSize() && CheckCancel(); ++i)
	{
		AddonFile *pAddonFile = aDownloadList.GetAt(i);
		UpdateStatusText(CString(_T("下载 ") + pAddonFile->m_strPath).GetString());
		CString strUrl = pAddonFile->m_strSite + pAddonFile->m_strPath + _T(".z");
		strUrl.Replace(_T('\\'), _T('/'));
		CString strPath = strTempPath + pAddonFile->m_strPath + _T(".z");
		CString strFolder = strPath.Mid(0, strPath.ReverseFind(_T('\\')));
		SHCreateDirectoryEx(NULL, strFolder, NULL);
		if (!Download(strUrl, strPath, pAddonFile->m_uCompressedSize))
		{
			UpdateStatusText(CString(_T("下载 ") + pAddonFile->m_strPath + _T("失败")).GetString());
			Cancel();
			CheckCancel();
			return;
		}
		UpdateSubProgress(100);
		UpdateStatusText(CString(_T("解压 ") + pAddonFile->m_strPath + _T("...")).GetString());
		CFile file(strPath, CFile::modeRead | CFile::shareDenyNone);
		unsigned char *compressed = new unsigned char[file.GetLength()];
		UINT uRead = 0;
		UINT uTotalRead = 0;
		file.Read(compressed, file.GetLength());
		ULONG uCompSize = file.GetLength();
		file.Close();
		unsigned char *uncompressed = new unsigned char[pAddonFile->m_uSize];
		DWORD uSize = pAddonFile->m_uSize;
		if (uncompress(uncompressed, &uSize, compressed, uCompSize) != Z_OK)
		{
			delete[] compressed;
			delete[] uncompressed;
			UpdateStatusText(CString(_T("解压 ") + pAddonFile->m_strPath + _T("失败")).GetString());
			Cancel();
			CheckCancel();
			return;
		}
		strPath = strWOWPath + pAddonFile->m_strPath;
		strFolder = strPath.Mid(0, strPath.ReverseFind(_T('\\')));
		SHCreateDirectoryEx(NULL, strFolder, NULL);
		if (!file.Open(strPath, CFile::modeCreate | CFile::shareExclusive | CFile::modeReadWrite))
		{
			delete[] compressed;
			delete[] uncompressed;
			UpdateStatusText(CString(_T("创建 ")) + strPath + _T("失败。"));
			Cancel();
			CheckCancel();
			return;
		}
		file.Write(uncompressed, pAddonFile->m_uSize);
		file.Close();

		delete[] compressed;
		delete[] uncompressed;
		uDownload += pAddonFile->m_uCompressedSize;
		UpdateMainProgress(8 + uDownload * step + 0.5);
	}

	if (!CheckCancel())
		return;

	// 需要删除的文件
	CArray<CString, LPCTSTR> oldFiles;
	LoadOldList(oldFiles);
	for (int i = 0; i < oldFiles.GetSize(); ++i)
	{
		CString &strPath = oldFiles.GetAt(i);
		if (!files.PLookup(strPath.MakeLower()))
		{
			CString strFilePath = strWOWPath + strPath;
			DeleteFile(strFilePath);
			RemoveFolderIfEmpty(strWOWPath, strFilePath.Mid(0, strFilePath.ReverseFind(_T('\\'))));
		}
	}

	// 保存本次更新的列表
	TiXmlDocument doc;
	TiXmlElement *root = new TiXmlElement("Files");
	pair = files.PGetFirstAssoc();
	USES_CONVERSION;
	while (pair)
	{
		TiXmlElement *file = new TiXmlElement("File");
		file->SetAttribute("Path", T2A(pair->value.m_strPath));
		root->LinkEndChild(file);
		pair = files.PGetNextAssoc(pair);
	}

	doc.LinkEndChild(root);
	doc.SaveFile(T2A(theApp.GetApplicationPath() + _T("AddonUpdater.xml")));
	UpdateStatusText(_T("更新完毕。"));
	UpdateMainProgress(100);
	UpdateCommandButton(COMMAND_BUTTON_PLAY);
}


BOOL CUpdateThread::GetFileList( LPCTSTR lpszUrl, CMap<CString, LPCTSTR, AddonFile, AddonFile &> &fileList )
{
	CString strTempPath = theApp.GetTempPath();
	TCHAR cPath[MAX_PATH] = {0};
	GetTempFileName(strTempPath, _T("SITE"), 0, cPath);
	CString strUrl = lpszUrl;
	strUrl.Append(_T("filelist.xml"));
	SendMessage(m_hWnd, WM_UPDATE_STATUS_MSG, NULL, (LPARAM)(CString(_T("获取列表 ") + strUrl).GetString()));
	if (Download(strUrl, cPath))
	{
		USES_CONVERSION;
		TiXmlDocument doc;
		if (doc.LoadFile(T2A(cPath)))
		{
			TiXmlElement *el = doc.RootElement();
			TiXmlElement *fileElement = el->FirstChildElement("File");
			while (fileElement)
			{
				USES_CONVERSION;
				const char *name = fileElement->Attribute("Path");
				if (!name)
					return FALSE;
				const char *md5 = fileElement->Attribute("MD5");
				if (!md5)
					return FALSE;
				int iSize = 0;
				if (!fileElement->Attribute("Size", &iSize))
					return FALSE;
				int iCompressedSize = 0;
				if (!fileElement->Attribute("CompressedSize", &iCompressedSize))
					return FALSE;
				CString strPath = A2T(name);
				fileList[strPath.MakeLower()] = AddonFile(lpszUrl, A2T(name), A2T(md5), iSize, iCompressedSize);
				fileElement = fileElement->NextSiblingElement("File");
			}
		}

		return TRUE;
	}
	return FALSE;
}

BOOL CUpdateThread::Download( LPCTSTR lpszUrl, LPCTSTR lpszSavePath, ULONG uSize)
{
	DWORD dwServiceType;
	CString strServer;
	CString strObject;
	INTERNET_PORT nPort;
	BOOL bResult = FALSE;
	if (AfxParseURL(lpszUrl, dwServiceType, strServer, strObject, nPort))
	{
		CHttpConnection *conn = NULL;
		CHttpFile *file = NULL;
		UpdateSubProgress(0);
		try
		{
			conn = m_Session.GetHttpConnection(strServer, nPort);
			file = conn->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject, NULL, 1, NULL, NULL, INTERNET_FLAG_SECURE | 
				INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
				INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
			file->SendRequest();
			UpdateSubProgress(10);
			DWORD dwStatus = 0;
			file->QueryInfoStatusCode(dwStatus);
			if (dwStatus == 200)
			{
				CFile localFile(lpszSavePath, CFile::modeReadWrite | CFile::shareExclusive | CFile::modeCreate);
				char buf[512];
				UINT nCount = 0;
				ULONG uTotal = 0;
				while ((nCount = file->Read(buf, 1)) > 0)
				{
					localFile.Write(buf, nCount);
					uTotal += nCount;
					if (uSize > 0)
						UpdateSubProgress(10 + 90.0 * uTotal / uSize);
				}
				localFile.Close();
				bResult = TRUE;
			}
		}
		catch (CException *e)
		{
			e->ReportError();
			e->Delete();
		}
		if (file)
		{
			file->Close();
			delete file;
		}
		if (conn)
		{
			conn->Close();
			delete conn;
		}
		UpdateSubProgress(100);
	};
	return bResult;
}

void CUpdateThread::LoadOldList( CArray<CString, LPCTSTR> &oldList )
{
	TiXmlDocument doc;
	USES_CONVERSION;
	if (doc.LoadFile(T2A(theApp.GetApplicationPath() + _T("AddonUpdater.xml"))))
	{
		TiXmlElement *el = doc.RootElement();
		TiXmlElement *file = el->FirstChildElement("File");
		while (file)
		{
			const char *path = file->Attribute("Path");
			if (path)
			{
				oldList.Add(A2T(path));
			}
			file = file->NextSiblingElement("File");
		}
	}
}

void CUpdateThread::RemoveFolderIfEmpty( CString &strWOWPath, CString &strPath )
{
	if (strPath.IsEmpty())
		return;
	CFileFind find;
	BOOL bWorking = find.FindFile(strWOWPath + strPath + _T("*.*"));
	while (bWorking)
	{
		bWorking = find.FindNextFile();
		if (!find.IsDots())
			return;
	}
	SHFILEOPSTRUCT op = {0};
	op.wFunc = FO_DELETE;
	op.pFrom = strWOWPath + strPath + _T('\0');
	op.fFlags = FOF_NOCONFIRMATION;
	SHFileOperation(&op);
	RemoveFolderIfEmpty(strWOWPath, strPath.Mid(0, strPath.ReverseFind(_T('\\'))));
}

void CUpdateThread::SetHWnd( HWND hWnd )
{
	m_hWnd = hWnd;
}

void CUpdateThread::UpdateStatusText( LPCTSTR lpszText )
{
	SendMessage(m_hWnd, WM_UPDATE_STATUS_MSG, NULL, (LPARAM)lpszText);
}

void CUpdateThread::UpdateSubProgress( int percent )
{
	SendMessage(m_hWnd, WM_UPDATE_SUB_PROGRESS, NULL, percent);
}

void CUpdateThread::UpdateMainProgress( int percent )
{
	SendMessage(m_hWnd, WM_UPDATE_MAIN_PROGRESS, NULL, percent);
}

BOOL CUpdateThread::CheckCancel()
{
	if (m_bCancel && !m_bChecked) 
	{
		UpdateMainProgress(100);
		UpdateSubProgress(100);
		UpdateStatusText(_T("已取消"));
		UpdateCommandButton(COMMAND_BUTTON_UPDATE);
		m_bChecked = TRUE;
		return FALSE;
	}
	return !m_bChecked;
}

void CUpdateThread::UpdateCommandButton( int action )
{
	SendMessage(m_hWnd, WM_UPDATE_COMMAND_BUTTON, NULL, action);
}

void CUpdateThread::Cancel()
{
	m_bCancel = TRUE;
}


// CUpdateThread message handlers
