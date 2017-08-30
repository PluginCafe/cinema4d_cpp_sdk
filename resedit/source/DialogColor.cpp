/*********************************************************************\
	File name        : DialogColor.cpp
	Description      :
	Created at       : 12.08.01, @ 16:38:19
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
	Function name    : CDialogColor::CDialogColor
	Description      :
	Created at       : 12.08.01, @ 16:38:54
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogColor::CDialogColor(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_COLOR);
	m_strDescr = m_strName;
	m_type = Color;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 100;
	m_lInitH = 60;
	m_lBorderStyle = 0;
	m_bIsColorField = false;
	m_lLayoutFlags = 0;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogColor::CreateElementBegin
	Description      :
	Created at       : 12.08.01, @ 16:38:56
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogColor::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	if (m_bIsColorField)
		pDlg->AddColorField(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));
	else
		pDlg->AddColorChooser(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH), m_lLayoutFlags);
}

/*********************************************************************\
	Function name    : CDialogColor::Load
	Description      :
	Created at       : 17.08.01, @ 12:44:31
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogColor::Load(BaseContainer* bc)
{
	m_lLayoutFlags = (bc->GetBool(DR_COLORFIELD_NO_BRIGHTNESS) ? DR_COLORFIELD_NO_BRIGHTNESS : 0) |
									 (bc->GetBool(DR_COLORFIELD_NO_COLOR) ? DR_COLORFIELD_NO_COLOR : 0);
	return CDialogItem::Load(bc);
}

/*********************************************************************\
	Function name    : CDialogColor::Save
	Description      :
	Created at       : 30.08.01, @ 10:27:27
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogColor::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "COLORFIELD ");
	WriteString(pFile, m_strControlID);
	WriteString(pFile, " { ");
	SaveAlignment(pFile);

	if (m_bIsColorField || (m_lLayoutFlags & DR_COLORFIELD_NO_BRIGHTNESS)) WriteString(pFile, "NOBRIGHTNESS; ");
	if (m_bIsColorField || (m_lLayoutFlags & DR_COLORFIELD_NO_COLOR)) WriteString(pFile, "NOCOLOR; ");
	WriteString(pFile, "}");
	return true;
}



CColorSettings::CColorSettings()
{
	m_lTabPageID = IDC_COLOR_SETTINGS_TAB;
}

CColorSettings::~CColorSettings()
{

}

/*********************************************************************\
	Function name    : CColorSettings::SetData
	Description      :
	Created at       : 12.08.01, @ 16:36:33
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CColorSettings::SetData()
{
	_Init();

	SetInt32(IDC_COLOR_TYPE_COMBO, m_pColor->m_bIsColorField ? 0 : 1);
	SetBool(IDC_COLORFIELD_NO_BRIGHTNESS_CHK, m_pColor->m_lLayoutFlags & DR_COLORFIELD_NO_BRIGHTNESS);
	SetBool(IDC_COLORFIELD_NO_COLOR_CHK, m_pColor->m_lLayoutFlags & DR_COLORFIELD_NO_COLOR);
	Enable(IDC_COLORFIELD_NO_BRIGHTNESS_CHK, !m_pColor->m_bIsColorField);
	Enable(IDC_COLORFIELD_NO_COLOR_CHK, !m_pColor->m_bIsColorField);
}

/*********************************************************************\
	Function name    : Command
	Description      :
	Created at       : 12.08.01, @ 16:36:40
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CColorSettings::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDC_COLOR_TYPE_COMBO:
	case IDC_COLORFIELD_NO_BRIGHTNESS_CHK:
	case IDC_COLORFIELD_NO_COLOR_CHK: {
		Int32 l;
		Bool b;
		GetInt32(IDC_COLOR_TYPE_COMBO, l);
		m_pColor->m_bIsColorField = (l == 0);
		m_pColor->m_lLayoutFlags = 0;
		GetBool(IDC_COLORFIELD_NO_BRIGHTNESS_CHK, b); if (b) m_pColor->m_lLayoutFlags |= DR_COLORFIELD_NO_BRIGHTNESS;
		GetBool(IDC_COLORFIELD_NO_COLOR_CHK, b); if (b) m_pColor->m_lLayoutFlags |= DR_COLORFIELD_NO_COLOR;
		Enable(IDC_COLORFIELD_NO_BRIGHTNESS_CHK, !m_pColor->m_bIsColorField);
		Enable(IDC_COLORFIELD_NO_COLOR_CHK, !m_pColor->m_bIsColorField);
		m_pColor->ItemChanged();
		break;
																		}
	}
	return true;
}
