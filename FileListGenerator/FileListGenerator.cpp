// FileListGenerator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FileListGenerator.h"

#include "../zlib/zlib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			if (argc == 3)
			{
				CString strPath = argv[1];
				if (strPath.ReverseFind(_T('\\')) != strPath.GetLength() - 1)
					strPath.AppendChar(_T('\\'));

				CString strOutput = argv[2];
				if (strOutput.ReverseFind(_T('\\')) != strOutput.GetLength() - 1)
					strOutput.AppendChar(_T('\\'));
				FileListGenerator gen(strPath, strOutput);
				TiXmlDocument doc;
				gen.Load(doc);
				USES_CONVERSION;
				doc.SaveFile(T2A(strOutput + _T("filelist.xml")));
			} 
			else
			{
				_tcprintf(_T("Usage:\n"));
				_tcprintf(_T("FileListGenerator.exe <AddonPath> <OutputPath>\n"));
			}
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}

FileListGenerator::FileListGenerator( LPCTSTR lpszPath, LPCTSTR lpszOutput )
{
	m_strPath = lpszPath;
	if(m_strPath.ReverseFind(_T('\\')) != m_strPath.GetLength() - 1)
		m_strPath.AppendChar(_T('\\'));
	m_strOutput = lpszOutput;
	if(m_strOutput.ReverseFind(_T('\\')) != m_strOutput.GetLength() - 1)
		m_strOutput.AppendChar(_T('\\'));
}

void FileListGenerator::Load( TiXmlDocument &doc )
{
	USES_CONVERSION;
	doc.Clear();
	TiXmlElement *root = new TiXmlElement(T2A(_T("Files")));
	LoadFolder(root, CString(_T("")));
	doc.LinkEndChild(root);
}

void FileListGenerator::LoadFolder( TiXmlElement *el, CString &strPath )
{
	if (strPath.CompareNoCase(_T(".git\\")) == 0 || strPath.CompareNoCase(_T("filelist.xml")) == 0)
		return;
	USES_CONVERSION;
	CString strRelativePath = strPath;
	if (strRelativePath.ReverseFind(_T('\\')) != strRelativePath.GetLength() - 1)
		strRelativePath.AppendChar(_T('\\'));
	CFileFind find;
	BOOL bWorking = find.FindFile(m_strPath + strRelativePath + _T("*.*"));
	
	if (bWorking) 
	{
		do 
		{
			bWorking = find.FindNextFile();
			if (!find.IsDots()) 
			{
				if (find.IsDirectory())
				{
					LoadFolder(el, strRelativePath + find.GetFileName() + _T("\\"));
				}
				else
				{
					_tprintf(_T("Find file %s...\n"), strRelativePath + find.GetFileName());
					SHCreateDirectoryEx(NULL, m_strOutput + strRelativePath, NULL);
					md5_byte_t digest[16] = {0};
					md5_state_t md5;
					md5_init(&md5);
					CFile file(m_strPath + strRelativePath + find.GetFileName(), CFile::shareDenyRead | CFile::modeRead);
					ULONG uSize = file.GetLength();
					unsigned char *buf = new unsigned char[uSize];
					file.Read(buf, uSize);
					file.Close();
					md5_append(&md5, buf, uSize);
					md5_finish(&md5, digest);
					DWORD dwCompSize = compressBound(uSize);
					unsigned char *compressBuf = new unsigned char[dwCompSize];
					int ret = 0;
					if ((ret = compress2(compressBuf, &dwCompSize, buf, uSize, Z_BEST_COMPRESSION)) != Z_OK)
					{
						delete []buf;
						delete []compressBuf;
						_tprintf(_T("Compress failed."));
						return;
					}

					file.Open(m_strOutput + strRelativePath + find.GetFileName() + _T(".z"), CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive);
					file.Write(compressBuf, dwCompSize);
					file.Close();
					delete []buf;
					delete []compressBuf;

					TiXmlElement *element = new TiXmlElement(T2A(_T("File")));
					element->SetAttribute(T2A(_T("Path")), T2A((strRelativePath + find.GetFileName()).GetBuffer()));
					CString md5String;
					for (int i = 0; i < 16; ++i)
					{
						md5String.AppendFormat(_T("%02x"), digest[i]);
					}
					element->SetAttribute(T2A(_T("MD5")), T2A(md5String.GetBuffer()));
					element->SetAttribute("Size", uSize);
					element->SetAttribute("CompressedSize", dwCompSize);
					el->LinkEndChild(element);
				}
			}
		} while (bWorking);
	}
}
