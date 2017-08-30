/*********************************************************************\
	File name        : LanguageList.cpp
	Description      : Implementierung der Klasse CLanguageList
	Created at       : 26.09.01, @ 16:45:39
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CLanguageList::CLanguageList()
{
	m_lLanguages = 0;
	m_ppLanguages = nullptr;
}

CLanguageList::~CLanguageList()
{
	for (Int32 l = 0; l < m_lLanguages; l++)
	{
		DeleteObj(m_ppLanguages[l]);
	}
	bDelete(m_ppLanguages);
}

/*********************************************************************\
	Function name    : CLanguageList::Init
	Description      :
	Created at       : 26.09.01, @ 16:11:23
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CLanguageList::Init()
{
	Filename resourcepath = GeGetStartupPath() + Filename("resource");
	AutoAlloc <BrowseFiles> pBrowse;
	pBrowse->Init(resourcepath, false);

	while (pBrowse->GetNext())
	{
		if (pBrowse->IsDir())
		{
			Filename fn = pBrowse->GetFilename();
			if (fn.GetString().SubStr(0, 8).ToLower() == "strings_")
			{
				String idx = fn.GetString();
				idx.Delete(0, 8);

				Filename stringname = resourcepath + fn+Filename("c4d_language.str");
				AutoAlloc <BaseFile> pFile;
				if (!pFile)
					return;
				if (!GeFExist(stringname))
				{
					GeOutString("Missing c4d_language.str to identify the string directory!!!", GEMB_ICONEXCLAMATION);
				}
				else if (pFile->Open(stringname))
				{
					Int32 len = pFile->GetLength();
					Char *buffer = NewMemClear(Char,len + 2);
					if (buffer)
					{
						pFile->ReadBytes(buffer,len);
						buffer[len]=0;

						Int32 i;

						for (i = 0; i < len && buffer[i] >= ' '; i++) { }
						buffer[i] = 0;

						for (i--; i > 0 && buffer[i]== ' '; i--) { }
						buffer[i + 1] = 0;

						AddLanguage(buffer, idx);
						DeleteMem(buffer);
					}
				}
			}
		}
	}
}

/*********************************************************************\
	Function name    : CLanguageList::AddLanguage
	Description      :
	Created at       : 26.09.01, @ 16:27:08
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CLanguageList::AddLanguage(String strName, String strSuffix)
{
	//TRACE("Found Language\n");
	//TRACE("    Name: "); TRACE_STRING(strName);
	//TRACE("  Suffix: "); TRACE_STRING(strSuffix);

	tagLanguage* pNewLang = NewObjClear(tagLanguage);
	tagLanguage** ppNewLang = bNewDeprecatedUseArraysInstead<tagLanguage*>(m_lLanguages + 1);
	CopyMemType(m_ppLanguages, ppNewLang, m_lLanguages);
	ppNewLang[m_lLanguages] = pNewLang;
	bDelete(m_ppLanguages);
	m_ppLanguages = ppNewLang;
	m_lLanguages++;
	pNewLang->strLanguageName = strName;
	pNewLang->strLanguageSuffix = strSuffix;
}

/*********************************************************************\
	Function name    : CLanguageList::GetName
	Description      :
	Created at       : 26.09.01, @ 16:36:29
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
String CLanguageList::GetName(Int32 a)
{
	if (a < 0 || a >= m_lLanguages) return String("");
	return m_ppLanguages[a]->strLanguageName;
}

/*********************************************************************\
	Function name    : CLanguageList::GetLanguageID
	Description      :
	Created at       : 26.09.01, @ 17:17:34
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
String CLanguageList::GetLanguageSuffix(Int32 a)
{
	if (a < 0 || a >= m_lLanguages) return String("");
	return m_ppLanguages[a]->strLanguageSuffix;
}

/*********************************************************************\
	Function name    : CLanguageList::GetLanguageID
	Description      :
	Created at       : 26.09.01, @ 19:51:04
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 CLanguageList::GetLanguageID(String strSuffix)
{
	for (Int32 a = 0; a < m_lLanguages; a++)
	{
		if (m_ppLanguages[a]->strLanguageSuffix == strSuffix)
			return a;
	}
	return -1;
}

/*********************************************************************\
	Function name    : CLanguageList::GetName
	Description      :
	Created at       : 26.09.01, @ 21:41:10
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
String CLanguageList::GetName(String strSuffix)
{
	Int32 l = GetLanguageID(strSuffix);
	if (l < 0) return String("");

	return GetName(l);
}

String CLanguageList::GetLanguageName(Int32 a)
{
	if (a < 0 || a >= m_lLanguages) return String("");
	return m_ppLanguages[a]->strLanguageName + "(" + m_ppLanguages[a]->strLanguageSuffix + ")";
}
