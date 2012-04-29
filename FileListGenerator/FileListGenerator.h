#pragma once

#include "resource.h"
#include <tinyxml.h>
#include "md5.h"

class FileListGenerator
{
public:
	FileListGenerator(LPCTSTR lpszPath, LPCTSTR lpszOutput);
	void Load(TiXmlDocument &doc);

private:
	void LoadFolder(TiXmlElement *el, CString &strPath);

	CString m_strPath;
	CString m_strOutput;
};