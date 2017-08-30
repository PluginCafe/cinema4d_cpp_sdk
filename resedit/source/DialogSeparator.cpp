/*********************************************************************\
	File name        : DialogSeparator.cpp
	Description      :
	Created at       : 23.08.01, @ 22:57:40
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

CDialogSeparator::CDialogSeparator(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_SEPARATOR);
	m_strDescr = m_strName;
	m_type = Separator;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 0;
	m_lInitH = 0;
	m_bHorz = true;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogSeparator::CreateElementBegin
	Description      :
	Created at       : 15.08.01, @ 17:09:08
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogSeparator::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	if (m_bHorz)
		pDlg->AddSeparatorH(CONVERT_WIDTH(m_lInitW));
	else
		pDlg->AddSeparatorV(CONVERT_HEIGHT(m_lInitH));
}

/*********************************************************************\
	Function name    : CDialogRadioBtn::Load
	Description      :
	Created at       : 17.08.01, @ 12:48:43
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogSeparator::Load(BaseContainer* bc)
{
	Bool bRes;
	bRes = CDialogItem::Load(bc);

	m_bHorz = !(m_lFlags & BFV_SCALE);

	return bRes;
}

/*********************************************************************\
	Function name    : CDialogSeparator::Save
	Description      :
	Created at       : 30.08.01, @ 10:29:39
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogSeparator::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "SEPARATOR { ");
	if (m_bHorz) WriteString(pFile, "SCALE_H;");
	else WriteString(pFile, "SCALE_V;");
	WriteString(pFile, " }");
	return true;
}




CSeparatorSettings::CSeparatorSettings()
{
	m_lTabPageID = IDC_SEPARATOR_SETTINGS_TAB;
}

CSeparatorSettings::~CSeparatorSettings()
{
}

/*********************************************************************\
	Function name    : CSeparatorSettings::SetData
	Description      :
	Created at       : 15.08.01, @ 17:11:24
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CSeparatorSettings::SetData()
{
	_Init();

	SetInt32(IDC_SEPARATOR_TYPE_COMBO, m_pSeparator->m_bHorz ? 0 : 1);
}

/*********************************************************************\
	Function name    : CSeparatorSettings::Command
	Description      :
	Created at       : 15.08.01, @ 17:11:22
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CSeparatorSettings::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID) {
	case IDC_SEPARATOR_TYPE_COMBO: {
		Int32 l;
		GetInt32(IDC_SEPARATOR_TYPE_COMBO, l);
		m_pSeparator->m_bHorz = (l == 0);
		m_pSeparator->ItemChanged();
		break;
																 }
	}
	return true;
}
