/*********************************************************************\
	File name        : ComboBox.cpp
	Description      :
	Created at       : 15.08.01, @ 18:51:56
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

CDialogComboBox::CDialogComboBox(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_COMBOBOX);
	m_strDescr = m_strName;
	m_type = ComboBox;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 150;
	m_lInitH = 0;
//  m_lChildren = 0;
//  m_pstrChildren = nullptr;
//  m_plChildIDs = nullptr;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

CDialogComboBox::~CDialogComboBox()
{
//  m_lChildren = 0;
//  bDelete(m_pstrChildren);
//  bDelete(m_plChildIDs);
}

/*********************************************************************\
	Function name    : CDialogComboBox::CreateElementBegin
	Description      :
	Created at       : 15.08.01, @ 18:58:36
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogComboBox::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	pDlg->AddComboBox(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));

	Int32 lChildren = m_Children.GetItemCount();
	String strID, strStringID;
	Bool bFound;
	for (Int32 a = 0; a < lChildren; a++)
	{
		m_Children.GetElement(a, strID, strStringID);
		pDlg->AddChild(lID, a, m_pDocument->GetString(strStringID, bFound));
	}
}

/*********************************************************************\
	Function name    : CDialogComboBox::Load
	Description      :
	Created at       : 17.08.01, @ 12:45:14
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogComboBox::Load(BaseContainer* bc)
{
//  bDelete(m_pstrChildren);
//  bDelete(m_plChildIDs);
//  m_lChildren = 0;

	BaseContainer child;
	Int32 lChildren = bc->GetInt32(DR_NUMCHILDREN);
	for (Int32 a = 0; a < lChildren; a++)
	{
		child = bc->GetContainer(DR_CHILD + a);

		//if (m_lChildren) continue;
		if (child.GetId()/*.GetInt32(DR_TYPE)*/ != DRTYPE_COMBO_POPUP_CHILDREN) continue;

		m_Children.Load(&child);
	}

	return CDialogItem::Load(bc);
}

/*********************************************************************\
	Function name    : CDialogComboBox::Save
	Description      :
	Created at       : 30.08.01, @ 10:27:44
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogComboBox::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "COMBOBOX ");
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
	return true;
}



CComboBoxSettings::CComboBoxSettings()
{
	m_lTabPageID = IDC_COMBO_BOX_SETTINGS_TAB;
}

CComboBoxSettings::~CComboBoxSettings()
{
}

/*********************************************************************\
	Function name    : CComboBoxSettings::SetData
	Description      :
	Created at       : 15.08.01, @ 18:59:46
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CComboBoxSettings::SetData()
{
	_Init();
	m_pComboBox->m_Children.FillEditBox(m_pSettingsDialog, IDC_COMBO_CHILD_ITEMS);
}

/*********************************************************************\
	Function name    : CComboBoxSettings::Command
	Description      :
	Created at       : 15.08.01, @ 18:59:47
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CComboBoxSettings::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
		case IDC_COMBO_CHILD_ITEMS:
			{
				String str;
				GetString(IDC_COMBO_CHILD_ITEMS, str);
				if (m_pComboBox)
				{
					m_pComboBox->GetChildren()->FromString(str);
					m_pComboBox->ItemChanged();
				}
			}
			break;
	};
	return true;
}

/*********************************************************************\
	Function name    : CDialogComboBox::GetChildItems
	Description      :
	Created at       : 03.10.01, @ 20:03:07
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
#if 0
void CDialogComboBox::GetChildItems(String str)
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



CDialogComboButton::CDialogComboButton(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_COMBOBUTTON);
	m_strDescr = m_strName;
	m_type = ComboButton;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 150;
	m_lInitH = 0;
	//  m_lChildren = 0;
	//  m_pstrChildren = nullptr;
	//  m_plChildIDs = nullptr;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

CDialogComboButton::~CDialogComboButton()
{
	//  m_lChildren = 0;
	//  bDelete(m_pstrChildren);
	//  bDelete(m_plChildIDs);
}

/*********************************************************************\
Function name    : CDialogComboButton::CreateElementBegin
Description      :
Created at       : 15.08.01, @ 18:58:36
Created by       : Thomas Kunert
Modified by      :
\*********************************************************************/
void CDialogComboButton::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	pDlg->AddComboButton(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));

	Int32 lChildren = m_Children.GetItemCount();
	String strID, strStringID;
	Bool bFound;
	for (Int32 a = 0; a < lChildren; a++)
	{
		m_Children.GetElement(a, strID, strStringID);
		pDlg->AddChild(lID, a, m_pDocument->GetString(strStringID, bFound));
	}
}

/*********************************************************************\
Function name    : CDialogComboButton::Load
Description      :
Created at       : 17.08.01, @ 12:45:14
Created by       : Thomas Kunert
Modified by      :
\*********************************************************************/
Bool CDialogComboButton::Load(BaseContainer* bc)
{
	//  bDelete(m_pstrChildren);
	//  bDelete(m_plChildIDs);
	//  m_lChildren = 0;

	BaseContainer child;
	Int32 lChildren = bc->GetInt32(DR_NUMCHILDREN);
	for (Int32 a = 0; a < lChildren; a++)
	{
		child = bc->GetContainer(DR_CHILD + a);

		//if (m_lChildren) continue;
		if (child.GetId()/*.GetInt32(DR_TYPE)*/ != DRTYPE_COMBO_POPUP_CHILDREN) continue;

		m_Children.Load(&child);
	}

	return CDialogItem::Load(bc);
}

/*********************************************************************\
Function name    : CDialogComboButton::Save
Description      :
Created at       : 30.08.01, @ 10:27:44
Created by       : Thomas Kunert
Modified by      :
\*********************************************************************/
Bool CDialogComboButton::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "COMBOBUTTON ");
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
	return true;
}



CComboButtonSettings::CComboButtonSettings()
{
	m_lTabPageID = IDC_COMBO_BUTTON_SETTINGS_TAB;
}

CComboButtonSettings::~CComboButtonSettings()
{
}

/*********************************************************************\
Function name    : CComboButtonSettings::SetData
Description      :
Created at       : 15.08.01, @ 18:59:46
Created by       : Thomas Kunert
Modified by      :
\*********************************************************************/
void CComboButtonSettings::SetData()
{
	_Init();
	m_pComboButton->m_Children.FillEditBox(m_pSettingsDialog, IDC_COMBOBUTTON_CHILD_ITEMS);
}

/*********************************************************************\
Function name    : CComboButtonSettings::Command
Description      :
Created at       : 15.08.01, @ 18:59:47
Created by       : Thomas Kunert
Modified by      :
\*********************************************************************/
Bool CComboButtonSettings::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
		case IDC_COMBOBUTTON_CHILD_ITEMS:
			{
				String str;
				GetString(IDC_COMBOBUTTON_CHILD_ITEMS, str);
				if (m_pComboButton)
				{
					m_pComboButton->GetChildren()->FromString(str);
					m_pComboButton->ItemChanged();
				}
			}
			break;
	};
	return true;
}
