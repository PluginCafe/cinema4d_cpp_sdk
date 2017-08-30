/*********************************************************************\
	File name        : SubDialog.cpp
	Description      :
	Created at       : 03.09.01, @ 20:53:18
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

CDialogSubDlg::CDialogSubDlg(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_SUB_DIALOG);
	m_strDescr = m_strName;
	m_type = SubDialogControl;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 0;
	m_lInitH = 0;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogSubDlg::CreateElementBegin
	Description      :
	Created at       : 03.09.01, @ 20:53:17
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogSubDlg::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	if (pDlg->Get()) C4DOS.Cd->AddGadget(pDlg->Get(), DIALOG_SUBDIALOG, lID, 0, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH), 0, nullptr, nullptr);
}

/*********************************************************************\
	Function name    : CDialogSubDlg::Load
	Description      :
	Created at       : 03.09.01, @ 21:02:18
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogSubDlg::Load(BaseContainer* bc)
{
	return CDialogItem::Load(bc);
}

/*********************************************************************\
	Function name    : CDialogSubDlg::Save
	Description      :
	Created at       : 03.09.01, @ 21:02:20
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogSubDlg::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "SUBDIALOG ");
	WriteString(pFile, m_strControlID);
	WriteString(pFile, " { ");
	SaveAlignment(pFile, false, false);
	WriteString(pFile, "}");
	return true;
}



CSubDialogSettings::CSubDialogSettings()
{
	m_lTabPageID = IDC_SUB_DLG_SETTINGS_TAB;
}

CSubDialogSettings::~CSubDialogSettings()
{
}

/*********************************************************************\
	Function name    : CSubDialogSettings::SetData
	Description      :
	Created at       : 03.09.01, @ 20:56:19
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CSubDialogSettings::SetData()
{
	_Init();
}

/*********************************************************************\
	Function name    : CSubDialogSettings::Command
	Description      :
	Created at       : 03.09.01, @ 20:56:17
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CSubDialogSettings::Command(Int32 lID, const BaseContainer &msg)
{
	return true;
}
