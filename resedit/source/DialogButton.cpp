/*********************************************************************\
	File name        : DialogButton.cpp
	Description      :
	Created at       : 11.08.01, @ 17:21:17
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
	Function name    : CDialogButton::CDialogButton
	Description      :
	Created at       : 11.08.01, @ 17:21:23
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogButton::CDialogButton(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_BUTTON);
	m_strDescr = m_strName;
	m_type = Button;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 0;
	m_lInitH = 0;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogButton::CreateElement
	Description      :
	Created at       : 11.08.01, @ 17:21:48
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogButton::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	pDlg->AddButton(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH), m_strName);
}

/*********************************************************************\
	Function name    : CDialogButton::Save
	Description      :
	Created at       : 11.08.01, @ 17:21:49
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogButton::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "BUTTON ");
	WriteString(pFile, m_strControlID);
	WriteString(pFile, " { ");
	SaveAlignment(pFile);
	WriteString(pFile, "}");

	return true;
}

/*********************************************************************\
	Function name    : CDialogButton::Load
	Description      :
	Created at       : 11.08.01, @ 17:21:51
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogButton::Load(BaseContainer* bc)
{
	return CDialogItem::Load(bc);
}



CButtonSettings::CButtonSettings()
{
	m_lTabPageID = IDC_BUTTON_SETTINGS_TAB;
}

CButtonSettings::~CButtonSettings()
{

}

/*********************************************************************\
	Function name    : CButtonSettings::SetData
	Description      :
	Created at       : 11.08.01, @ 23:19:29
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CButtonSettings::SetData()
{
	_Init();
}
