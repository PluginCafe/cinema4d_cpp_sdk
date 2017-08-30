/*********************************************************************\
	File name        : ItemSettingsDialog.h
	Description      : Interface of the class CItemSettingsDialog
	Created at       : 11.08.01, @ 09:52:41
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#if !defined(AFX_ITEMSETTINGSDIALOG_H__71133B61_8E3E_11D5_9B3B_004095418E0F__INCLUDED_)
#define AFX_ITEMSETTINGSDIALOG_H__71133B61_8E3E_11D5_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDialogDoc;
class CDialogItem;

#include "DialogItemSettings.h"

class CItemSettingsDialog : public GeDialog
{
	friend class CDialogDoc;
public:
	Bool m_bCloseState;
	CItemSettingsDialog(CDialogDoc* pDoc);
	virtual ~CItemSettingsDialog();

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
	Bool AskClose();// { return true; } // user may not close the dialog
	void Update();
	void SetSelItem(CDialogItem* pItem);

protected:
	CDialogDoc* m_pDocument;

	ItemType m_LastType;

	CDialogSettings m_wndDialogSettings;
	CStaticSettings m_wndStaticSettings;

	CEditboxSettings m_wndEditboxSettings;
	CCheckboxSettings m_wndCheckboxSettings;
	CButtonSettings m_wndButtonSettings;
	CGroupSettings m_wndGroupSettings;
	CColorSettings m_wndColorSettings;
	CArrowSettings m_wndArrowSettings;
	CSliderSettings m_wndSliderSettings;
	CRadioBtnSettings m_wndRadioButtonSettings;
	CSeparatorSettings m_wndSeparatorSettings;
	CListBoxSettings m_wndListBoxSettings;
	CComboBoxSettings m_wndComboBoxSettings;
	CComboButtonSettings m_wndComboButtonSettings;
	CDialogGroupSettings m_wndDialogGroupSettings;
	CUserAreaSettings m_wndUserAreaSettings;
	CSubDialogSettings m_wndSubDialogSettings;
	CCustomElementSettings m_wndCustomElementSettings;

	CDialogItem* m_pCurrentItem;
};

#endif // !defined(AFX_ITEMSETTINGSDIALOG_H__71133B61_8E3E_11D5_9B3B_004095418E0F__INCLUDED_)
