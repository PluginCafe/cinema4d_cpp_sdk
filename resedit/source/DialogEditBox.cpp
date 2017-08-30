/*********************************************************************\
	File name        : DialogEditBox.cpp
	Description      :
	Created at       : 11.08.01, @ 19:52:13
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"
#include "DialogItem.h"
#include "DialogItemSettings.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*********************************************************************\
	Function name    : CDialogEditBox::CDialogEditBox
	Description      :
	Created at       : 11.08.01, @ 19:52:58
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogEditBox::CDialogEditBox(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_EDIT_BOX);
	m_strDescr = m_strName;
	m_type = EditBox;
	m_bIsNumberEdit = true;
	m_bHasArrows = true;
	m_bHasSlider = false;
	m_lInitW = 70;
	m_lInitH = 0;
	m_bIsMultilineText = false;
	m_lMultilineStyle = 0;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogEditBox::CreateElement
	Description      :
	Created at       : 11.08.01, @ 19:53:13
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogEditBox::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	if (m_bIsNumberEdit)
	{
		if (m_bHasArrows)
			pDlg->AddEditNumberArrows(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));
		else
		{
			if (m_bHasSlider)
				pDlg->AddEditSlider(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));
			else
				pDlg->AddEditNumber(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));
		}
	}
	else // it's a text edit
	{
		if (m_bIsMultilineText)
			pDlg->AddMultiLineEditText(m_lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH), m_lMultilineStyle);
		else
			pDlg->AddEditText(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));
	}
}

/*********************************************************************\
	Function name    : CDialogEditBox::Load
	Description      :
	Created at       : 17.08.01, @ 12:46:19
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogEditBox::Load(BaseContainer* bc)
{
	Int32 lType = bc->GetId();/*>GetInt32(DR_TYPE);*/
	m_bHasArrows = false;
	m_bIsNumberEdit = false;
	if (lType == DROLDWIN_EDITTEXT || lType == DROLDWIN_MULTILINETEXT)
	{
		if (lType == DROLDWIN_MULTILINETEXT)
		{
			m_bIsMultilineText = true;
			m_lMultilineStyle = (bc->GetBool(DR_MULTILINE_MONOSPACED) ? DR_MULTILINE_MONOSPACED : 0) |
													(bc->GetBool(DR_MULTILINE_SYNTAXCOLOR) ? DR_MULTILINE_SYNTAXCOLOR : 0) |
													(bc->GetBool(DR_MULTILINE_WORDWRAP) ? DR_MULTILINE_WORDWRAP : 0) |
													(bc->GetBool(DR_MULTILINE_PYTHON) ? DR_MULTILINE_PYTHON : 0) |
													(bc->GetBool(DR_MULTILINE_STATUSBAR) ? DR_MULTILINE_STATUSBAR : 0);
		}
	}
	else
	{
		m_bIsNumberEdit = true;
		if (lType == DROLDWIN_EDITNUMBERARROWS)
		{
			m_bHasArrows = true;
			m_bHasSlider = false;
		}
		else if (lType == DROLDWIN_SLIDERNUM)
		{
			m_bHasArrows = false;
			m_bHasSlider = true;
		}
		else
		{
			m_bHasArrows = false;
			m_bHasSlider = false;
		}
	}

	return CDialogItem::Load(bc);
}

/*********************************************************************\
	Function name    : CDialogEditBox::Save
	Description      :
	Created at       : 30.08.01, @ 10:28:15
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogEditBox::Save(BaseFile* pFile, String strFill)
{
	if (m_bIsNumberEdit)
	{
		if (m_bHasArrows)
			WriteString(pFile, "EDITNUMBERARROWS ");
		else
		{
			if (m_bHasSlider) WriteString(pFile, "EDITSLIDER ");
			else  WriteString(pFile, "EDITNUMBER ");
		}
	}
	else // it's a text edit
	{
		if (m_bIsMultilineText) WriteString(pFile, "MULTILINEEDIT ");
		else WriteString(pFile, "EDITTEXT ");
	}
	WriteString(pFile, m_strControlID);
	LineBreak(pFile, strFill);
	WriteString(pFile, "{ ");
	if (!m_bIsNumberEdit && m_bIsMultilineText && (m_lMultilineStyle != 0))
		LineBreak(pFile, strFill + SPACE_NEW_LINE);

	SaveAlignment(pFile);
	if (!m_bIsNumberEdit && m_bIsMultilineText && (m_lMultilineStyle != 0))
	{
		LineBreak(pFile, strFill + SPACE_NEW_LINE);
		if (m_lMultilineStyle & DR_MULTILINE_MONOSPACED)
			WriteString(pFile, "MONOSPACED; ");
		if (m_lMultilineStyle & DR_MULTILINE_SYNTAXCOLOR)
			WriteString(pFile, "SYNTAXCOLOR; ");
		if (m_lMultilineStyle & DR_MULTILINE_WORDWRAP)
			WriteString(pFile, "WORDWRAP; ");
		if (m_lMultilineStyle & DR_MULTILINE_PYTHON)
			WriteString(pFile, "PYTHON; ");
		if (m_lMultilineStyle & DR_MULTILINE_STATUSBAR)
			WriteString(pFile, "STATUSBAR; ");
	}


	if (!m_bHasArrows && m_bIsMultilineText && (m_lMultilineStyle != 0))
		LineBreak(pFile, strFill);

	WriteString(pFile, "}");

	return true;
}



CEditboxSettings::CEditboxSettings()
{
	m_lTabPageID = IDC_EDIT_SETTINGS_TAB;
}

CEditboxSettings::~CEditboxSettings()
{

}

/*********************************************************************\
	Function name    : CEditboxSettings::SetData
	Description      :
	Created at       : 11.08.01, @ 23:18:32
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CEditboxSettings::SetData()
{
	_Init();

	SetInt32(IDC_TYPE_SELECT_COMBO, m_pEditBox->m_bIsNumberEdit ? 0 : 1);
	Enable(IDC_HAS_SLIDER_CHK, m_pEditBox->m_bIsNumberEdit);
	Enable(IDC_HAS_ARROWS_CHK, m_pEditBox->m_bIsNumberEdit);
	SetBool(IDC_HAS_SLIDER_CHK,m_pEditBox->m_bHasSlider);
	SetBool(IDC_HAS_ARROWS_CHK,m_pEditBox->m_bHasArrows);
	Enable(IDC_TEXT_MULTILINE_CHK, !m_pEditBox->m_bIsNumberEdit);
	SetBool(IDC_TEXT_MULTILINE_CHK, m_pEditBox->m_bIsMultilineText);
	Enable(IDC_TEXT_MULTILINE_MONOSPACED_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
	Enable(IDC_TEXT_MULTILINE_SYNTAXCOLOR_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
	Enable(IDC_TEXT_MULTILINE_WORDWRAP_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
	Enable(IDC_TEXT_MULTILINE_STATUSBAR_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
	Enable(IDC_TEXT_MULTILINE_PYTHON_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
	SetBool(IDC_TEXT_MULTILINE_MONOSPACED_CHK, m_pEditBox->m_lMultilineStyle & DR_MULTILINE_MONOSPACED);
	SetBool(IDC_TEXT_MULTILINE_SYNTAXCOLOR_CHK, m_pEditBox->m_lMultilineStyle & DR_MULTILINE_SYNTAXCOLOR);
	SetBool(IDC_TEXT_MULTILINE_WORDWRAP_CHK, m_pEditBox->m_lMultilineStyle & DR_MULTILINE_WORDWRAP);
	SetBool(IDC_TEXT_MULTILINE_PYTHON_CHK, m_pEditBox->m_lMultilineStyle & DR_MULTILINE_PYTHON);
	SetBool(IDC_TEXT_MULTILINE_STATUSBAR_CHK, m_pEditBox->m_lMultilineStyle & DR_MULTILINE_STATUSBAR);
}

/*********************************************************************\
	Function name    : CEditboxSettings::Command
	Description      :
	Created at       : 12.08.01, @ 15:24:41
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CEditboxSettings::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDC_TYPE_SELECT_COMBO: {
		Int32 l;
		GetInt32(IDC_TYPE_SELECT_COMBO, l);
		m_pEditBox->m_bIsNumberEdit = (l == 0);
		Enable(IDC_HAS_SLIDER_CHK, m_pEditBox->m_bIsNumberEdit);
		Enable(IDC_HAS_ARROWS_CHK, m_pEditBox->m_bIsNumberEdit);
		Enable(IDC_TEXT_MULTILINE_CHK, !m_pEditBox->m_bIsNumberEdit);
		Enable(IDC_TEXT_MULTILINE_MONOSPACED_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
		Enable(IDC_TEXT_MULTILINE_SYNTAXCOLOR_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
		Enable(IDC_TEXT_MULTILINE_PYTHON_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
		Enable(IDC_TEXT_MULTILINE_WORDWRAP_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
		Enable(IDC_TEXT_MULTILINE_STATUSBAR_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
		m_pEditBox->ItemChanged();
		break;
															}
	case IDC_HAS_SLIDER_CHK:
		GetBool(IDC_HAS_SLIDER_CHK, m_pEditBox->m_bHasSlider);
		if (m_pEditBox->m_bHasSlider)
		{
			m_pEditBox->m_bHasArrows = false;
			SetBool(IDC_HAS_ARROWS_CHK, m_pEditBox->m_bHasArrows);
		}
		m_pEditBox->ItemChanged();
		break;
	case IDC_HAS_ARROWS_CHK:
		GetBool(IDC_HAS_ARROWS_CHK, m_pEditBox->m_bHasArrows);
		if (m_pEditBox->m_bHasArrows)
		{
			m_pEditBox->m_bHasSlider = false;
			SetBool(IDC_HAS_SLIDER_CHK, m_pEditBox->m_bHasSlider);
		}
		m_pEditBox->ItemChanged();
		break;
	case IDC_TEXT_MULTILINE_CHK:
		GetBool(IDC_TEXT_MULTILINE_CHK, m_pEditBox->m_bIsMultilineText);
		Enable(IDC_TEXT_MULTILINE_MONOSPACED_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
		Enable(IDC_TEXT_MULTILINE_SYNTAXCOLOR_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
		Enable(IDC_TEXT_MULTILINE_PYTHON_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
		Enable(IDC_TEXT_MULTILINE_WORDWRAP_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
		Enable(IDC_TEXT_MULTILINE_STATUSBAR_CHK, (!m_pEditBox->m_bIsNumberEdit) && m_pEditBox->m_bIsMultilineText);
		m_pEditBox->ItemChanged();
		break;
	case IDC_TEXT_MULTILINE_MONOSPACED_CHK:
	case IDC_TEXT_MULTILINE_SYNTAXCOLOR_CHK:
	case IDC_TEXT_MULTILINE_WORDWRAP_CHK:
	case IDC_TEXT_MULTILINE_PYTHON_CHK:
	case IDC_TEXT_MULTILINE_STATUSBAR_CHK: {
		Bool b;
		m_pEditBox->m_lMultilineStyle = 0;
		GetBool(IDC_TEXT_MULTILINE_MONOSPACED_CHK, b); if (b) m_pEditBox->m_lMultilineStyle |= DR_MULTILINE_MONOSPACED;
		GetBool(IDC_TEXT_MULTILINE_SYNTAXCOLOR_CHK, b); if (b) m_pEditBox->m_lMultilineStyle |= DR_MULTILINE_SYNTAXCOLOR;
		GetBool(IDC_TEXT_MULTILINE_WORDWRAP_CHK, b); if (b) m_pEditBox->m_lMultilineStyle |= DR_MULTILINE_WORDWRAP;
		GetBool(IDC_TEXT_MULTILINE_PYTHON_CHK, b); if (b) m_pEditBox->m_lMultilineStyle |= DR_MULTILINE_PYTHON;
		GetBool(IDC_TEXT_MULTILINE_STATUSBAR_CHK, b); if (b) m_pEditBox->m_lMultilineStyle |= DR_MULTILINE_STATUSBAR;
		m_pEditBox->ItemChanged();
		break;
																				 }

	}

	return true;
}

