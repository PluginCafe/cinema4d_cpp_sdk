/*********************************************************************\
	File name        : DialogStatic.cpp
	Description      : Implementation of the class CDialogStatic
	Created at       : 11.08.01, @ 21:59:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"
#include "DialogItem.h"
#include "DialogItemSettings.h"
#include "DialogDoc.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*********************************************************************\
	Function name    : CDialogStatic::CDialogStatic
	Description      :
	Created at       : 11.08.01, @ 22:11:18
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogStatic::CDialogStatic(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_STATIC);
	m_strDescr = m_strName;
	m_type = Static;
	m_lFlags = BFH_LEFT | BFV_CENTER;
	m_lInitW = 0;
	m_lInitH = 0;
	m_lBorderStyle = 0;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogStatic::CreateElementBegin
	Description      :
	Created at       : 11.08.01, @ 22:00:08
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogStatic::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	pDlg->AddStaticText(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH), m_strName, m_lBorderStyle);
}

/*********************************************************************\
	Function name    : CDialogStatic::Load
	Description      :
	Created at       : 17.08.01, @ 12:49:16
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogStatic::Load(BaseContainer* bc)
{
	m_lBorderStyle = bc->GetInt32(DR_BORDERSTYLE);

	Bool bRes = CDialogItem::Load(bc);

	if (!m_strNameID.Content())
		m_strName = "";

	return bRes;
}

/*********************************************************************\
	Function name    : CDialogStatic::Save
	Description      :
	Created at       : 30.08.01, @ 10:30:08
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogStatic::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "STATICTEXT ");
	WriteString(pFile, m_strControlID);
	WriteString(pFile, " { ");
	SaveAlignment(pFile);

	if (GetC4DVersion() >= 7400)
	{
		if (m_lBorderStyle != BORDER_NONE)
		{
			WriteString(pFile, "BORDERSTYLE ");
			//if (m_lBorderStyle == BORDER_NONE) WriteString(pFile, "BORDER_NONE");
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
			WriteString(pFile, "; ");
		}
	}

	WriteString(pFile, "}");
	return true;
}



CStaticSettings::CStaticSettings()
{
	m_lTabPageID = IDC_STATIC_SETTINGS_TAB;
}

CStaticSettings::~CStaticSettings()
{

}

/*********************************************************************\
	Function name    : CStaticSettings::Command
	Description      :
	Created at       : 11.08.01, @ 22:59:11
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CStaticSettings::Command(Int32 lID, const BaseContainer &msg)
{
	if (lID == IDC_BORDER_COMBO)
	{
		GetInt32(IDC_BORDER_COMBO, m_pStatic->m_lBorderStyle);
		m_pStatic->ItemChanged();
	}
	return true;
}

/*********************************************************************\
	Function name    : CStaticSettings::SetData
	Description      :
	Created at       : 11.08.01, @ 23:21:37
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CStaticSettings::SetData()
{
	_Init();
	SetInt32(IDC_BORDER_COMBO, m_pStatic->m_lBorderStyle);
}

