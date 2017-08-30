/*********************************************************************\
	File name        : ItemSettingsDialog.cpp
	Description      : Implementation of the class CItemSettingsDialog
	Created at       : 11.08.01, @ 10:25:11
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"
#include "DialogItem.h"
#include "ItemSettingsDialog.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CItemSettingsDialog::CItemSettingsDialog(CDialogDoc* pDoc)
{
	m_pDocument = pDoc;
	m_pDocument->m_pItemSettingsDlg = this;
	m_LastType = unknown;
	m_pCurrentItem = nullptr;

	m_bCloseState = false;

	m_wndEditboxSettings.m_pSettingsDialog = this;
	m_wndCheckboxSettings.m_pSettingsDialog = this;
	m_wndButtonSettings.m_pSettingsDialog = this;
	m_wndGroupSettings.m_pSettingsDialog = this;
	m_wndColorSettings.m_pSettingsDialog = this;
	m_wndArrowSettings.m_pSettingsDialog = this;
	m_wndSliderSettings.m_pSettingsDialog = this;
	m_wndRadioButtonSettings.m_pSettingsDialog = this;
	m_wndSeparatorSettings.m_pSettingsDialog = this;
	m_wndListBoxSettings.m_pSettingsDialog = this;
	m_wndComboBoxSettings.m_pSettingsDialog = this;
	m_wndComboButtonSettings.m_pSettingsDialog = this;
	m_wndDialogGroupSettings.m_pSettingsDialog = this;
	m_wndUserAreaSettings.m_pSettingsDialog = this;
	m_wndSubDialogSettings.m_pSettingsDialog = this;
	m_wndDialogSettings.m_pSettingsDialog = this;
	m_wndStaticSettings.m_pSettingsDialog = this;
	m_wndCustomElementSettings.m_pSettingsDialog = this;
}

CItemSettingsDialog::~CItemSettingsDialog()
{

}

/*********************************************************************\
	Funktionsname    : CItemSettingsDialog::CreateLayout
	Beschreibung     :
	Rückgabewert     : Bool
	Erstellt am      : 11.08.01, @ 10:41:30
	Argument         : void
\*********************************************************************/
Bool CItemSettingsDialog::CreateLayout(void)
{
	Bool bRes = GeDialog::CreateLayout() && LoadDialogResource(IDC_PROPERTIES_DIALOG, nullptr, 0);

	m_wndEditboxSettings.Invalidate();
	m_wndCheckboxSettings.Invalidate();
	m_wndButtonSettings.Invalidate();
	m_wndGroupSettings.Invalidate();
	m_wndColorSettings.Invalidate();
	m_wndArrowSettings.Invalidate();
	m_wndSliderSettings.Invalidate();
	m_wndRadioButtonSettings.Invalidate();
	m_wndSeparatorSettings.Invalidate();
	m_wndListBoxSettings.Invalidate();
	m_wndComboBoxSettings.Invalidate();
	m_wndComboButtonSettings.Invalidate();
	m_wndDialogGroupSettings.Invalidate();
	m_wndUserAreaSettings.Invalidate();
	m_wndSubDialogSettings.Invalidate();
	m_wndStaticSettings.Invalidate();
	m_wndCustomElementSettings.Invalidate();

	if (++g_lNeedFileNew == 4) m_pDocument->OnFileNew();

	return bRes;
}

/*********************************************************************\
	Function name    : CItemSettingsDialog::Command
	Description      :
	Created at       : 11.08.01, @ 22:40:28
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CItemSettingsDialog::Command(Int32 lID, const BaseContainer &msg)
{
	if (!m_pCurrentItem) return true;

	switch (lID)
	{
	case IDC_UPDATE_BTN:
		m_pDocument->UpdateAllViews(true);
		break;

	case IDC_NAME_EDIT:
		{
			// if the item didn't have a name ID, give one
			if (!m_pCurrentItem->m_strNameID.Content())
			{
				//pItem->m_strNameID
				SetString(IDC_NAME_ID_STAT, m_pCurrentItem->m_strNameID);
			}
			GetString(IDC_NAME_EDIT, m_pCurrentItem->m_strName);
		}
		break;

	case IDC_ID_EDIT:
		GetString(IDC_ID_EDIT, m_pCurrentItem->m_strControlID);
		m_pDocument->m_bControlSymbolsChanged = true;
		break;


	case IDC_VERT_ALIGN_COMBO:
	case IDC_HORZ_ALIGN_COMBO: {
		Int32 lFlags = 0;
		Int32 lSel;
		GetInt32(IDC_VERT_ALIGN_COMBO, lSel);
		if (lSel == 1) lFlags |= BFV_TOP;
		else if (lSel == 2) lFlags |= BFV_BOTTOM;
		else if (lSel == 3) lFlags |= BFV_FIT;
		else if (lSel == 4) lFlags |= BFV_SCALE;
		else if (lSel == 5) lFlags |= BFV_SCALEFIT;

		GetInt32(IDC_HORZ_ALIGN_COMBO, lSel);
		if (lSel == 1) lFlags |= BFH_LEFT;
		else if (lSel == 2) lFlags |= BFH_RIGHT;
		else if (lSel == 3) lFlags |= BFH_FIT;
		else if (lSel == 4) lFlags |= BFH_SCALE;
		else if (lSel == 5) lFlags |= BFH_SCALEFIT;
		m_pCurrentItem->m_lFlags = lFlags;

		m_pDocument->ItemChanged();
		break;
														 }
	case IDC_WIDTH_EDIT:
	case IDC_HEIGHT_EDIT: {
		GetInt32(IDC_WIDTH_EDIT, m_pCurrentItem->m_lInitW);
		GetInt32(IDC_HEIGHT_EDIT, m_pCurrentItem->m_lInitH);
		m_pDocument->ItemChanged();
		break;
												}
	case IDC_CUSTOM_RESET_ARROW:
	{
		if (m_pCurrentItem->GetType() == CustomElement)
		{
			CDialogCustomElement* pCustom = (CDialogCustomElement*)m_pCurrentItem;
			Int32 l;
			GetInt32(IDC_CUSTOM_RESET_ARROW, l);
			if (l == IDS_CUSTOM_RESET) pCustom->ResetData();
			else if (l == IDS_CUSTOM_RESET_ALL) pCustom->ResetData(true);
			pCustom->InitValues();
		}
		break;
	}

	default:
		// send this message to all tab pages
		m_wndEditboxSettings.Command(lID, msg);
		m_wndCheckboxSettings.Command(lID, msg);
		m_wndButtonSettings.Command(lID, msg);
		m_wndGroupSettings.Command(lID, msg);
		m_wndColorSettings.Command(lID, msg);
		m_wndArrowSettings.Command(lID, msg);
		m_wndSliderSettings.Command(lID, msg);
		m_wndRadioButtonSettings.Command(lID, msg);
		m_wndSeparatorSettings.Command(lID, msg);
		m_wndListBoxSettings.Command(lID, msg);
		m_wndComboBoxSettings.Command(lID, msg);
		m_wndComboButtonSettings.Command(lID, msg);
		m_wndDialogGroupSettings.Command(lID, msg);
		m_wndUserAreaSettings.Command(lID, msg);
		m_wndSubDialogSettings.Command(lID, msg);
		m_wndStaticSettings.Command(lID, msg);
		m_wndCustomElementSettings.Command(lID, msg);
		break;
	}

	return true;
}

/*********************************************************************\
	Function name    : CItemSettingsDialog::AskClose
	Description      :
	Created at       : 11.10.01, @ 21:13:12
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CItemSettingsDialog::AskClose()
{
	if (m_bCloseState) return false;

	m_bCloseState = true;
	if (!m_pDocument->CloseEditor()) return true;

	return false;
}

/*********************************************************************\
	Function name    : CItemSettingsDialog::Update
	Description      :
	Created at       : 11.08.01, @ 15:10:22
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CItemSettingsDialog::Update()
{
}

/*********************************************************************\
	Function name    : CItemSettingsDialog::SetSelItem
	Description      :
	Created at       : 11.08.01, @ 19:29:35
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CItemSettingsDialog::SetSelItem(CDialogItem* pItem)
{
	ItemType type = pItem->GetType();

	CSubSettingsDialog* pNew = nullptr;
	if (type == Dialog)
	{
		m_wndDialogSettings.SetItem((CDialogTemplate*)pItem);
		pNew = &m_wndDialogSettings;
	}
	else if (type == Static)
	{
		m_wndStaticSettings.SetItem((CDialogStatic*)pItem);
		pNew = &m_wndStaticSettings;
	}
	else if (type == Group)
	{
		m_wndGroupSettings.SetItem((CDialogGroup*)pItem);
		pNew = &m_wndGroupSettings;
	}
	else if (type == CheckBox)
	{
		m_wndCheckboxSettings.SetItem((CDialogCheckBox*)pItem);
		pNew = &m_wndCheckboxSettings;
	}
	else if (type == EditBox)
	{
		m_wndEditboxSettings.SetItem((CDialogEditBox*)pItem);
		pNew = &m_wndEditboxSettings;
	}
	else if (type == Button)
	{
		m_wndButtonSettings.SetItem((CDialogButton*)pItem);
		pNew = &m_wndButtonSettings;
	}
	else if (type == Color)
	{
		m_wndColorSettings.SetItem((CDialogColor*)pItem);
		pNew = &m_wndColorSettings;
	}
	else if (type == ArrowBtn)
	{
		m_wndArrowSettings.SetItem((CDialogArrowBtn*)pItem);
		pNew = &m_wndArrowSettings;
	}
	else if (type == Slider)
	{
		m_wndSliderSettings.SetItem((CDialogSlider*)pItem);
		pNew = &m_wndSliderSettings;
	}
	else if (type == RadioBtn)
	{
		m_wndRadioButtonSettings.SetItem((CDialogRadioBtn*)pItem);
		pNew = &m_wndRadioButtonSettings;
	}
	else if (type == Separator)
	{
		m_wndSeparatorSettings.SetItem((CDialogSeparator*)pItem);
		pNew = &m_wndSeparatorSettings;
	}
	else if (type == ListBox)
	{
		m_wndListBoxSettings.SetItem((CDialogListBox*)pItem);
		pNew = &m_wndListBoxSettings;
	}
	else if (type == ComboBox)
	{
		m_wndComboBoxSettings.SetItem((CDialogComboBox*)pItem);
		pNew = &m_wndComboBoxSettings;
	}
	else if (type == ComboButton)
	{
		m_wndComboButtonSettings.SetItem((CDialogComboButton*)pItem);
		pNew = &m_wndComboButtonSettings;
	}
	else if (type == DialogGroup)
	{
		m_wndDialogGroupSettings.SetItem((CDialogDlgGroup*)pItem);
		pNew = &m_wndDialogGroupSettings;
	}
	else if (type == UserArea)
	{
		m_wndUserAreaSettings.SetItem((CDialogUserArea*)pItem);
		pNew = &m_wndUserAreaSettings;
	}
	else if (type == SubDialogControl)
	{
		m_wndSubDialogSettings.SetItem((CDialogSubDlg*)pItem);
		pNew = &m_wndSubDialogSettings;
	}
	else if (type == CustomElement)
	{
		m_wndCustomElementSettings.SetItem((CDialogCustomElement*)pItem);
		pNew = &m_wndCustomElementSettings;
	}

	if (!pNew) return;

	if (type != m_LastType)
	{
		SetInt32(IDC_MAIN_TAB, pNew->m_lTabPageID);
		m_LastType = type;

		ASSERT(pNew->m_lTabPageID != 0);
	}

	// now, change the values
	pNew->SetData();
	if (type != Group)
	{
		Enable(IDC_WIDTH_EDIT, true);
		Enable(IDC_HEIGHT_EDIT, true);
	}
	else
	{
		Enable(IDC_WIDTH_EDIT, ((CDialogGroup*)pItem)->m_lGroupType == GROUP_TYPE_SCROLL);
		Enable(IDC_HEIGHT_EDIT, ((CDialogGroup*)pItem)->m_lGroupType == GROUP_TYPE_SCROLL);
	}
	m_pCurrentItem = pItem;
	Int32 lFlags = pItem->GetFlags();
	Int32 lSel;
	if (pItem->m_strNameID.Content())
		Enable(IDC_NAME_EDIT, true);
	else
		Enable(IDC_NAME_EDIT, false);

	SetString(IDC_NAME_EDIT, pItem->m_strName);
	SetString(IDC_ID_EDIT, pItem->m_strControlID);
	SetString(IDC_NAME_ID_STAT, pItem->m_strNameID);

	if ((lFlags & BFV_SCALEFIT) == BFV_SCALEFIT) lSel = 5;
	else if ((lFlags & BFV_FIT) == BFV_FIT) lSel = 3;
	else if (lFlags & BFV_TOP) lSel = 1;
	else if (lFlags & BFV_BOTTOM) lSel = 2;
	else if (lFlags & BFV_SCALE) lSel = 4;
	else lSel = 0;
	SetInt32(IDC_VERT_ALIGN_COMBO, lSel);

	if ((lFlags & BFH_SCALEFIT) == BFH_SCALEFIT) lSel = 5;
	else if ((lFlags & BFH_FIT) == BFH_FIT) lSel = 3;
	else if (lFlags & BFH_LEFT) lSel = 1;
	else if (lFlags & BFH_RIGHT) lSel = 2;
	else if (lFlags & BFH_SCALE) lSel = 4;
	else lSel = 0;
	SetInt32(IDC_HORZ_ALIGN_COMBO, lSel);

	SetInt32(IDC_WIDTH_EDIT, pItem->m_lInitW, 0);
	SetInt32(IDC_HEIGHT_EDIT, pItem->m_lInitH, 0);
}
