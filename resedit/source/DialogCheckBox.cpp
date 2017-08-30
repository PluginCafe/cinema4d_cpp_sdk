/*********************************************************************\
	File name        : DialogCheckBox.cpp
	Description      :
	Created at       : 11.08.01, @ 16:52:04
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
	Function name    : CDialogCheckBox::CDialogCheckBox
	Description      :
	Created at       : 11.08.01, @ 17:19:05
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogCheckBox::CDialogCheckBox(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_CHECKBOX);
	m_strDescr = m_strName;
	m_type = CheckBox;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 0;
	m_lInitH = 0;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogCheckBox::CreateElement
	Description      :
	Created at       : 11.08.01, @ 16:52:39
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogCheckBox::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	pDlg->AddCheckbox(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH), m_strName);
}

/*********************************************************************\
	Function name    : CDialogCheckBox::Save
	Description      :
	Created at       : 11.08.01, @ 16:52:41
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogCheckBox::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "CHECKBOX ");
	WriteString(pFile, m_strControlID);
	//LineBreak(pFile, strFill);

	WriteString(pFile, " { ");
	SaveAlignment(pFile);
	WriteString(pFile, " }");

	return true;
}

/*********************************************************************\
	Function name    : CDialogCheckBox::Load
	Description      :
	Created at       : 11.08.01, @ 16:52:45
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogCheckBox::Load(BaseContainer* bc)
{
	return CDialogItem::Load(bc);
}



CCheckboxSettings::CCheckboxSettings()
{
	m_lTabPageID = IDC_CHECKBOX_SETTINGS_TAB;
}

CCheckboxSettings::~CCheckboxSettings()
{

}

/*********************************************************************\
	Function name    : CCheckboxSettings::SetData
	Description      :
	Created at       : 11.08.01, @ 23:18:59
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CCheckboxSettings::SetData()
{
	_Init();
}

