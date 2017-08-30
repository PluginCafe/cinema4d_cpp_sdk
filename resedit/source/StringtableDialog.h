// StringtableDialog.h: Schnittstelle für die Klasse CStringtableDialog.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRINGTABLEDIALOG_H__7AD67141_9874_11D5_9B3B_004095418E0F__INCLUDED_)
#define AFX_STRINGTABLEDIALOG_H__7AD67141_9874_11D5_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "DirectListView.h"

class CStringtableDialog : public GeModalDialog
{
public:
	CStringtableDialog(CDialogDoc* pDoc, Bool bGlobal = false);
	virtual ~CStringtableDialog();

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 id,const BaseContainer &msg);
	virtual Bool InitValues(void);
	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);

protected:
	void ApplyChanges();
	void StringTableChanged();
	void InitPopupButtons();
	void FillList();

	Int32 m_lLastSel;
	Int32 m_lFirstSelLanguage;
	CDialogDoc* m_pDocument;

	//CDirectListView m_wndListView;
	SimpleListView m_wndListView;
	CStringTable* m_pStringTable;
	Bool m_bHasGlobalStringTable, m_bIsCommandTable;
	Int32 m_lLangEnglish, m_lNumLanguages;
};

#endif // !defined(AFX_STRINGTABLEDIALOG_H__7AD67141_9874_11D5_9B3B_004095418E0F__INCLUDED_)
