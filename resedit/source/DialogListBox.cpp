/*********************************************************************\
	File name        : DialogListBox.cpp
	Description      :
	Created at       : 15.08.01, @ 18:59:26
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

CDialogListBox::CDialogListBox(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_LISTBOX);
	m_strDescr = m_strName;
	m_type = ListBox;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 150;
	m_lInitH = 50;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

CDialogListBox::~CDialogListBox()
{
}

/*********************************************************************\
	Function name    : CDialogListBox::CreateElementBegin
	Description      :
	Created at       : 15.08.01, @ 18:53:17
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogListBox::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	pDlg->AddListView(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));
}

/*********************************************************************\
	Function name    : CDialogGroup::Load
	Description      :
	Created at       : 17.08.01, @ 12:48:00
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogListBox::Load(BaseContainer* bc)
{
	return CDialogItem::Load(bc);
}

/*********************************************************************\
	Function name    : CDialogListBox::Save
	Description      :
	Created at       : 30.08.01, @ 10:28:57
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogListBox::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "LISTVIEW ");
	WriteString(pFile, m_strControlID);
	WriteString(pFile, " { ");
	SaveAlignment(pFile);
	WriteString(pFile, "}");
	return true;
}



CListBoxSettings::CListBoxSettings()
{
	m_lTabPageID = IDC_LIST_VIEW_SETTINGS_TAB;
}

CListBoxSettings::~CListBoxSettings()
{
}

/*********************************************************************\
	Function name    : CListBoxSettings::SetData
	Description      :
	Created at       : 15.08.01, @ 18:53:48
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CListBoxSettings::SetData()
{
	_Init();
}

/*********************************************************************\
	Function name    : CListBoxSettings::Command
	Description      :
	Created at       : 15.08.01, @ 18:53:50
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CListBoxSettings::Command(Int32 lID, const BaseContainer &msg)
{
	return true;
}
