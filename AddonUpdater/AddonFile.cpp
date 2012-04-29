#include "StdAfx.h"
#include "AddonFile.h"

AddonFile::AddonFile( LPCTSTR lpszSite, LPCTSTR lpszPath, LPCTSTR lpszMD5, ULONG uSize, ULONG uCompressedSize)
{
	m_strSite = lpszSite;
	m_strPath = lpszPath;
	m_strMD5 = lpszMD5;
	m_uSize = uSize;
	m_uCompressedSize = uCompressedSize;
}

AddonFile::AddonFile()
{

}

void AddonFile::operator=( const AddonFile &file )
{
	m_strSite = file.m_strSite;
	m_strPath = file.m_strPath;
	m_strMD5 = file.m_strMD5;
	m_uSize = file.m_uSize;
	m_uCompressedSize = file.m_uCompressedSize;
}