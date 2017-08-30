/*********************************************************************\
	File name        : DialogArrow.cpp
	Description      :
	Created at       : 12.08.01, @ 21:51:42
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
	Function name    : CDialogArrowBtn::CDialogArrowBtn
	Description      :
	Created at       : 12.08.01, @ 21:52:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogArrowBtn::CDialogArrowBtn(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_ARROW);
	m_strDescr = m_strName;
	m_type = ArrowBtn;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 0;
	m_lInitH = 0;
	m_lArrowType = ARROW_LEFT;
	m_bIsPopupButton = false;
//  m_lChildren = 0;
//  m_pstrChildren = nullptr;
//  m_plChildIDs = nullptr;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogArrowBtn::~CDialogArrowBtn
	Description      :
	Created at       : 22.08.01, @ 16:11:34
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogArrowBtn::~CDialogArrowBtn()
{
//  m_lChildren = 0;
//  bDelete(m_pstrChildren);
//  bDelete(m_plChildIDs);
}

/*********************************************************************\
	Function name    : CDialogArrowBtn::CreateElementBegin
	Description      :
	Created at       : 12.08.01, @ 21:53:03
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogArrowBtn::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	if (m_bIsPopupButton)
	{
		pDlg->AddPopupButton(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));
		String strID, strStringID;
		Bool bFound;
		for (Int32 a = 0; a < m_Children.GetItemCount(); a++)
		{
			m_Children.GetElement(a, strID, strStringID);
			pDlg->AddChild(lID, a, m_pDocument->GetString(strStringID, bFound));
		}
	}
	else
		pDlg->AddArrowButton(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH), m_lArrowType);
}

/*********************************************************************\
	Function name    : CDialogArrowBtn::Load
	Description      :
	Created at       : 17.08.01, @ 12:43:50
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogArrowBtn::Load(BaseContainer* bc)
{
	Int32 lType = bc->GetId();//>GetInt32(DR_TYPE);

	m_bIsPopupButton = (lType == DROLDWIN_POPUPBUTTON);
	m_lArrowType = bc->GetInt32(DR_ARROWTYPE);

//  bDelete(m_pstrChildren);
//  bDelete(m_plChildIDs);
	m_Children.Destroy();
//  m_lChildren = 0;

	BaseContainer child;
	Int32 lChildren = bc->GetInt32(DR_NUMCHILDREN);
	for (Int32 a = 0; a < lChildren; a++)
	{
		child = bc->GetContainer(DR_CHILD + a);

		//if (m_lChildren) continue;
		if (child.GetId()/*.GetInt32(DR_TYPE)*/ != DRTYPE_COMBO_POPUP_CHILDREN) continue;

		m_Children.Load(&child);

/*    Int32 lChildItems = child.GetInt32(0);
//    m_pstrChildren = bNewDeprecatedUseArraysInstead String[m_lChildren];
//    m_plChildIDs = bNewDeprecatedUseArraysInstead Int32[m_lChildren];
		for (Int32 a = 0; a < lChildItems; a++)
		{
			m_Children.AddElement(child.GetString(2 * a + 2), child.GetString(2 * a + 3));
			//m_plChildIDs[a] = child.GetInt32(2 * a + 2);
			//m_pstrChildren[a] = child.GetString(2 * a + 3);
		}*/
	}

	return CDialogItem::Load(bc);
}


/*********************************************************************\
	Function name    : CDialogArrowBtn::Save
	Description      :
	Created at       : 30.08.01, @ 10:27:02
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogArrowBtn::Save(BaseFile* pFile, String strFill)
{
	if (m_bIsPopupButton)
	{
		WriteString(pFile, "POPUPBUTTON ");
		WriteString(pFile, m_strControlID);

		LineBreak(pFile, strFill);
		WriteString(pFile, "{");
		LineBreak(pFile, strFill + SPACE_NEW_LINE);
		SaveAlignment(pFile);
		LineBreak(pFile, strFill + SPACE_NEW_LINE);
		WriteString(pFile, "CHILDS");
		LineBreak(pFile, strFill + SPACE_NEW_LINE);
		WriteString(pFile, "{");

		if (m_Children.GetItemCount() > 0)
			LineBreak(pFile, strFill + SPACE_NEW_LINE + SPACE_NEW_LINE);
		else
			LineBreak(pFile, strFill + SPACE_NEW_LINE);

		m_Children.Write(pFile, strFill);

		WriteString(pFile, "}");
		LineBreak(pFile, strFill);
		WriteString(pFile, "}");
	}
	else
	{
		// it's just an arrow button
		WriteString(pFile, "ARROWBUTTON ");
		WriteString(pFile, m_strControlID);

		WriteString(pFile, " { ");
		SaveAlignment(pFile);

		if (m_lArrowType == ARROW_LEFT) WriteString(pFile, "ARROW_LEFT; ");
		if (m_lArrowType == ARROW_RIGHT) WriteString(pFile, "ARROW_RIGHT; ");
		if (m_lArrowType == ARROW_UP) WriteString(pFile, "ARROW_UP; ");
		if (m_lArrowType == ARROW_DOWN) WriteString(pFile, "ARROW_DOWN; ");

		if (m_lArrowType == ARROW_SMALL_LEFT) WriteString(pFile, "ARROW_SMALL_LEFT; ");
		if (m_lArrowType == ARROW_SMALL_RIGHT) WriteString(pFile, "ARROW_SMALL_RIGHT; ");
		if (m_lArrowType == ARROW_SMALL_UP) WriteString(pFile, "ARROW_SMALL_UP; ");
		if (m_lArrowType == ARROW_SMALL_DOWN) WriteString(pFile, "ARROW_SMALL_DOWN; ");

		WriteString(pFile, " }");
	}

	return true;
}

CArrowSettings::CArrowSettings()
{
	m_lTabPageID = IDC_ARROW_SETTINGS_TAB;
}

CArrowSettings::~CArrowSettings()
{

}

/*********************************************************************\
	Function name    : CArrowSettings::SetData
	Description      :
	Created at       : 12.08.01, @ 21:55:29
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CArrowSettings::SetData()
{
	_Init();

	SetBool(IDS_ARROW_SMALL_CHK, m_pArrow->m_lArrowType > 4);
	Int32 l = m_pArrow->m_lArrowType;
	if (l > 4) l -= 4;
	SetInt32(IDS_ARROW_TYPE_COMBO, l - 1);
	SetBool(IDC_IS_POPUP_CHK, m_pArrow->m_bIsPopupButton);
	Enable(IDS_ARROW_SMALL_CHK, !m_pArrow->m_bIsPopupButton);
	Enable(IDS_ARROW_TYPE_COMBO, !m_pArrow->m_bIsPopupButton);
	Enable(IDC_ARROW_CHILD_ITEMS, m_pArrow->m_bIsPopupButton);

	m_pArrow->m_Children.FillEditBox(m_pSettingsDialog, IDC_ARROW_CHILD_ITEMS);
}


/*********************************************************************\
	Function name    : CArrowSettings::Command
	Description      :
	Created at       : 12.08.01, @ 21:55:31
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CArrowSettings::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDS_ARROW_SMALL_CHK:
	case IDS_ARROW_TYPE_COMBO: {
		Bool b;
		Int32 l;
		GetBool(IDS_ARROW_SMALL_CHK, b);
		GetInt32(IDS_ARROW_TYPE_COMBO, l);
		m_pArrow->m_lArrowType = l + 1 + (b ? 4 : 0);
		m_pArrow->ItemChanged();
		break;
														 }
	case IDC_IS_POPUP_CHK:
		GetBool(IDC_IS_POPUP_CHK, m_pArrow->m_bIsPopupButton);
		Enable(IDS_ARROW_SMALL_CHK, !m_pArrow->m_bIsPopupButton);
		Enable(IDS_ARROW_TYPE_COMBO, !m_pArrow->m_bIsPopupButton);
		Enable(IDC_ARROW_CHILD_ITEMS, m_pArrow->m_bIsPopupButton);
		m_pArrow->ItemChanged();
		break;

	case IDC_ARROW_CHILD_ITEMS: {
		String str;
		GetString(IDC_ARROW_CHILD_ITEMS, str);
		m_pArrow->GetChildren()->FromString(str);//>GetChildItems(str);
		m_pArrow->ItemChanged();
		break;
												}
	}
	return true;
}

/*********************************************************************\
	Function name    : CDialogArrowBtn::GetChildItems
	Description      :
	Created at       : 03.10.01, @ 20:03:07
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
#if 0
void CDialogArrowBtn::GetChildItems(String str)
{
	m_Children.FromString(str);
	// get all lines of the edit box
	/*if (str.SubStr(str.GetLength() - 1, 1) != "\n")
		str += "\n";

	Int32 lPos, lStart = 0;
	Int32 lElements = 0;

	while (str.FindFirst("\n", &lPos, lStart))
	{
		lElements++;
		lStart = lPos + 1;
	}

	bDelete(m_pstrChildren);
	bDelete(m_plChildIDs);

	m_lChildren = lElements;
	m_pstrChildren = bNewDeprecatedUseArraysInstead String[lElements];
	m_plChildIDs = bNewDeprecatedUseArraysInstead Int32[lElements];
	lStart = 0;

	lElements = 0;
	while (str.FindFirst("\n", &lPos, lStart))
	{
		String strElement = str.SubStr(lStart, lPos - lStart);
		lStart = lPos + 1;
		m_pstrChildren[lElements] = strElement;
		m_plChildIDs[lElements] = lElements;
		lElements++;
		//TRACE_STRING(strElement);
	}*/
}
#endif
