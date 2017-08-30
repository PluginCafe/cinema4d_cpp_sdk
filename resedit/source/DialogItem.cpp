 /*********************************************************************\
	File name        : DialogItem.cpp
	Description      : Implementation of the class CDialogItem
	Created at       : 11.08.01, @ 14:51:33
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"
#include "DialogItem.h"

#include "TreeDialog.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CDialogItem::CDialogItem()
{
	m_type = unknown;
	m_pDocument = nullptr;
	m_lID = -1;
}

CDialogItem::CDialogItem(CDialogDoc* pDoc)
{
	m_type = unknown;
	m_pDocument = pDoc;
	m_lID = -1;
}

CDialogItem::~CDialogItem()
{
}

/*********************************************************************\
	Function name    : CDialogItem::Load
	Description      :
	Created at       : 17.08.01, @ 12:40:11
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogItem::Load(BaseContainer* bc)
{
	/*String str = bc->GetString(DR_NAME);
	if (str.Content()) m_strName = str;*/

	Int32 l = bc->GetInt32(DR_ALIGNMENT);
	m_lFlags = l;

	m_lInitW = bc->GetInt32(DROLDWIN_POS_W, 0);
	//if (l > -1) m_lInitW = l;
	m_lInitH = bc->GetInt32(DROLDWIN_POS_H, 0);
	//if (l > -1) m_lInitH = l;

	//if (m_lInitW > (1 << 14)) m_lInitW >>= 14;
	//if (m_lInitH > (1 << 14)) m_lInitH >>= 14;

	String str = bc->GetString(DR_NAME);
	m_strNameID = str;
	m_strControlID = bc->GetString(DR_ID);
	//TRACE_STRING(m_strControlID);

	return true;
}

/*********************************************************************\
	Function name    : CDialogItem::SaveAlignment
	Description      : stores the alignment and the name of an item
	Created at       : 30.08.01, @ 10:53:25
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogItem::SaveAlignment(BaseFile* pFile, Bool bInsertBreakes, Bool bWriteSize)
{
	String strName = GetNameID();
	if (strName.Content())
	{
		WriteString(pFile, "NAME ");
		WriteString(pFile, strName);
		WriteString(pFile, "; ");
	}

	if ((m_lFlags & BFV_SCALEFIT) == BFV_SCALEFIT) WriteString(pFile, "SCALE_V");
	else if ((m_lFlags & BFV_FIT) == BFV_FIT) WriteString(pFile, "FIT_V");
	else if (m_lFlags & BFV_TOP) WriteString(pFile, "ALIGN_TOP");
	else if (m_lFlags & BFV_BOTTOM) WriteString(pFile, "ALIGN_BOTTOM");
	else if (m_lFlags & BFV_SCALE) WriteString(pFile, "SCALE_V");
	else WriteString(pFile, "CENTER_V");
	WriteString(pFile, "; ");

	if ((m_lFlags & BFH_SCALEFIT) == BFH_SCALEFIT) WriteString(pFile, "SCALE_H");
	else if ((m_lFlags & BFH_FIT) == BFH_FIT) WriteString(pFile, "FIT_H");
	else if (m_lFlags & BFH_LEFT) WriteString(pFile, "ALIGN_LEFT");
	else if (m_lFlags & BFH_RIGHT) WriteString(pFile, "ALIGN_RIGHT");
	else if (m_lFlags & BFH_SCALE) WriteString(pFile, "SCALE_H");
	else WriteString(pFile, "CENTER_H");
	WriteString(pFile, "; ");

	if (bWriteSize)
	{
		if (m_lInitW != 0 || m_lInitH != 0)
		{
			WriteString(pFile, "SIZE ");
			WriteInt32(pFile, m_lInitW);
			WriteString(pFile, ", ");
			WriteInt32(pFile, m_lInitH);
			WriteString(pFile, "; ");
		}
	}

	// ??????
	/*l = bc->GetInt32(DROLDWIN_POS_W, -1);
	if (l > -1) m_lInitW = l;
	l = bc->GetInt32(DROLDWIN_POS_H, -1);
	if (l > -1) m_lInitH = l;*/

	return true;
}

/*********************************************************************\
	Function name    : CDialogItem::Save
	Description      : stores the cildren of an item
	Created at       : 30.08.01, @ 10:12:39
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogItem::Save(BaseFile* pFile, String strFill)
{
	Int32 lChildren = m_pTreeViewItem->GetChildCount();
	for (Int32 l = 0; l < lChildren; l++)
	{
		TreeViewItem* pChildItem = m_pTreeViewItem->GetChild(l);
		CDialogItem* pDlgItem = (CDialogItem*)pChildItem->GetData()->GetVoid(TREEVIEW_DATA);
		if (!pDlgItem) continue;

		if (l > 0)
			LineBreak(pFile, strFill);

		pDlgItem->Save(pFile, strFill);

	}
	return true;
}

typedef struct
{
	String* pStr;
	Int32 lMinID;
} tagCreateControlID;

/*********************************************************************\
	Function name    : CreateControlIDVisitCallback
	Description      :
	Created at       : 04.09.01, @ 10:00:15
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CreateControlIDVisitCallback(TreeViewItem* pItem, void* pData)
{
	tagCreateControlID* pID = (tagCreateControlID*)pData;
	CDialogItem* pDlgItem = (CDialogItem*)pItem->GetData()->GetVoid(TREEVIEW_DATA);

	if (!pDlgItem) return;

	String strControlID = pDlgItem->m_strControlID;
	Int32 lPos;
	if (strControlID.FindFirst(*(pID->pStr), &lPos) && lPos == 0)
	{
		String str = strControlID.SubStr(pID->pStr->GetLength(), strControlID.GetLength() - pID->pStr->GetLength());
		Int32 lError;
		Int32 lMin = str.ParseToInt32(&lError);
		if (lError == 0)
			pID->lMinID = LMax(lMin + 1, pID->lMinID);
	}
}

/*********************************************************************\
	Function name    : CDialogItem::CreateControlID
	Description      : Creates a unique control ID
										 exception : static text boxes and group boxes get an IDC_STATIC
	Created at       : 04.09.01, @ 09:52:49
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
String CDialogItem::CreateControlID()
{
	String str;
	if (m_type == Dialog) str = "IDD_DIALOG";
	else if (m_type == Static) str = "IDC_STATIC";
	else if (m_type == Group) str = "IDC_STATIC";
	else if (m_type == CheckBox) str = "IDC_CHECK";
	else if (m_type == RadioBtn) str = "IDC_RADIO";
	else if (m_type == Button) str = "IDC_BUTTON";
	else if (m_type == ArrowBtn) str = "IDC_ARROW";
	else if (m_type == Color) str = "IDC_COLOR";
	else if (m_type == Separator) str = "";
	else if (m_type == ListBox) str = "IDC_LISTBOX";
	else if (m_type == ComboBox) str = "IDC_COMBO";
	else if (m_type == ComboButton) str = "IDC_COMBOBUTTON";
	else if (m_type == DialogGroup) str = "";
	else if (m_type == UserArea) str = "IDC_USER";
	else if (m_type == SubDialogControl) str = "IDC_SUBDLG";
	else if (m_type == EditBox) str = "IDC_EDIT";
	else if (m_type == Slider) str = "IDC_SLIDER";
	else if (m_type == CustomElement) str = "IDC_CUSTOM";

	if (str.Content() && str != "IDC_STATIC")
	{
		tagCreateControlID id;
		id.pStr = &str;
		id.lMinID = -1;
		m_pDocument->m_pTreeDlg->GetTreeView()->VisitChildren(CreateControlIDVisitCallback, nullptr, (void*)&id);

		if (id.lMinID == -1) id.lMinID = 1;
		str += String::IntToString(id.lMinID);
	}

	return str;
}

/*********************************************************************\
	Function name    : CDialogItem::AddString
	Description      : adds a string to the string table
	Created at       : 07.09.01, @ 10:31:32
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogItem::AddString()
{
	String str;

	if (m_type != Dialog && m_type != Static && m_type != Group &&
			m_type != CheckBox && m_type != RadioBtn && m_type != Button)
		return;

	if (m_strControlID.Content())
	{
		Int32 l;
		if (m_strControlID.FindFirst("IDC_", &l))
		{
			str = m_strControlID.SubStr(l + 4, m_strControlID.GetLength() - l - 4);
			str = "IDS_" + str;
		}
		else if (m_strControlID.FindFirst("IDD_", &l))
		{
			str = m_strControlID.SubStr(l + 4, m_strControlID.GetLength() - l - 4);
			str = "IDS_" + str;
		}
		else
			str = "IDS_" + m_strControlID;
	}
	else
	{
		if (m_type == Dialog) str = "IDS_DIALOG";
		else if (m_type == Static) str = "IDS_STATIC";
		else if (m_type == Group) str = "IDS_STATIC";
		else if (m_type == CheckBox) str = "IDS_CHECK";
		else if (m_type == RadioBtn) str = "IDS_RADIO";
		else if (m_type == Button) str = "IDS_BUTTON";
	}

	if (str.Content())
	{
		CStringTable* pTable = m_pDocument->GetStringTable();
		Int32 lElements = pTable->GetNumElements();
		Int32 lMinID = -1;
		Int32 a;
		str = TrimNumbers(str);

		for (a = 0; a < lElements; a++)
		{
			String strID = pTable->GetElementID(a);

			Int32 lPos;
			if (strID.FindFirst(str, &lPos) && lPos == 0)
			{
				String strNewID = strID.SubStr(str.GetLength(), strID.GetLength() - str.GetLength());
				Int32 lError;
				Int32 lMin = strNewID.ParseToInt32(&lError);
				if (lError == 0)
					lMinID = LMax(lMin + 1, lMinID);
				else
					lMinID = LMax(lMinID, 1);
			}
		}
		if (lMinID > -1)
			str += String::IntToString(lMinID);

		for (a = 0; a < pTable->GetUsedLanguages(); a++)
		{
			pTable->SetItemText(str, a, m_strName);
		}

		m_strNameID = str;
	}
}


/*********************************************************************\
	Function name    : CChildItems::CChildItems
	Description      :
	Created at       : 07.01.02, @ 20:39:12
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CChildItems::CChildItems()
{
	m_lElements = 0;
	m_pstrID = nullptr; m_pstrStringID = nullptr;
}

/*********************************************************************\
	Function name    : CChildItems::~CChildItems
	Description      :
	Created at       : 07.01.02, @ 20:39:20
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CChildItems::~CChildItems()
{
	Destroy();
}

/*********************************************************************\
	Function name    : CChildItems::Destroy
	Description      :
	Created at       : 07.01.02, @ 20:39:24
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CChildItems::Destroy()
{
	for (Int32 a = 0; a < m_lElements; a++)
	{
		DeleteObj(m_pstrID[a]);
		DeleteObj(m_pstrStringID[a]);
	}
	bDelete(m_pstrID);
	bDelete(m_pstrStringID);
	m_lElements = 0;
}

/*********************************************************************\
	Function name    : CChildItems::AddElement
	Description      :
	Created at       : 07.01.02, @ 20:39:29
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CChildItems::AddElement(String strID, String strStringID)
{
	String** ppNewID = bNewDeprecatedUseArraysInstead<String*>(m_lElements + 1);
	String** ppNewString = bNewDeprecatedUseArraysInstead<String*>(m_lElements + 1);

	ppNewID[m_lElements] = NewObjClear(String, strID);
	ppNewString[m_lElements] = NewObjClear(String, strStringID);

	CopyMemType(m_pstrID, ppNewID, m_lElements);
	CopyMemType(m_pstrStringID, ppNewString, m_lElements);

	bDelete(m_pstrID);
	bDelete(m_pstrStringID);

	m_pstrID = ppNewID;
	m_pstrStringID = ppNewString;

	m_lElements++;
}

/*********************************************************************\
	Function name    : CChildItems::GetElement
	Description      :
	Created at       : 07.01.02, @ 20:46:06
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CChildItems::GetElement(Int32 lID, String &strID, String &strStringID)
{
	if (lID < 0 || lID >= m_lElements) return false;

	strID = *m_pstrID[lID];
	strStringID = *m_pstrStringID[lID];

	return true;
}

/*********************************************************************\
	Function name    : CChildItems::SetElement
	Description      :
	Created at       : 07.01.02, @ 20:46:06
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CChildItems::SetElement(Int32 lID, String &strID, String &strStringID)
{
	if (lID < 0 || lID >= m_lElements) return false;

	*m_pstrID[lID] = strID;
	*m_pstrStringID[lID] = strStringID;

	return true;
}

/*********************************************************************\
	Function name    : CChildItems::FillEditBox
	Description      :
	Created at       : 07.01.02, @ 21:05:03
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CChildItems::FillEditBox(GeDialog* pDlg, Int32 lEditID)
{
	String str;
	String strID, strStringID;
	for (Int32 a = 0; a < m_lElements; a++)
	{
		str += *m_pstrID[a];
		str += ", ";
		str += *m_pstrStringID[a];
		str += ", ";

		if (a < m_lElements - 1)
			str += "\n";
	}
	pDlg->SetString(lEditID, str);
}

/*********************************************************************\
	Function name    : CChildItems::Write
	Description      :
	Created at       : 07.01.02, @ 21:11:33
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CChildItems::Write(BaseFile* pFile, String strFill)
{
	for (Int32 a = 0; a < m_lElements; a++)
	{
		WriteString(pFile, *m_pstrID[a]);
		WriteString(pFile, ", ");
		WriteString(pFile, *m_pstrStringID[a]);
		WriteString(pFile, "; ");
		if (a < m_lElements - 1)
			LineBreak(pFile, strFill + SPACE_NEW_LINE + SPACE_NEW_LINE);
		else
			LineBreak(pFile, strFill + SPACE_NEW_LINE);
	}
	return true;
}

/*********************************************************************\
	Function name    : CChildItems::Load
	Description      :
	Created at       : 07.01.02, @ 21:23:42
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CChildItems::Load(BaseContainer* pContainer)
{
	Destroy();

	Int32 lChildren = pContainer->GetInt32(0);
	for (Int32 a = 0; a < lChildren; a++)
		AddElement(pContainer->GetString(2 * a + 2), pContainer->GetString(2 * a + 3));
}

/*********************************************************************\
	Function name    : CChildItems::FromString
	Description      :
	Created at       : 07.01.02, @ 21:28:32
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CChildItems::FromString(String &str)
{
	// TODO
	Bool bIsID = true, bNext = true;
	String strID, strString, strFound;
	Int32 lPos;

	Destroy();

	while (bNext)
	{
		SkipChar(str, ' ');
		SkipChar(str, '\n');
		SkipChar(str, ' ');

		if (!str.FindFirst(',', &lPos))
		{
			strFound = str;
			bNext = false;
		}
		else
		{
			strFound = str.SubStr(0, lPos);
			str = str.SubStr(lPos + 1, str.GetLength() - lPos - 1);
		}

		if (bIsID)
		{
			strID = strFound;
		}
		else
		{
			//strStringID = strFound;
			AddElement(strID, strFound);
		}

		bIsID = !bIsID;
	}


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
