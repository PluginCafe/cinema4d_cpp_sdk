/*********************************************************************\
	File name        : DialogGroup.cpp
	Description      :
	Created at       : 12.08.01, @ 09:17:27
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"
#include "DialogItem.h"
#include "DialogItemSettings.h"
#include "ItemSettingsDialog.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*********************************************************************\
	Function name    : CDialogGroup::CDialogGroup
	Description      :
	Created at       : 12.08.01, @ 09:17:45
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogGroup::CDialogGroup(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_GROUP);
	m_strDescr = m_strName;
	m_type = Group;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 0;
	m_lInitH = 0;

	m_lBorderStyle = BORDER_NONE;
	m_bHasTitleCheckbox = false;
	m_bHasBorder = true;
	m_bBorderNoTitle = 0;
	m_lCols = m_lRows = 1;
	m_lGroupFlags = 0;

	m_lTabType = TAB_TABS;

	m_lScrollType = SCROLLGROUP_VERT | SCROLLGROUP_HORIZ;

	m_lGroupType = 0; // simple (1 : tab, 2 : scroll, 3 : radio)

	m_lLeftBorder = m_lRightBorder = m_lTopBorder = m_lBottomBorder = 0;
	m_lSpaceX = m_lSpaceY = -1;

	m_pDocument = pDoc;
	m_strControlID = CreateControlID();

	m_lLastChildID = -1;
}

/*********************************************************************\
	Function name    : CDialogGroup::CreateElementBegin
	Description      :
	Created at       : 12.08.01, @ 09:17:47
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogGroup::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	if (m_lGroupType == 0)
		pDlg->GroupBegin(lID, m_lFlags, m_lCols, m_lRows, m_strName, m_lGroupFlags | (m_bHasTitleCheckbox ? BFV_BORDERGROUP_CHECKBOX : 0));
	else if (m_lGroupType == 1)
		pDlg->TabGroupBegin(lID, m_lFlags, m_lTabType);
	else if (m_lGroupType == 2)
		pDlg->ScrollGroupBegin(lID, m_lFlags, m_lScrollType, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));
	else if (m_lGroupType == 3)
		pDlg->AddRadioGroup(lID, m_lFlags, m_lCols, m_lRows);

	if (m_bHasBorder && (m_lGroupType != 1) && m_lBorderStyle != BORDER_NONE) // tab groups don't have a border
	{
		if (!m_strName.Content() || m_bBorderNoTitle || m_lGroupType) // scroll groups don't have a name
			pDlg->GroupBorderNoTitle(m_lBorderStyle);
		else
			pDlg->GroupBorder(m_lBorderStyle);
	}

	pDlg->GroupBorderSpace(m_lLeftBorder, m_lTopBorder, m_lRightBorder, m_lBottomBorder);
	if (m_lSpaceX >= 0 &&  m_lSpaceY >= 0)
		pDlg->GroupSpace(m_lSpaceX, m_lSpaceY);

	if (m_lLastChildID >= 0)
		pDlg->SetInt32(lID, m_lLastChildID);
}

/*********************************************************************\
	Function name    : CDialogGroup::CreateElementEnd
	Description      :
	Created at       : 12.08.01, @ 09:17:49
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogGroup::CreateElementEnd(Int32 lID, GeDialog *pDlg)
{
	if (m_lGroupType != 3)
		pDlg->GroupEnd();
}

/*********************************************************************\
	Function name    : CDialogGroup::Load
	Description      :
	Created at       : 17.08.01, @ 12:47:48
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogGroup::Load(BaseContainer* bc)
{
	Bool bRes = CDialogItem::Load(bc);

	Int32 lType = bc->GetId();/*>GetInt32(DR_TYPE);*/

	if (lType == DRTYPE_GROUP) m_lGroupType = 0;
	else if (lType == DRTYPE_TAB) m_lGroupType = 1;
	else if (lType == DRTYPE_SCROLLGROUP) m_lGroupType = 2;
	else if (lType == DRTYPE_RADIOGROUP) m_lGroupType = 3;

	m_lCols = bc->GetInt32(DR_COLUMNS);
	m_lRows = bc->GetInt32(DR_ROWS);

	m_lGroupFlags = (bc->GetBool(DR_EQUALCOLS) ? BFV_GRIDGROUP_EQUALCOLS : 0) |
									(bc->GetBool(DR_EQUALROWS) ? BFV_GRIDGROUP_EQUALROWS : 0) |
									(bc->GetBool(DR_ALLOW_WEIGHTS) ? BFV_GRIDGROUP_ALLOW_WEIGHTS : 0);

	//String str = bc->GetString(DR_NAME);
	//if (!str.Content()) m_bBorderNoTitle = true;

	m_lBorderStyle = bc->GetInt32(DR_BORDERSTYLE);
	m_bHasBorder = (m_lBorderStyle != BORDER_NONE);
	m_bHasTitleCheckbox = bc->GetBool(BFV_BORDERGROUP_CHECKBOX);

	m_lLeftBorder = bc->GetInt32(DR_BORDERSIZE_L);
	m_lRightBorder = bc->GetInt32(DR_BORDERSIZE_R);
	m_lTopBorder = bc->GetInt32(DR_BORDERSIZE_T);
	m_lBottomBorder = bc->GetInt32(DR_BORDERSIZE_B);

	m_lScrollType = (bc->GetBool(DR_SCROLL_V) ? SCROLLGROUP_VERT : 0) |
									(bc->GetBool(DR_SCROLL_H) ? SCROLLGROUP_HORIZ : 0) |
									(bc->GetBool(DR_SCROLL_BORDERIN) ? SCROLLGROUP_BORDERIN : 0) |
									(bc->GetBool(DR_SCROLL_STATUSBAR) ? SCROLLGROUP_STATUSBAR : 0) |
									(bc->GetBool(DR_SCROLL_AUTO_H) ? SCROLLGROUP_AUTOHORIZ : 0) |
									(bc->GetBool(DR_SCROLL_AUTO_V) ? SCROLLGROUP_AUTOVERT : 0) |
									(bc->GetBool(DR_SCROLL_NOBLIT) ? SCROLLGROUP_NOBLIT : 0) |
									(bc->GetBool(DR_SCROLL_LEFT) ? SCROLLGROUP_LEFT : 0);

	m_lSpaceX = bc->GetInt32(DR_SPACE_X);
	m_lSpaceY = bc->GetInt32(DR_SPACE_Y);

	m_lTabType = bc->GetInt32(DR_TAB_SELECTBY);

	return bRes;
}

/*********************************************************************\
	Function name    : CDialogGroup::Save
	Description      :
	Created at       : 30.08.01, @ 10:28:37
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogGroup::Save(BaseFile* pFile, String strFill)
{
	if (m_bBorderNoTitle)
	{
		m_strNameID = "";
		m_strName = "";
	}

	if (m_lGroupType == 0)
		// DRTYPE_GROUP
		WriteString(pFile, "GROUP ");
	else if (m_lGroupType == 1)
		// DRTYPE_TAB
		WriteString(pFile, "TAB ");
	else if (m_lGroupType == 2)
		// DRTYPE_SCROLLGROUP
		WriteString(pFile, "SCROLLGROUP ");
	else if (m_lGroupType == 3)
		// DRTYPE_RADIOGROUP
		WriteString(pFile, "RADIOGROUP ");

	WriteString(pFile, m_strControlID);

	LineBreak(pFile, strFill);
	WriteString(pFile, "{");
	LineBreak(pFile, strFill + SPACE_NEW_LINE);
	SaveAlignment(pFile);
	LineBreak(pFile, strFill + SPACE_NEW_LINE);

	if ((m_lGroupType == GROUP_TYPE_SIMPLE) || (m_lGroupType == GROUP_TYPE_RADIO)) // tab groups don't have a border
	{
		if (m_bHasBorder)
		{
			WriteString(pFile, "BORDERSTYLE ");
			if (m_lBorderStyle == BORDER_NONE) WriteString(pFile, "BORDER_NONE");
			if (m_lBorderStyle == BORDER_THIN_IN) WriteString(pFile, "BORDER_THIN_IN");
			if (m_lBorderStyle == BORDER_THIN_OUT) WriteString(pFile, "BORDER_THIN_OUT");
			if (m_lBorderStyle == BORDER_IN) WriteString(pFile, "BORDER_IN");
			if (m_lBorderStyle == BORDER_OUT) WriteString(pFile, "BORDER_OUT");
			if (m_lBorderStyle == BORDER_GROUP_IN) WriteString(pFile, "BORDER_GROUP_IN");
			if (m_lBorderStyle == BORDER_GROUP_TOP) WriteString(pFile, "BORDER_GROUP_TOP");
			if (m_lBorderStyle == BORDER_GROUP_OUT) WriteString(pFile, "BORDER_GROUP_OUT");
			if (m_lBorderStyle == BORDER_OUT2) WriteString(pFile, "BORDER_OUT2");
			if (m_lBorderStyle == BORDER_OUT3) WriteString(pFile, "BORDER_OUT3");
			if (m_lBorderStyle == BORDER_OUT3l) WriteString(pFile, "BORDER_OUT3l");
			if (m_lBorderStyle == BORDER_OUT3r) WriteString(pFile, "BORDER_OUT3r");
			if (m_lBorderStyle == BORDER_BLACK) WriteString(pFile, "BORDER_BLACK");
			if (m_lBorderStyle == BORDER_ACTIVE_1) WriteString(pFile, "BORDER_OUT2");
			if (m_lBorderStyle == BORDER_ACTIVE_2) WriteString(pFile, "BORDER_OUT3");
			WriteString(pFile, "; ");
		}

		WriteString(pFile, "BORDERSIZE ");
		WriteInt32(pFile, m_lLeftBorder); WriteString(pFile, ", ");
		WriteInt32(pFile, m_lTopBorder); WriteString(pFile, ", ");
		WriteInt32(pFile, m_lRightBorder); WriteString(pFile, ", ");
		WriteInt32(pFile, m_lBottomBorder); WriteString(pFile, "; ");
		LineBreak(pFile, strFill + SPACE_NEW_LINE);

		if (m_lCols > 0)
		{
			WriteString(pFile, "COLUMNS ");
			WriteInt32(pFile, m_lCols);
			WriteString(pFile, ";");
			LineBreak(pFile, strFill + SPACE_NEW_LINE);
		}
		else if (m_lRows > 0)
		{
			WriteString(pFile, "ROWS ");
			WriteInt32(pFile, m_lRows);
			WriteString(pFile, ";");
			LineBreak(pFile, strFill + SPACE_NEW_LINE);
		}
		if (m_lGroupFlags & (BFV_GRIDGROUP_EQUALCOLS | BFV_GRIDGROUP_EQUALROWS | BFV_GRIDGROUP_ALLOW_WEIGHTS))
		{
			if (m_lGroupFlags & BFV_GRIDGROUP_EQUALCOLS)
				WriteString(pFile, "EQUAL_COLS; ");
			if (m_lGroupFlags & BFV_GRIDGROUP_EQUALROWS)
				WriteString(pFile, "EQUAL_ROWS; ");
			if (m_lGroupFlags & BFV_GRIDGROUP_ALLOW_WEIGHTS)
				WriteString(pFile, "ALLOW_WEIGHTS; ");
			LineBreak(pFile, strFill + SPACE_NEW_LINE);
		}
		if (m_lSpaceX >= 0 &&  m_lSpaceY >= 0)
		{
			WriteString(pFile, "SPACE ");
			WriteInt32(pFile, m_lSpaceX); WriteString(pFile, ", ");
			WriteInt32(pFile, m_lSpaceY);
			WriteString(pFile, ";");
			LineBreak(pFile, strFill + SPACE_NEW_LINE);
		}
	}
	if (m_lGroupType == GROUP_TYPE_TAB)
	{
		if (m_lTabType == TAB_NOSELECT)
			WriteString(pFile, "SELECTION_NONE");
		else if (m_lTabType == TAB_TABS)
			WriteString(pFile, "SELECTION_TABS");
		else if (m_lTabType == TAB_VLTABS)
			WriteString(pFile, "SELECTION_VLTABS");
		else if (m_lTabType == TAB_VRTABS)
			WriteString(pFile, "SELECTION_VRTABS");
		else if (m_lTabType == TAB_CYCLE)
			WriteString(pFile, "SELECTION_CYCLE");
		else if (m_lTabType == TAB_RADIO)
			WriteString(pFile, "SELECTION_RADIO");
		WriteString(pFile, "; ");
		LineBreak(pFile, strFill + SPACE_NEW_LINE);
	}
	if (m_lGroupType == GROUP_TYPE_SCROLL)
	{
		SaveAlignment(pFile);
		if (m_lScrollType & SCROLLGROUP_VERT)
			WriteString(pFile, "SCROLL_V; ");
		if (m_lScrollType & SCROLLGROUP_HORIZ)
			WriteString(pFile, "SCROLL_H; ");
		if (m_lScrollType & SCROLLGROUP_BORDERIN)
			WriteString(pFile, "SCROLL_BORDER; ");
		if (m_lScrollType & SCROLLGROUP_STATUSBAR)
			WriteString(pFile, "SCROLL_STATUSBAR; ");
		if (m_lScrollType & SCROLLGROUP_AUTOHORIZ)
			WriteString(pFile, "SCROLL_AUTO_H; ");
		if (m_lScrollType & SCROLLGROUP_AUTOVERT)
			WriteString(pFile, "SCROLL_AUTO_V; ");
		if (m_lScrollType & SCROLLGROUP_NOBLIT)
			WriteString(pFile, "SCROLL_NOBLIT; ");
		if (m_lScrollType & SCROLLGROUP_LEFT)
			WriteString(pFile, "SCROLL_LEFT; ");

		LineBreak(pFile, strFill + SPACE_NEW_LINE);
	}

	if (m_bHasTitleCheckbox)
		WriteString(pFile, "TITLECHECKBOX;");

	LineBreak(pFile, strFill + SPACE_NEW_LINE);
	CDialogItem::Save(pFile, strFill + SPACE_NEW_LINE);

	LineBreak(pFile, strFill);
	WriteString(pFile, "}");

	return true;
}

String CDialogGroup::GetNameID()
{
	if (m_bHasBorder && !m_bBorderNoTitle)
		return m_strNameID;

	if (m_strNameID != "IDS_STATIC")
		return m_strNameID;
	return "";
}




CGroupSettings::CGroupSettings()
{
	m_lTabPageID = IDC_GROUP_SETTINGS_TAB;
}

CGroupSettings::~CGroupSettings()
{

}

/*********************************************************************\
	Function name    : CGroupSettings::SetData
	Description      :
	Created at       : 12.08.01, @ 09:24:31
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CGroupSettings::SetData()
{
	_Init();

	SetInt32(IDC_GROUP_TYPE_TAB, m_pGroup->m_lGroupType);

	SetInt32(IDC_GROUP_ROWS_EDIT, m_pGroup->m_lRows, 0);
	SetInt32(IDC_GROUP_COLS_EDIT, m_pGroup->m_lCols, 0);
	SetInt32(IDC_GROUP_ROWS_EDIT1, m_pGroup->m_lRows, 0);
	SetInt32(IDC_GROUP_COLS_EDIT1, m_pGroup->m_lCols, 0);
	SetBool(IDC_GROUP_EQUALCOLS_CHK, m_pGroup->m_lGroupFlags & BFV_GRIDGROUP_EQUALCOLS);
	SetBool(IDC_GROUP_EQUALROWS_CHK, m_pGroup->m_lGroupFlags & BFV_GRIDGROUP_EQUALROWS);
	SetInt32(IDC_SIMPLE_GROUP_BORDER_COMBO, m_pGroup->m_lBorderStyle);
	SetBool(IDC_SIMPLE_GROUP_BORDER_CHK, m_pGroup->m_bHasBorder);
	SetBool(IDC_SIMPLE_GROUP_BORDER_NO_TITLE_CHK, m_pGroup->m_bBorderNoTitle);
	Enable(IDC_SIMPLE_GROUP_BORDER_COMBO, m_pGroup->m_bHasBorder);
	Enable(IDC_SIMPLE_GROUP_BORDER_NO_TITLE_CHK, (m_pGroup->m_lGroupType == 0) && m_pGroup->m_bHasBorder);
	SetInt32(IDC_BW_LEFT_EDIT, m_pGroup->m_lLeftBorder);
	SetInt32(IDC_BW_RIGHT_EDIT, m_pGroup->m_lRightBorder);
	SetInt32(IDC_BW_TOP_EDIT, m_pGroup->m_lTopBorder);
	SetInt32(IDC_BW_BOTTOM_EDIT, m_pGroup->m_lBottomBorder);
	SetInt32(IDC_SPACE_X_EDIT, m_pGroup->m_lSpaceX, -1);
	SetInt32(IDC_SPACE_Y_EDIT, m_pGroup->m_lSpaceY, -1);
	SetBool(IDC_GROUP_ALLOW_WEIGHTS_CHK, m_pGroup->m_lGroupFlags & BFV_GRIDGROUP_ALLOW_WEIGHTS);

	SetInt32(IDC_TAB_GROUP_TYPE_COMBO, m_pGroup->m_lTabType);

	SetBool(IDC_SCROLL_GROUP_VERT_CHK, m_pGroup->m_lScrollType & SCROLLGROUP_VERT);
	SetBool(IDC_SCROLL_GROUP_HORZ_CHK, m_pGroup->m_lScrollType & SCROLLGROUP_HORIZ);
	SetBool(IDC_SCROLL_GROUP_NOBLIT_CHK, m_pGroup->m_lScrollType & SCROLLGROUP_NOBLIT);
	SetBool(IDC_SCROLL_GROUP_LEFT_CHK, m_pGroup->m_lScrollType & SCROLLGROUP_LEFT);
	SetBool(IDC_SCROLL_GROUP_BORDERIN_CHK, m_pGroup->m_lScrollType & SCROLLGROUP_BORDERIN);
	SetBool(IDC_SCROLL_GROUP_STATUSBAR_CHK, m_pGroup->m_lScrollType & SCROLLGROUP_STATUSBAR);
	SetBool(IDC_SCROLL_GROUP_AUTOHORIZ_CHK, m_pGroup->m_lScrollType & SCROLLGROUP_AUTOHORIZ);
	SetBool(IDC_SCROLL_GROUP_AUTOVERT_CHK, m_pGroup->m_lScrollType & SCROLLGROUP_AUTOVERT);

	SetBool(IDC_HAS_BORDER_CHECKBOX, m_pGroup->m_bHasTitleCheckbox);
	Enable(IDC_HAS_BORDER_CHECKBOX, (m_pGroup->m_lGroupType == 0) && m_pGroup->m_bHasBorder && !m_pGroup->m_bBorderNoTitle);
}

/*********************************************************************\
	Function name    : CGroupSettings::Command
	Description      :
	Created at       : 12.08.01, @ 09:31:10
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CGroupSettings::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDC_GROUP_TYPE_TAB:
		GetInt32(IDC_GROUP_TYPE_TAB, m_pGroup->m_lGroupType);
		((CItemSettingsDialog*)m_pSettingsDialog)->SetSelItem(m_pGroup);
		//SetInt32(IDC_BORDER_TAB, IDC_BORDER_SETTINGS1);

		Enable(IDC_SIMPLE_GROUP_BORDER_CHK, m_pGroup->m_lGroupType != 1);
		Enable(IDC_SIMPLE_GROUP_BORDER_COMBO, m_pGroup->m_lGroupType != 1);
		Enable(IDC_SIMPLE_GROUP_BORDER_NO_TITLE_CHK, (m_pGroup->m_lGroupType == 0) && m_pGroup->m_bHasBorder);
		Enable(IDC_HAS_BORDER_CHECKBOX, (m_pGroup->m_lGroupType == 0) && m_pGroup->m_bHasBorder && !m_pGroup->m_bBorderNoTitle);
		m_pGroup->ItemChanged();
		break;

	case IDC_GROUP_ROWS_EDIT:
		GetInt32(IDC_GROUP_ROWS_EDIT, m_pGroup->m_lRows);
		m_pGroup->m_lCols = 0;
		SetInt32(IDC_GROUP_COLS_EDIT, m_pGroup->m_lCols);
		SetInt32(IDC_GROUP_COLS_EDIT1, m_pGroup->m_lCols);
		m_pGroup->ItemChanged();
		break;
	case IDC_GROUP_COLS_EDIT:
		GetInt32(IDC_GROUP_COLS_EDIT, m_pGroup->m_lCols);
		m_pGroup->m_lRows = 0;
		SetInt32(IDC_GROUP_ROWS_EDIT, m_pGroup->m_lRows);
		SetInt32(IDC_GROUP_ROWS_EDIT1, m_pGroup->m_lRows);
		m_pGroup->ItemChanged();
		break;
	case IDC_GROUP_ROWS_EDIT1:
		GetInt32(IDC_GROUP_ROWS_EDIT1, m_pGroup->m_lRows);
		m_pGroup->m_lCols = 0;
		SetInt32(IDC_GROUP_ROWS_EDIT, m_pGroup->m_lRows);
		SetInt32(IDC_GROUP_COLS_EDIT, m_pGroup->m_lCols);
		SetInt32(IDC_GROUP_COLS_EDIT1, m_pGroup->m_lCols);
		m_pGroup->ItemChanged();
		break;
	case IDC_GROUP_COLS_EDIT1:
		GetInt32(IDC_GROUP_COLS_EDIT1, m_pGroup->m_lCols);
		m_pGroup->m_lRows = 0;
		SetInt32(IDC_GROUP_COLS_EDIT, m_pGroup->m_lCols);
		SetInt32(IDC_GROUP_ROWS_EDIT, m_pGroup->m_lRows);
		SetInt32(IDC_GROUP_ROWS_EDIT1, m_pGroup->m_lRows);
		m_pGroup->ItemChanged();
		break;
	case IDC_GROUP_EQUALCOLS_CHK:
	case IDC_GROUP_EQUALROWS_CHK:
	case IDC_GROUP_ALLOW_WEIGHTS_CHK:
		{
		Bool b;
		m_pGroup->m_lGroupFlags = 0;
		GetBool(IDC_GROUP_EQUALCOLS_CHK, b); if (b) m_pGroup->m_lGroupFlags |= BFV_GRIDGROUP_EQUALCOLS;
		GetBool(IDC_GROUP_EQUALROWS_CHK, b); if (b) m_pGroup->m_lGroupFlags |= BFV_GRIDGROUP_EQUALROWS;
		GetBool(IDC_GROUP_ALLOW_WEIGHTS_CHK, b); if (b) m_pGroup->m_lGroupFlags |= BFV_GRIDGROUP_ALLOW_WEIGHTS;
		m_pGroup->ItemChanged();
		break;
																}
	case IDC_TAB_GROUP_TYPE_COMBO:
		GetInt32(IDC_TAB_GROUP_TYPE_COMBO, m_pGroup->m_lTabType);
		m_pGroup->ItemChanged();
		break;

	case IDC_SCROLL_GROUP_VERT_CHK:
	case IDC_SCROLL_GROUP_HORZ_CHK:
	case IDC_SCROLL_GROUP_NOBLIT_CHK:
	case IDC_SCROLL_GROUP_LEFT_CHK:
	case IDC_SCROLL_GROUP_BORDERIN_CHK:
	case IDC_SCROLL_GROUP_STATUSBAR_CHK:
	case IDC_SCROLL_GROUP_AUTOHORIZ_CHK:
	case IDC_SCROLL_GROUP_AUTOVERT_CHK: {
		Bool b;
		m_pGroup->m_lScrollType = 0;
		GetBool(IDC_SCROLL_GROUP_VERT_CHK, b); if (b) m_pGroup->m_lScrollType |= SCROLLGROUP_VERT;
		GetBool(IDC_SCROLL_GROUP_HORZ_CHK, b); if (b) m_pGroup->m_lScrollType |= SCROLLGROUP_HORIZ;
		GetBool(IDC_SCROLL_GROUP_NOBLIT_CHK, b); if (b) m_pGroup->m_lScrollType |= SCROLLGROUP_NOBLIT;
		GetBool(IDC_SCROLL_GROUP_LEFT_CHK, b); if (b) m_pGroup->m_lScrollType |= SCROLLGROUP_LEFT;
		GetBool(IDC_SCROLL_GROUP_BORDERIN_CHK, b); if (b) m_pGroup->m_lScrollType |= SCROLLGROUP_BORDERIN;
		GetBool(IDC_SCROLL_GROUP_STATUSBAR_CHK, b); if (b) m_pGroup->m_lScrollType |= SCROLLGROUP_STATUSBAR;
		GetBool(IDC_SCROLL_GROUP_AUTOHORIZ_CHK, b); if (b) m_pGroup->m_lScrollType |= SCROLLGROUP_AUTOHORIZ;
		GetBool(IDC_SCROLL_GROUP_AUTOVERT_CHK, b); if (b) m_pGroup->m_lScrollType |= SCROLLGROUP_AUTOVERT;
		m_pGroup->ItemChanged();
		break;
																			}
	case IDC_SIMPLE_GROUP_BORDER_CHK:
		GetBool(IDC_SIMPLE_GROUP_BORDER_CHK, m_pGroup->m_bHasBorder);
		Enable(IDC_SIMPLE_GROUP_BORDER_COMBO, m_pGroup->m_bHasBorder);
		Enable(IDC_SIMPLE_GROUP_BORDER_NO_TITLE_CHK, (m_pGroup->m_lGroupType == 0) && m_pGroup->m_bHasBorder);
		Enable(IDC_HAS_BORDER_CHECKBOX, (m_pGroup->m_lGroupType == 0) && m_pGroup->m_bHasBorder && !m_pGroup->m_bBorderNoTitle);
		m_pGroup->ItemChanged();
		break;
	case IDC_SIMPLE_GROUP_BORDER_COMBO:
		GetInt32(IDC_SIMPLE_GROUP_BORDER_COMBO, m_pGroup->m_lBorderStyle);
		if (m_pGroup->m_lLeftBorder == 0 && m_pGroup->m_lRightBorder == 0 && m_pGroup->m_lTopBorder == 0 && m_pGroup->m_lBottomBorder == 0 &&
				m_pGroup->m_lBorderStyle != BORDER_NONE)
		{
			m_pGroup->m_lLeftBorder = m_pGroup->m_lRightBorder = m_pGroup->m_lTopBorder = m_pGroup->m_lBottomBorder = 4;
			SetInt32(IDC_BW_LEFT_EDIT, m_pGroup->m_lLeftBorder);
			SetInt32(IDC_BW_RIGHT_EDIT, m_pGroup->m_lRightBorder);
			SetInt32(IDC_BW_TOP_EDIT, m_pGroup->m_lTopBorder);
			SetInt32(IDC_BW_BOTTOM_EDIT, m_pGroup->m_lBottomBorder);
		}
		// lint -fallthrough
	case IDC_SIMPLE_GROUP_BORDER_NO_TITLE_CHK:
		GetBool(IDC_SIMPLE_GROUP_BORDER_NO_TITLE_CHK, m_pGroup->m_bBorderNoTitle);
		Enable(IDC_HAS_BORDER_CHECKBOX, (m_pGroup->m_lGroupType == 0) && m_pGroup->m_bHasBorder && !m_pGroup->m_bBorderNoTitle);
		m_pGroup->ItemChanged();
		break;

	case IDC_BW_LEFT_EDIT:
	case IDC_BW_RIGHT_EDIT:
	case IDC_BW_TOP_EDIT:
	case IDC_BW_BOTTOM_EDIT:
		GetInt32(IDC_BW_LEFT_EDIT, m_pGroup->m_lLeftBorder);
		GetInt32(IDC_BW_RIGHT_EDIT, m_pGroup->m_lRightBorder);
		GetInt32(IDC_BW_TOP_EDIT, m_pGroup->m_lTopBorder);
		GetInt32(IDC_BW_BOTTOM_EDIT, m_pGroup->m_lBottomBorder);
		m_pGroup->ItemChanged();
		break;
	case IDC_SPACE_X_EDIT:
	case IDC_SPACE_Y_EDIT:
		GetInt32(IDC_SPACE_X_EDIT, m_pGroup->m_lSpaceX);
		GetInt32(IDC_SPACE_Y_EDIT, m_pGroup->m_lSpaceY);
		m_pGroup->ItemChanged();
		break;
	case IDC_HAS_BORDER_CHECKBOX:
		GetBool(IDC_HAS_BORDER_CHECKBOX, m_pGroup->m_bHasTitleCheckbox);
		m_pGroup->ItemChanged();
		break;
	}
	return true;
}
