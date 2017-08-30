/*********************************************************************\
	File name        : CompareTableDialog.h
	Description      :
	Created at       : 09.10.01, @ 20:57:43
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#if !defined(AFX_COMPARETABLEDIALOG_H__E842D0C1_BCF5_11D5_B60C_0040D01B1EC1__INCLUDED_)
#define AFX_COMPARETABLEDIALOG_H__E842D0C1_BCF5_11D5_B60C_0040D01B1EC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DialogDoc.h"

class CCompareTableDialog : public GeModalDialog
{
public:
	CCompareTableDialog(Filename fnOriginal, Filename fnNew);
	virtual ~CCompareTableDialog();

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 id,const BaseContainer &msg);
	virtual Bool InitValues(void);

protected:
	CDialogDoc m_OriginalDoc, m_NewDoc;
	void SaveResult();

	CStringTable* m_pOriginalTable, *m_pNewTable;
	SimpleListView m_wndOutputList;
	Filename m_fnOriginal, m_fnNew;
	Bool m_bIsCommandTable;
};

#endif // !defined(AFX_COMPARETABLEDIALOG_H__E842D0C1_BCF5_11D5_B60C_0040D01B1EC1__INCLUDED_)
