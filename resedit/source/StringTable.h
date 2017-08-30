/*********************************************************************\
	File name        : StringTable.h
	Description      :
	Created at       : 22.08.01, @ 10:57:27
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#ifndef STRINGTABLE_H__
#define STRINGTABLE_H__

class CStringTable;


class CStringTableElement
{
	friend class CStringTable;
public:
	CStringTableElement();
	~CStringTableElement();

	void Init(String strID, Int32 lInitLanguages);
	void SetItemText(Int32 lLang, String str, String* pCommand = nullptr);

	String GetID() const { return m_strID; }
	String GetString(Int32 l) { return *m_ppstrText[l]; }
	String GetCommandString(Int32 l) { return *m_ppstrText[l + m_lLanguages]; }
protected:
	Int32 AddLanguage();

	String m_strID;
	String **m_ppstrText;

	Int32 m_lLanguages;
};

class CStringTable
{
public:
	CStringTable();
	~CStringTable();

	void Sort();
	void SetItemText(String strID, Int32 lLang, String strText, String* pCommand = nullptr);

	void Free();
	String GetString(String strID, Int32 lLang, Bool &bFound, String* pCommand = nullptr);
	String GetString(Int32 lIndex, Int32 lLang, String* pCommand = nullptr);
	String GetElementID(Int32 lIndex);
	String CopyElement(CStringTable* pSrcTable, String strNameID);
	CStringTableElement* AddElement(String strID);

	Int32 GetElementIndex(String strID);
	void DeleteElement(Int32 lIndex);
	void DeleteElement(String strID) { DeleteElement(GetElementIndex(strID)); }

	Int32 GetNumElements() { return m_lElements; }
	void SetElementID(Int32 lIndex, String strNew);

	Int32 GetLanguageID(String strSuffix);
	Int32 GetUsedLanguages() { return m_lLanguages; }
	Int32 AddLanguage(String strSuffix);
	String GetSuffix(Int32 a) { return *m_ppSuffixes[a]; }
	Bool ContainsElement(String strID);

#ifdef OWN_TRACE
	void Dump();
#else
	inline void Dump() { }
#endif

protected:
	CStringTableElement* FindElement(String strID);
	CStringTableElement* _FindElement(Int32 lFirst, Int32 lLast, String strID);
	//void AddElement(CStringTableElement* pSrcElement);

	String** m_ppSuffixes;
	Int32 m_lLanguages, m_lLastFoundIndex;

	CStringTableElement** m_ppElements;
	Int32 m_lElements;
};

#endif	// STRINGTABLE_H__
