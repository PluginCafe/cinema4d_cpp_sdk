/*********************************************************************\
	File name        : DialogDlgGroup.cpp
	Description      :
	Created at       : 15.08.01, @ 19:19:31
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

CDialogDlgGroup::CDialogDlgGroup(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_DIALOG_GROUP);
	m_strDescr = m_strName;
	m_type = DialogGroup;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 0;
	m_lInitH = 0;
	m_lItems = DLG_OK | DLG_CANCEL;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogDialogGroup::CreateElementBegin
	Description      :
	Created at       : 15.08.01, @ 19:20:31
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDlgGroup::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	pDlg->AddDlgGroup(m_lItems);
}

/*********************************************************************\
	Function name    : CDialogDlgGroup::Load
	Description      :
	Created at       : 17.08.01, @ 12:47:15
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDlgGroup::Load(BaseContainer* bc)
{
	m_lItems = (bc->GetBool(DR_DLGGROUP_OK) ? DLG_OK : 0) |
						 (bc->GetBool(DR_DLGGROUP_CANCEL) ? DLG_CANCEL : 0);

	return CDialogItem::Load(bc);
}

/*********************************************************************\
	Function name    : CDialogDlgGroup::Save
	Description      :
	Created at       : 30.08.01, @ 10:27:56
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDlgGroup::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "DLGGROUP ");
	WriteString(pFile, "{ ");
	if (m_lItems & DLG_OK) WriteString(pFile, "OK; ");
	if (m_lItems & DLG_CANCEL) WriteString(pFile, "CANCEL; ");
	WriteString(pFile, "}");

	return true;
}



CDialogGroupSettings::CDialogGroupSettings()
{
	m_lTabPageID = IDC_DIALOG_GROUP_SETTINGS_TAB;
}

CDialogGroupSettings::~CDialogGroupSettings()
{
}

/*********************************************************************\
	Function name    : CDialogGroupSettings::SetData
	Description      :
	Created at       : 15.08.01, @ 19:20:48
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogGroupSettings::SetData()
{
	_Init();
	SetBool(IDC_DLG_GROUP_OK_CHK, m_pDialogGroup->m_lItems & DLG_OK);
	SetBool(IDC_DLG_GROUP_CANCEL_CHK, m_pDialogGroup->m_lItems & DLG_CANCEL);
}

/*********************************************************************\
	Function name    : CDialogGroupSettings::Command
	Description      :
	Created at       : 15.08.01, @ 19:20:49
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogGroupSettings::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
		case IDC_DLG_GROUP_OK_CHK:
		case IDC_DLG_GROUP_CANCEL_CHK:
		case IDC_DLG_GROUP_HELP_CHK: {
			m_pDialogGroup->m_lItems = 0;
			Bool b;
			GetBool(IDC_DLG_GROUP_OK_CHK, b); if (b) m_pDialogGroup->m_lItems |= DLG_OK;
			GetBool(IDC_DLG_GROUP_CANCEL_CHK, b); if (b) m_pDialogGroup->m_lItems |= DLG_CANCEL;
			m_pDialogGroup->ItemChanged();
			break;
																 }
	}
	return true;
}
