/*********************************************************************\
	File name        : LanguageList.h
	Description      : Schnittstelle für die Klasse CLanguageList
	Created at       : 26.09.01, @ 16:07:46
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#if !defined(AFX_LANGUAGELIST_H__609A5D80_B298_11D5_9B3B_004095418E0F__INCLUDED_)
#define AFX_LANGUAGELIST_H__609A5D80_B298_11D5_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct
{
	String strLanguageName, strLanguageSuffix;
} tagLanguage;

class CLanguageList
{
public:
	CLanguageList();
	virtual ~CLanguageList();
	inline Int32 GetNumLanguages() { return m_lLanguages; }
	void Init();

	String GetName(Int32 a);
	String GetName(String strSuffix);
	String GetLanguageSuffix(Int32 a);
	Int32 GetLanguageID(String strSuffix);
	String GetLanguageName(Int32 a);

protected:
	void AddLanguage(String strName, String strSuffix);
	tagLanguage** m_ppLanguages;
	Int32 m_lLanguages;
};

#endif // !defined(AFX_LANGUAGELIST_H__609A5D80_B298_11D5_9B3B_004095418E0F__INCLUDED_)
