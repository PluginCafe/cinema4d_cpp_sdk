/*********************************************************************\
	File name        : StringTable.cpp
	Description      :
	Created at       : 22.08.01, @ 10:57:23
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "StringTable.h"

#include "shellsort.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CStringTableElement::CStringTableElement()
{
	m_lLanguages = 0;
	m_ppstrText = nullptr;
}

CStringTableElement::~CStringTableElement()
{
	for (Int32 a = 0; a < 2 * m_lLanguages; a++)
		DeleteObj(m_ppstrText[a]);
	bDelete(m_ppstrText);
}

/*********************************************************************\
	Function name    : CStringTableElement::Init
	Description      :
	Created at       : 22.08.01, @ 11:22:48
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringTableElement::Init(String strID, Int32 lInitLanguages)
{
	m_strID = strID;
	m_lLanguages = lInitLanguages;
	m_ppstrText = bNewDeprecatedUseArraysInstead<String*>(2 * m_lLanguages);

	for (Int32 a = 0; a < m_lLanguages; a++)
	{
		m_ppstrText[a] = NewObjClear(String, "strNotFound");
		m_ppstrText[a + m_lLanguages] = NewObjClear(String);
	}
}

/*********************************************************************\
	Function name    : CStringTableElement::SetItemText
	Description      :
	Created at       : 22.08.01, @ 11:27:36
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringTableElement::SetItemText(Int32 lLang, String str, String* pCommand)
{
	*m_ppstrText[lLang] = str;
	if (pCommand)
		*m_ppstrText[lLang + m_lLanguages] = *pCommand;
}

/*********************************************************************\
	Function name    : CStringTableElement::AddLanguage
	Description      :
	Created at       : 26.09.01, @ 18:42:00
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 CStringTableElement::AddLanguage()
{
	String **ppNewstrText = bNewDeprecatedUseArraysInstead<String*>(2 * (m_lLanguages + 1));

	Int32 a;
	for (a = 0; a < m_lLanguages; a++)
	{
		ppNewstrText[a] = m_ppstrText[a];
		ppNewstrText[a + m_lLanguages + 1] = m_ppstrText[a + m_lLanguages];
	}
	ppNewstrText[a] = NewObjClear(String);
	ppNewstrText[a + m_lLanguages + 1] = NewObjClear(String);

	bDelete(m_ppstrText);
	m_ppstrText = ppNewstrText;

	m_lLanguages++;

	return m_lLanguages;
}




CStringTable::CStringTable()
{
	m_lElements = 0;
	m_ppElements = nullptr;
	m_lLanguages = 0;
	m_ppSuffixes = nullptr;
	m_lLastFoundIndex = 0;
}

CStringTable::~CStringTable()
{
	Free();
}


/*********************************************************************\
	Function name    : CStringTable::Free
	Description      : frees the string table
	Created at       : 22.08.01, @ 11:04:47
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringTable::Free()
{
	Int32 a;
	for (a = 0; a < m_lElements; a++)
		DeleteObj(m_ppElements[a]);
	bDelete(m_ppElements);
	m_lElements = 0;

	for (a = 0; a < m_lLanguages; a++)
	{
		DeleteObj(m_ppSuffixes[a]);
	}
	bDelete(m_ppSuffixes);
	m_lLanguages = 0;
}

/*********************************************************************\
	Function name    : CompareStringTableElement
	Description      :
	Created at       : 06.10.01, @ 10:46:46
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 CompareStringTableElement(CStringTableElement* const* a, CStringTableElement* const* b)
{
	/*CStringTableElement* x = *((CStringTableElement**)a);
	CStringTableElement* y = *((CStringTableElement**)b);*/

	return ((*a)->GetID().LexCompare((*b)->GetID()));

	/*if (x->GetID() < y->GetID()) return -1;
	else if (x->GetID() > y->GetID()) return 1;
	return 0;*/
}

#define SHELL_SORT
/*********************************************************************\
	Function name    : CStringTable::Sort
	Description      : sorts the elements of the string table using the Quicksort algorithm
	Created at       : 22.08.01, @ 11:00:17
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringTable::Sort()
{
#ifdef SHELL_SORT
	CShellSort <CStringTableElement*> sort;
	sort.Sort(m_ppElements, CompareStringTableElement, m_lElements);
#else
	Bool bAgain = true;
	while (bAgain)
	{
		bAgain = false;
		for (Int32 a = 0; a < m_lElements - 1; a++)
		{
			for (Int32 b = a + 1; b < m_lElements; b++)
			{
				if (m_ppElements[a]->GetID() > m_ppElements[b]->GetID())
				{
					CStringTableElement* tmp = m_ppElements[a];
					m_ppElements[a] = m_ppElements[b];
					m_ppElements[b] = tmp;
					bAgain = true;
				}
			}
		}
	}
#endif
}


/*********************************************************************\
	Function name    : CStringTable::SetItemText
	Description      :
	Created at       : 22.08.01, @ 11:01:01
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringTable::SetItemText(String strID, Int32 lLang, String strText, String* pCommand)
{
	CStringTableElement* pElement = FindElement(strID);
	if (!pElement)
		pElement = AddElement(strID);

	pElement->SetItemText(lLang, strText, pCommand);
}

/*********************************************************************\
	Function name    : CStringTable::FindElement
	Description      :
	Created at       : 22.08.01, @ 11:08:46
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CStringTableElement* CStringTable::FindElement(String strID)
{
	if (m_lElements == 0) return nullptr;

	//TRACE("Searching ID ");
	//TRACE_STRING(strID);

	return _FindElement(0, m_lElements - 1, strID);
}

/*********************************************************************\
	Function name    : CStringTable::FindElement
	Description      :
	Created at       : 22.08.01, @ 11:10:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CStringTableElement* CStringTable::_FindElement(Int32 lFirst, Int32 lLast, String strID)
{
	Int32 lCenter = (lFirst + lLast) / 2;

	String strCenter = m_ppElements[lCenter]->GetID();

	if (lFirst > lLast) return nullptr;

	Int32 lCompare = strCenter.LexCompare(strID);
	if (lCompare > 0) return _FindElement(lFirst, lCenter - 1, strID);
	else if (lCompare < 0) return _FindElement(lCenter + 1, lLast, strID);

	m_lLastFoundIndex = lCenter;
	return m_ppElements[lCenter];

	/*if (m_ppElements[lFirst]->GetID() == strID)
	{
		m_lLastFoundIndex = lFirst;
		return m_ppElements[lFirst];
	}
	if (m_ppElements[lLast]->GetID() == strID)
	{
		m_lLastFoundIndex = lLast;
		return m_ppElements[lLast];
	}

	Int32 lCompare = strCenter.LexCompare(strID);
	if (lCompare == 0)
	{
		m_lLastFoundIndex = lCenter;
		return m_ppElements[lCenter];
	}
	if (lFirst == lCenter) return nullptr;

	if (lCompare > 0)
		return _FindElement(lFirst, lCenter, strID);
	if (lCompare < 0)
		return _FindElement(lCenter, lLast, strID);*/

	//return nullptr;
}

/*********************************************************************\
	Function name    : CStringTable::AddElement
	Description      :
	Created at       : 22.08.01, @ 11:08:44
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CStringTableElement* CStringTable::AddElement(String strID)
{
	CStringTableElement* pNew = NewObjClear(CStringTableElement);
	pNew->Init(strID, m_lLanguages);

	CStringTableElement** ppNew = bNewDeprecatedUseArraysInstead<CStringTableElement*>(m_lElements + 1);

	Int32 lNewPos = 0;

	do {
		if (m_lElements >= 1)
		{
			Int32 l = 0, r = m_lElements - 1;
			Int32 c;
			if (strID.LexCompare(m_ppElements[l]->m_strID) < 0) { lNewPos = l; break; }
			if (strID.LexCompare(m_ppElements[r]->m_strID) > 0) { lNewPos = r + 1; break; }

			while (1)
			{
				Int32 lCompare;
				c = (l + r) / 2;
				if (c == l) { lNewPos = l + 1; break; }

				lCompare = strID.LexCompare(m_ppElements[c]->m_strID);
				if (lCompare < 0) r = c;
				else if (lCompare > 0) l = c;
				else { lNewPos = c + 1; break; }
			}
		}
		else if (m_lElements == 0) lNewPos = 0;
	} while (0);

	CopyMemType(m_ppElements, ppNew, lNewPos);
	CopyMemType(&m_ppElements[lNewPos], &ppNew[lNewPos + 1], (m_lElements - lNewPos));
	ppNew[lNewPos] = pNew;

	bDelete(m_ppElements);
	m_ppElements = ppNew;

	m_lElements++;
	//Sort();

	return pNew;
}

/*********************************************************************\
	Function name    : CStringTable::GetString
	Description      :
	Created at       : 22.08.01, @ 11:29:05
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
String CStringTable::GetString(String strID, Int32 lLang, Bool &bFound, String* pCommand)
{
	///TRACE_STRING(strID);

	bFound = false;
	CStringTableElement* pElement = FindElement(strID);
	if (!pElement || lLang < 0 || lLang >= m_lLanguages) return String("");

	bFound = true;
	if (pCommand)
		*pCommand = pElement->GetCommandString(lLang);

	return pElement->GetString(lLang);
}

/*********************************************************************\
	Function name    : CStringTable::GetString
	Description      :
	Created at       : 24.08.01, @ 10:12:59
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
String CStringTable::GetString(Int32 lIndex, Int32 lLang, String* pCommand)
{
	if (lIndex < 0 || lIndex >= m_lElements) return String("");
	if (lLang < 0 || lLang >= m_lLanguages) return String("");

	if (pCommand)
		*pCommand = m_ppElements[lIndex]->GetCommandString(lLang);

	return m_ppElements[lIndex]->GetString(lLang);
}

/*********************************************************************\
	Function name    : CStringTable::GetElementID
	Description      :
	Created at       : 24.08.01, @ 10:17:23
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
String CStringTable::GetElementID(Int32 lIndex)
{
	if (lIndex < 0 || lIndex >= m_lElements) return String("");
	return m_ppElements[lIndex]->GetID();
}


/*********************************************************************\
	Function name    : CStringTable::SetElementID
	Description      :
	Created at       : 07.09.01, @ 14:49:05
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringTable::SetElementID(Int32 lIndex, String strNew)
{
	if (lIndex < 0 || lIndex >= m_lElements) return;
	m_ppElements[lIndex]->m_strID = strNew;
}

/*********************************************************************\
	Function name    : CStringTable::CopyElement
	Description      :
	Created at       : 10.09.01, @ 16:08:21
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
String CStringTable::CopyElement(CStringTable* pSrcTable, String strNameID)
{
	//TRACE_STRING(strNameID);

	String strNameIDNew = strNameID;
	CStringTableElement* pElement = FindElement(strNameID);
	CStringTableElement* pSrcElement = pSrcTable->FindElement(strNameID);
	if (!pSrcElement) return strNameID;
	CStringTableElement* pNewElement;
	strNameID = TrimNumbers(strNameID);

	if (pElement)
	{
		// we still have an element with this name
		Int32 lMinID = -1;
		Int32 a;
		for (a = 0; a < m_lElements; a++)
		{
			String strID = GetElementID(a);

			Int32 lPos;
			if (strID.FindFirst(strNameID, &lPos) && lPos == 0)
			{
				String strNewID = strID.SubStr(strNameID.GetLength(), strID.GetLength() - strNameID.GetLength());
				Int32 lError;
				Int32 lMin = strNewID.ParseToInt32(&lError);
				if (lError != 0)
					lMin = 0;
				lMinID = LMax(lMin + 1, lMinID);
			}
		}
		if (lMinID > -1)
			strNameIDNew = strNameID + String::IntToString(lMinID);
	}

	pNewElement = AddElement(strNameIDNew);

	for (Int32 l = 0; l < m_lLanguages; l++)
	{
		//TRACE_STRING(*m_ppSuffixes[l]);
		Int32 lSrcID = pSrcTable->GetLanguageID(*m_ppSuffixes[l]);
		if (lSrcID < 0) continue;

		pNewElement->SetItemText(l, pSrcElement->GetString(lSrcID));
	}

	//Sort();
	return strNameIDNew;
}

/*********************************************************************\
	Function name    : CStringTable::AddElement
	Description      :
	Created at       : 10.09.01, @ 16:12:48
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
/*void CStringTable::AddElement(CStringTableElement* pSrcElement)
{
	CStringTableElement* pNewElement = AddElement(pSrcElement->GetID());
	for (Int32 a = 0; a < m_lLanguages; a++)
	{
		pNewElement->SetItemText(a, pSrcElement->GetString(a));
	}
}*/

#ifdef OWN_TRACE
/*********************************************************************\
	Function name    : CStringTable::Dump
	Description      :
	Created at       : 22.08.01, @ 13:49:41
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringTable::Dump()
{
	TRACE("Dumping string table\n");
	for (Int32 a = 0; a < m_lElements; a++)
	{
		TRACE("Element #%d : ", a);
		TRACE_STRING(m_ppElements[a]->GetID());
		for (Int32 b = 0; b < m_ppElements[a]->m_lLanguages; b++)
		{
			TRACE("   "); TRACE_STRING(m_ppElements[a]->GetString(b));
		}
	}
}
#endif

/*********************************************************************\
	Function name    : CStringTable::HasLanguage
	Description      :
	Created at       : 26.09.01, @ 17:18:36
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 CStringTable::GetLanguageID(String strSuffix)
{
	for (Int32 a = 0; a < m_lLanguages; a++)
	{
		if (*m_ppSuffixes[a] == strSuffix)
			return a;
	}
	return -1;
}

/*********************************************************************\
	Function name    : CStringTable::AddLanguage
	Description      :
	Created at       : 26.09.01, @ 18:56:15
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 CStringTable::AddLanguage(String strSuffix)
{
	String** ppNewSuffix = bNewDeprecatedUseArraysInstead<String*>(m_lLanguages + 1);
	CopyMemType(m_ppSuffixes, ppNewSuffix, m_lLanguages);

	ppNewSuffix[m_lLanguages] = NewObjClear(String, strSuffix);

	bDelete(m_ppSuffixes);
	m_ppSuffixes = ppNewSuffix;
	m_lLanguages++;

	for (Int32 a = 0; a < m_lElements; a++)
		m_ppElements[a]->AddLanguage();

	return m_lLanguages - 1;
}

/*********************************************************************\
	Function name    : CStringTable::ContainsElement
	Description      :
	Created at       : 10.10.01, @ 21:38:53
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CStringTable::ContainsElement(String strID)
{
	return (FindElement(strID) != nullptr);
}

/*********************************************************************\
	Function name    : CStringTable::GetElementIndex
	Description      :
	Created at       : 24.11.01, @ 18:38:02
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 CStringTable::GetElementIndex(String strID)
{
	if (!FindElement(strID))
		return -1;

	return m_lLastFoundIndex;
}

/*********************************************************************\
	Function name    : CStringTable::DeleteElement
	Description      :
	Created at       : 05.01.02, @ 23:14:24
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringTable::DeleteElement(Int32 lIndex)
{
	if (lIndex < 0 || lIndex >= m_lElements) return;

	CStringTableElement** ppNewElements = bNewDeprecatedUseArraysInstead<CStringTableElement*>(m_lElements - 1);
	CopyMemType(m_ppElements, ppNewElements, lIndex);
	CopyMemType(&m_ppElements[lIndex + 1], &ppNewElements[lIndex], (m_lElements - lIndex - 1));
	DeleteObj(m_ppElements[lIndex]);
	bDelete(m_ppElements);
	m_ppElements = ppNewElements;
	m_lElements--;
}
