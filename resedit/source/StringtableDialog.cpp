/*********************************************************************\
	File name        : StringtableDialog.cpp
	Description      :
	Created at       : 24.08.01, @ 09:45:14
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/


#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"
#include "StringtableDialog.h"
#include "StringTable.h"
#include "DialogItem.h"
#include "TreeDialog.h"
#include "ItemSettingsDialog.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void SetModalResizeFlag(void* pHandle);

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

#define LISTVIEW_STRINGS_OLD_ID   100
#define LISTVIEW_STRING_ID        101
#define LISTVIEW_STRINGS_OFFSET   102

enum
{
	IDC_STRINGTABLE_LIST = 1000,
	IDC_ELEMENT_ID_EDIT,
	IDC_US_EDIT,
	IDC_SEL_EDIT,
	IDC_NEW_ITEM,
	IDC_LANGUAGE_POPUP_BTN,
	IDC_LANGUAGE_NEW_BTN,
	IDC_LANGUAGE_SELECT_STATIC,
	IDC_US_EDIT_COMMAND,
	IDC_SEL_EDIT_COMMAND,

	IDC_LANGUAGE_SWITCH_FIRST = 1100,
	IDC_LANGUAGE_NEW_FIRST = 2100,

	IDC_DUMMY
};

CStringtableDialog::CStringtableDialog(CDialogDoc* pDoc, Bool bGlobal)
{
	m_pDocument = pDoc;
	m_bHasGlobalStringTable = bGlobal;

	m_bIsCommandTable = (m_pDocument->m_StringTableType == typeCommandStrings);
}

CStringtableDialog::~CStringtableDialog()
{

}

/*********************************************************************\
	Function name    : CStringtableDialog::CreateLayout
	Description      :
	Created at       : 24.08.01, @ 10:01:34
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CStringtableDialog::CreateLayout(void)
{
	Bool bRes = GeDialog::CreateLayout();

	SetTitle(GeLoadString(IDS_EDIT_STRINGTABLE));

	GroupBegin(100, BFH_SCALEFIT | BFV_SCALEFIT, 1, 0, "", 0);
		AddListView(IDC_STRINGTABLE_LIST, BFV_SCALEFIT | BFH_SCALEFIT, 700, 200);
		GroupBegin(102, BFH_FIT | BFV_FIT, 1, 0, "", 0);
			AddButton(IDC_NEW_ITEM, BFH_LEFT, 0, 0, GeLoadString(IDS_INSERT_STRING));
		GroupEnd();

		GroupBegin(103, BFH_FIT | BFV_FIT, 2, 0, "", 0);
			AddStaticText(131, BFH_LEFT, 0, 0, GeLoadString(IDS_SWITCH_LANGUAGE), 0);
			AddPopupButton(IDC_LANGUAGE_POPUP_BTN, BFH_LEFT, 0, 0);
			m_lFirstSelLanguage = -1;
			Int32 a;
			for (a = 0; a < g_LanguageList.GetNumLanguages(); a++)
			{
				if ((m_pDocument->GetStringTable()->GetLanguageID(g_LanguageList.GetLanguageSuffix(a))) >= 0)
				{
					AddChild(IDC_LANGUAGE_POPUP_BTN, IDC_LANGUAGE_SWITCH_FIRST + a, g_LanguageList.GetName(a));
					if (m_lFirstSelLanguage == -1) m_lFirstSelLanguage = a;
				}
			}
		GroupEnd();

		GroupBegin(104, BFH_FIT | BFV_FIT, 2, 0, "", 0);
			AddStaticText(141, BFH_LEFT, 0, 0, GeLoadString(IDS_NEW_LANGUAGE), 0);
			AddPopupButton(IDC_LANGUAGE_NEW_BTN, BFH_LEFT, 0, 0);
			for (a = 0; a < g_LanguageList.GetNumLanguages(); a++)
			{
				if ((m_pDocument->GetStringTable()->GetLanguageID(g_LanguageList.GetLanguageSuffix(a))) < 0)
				{
					AddChild(IDC_LANGUAGE_NEW_BTN, IDC_LANGUAGE_NEW_FIRST + a, g_LanguageList.GetName(a));
				}
			}
		GroupEnd();
		InitPopupButtons();

		GroupBegin(101, BFH_FIT | BFV_FIT, 2, 0, "", 0);
			AddStaticText(110, BFH_LEFT, 0, 0, GeLoadString(IDS_ID), 0);
			AddEditText(IDC_ELEMENT_ID_EDIT, BFH_SCALEFIT);

			AddStaticText(111, BFH_LEFT, 0, 0, GeLoadString(IDS_LANGUAGE_US), 0);
			AddEditText(IDC_US_EDIT, BFH_SCALEFIT);

			if (m_bIsCommandTable)
			{
				AddStaticText(112, BFH_LEFT, 0, 0, GeLoadString(IDS_COMMAND_STRING), 0);
				AddEditText(IDC_US_EDIT_COMMAND, BFH_SCALEFIT);
			}

			AddStaticText(IDC_LANGUAGE_SELECT_STATIC, BFH_LEFT, 0, 0, g_LanguageList.GetName(m_pDocument->GetLanguageIDList()), 0);
			AddEditText(IDC_SEL_EDIT, BFH_SCALEFIT);

			if (m_bIsCommandTable)
			{
				AddStaticText(113, BFH_LEFT, 0, 0, GeLoadString(IDS_COMMAND_STRING), 0);
				AddEditText(IDC_SEL_EDIT_COMMAND, BFH_SCALEFIT);
			}

			GroupEnd();
	GroupEnd();

	m_wndListView.AttachListView(this, IDC_STRINGTABLE_LIST);

	AddDlgGroup(DLG_OK);
	SetModalResizeFlag(GetWindowHandle());

	return bRes;
}

/*********************************************************************\
	Function name    : CStringtableDialog::Command
	Description      :
	Created at       : 24.08.01, @ 10:01:32
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CStringtableDialog::Command(Int32 id,const BaseContainer &msg)
{
	switch (id)
	{
		case IDC_ELEMENT_ID_EDIT:
			m_pDocument->m_bStringSymbolsChanged = true;
			break;

		case IDC_STRINGTABLE_LIST:
			{
				switch (msg.GetInt32(BFM_ACTION_VALUE))
				{
					case LV_SIMPLE_SELECTIONCHANGED:
						{
							Int32 lItemID = msg.GetInt32(LV_SIMPLE_ITEM_ID);
							Bool bIsSelected = false;
							AutoAlloc<BaseSelect> pSel1; if (!pSel1) break;
							m_wndListView.GetSelection(pSel1);
							if (pSel1->IsSelected(lItemID)) bIsSelected = true;

							if (!bIsSelected) break;

							if (lItemID == -1)
							{
								AutoAlloc<BaseSelect> pSel; if (!pSel) break;
								pSel->Select(0);
								m_wndListView.SetSelection(pSel);
								m_wndListView.DataChanged();
								lItemID = 0;
							}

							if (m_lLastSel != lItemID)
							{
								if (m_pStringTable->GetNumElements() == 0) break;

								ApplyChanges();

								BaseContainer data;
								m_wndListView.GetItem(lItemID, &data);

								SetString(IDC_ELEMENT_ID_EDIT, data.GetString(LISTVIEW_STRING_ID));

								SetString(IDC_US_EDIT, data.GetString(LISTVIEW_STRINGS_OFFSET + m_lLangEnglish));
								SetString(IDC_SEL_EDIT, data.GetString(LISTVIEW_STRINGS_OFFSET + m_pDocument->GetLanguageIDTable()));

								if (m_bIsCommandTable)
								{
									SetString(IDC_US_EDIT_COMMAND, data.GetString(LISTVIEW_STRINGS_OFFSET + m_lNumLanguages + m_lLangEnglish));
									SetString(IDC_SEL_EDIT_COMMAND, data.GetString(LISTVIEW_STRINGS_OFFSET + m_pDocument->GetLanguageIDTable() + m_lNumLanguages));
								}

								m_lLastSel = lItemID;
							}
							break;
						}
				}
				break;
			}

		case IDC_OK:
			{
				ApplyChanges();
				StringTableChanged();
				Close(true);
				return false;
				break;
			}
		case IDC_NEW_ITEM:
			{
				ApplyChanges();

				Int32 lItems = m_wndListView.GetItemCount() - 1;
				BaseContainer data;

				String str = "IDS_CHILD_ELEMENT";
				Int32 lMinID = 1;
				for (Int32 a = 0; a < lItems; a++)
				{
					BaseContainer data;
					m_wndListView.GetItem(a, &data);

					String strID = data.GetString(LISTVIEW_STRING_ID);
					Int32 lPos;
					//TRACE("comparing strings\n");
					//TRACE("  "); TRACE_STRING(strID);
					//TRACE("  "); TRACE_STRING(str);
					if (strID.FindFirst(str, &lPos) && lPos == 0)
					{
						strID = strID.SubStr(str.GetLength(), strID.GetLength() - str.GetLength());
						Int32 lError;
						Int32 lMin = strID.ParseToInt32(&lError);
						if (lError == 0)
							lMinID = LMax(lMin + 1, lMinID);
					}
				}
				str += String::IntToString(lMinID);

				m_pStringTable->AddElement(str);
				data.FlushAll();
				data.SetString(LISTVIEW_STRINGS_OLD_ID, str);
				data.SetString(LISTVIEW_STRING_ID, str);
				m_wndListView.SetItem(lItems, data);

				AutoAlloc<BaseSelect> pSel; if (!pSel) break;
				pSel->Select(lItems);
				m_wndListView.SetSelection(pSel);
				m_lLastSel = lItems;

				SetString(IDC_ELEMENT_ID_EDIT, str);
				SetString(IDC_US_EDIT, "");
				SetString(IDC_SEL_EDIT, "");
				SetString(IDC_US_EDIT_COMMAND, "");
				SetString(IDC_SEL_EDIT_COMMAND, "");
				m_wndListView.DataChanged();
				m_pDocument->m_bStringSymbolsChanged = true;

				break;
			}

		case IDC_LANGUAGE_POPUP_BTN:
			{
				Int32 lSelLanguage;
				GetInt32(IDC_LANGUAGE_POPUP_BTN, lSelLanguage);
				lSelLanguage -= IDC_LANGUAGE_SWITCH_FIRST;

				if (m_pDocument->m_lLanguageIDTable >= 0)
				{
					ApplyChanges();
					StringTableChanged();
				}

				String strSuffix = g_LanguageList.GetLanguageSuffix(lSelLanguage);
				m_pDocument->ChangeLanguage(strSuffix, true);
				if (!m_bHasGlobalStringTable)
					m_pDocument->m_pTreeDlg->SelectLanguage(lSelLanguage);

				m_pDocument->m_lLanguageIDList = g_LanguageList.GetLanguageID(strSuffix);
				m_pDocument->m_lLanguageIDTable = m_pDocument->GetStringTable()->GetLanguageID(strSuffix);

				SetString(IDC_LANGUAGE_SELECT_STATIC, g_LanguageList.GetName(m_pDocument->GetLanguageIDList()));
				LayoutChanged(IDC_LANGUAGE_SELECT_STATIC);

				Enable(IDC_US_EDIT, g_LanguageList.GetLanguageSuffix(m_pDocument->GetLanguageIDList()) != "us");
				if (m_bIsCommandTable)
					Enable(IDC_US_EDIT_COMMAND, g_LanguageList.GetLanguageSuffix(m_pDocument->GetLanguageIDList()) != "us");

				BaseContainer data;
				m_wndListView.GetItem(m_lLastSel, &data);
				SetString(IDC_ELEMENT_ID_EDIT, data.GetString(LISTVIEW_STRING_ID));
				SetString(IDC_US_EDIT, data.GetString(LISTVIEW_STRINGS_OFFSET + m_lLangEnglish));
				SetString(IDC_SEL_EDIT, data.GetString(LISTVIEW_STRINGS_OFFSET + m_pDocument->GetLanguageIDTable()));
				SetString(IDC_US_EDIT_COMMAND, data.GetString(LISTVIEW_STRINGS_OFFSET + m_lNumLanguages + m_lLangEnglish));
				SetString(IDC_SEL_EDIT_COMMAND, data.GetString(LISTVIEW_STRINGS_OFFSET + m_pDocument->GetLanguageIDTable() + m_lNumLanguages));

				InitPopupButtons();

				break;
			}

		case IDC_LANGUAGE_NEW_BTN:
			{
				Int32 lSelLanguage;
				GetInt32(IDC_LANGUAGE_NEW_BTN, lSelLanguage);
				lSelLanguage -= IDC_LANGUAGE_NEW_FIRST;

				ApplyChanges();
				StringTableChanged();
				m_pDocument->GetStringTable()->AddLanguage(g_LanguageList.GetLanguageSuffix(lSelLanguage));

				BaseContainer bc;
				FillList();

				SetInt32(IDC_LANGUAGE_POPUP_BTN, lSelLanguage + IDC_LANGUAGE_SWITCH_FIRST);
				Command(IDC_LANGUAGE_POPUP_BTN, bc);

				break;
			}
	}

	return true;
}

/*********************************************************************\
	Function name    : CStringtableDialog::InitValues
	Description      :
	Created at       : 24.08.01, @ 10:01:30
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CStringtableDialog::InitValues(void)
{
	// first call the parent instance
	if (!GeDialog::InitValues()) return false;

	m_pStringTable = m_pDocument->GetStringTable();
	m_lLangEnglish = m_pStringTable->GetLanguageID("us");

	FillList();

	return true;
}

/*********************************************************************\
	Function name    : CStringtableDialog::ApplyChanges
	Description      :
	Created at       : 07.09.01, @ 14:33:36
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringtableDialog::ApplyChanges()
{
	BaseContainer data;
	m_wndListView.GetItem(m_lLastSel, &data);

	String str;
	GetString(IDC_ELEMENT_ID_EDIT, str); data.SetString(LISTVIEW_STRING_ID, str);
	GetString(IDC_US_EDIT, str); data.SetString(LISTVIEW_STRINGS_OFFSET + m_lLangEnglish, str);
	GetString(IDC_SEL_EDIT, str); data.SetString(LISTVIEW_STRINGS_OFFSET + m_pDocument->GetLanguageIDTable(), str);

	GetString(IDC_US_EDIT_COMMAND, str); data.SetString(LISTVIEW_STRINGS_OFFSET + m_lNumLanguages + m_lLangEnglish, str);
	GetString(IDC_SEL_EDIT_COMMAND, str); data.SetString(LISTVIEW_STRINGS_OFFSET + m_pDocument->GetLanguageIDTable() + m_lNumLanguages, str);

	m_wndListView.SetItem(m_lLastSel, data);
	m_wndListView.DataChanged();
}

typedef struct {
	String *pStrOld, *pStrNew, *pStrName, *pStrEnglish;
	CStringTable* pTable;
	Int32 lLang;
} tagUpdateID;

/*********************************************************************\
	Function name    : UpdateStringIDCallback
	Description      :
	Created at       : 07.09.01, @ 14:41:53
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void UpdateStringIDCallback(TreeViewItem* pItem, void *pData)
{
	tagUpdateID* pID = (tagUpdateID*)pData;
	CDialogItem* pDlgItem = (CDialogItem*)pItem->GetData()->GetVoid(TREEVIEW_DATA);
	if (!pDlgItem) return;

	if (pDlgItem->m_strNameID == *pID->pStrOld)
	{
		pDlgItem->m_strNameID = *pID->pStrNew;
		pDlgItem->m_strName = *pID->pStrName;
	}
}

/*********************************************************************\
	Function name    : CStringtableDialog::StringTableChanged
	Description      : must be called when the dialog is closed,
										 updates all control's string IDs
	Created at       : 07.09.01, @ 14:37:33
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringtableDialog::StringTableChanged()
{
	Int32 lItems = m_wndListView.GetItemCount();

	Int32 a;
	m_pStringTable->Sort();

	for (a = 0; a < lItems - 1; a++)
	{
		BaseContainer data;
		m_wndListView.GetItem(a, &data);

		String strOldID = data.GetString(LISTVIEW_STRINGS_OLD_ID);
		String strID = data.GetString(LISTVIEW_STRING_ID);
		String strName = data.GetString(LISTVIEW_STRINGS_OFFSET + m_pDocument->GetLanguageIDTable());

		if (/*strOldID != strID*/1)
		{
			tagUpdateID id;
			id.pStrNew = &strID;
			id.pStrOld = &strOldID;
			id.pStrName = &strName;
			id.pTable = m_pStringTable;
			id.lLang = m_pDocument->GetLanguageIDTable();

			Int32 b = m_pStringTable->GetElementIndex(strOldID);
			if (b >= 0)
			{
				if (strID != strOldID)
				{
					m_pStringTable->SetElementID(b, strID);
					m_pStringTable->Sort();
				}

				if (!m_bHasGlobalStringTable)
					m_pDocument->m_pTreeDlg->GetTreeView()->VisitChildren(UpdateStringIDCallback, nullptr, (void*)&id);
			}
		}
	}
	m_pStringTable->Sort();

	for (a = 0; a < lItems - 1; a++)
	{
		BaseContainer data;
		m_wndListView.GetItem(a, &data);
		String strID = data.GetString(LISTVIEW_STRING_ID);
		String strName = data.GetString(LISTVIEW_STRINGS_OFFSET + m_pDocument->GetLanguageIDTable());
		String strEnglish = data.GetString(LISTVIEW_STRINGS_OFFSET + m_lLangEnglish);

		if (m_bIsCommandTable)
		{
			String strCommand = data.GetString(LISTVIEW_STRINGS_OFFSET + m_pDocument->GetLanguageIDTable() + m_lNumLanguages);
			m_pStringTable->SetItemText(strID, m_pDocument->GetLanguageIDTable(), strName, &strCommand);

			if (m_pDocument->GetLanguageIDTable() != m_lLangEnglish)
			{
				String strCommandE = data.GetString(LISTVIEW_STRINGS_OFFSET + m_lLangEnglish + m_lNumLanguages);
				m_pStringTable->SetItemText(strID, m_lLangEnglish, strEnglish, &strCommandE);
			}
		}
		else
		{
			m_pStringTable->SetItemText(strID, m_pDocument->GetLanguageIDTable(), strName);
			if (m_pDocument->GetLanguageIDTable() != m_lLangEnglish)
			{
				m_pStringTable->SetItemText(strID, m_lLangEnglish, strEnglish);
			}
		}

		//TRACE_STRING(strID);
	}

	if (!m_bHasGlobalStringTable)
	{
		m_pDocument->m_pItemSettingsDlg->SetSelItem(m_pDocument->GetSelectedItem());
		m_pDocument->UpdateAllViews(true, true);
	}
	//m_pStringTable->Dump();
}

/*********************************************************************\
	Function name    : CStringtableDialog::InitPopupButtons
	Description      :
	Created at       : 20.10.01, @ 18:40:09
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringtableDialog::InitPopupButtons()
{
	FreeChildren(IDC_LANGUAGE_POPUP_BTN);
	FreeChildren(IDC_LANGUAGE_NEW_BTN);

	Int32 a;

	String str;

	for (a = 0; a < g_LanguageList.GetNumLanguages(); a++)
	{
		str = g_LanguageList.GetName(a);

		if ((m_pDocument->GetStringTable()->GetLanguageID(g_LanguageList.GetLanguageSuffix(a))) < 0)
			AddChild(IDC_LANGUAGE_POPUP_BTN, IDC_LANGUAGE_SWITCH_FIRST + a, str + "&d&");
		else
			AddChild(IDC_LANGUAGE_POPUP_BTN, IDC_LANGUAGE_SWITCH_FIRST + a, str);

		if ((m_pDocument->GetStringTable()->GetLanguageID(g_LanguageList.GetLanguageSuffix(a))) >= 0)
			AddChild(IDC_LANGUAGE_NEW_BTN, IDC_LANGUAGE_NEW_FIRST + a, str + "&d&");
		else
			AddChild(IDC_LANGUAGE_NEW_BTN, IDC_LANGUAGE_NEW_FIRST + a, str);

		/*if ((m_pDocument->GetStringTable()->GetLanguageID(g_LanguageList.GetLanguageSuffix(a))) >= 0)
		{
			AddChild(IDC_LANGUAGE_POPUP_BTN, IDC_LANGUAGE_SWITCH_FIRST + a, g_LanguageList.GetName(a));
			if (m_lFirstSelLanguage == -1) m_lFirstSelLanguage = a;
		}*/
	}

	/*for (a = 0; a < g_LanguageList.GetNumLanguages(); a++)
	{
		if ((m_pDocument->GetStringTable()->GetLanguageID(g_LanguageList.GetLanguageSuffix(a))) < 0)
		{
			AddChild(IDC_LANGUAGE_NEW_BTN, IDC_LANGUAGE_NEW_FIRST + a, g_LanguageList.GetName(a));
		}
	}*/
}

/*********************************************************************\
	Function name    : CStringtableDialog::FillList
	Description      :
	Created at       : 20.10.01, @ 18:59:48
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStringtableDialog::FillList()
{
	Int32 a, b;

	Int32 ltems = m_wndListView.GetItemCount();

	BaseContainer bc;
	for (a = 0; a < ltems; a++)
	{
		Int32 lID;
		m_wndListView.GetItemLine(0, &lID, &bc);
		m_wndListView.RemoveItem(lID);
	}

	BaseContainer layout;
	m_lNumLanguages = m_pStringTable->GetUsedLanguages();
	for (a = -1; a < m_lNumLanguages; a++)
		layout.SetInt32(a + LISTVIEW_STRINGS_OFFSET, LV_COLUMN_TEXT);
	m_wndListView.SetLayout(m_lNumLanguages + 1, layout);

	Int32 lNumElements = m_pStringTable->GetNumElements();
	for (a = -1; a < lNumElements; a++)
	{
		BaseContainer data;
		if (a == -1)
		{
			for (b = 0; b < m_lNumLanguages; b++)
			{
				data.SetString(b + LISTVIEW_STRINGS_OFFSET, g_LanguageList.GetName(m_pStringTable->GetSuffix(b)));
			}
		}
		else
		{
			for (b = -1; b < m_lNumLanguages; b++)
			{
				String str;
				if (b == -1)
				{
					str = m_pStringTable->GetElementID(a);
					data.SetString(LISTVIEW_STRINGS_OLD_ID, str);
				}
				else
				{
					if (m_bIsCommandTable)
					{
						String strCommand;
						str = m_pStringTable->GetString(a, b, &strCommand);
						data.SetString(b + LISTVIEW_STRINGS_OFFSET + m_lNumLanguages, strCommand);
					}
					else
						str = m_pStringTable->GetString(a, b);
				}

				data.SetString(b + LISTVIEW_STRINGS_OFFSET, str);
			}
		}
		m_wndListView.SetItem(a, data);
	}
	m_wndListView.DataChanged();

	if (lNumElements > 0)
	{
		SetString(IDC_ELEMENT_ID_EDIT, m_pStringTable->GetElementID(0));

		if (m_bIsCommandTable)
		{
			String strTitle, strCommand;
			strTitle = m_pStringTable->GetString(0, m_lLangEnglish, &strCommand);
			SetString(IDC_US_EDIT, strTitle);
			SetString(IDC_US_EDIT_COMMAND, strCommand);

			strTitle = m_pStringTable->GetString(0, m_pDocument->GetLanguageIDTable(), &strCommand);
			SetString(IDC_SEL_EDIT, strTitle);
			SetString(IDC_SEL_EDIT_COMMAND, strCommand);

			/*String strTitle, strCommand;
			SetString(IDC_US_EDIT_COMMAND, m_pStringTable->GetString(0, 0));
			SetString(IDC_SEL_EDIT_COMMAND, m_pStringTable->GetString(0, m_pDocument->GetLanguageIDTable()));*/
		}
		else
		{
			SetString(IDC_US_EDIT, m_pStringTable->GetString(0, m_lLangEnglish));
			SetString(IDC_SEL_EDIT, m_pStringTable->GetString(0, m_pDocument->GetLanguageIDTable()));
		}

		AutoAlloc<BaseSelect> pSel;
		if (pSel) pSel->Select(0);
		m_wndListView.SetSelection(pSel);
		m_lLastSel = 0;
	}
	else
		m_lLastSel = -1;

	if (m_bHasGlobalStringTable)
	{
		SetInt32(IDC_LANGUAGE_POPUP_BTN, IDC_LANGUAGE_SWITCH_FIRST + m_lFirstSelLanguage);
		BaseContainer msg;
		Command(IDC_LANGUAGE_POPUP_BTN, msg);
		/*m_pDocument->ChangeLanguage(g_LanguageList.GetLanguageSuffix(m_lFirstSelLanguage), true);
		SetString(IDC_LANGUAGE_SELECT_STATIC, g_LanguageList.GetName(m_pDocument->GetLanguageIDList()));*/
	}

	Enable(IDC_US_EDIT, g_LanguageList.GetLanguageSuffix(m_pDocument->GetLanguageIDList()) != "us");
	if (m_bIsCommandTable)
		Enable(IDC_US_EDIT_COMMAND, g_LanguageList.GetLanguageSuffix(m_pDocument->GetLanguageIDList()) != "us");
}

/*********************************************************************\
	Function name    : CStringtableDialog::Message
	Description      :
	Created at       : 24.11.01, @ 17:13:23
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 CStringtableDialog::Message(const BaseContainer &msg, BaseContainer &result)
{
	Int32 lID = msg.GetId();
	Int32 lDevice;
	switch (lID)
	{
		case BFM_INPUT:
			lDevice = msg.GetInt32(BFM_INPUT_DEVICE);
			if (lDevice == BFM_INPUT_KEYBOARD)
			{
				Int32 lValue = msg.GetInt32(BFM_INPUT_CHANNEL);
				lValue = lValue;
				if (lValue == 32773)
				{
					AutoAlloc<BaseSelect> pSel; if (!pSel) break;

					if (m_lLastSel < m_pStringTable->GetNumElements() - 1)
					{
						pSel->Select(m_lLastSel + 1);
						m_wndListView.SetSelection(pSel);

						m_wndListView.DataChanged();

						BaseContainer msg;
						msg.SetInt32(BFM_ACTION_VALUE, LV_SIMPLE_SELECTIONCHANGED);
						msg.SetInt32(LV_SIMPLE_ITEM_ID, m_lLastSel + 1);
						Command(IDC_STRINGTABLE_LIST, msg);
					}
				}
				else if (lValue == 32772)
				{
					AutoAlloc<BaseSelect> pSel; if (!pSel) break;

					if (m_lLastSel > 0)
					{
						pSel->Select(m_lLastSel - 1);
						m_wndListView.SetSelection(pSel);

						m_wndListView.DataChanged();

						BaseContainer msg;
						msg.SetInt32(BFM_ACTION_VALUE, LV_SIMPLE_SELECTIONCHANGED);
						msg.SetInt32(LV_SIMPLE_ITEM_ID, m_lLastSel - 1);
						Command(IDC_STRINGTABLE_LIST, msg);
					}
				}

				return true;
			}
			break;
	}

	return GeDialog::Message(msg, result);
}
