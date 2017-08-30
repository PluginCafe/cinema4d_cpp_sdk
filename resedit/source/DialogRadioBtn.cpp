/*********************************************************************\
	File name        : DialogRadioBtn.cpp
	Description      :
	Created at       : 23.08.01, @ 22:57:37
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

CDialogRadioBtn::CDialogRadioBtn(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_RADIO_BTN);
	m_strDescr = m_strName;
	m_type = RadioBtn;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 0;
	m_lInitH = 0;

	m_bText = false;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogRadioBtn::CreateElementBegin
	Description      :
	Created at       : 15.08.01, @ 16:37:32
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogRadioBtn::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	if (m_bText)
		pDlg->AddRadioText(lID,m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH), m_strName);
	else
		pDlg->AddRadioButton(lID,m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH), m_strName);
}

/*********************************************************************\
	Function name    : CDialogListBox::Load
	Description      :
	Created at       : 17.08.01, @ 12:48:16
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogRadioBtn::Load(BaseContainer* bc)
{
	m_bText = (bc->GetId()/*>GetInt32(DR_TYPE)*/ == DROLDWIN_SPECIALRADIO);
	return CDialogItem::Load(bc);
}

/*********************************************************************\
	Function name    : CDialogRadioBtn::Save
	Description      :
	Created at       : 30.08.01, @ 10:29:19
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogRadioBtn::Save(BaseFile* pFile, String strFill)
{
	if (m_bText)
		WriteString(pFile, "SPECIALRADIO ");
	else
		WriteString(pFile, "RADIOGADGET ");
	WriteString(pFile, m_strControlID);
	//LineBreak(pFile, strFill);

	WriteString(pFile, " { ");
	SaveAlignment(pFile);
	WriteString(pFile, " }");

	return true;
}


CRadioBtnSettings::CRadioBtnSettings()
{
	m_lTabPageID = IDC_RADIO_BTN_SETTINGS_TAB;
}

CRadioBtnSettings::~CRadioBtnSettings()
{
}

/*********************************************************************\
	Function name    : CRadioBtnSettings::SetData
	Description      :
	Created at       : 15.08.01, @ 16:37:41
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CRadioBtnSettings::SetData()
{
	_Init();

	SetBool(IDC_RADIO_BTN_IS_TEXT_CHK, m_pRadioBtn->m_bText);
}

/*********************************************************************\
	Function name    : CRadioBtnSettings::Command
	Description      :
	Created at       : 15.08.01, @ 16:37:44
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CRadioBtnSettings::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDC_RADIO_BTN_IS_TEXT_CHK:
		GetBool(IDC_RADIO_BTN_IS_TEXT_CHK, m_pRadioBtn->m_bText);
		m_pRadioBtn->ItemChanged();
		break;
	};
	return true;
}
