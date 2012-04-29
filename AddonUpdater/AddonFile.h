#pragma once
class AddonFile
{
public:
	CString m_strSite;
	CString m_strPath;
	CString m_strMD5;
	ULONG m_uSize;
	ULONG m_uCompressedSize;

	AddonFile();
	AddonFile(LPCTSTR lpszSite, LPCTSTR lpszPath, LPCTSTR lpszMD5, ULONG uSize, ULONG uCompressedSize);
	void operator=(const AddonFile &file);
};

