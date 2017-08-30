/*********************************************************************\
	File name        : DialogCustomElement.cpp
	Description      :
	Created at       : 26.03.02, @ 13:08:04
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"
#include "DialogItem.h"
#include "DialogItemSettings.h"
#include "shellsort.h"
#include "TreeDialog.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FIRST_CUSTOM_ELEMENT_ID       20000
#define IDC_CUSTOM_OPEN_CLOSE         99

/*********************************************************************\
	Function name    : CDialogCustomElement::CDialogCustomElement
	Description      :
	Created at       : 11.08.01, @ 17:19:05
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogCustomElement::CDialogCustomElement(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_CUSTOM_ELEMENT);
	m_strDescr = m_strName;
	m_type = CustomElement;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 0;
	m_lInitH = 0;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
	m_pbcGUI = nullptr;
	m_pData = nullptr;

	CreateElementList();
	if (g_pCustomElements)
	{
		m_lElement = (g_pCustomElements->Entries() > 0) ? 0 : -1;
		CCustomElements* pElement = g_pCustomElements->GetItem(m_lElement);
		m_strName += " [" + pElement->m_strName + "]";
		m_strDescr += " [" + pElement->m_strName + "]";
	}
	else
		m_lElement = -1;

	if (g_pCustomElements->Entries() > 0)
	{
		m_pbcGUI = bNewDeprecatedUseArraysInstead<BaseContainer>(g_pCustomElements->Entries());
		m_pData = bNewDeprecatedUseArraysInstead<TriState<GeData>>(g_pCustomElements->Entries());
	}
}

/*********************************************************************\
	Function name    : CDialogCustomElement::~CDialogCustomElement
	Description      :
	Created at       : 28.03.02, @ 14:59:05
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogCustomElement::~CDialogCustomElement()
{
	bDelete(m_pData);
	bDelete(m_pbcGUI);
}

/*********************************************************************\
	Function name    : CDialogCustomElement::CreateElementBegin
	Description      :
	Created at       : 27.03.02, @ 11:22:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogCustomElement::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	if (!m_pbcGUI) return;

	if (g_pCustomElements && m_lElement >= 0 && m_lElement < g_pCustomElements->Entries())
	{
		CCustomElements* pElement = g_pCustomElements->GetItem(m_lElement);

		if (pElement)
		{
			BaseContainer bc = m_pbcGUI[m_lElement];
			for (Int32 i = 0; pElement->m_pProp && pElement->m_pProp[i].type != CUSTOMTYPE_END; i++)
			{
				if (pElement->m_pProp[i].type == CUSTOMTYPE_STRING)
				{
					String str = m_pbcGUI[m_lElement].GetString(pElement->m_pProp[i].id);
					if (str.Content())
					{
						Bool b;
						String str1 = m_pDocument->GetString(str, b);
						if (!b) str1 = "[" + str + "]";
						bc.SetString(pElement->m_pProp[i].id, str1);
					}
				}
			}
			pDlg->AddCustomGui(lID, pElement->m_lID, m_strName, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH), bc);
			//BaseCustomGuiLib* pGUI = (BaseCustomGuiLib*)pDlg->FindCustomGui(lID, pElement->m_lID);
			//if (pGUI) pGUI->SetDefaultForResEdit();
		}
	}
}

/*********************************************************************\
	Function name    : CDialogCustomElement::InitValues
	Description      :
	Created at       : 14.04.02, @ 13:31:22
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogCustomElement::InitValues()
{
	if (g_pCustomElements && m_lElement >= 0 && m_lElement < g_pCustomElements->Entries())
	{
		CCustomElements* pElement = g_pCustomElements->GetItem(m_lElement);

		if (pElement)
		{
			Bool bSetDefault = false;
			_BaseCustomGui* pGUI = (_BaseCustomGui*)m_pDialog->FindCustomGui(m_lID, pElement->m_lID);
			if (pGUI)
			{
				if (pGUI->SupportLayoutSwitch(pElement->m_lID))
				{
					if (pElement->m_bIsOpen) pGUI->SetLayoutMode(pElement->m_lID, LAYOUTMODE_MAXIMIZED);
					else pGUI->SetLayoutMode(pElement->m_lID, LAYOUTMODE_MINIMIZED);
				}
				bSetDefault = true;
				if (m_lElement >= 0 && m_lElement < g_pCustomElements->Entries() && m_pData)
				{
					if (m_pData[m_lElement].GetValue().GetType() != DA_NIL)
						bSetDefault = !pGUI->SetData(pElement->m_lID, m_pData[m_lElement]);
				}
			}

			if (bSetDefault)
			{
				pGUI->SetDefaultForResEdit(pElement->m_lID);
			}
		}
	}
}

/*********************************************************************\
	Function name    : CDialogCustomElement::Load
	Description      :
	Created at       : 27.03.02, @ 14:54:02
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogCustomElement::Load(BaseContainer* bc)
{
	Int32 lPlugID = bc->GetInt32(DROLDWIN_SDK);
	BasePlugin *bs = (BasePlugin*)FindPlugin(lPlugID, PLUGINTYPE_CUSTOMGUI);

	CDialogItem::Load(bc);

	// check out, which element this is in our list
	if (!bs) return true;

	CCustomElements* pFirst = (CCustomElements*)g_pCustomElements->First();
	Int32 lElement = 0;
	Bool bFound = false;
	while (pFirst)
	{
		if (pFirst->m_lID == lPlugID) { bFound = true; break; }
		pFirst = (CCustomElements*)g_pCustomElements->Next(pFirst);
		lElement++;
	}
	if (!bFound) return true;

	m_lElement = lElement;
	BaseContainer* pBC = &m_pbcGUI[m_lElement];
	*pBC = *bc;

	if (bc->GetBool(DR_CUSTOMGUI_OPEN))
		pFirst->m_bIsOpen = true;

	CCustomElements* pElement = g_pCustomElements->GetItem(m_lElement);
	m_strName = GeLoadString(IDS_CUSTOM_ELEMENT);
	m_strName += " [" + pElement->m_strName + "]";
	m_strDescr = m_strName;

	return true;
}

/*********************************************************************\
	Function name    : CDialogCustomElement::Save
	Description      :
	Created at       : 27.03.02, @ 14:54:04
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogCustomElement::Save(BaseFile* pFile, String strFill)
{
	if (g_pCustomElements && m_lElement >= 0 && m_lElement < g_pCustomElements->Entries())
	{
		CustomProperty* pProp;

		CCustomElements* pElement = g_pCustomElements->GetItem(m_lElement);
		if (!pElement) return true;
		if (!m_pbcGUI) return false;
		BaseContainer* pBC = &m_pbcGUI[m_lElement];

		pProp = pElement->m_pProp;

		WriteString(pFile, pElement->m_pChResSym);
		WriteString(pFile, " ");
		WriteString(pFile, m_strControlID);
		LineBreak(pFile, strFill);

		WriteString(pFile, "{");
		LineBreak(pFile, strFill + *g_pstrFillSave);
		SaveAlignment(pFile);
		LineBreak(pFile, strFill);

		if (pElement->m_bIsOpen)
		{
			WriteString(pFile, *g_pstrFillSave);
			WriteString(pFile, "OPEN");
			WriteString(pFile, "; ");
			LineBreak(pFile, strFill);
		}

		Int32 i;
		for (i = 0; pProp && pProp[i].type != CUSTOMTYPE_END; i++)
		{
			Bool b = false;
			if (pProp[i].type == CUSTOMTYPE_FLAG)
			{
				if (pBC->GetBool(pProp[i].id))
				{
					WriteString(pFile, *g_pstrFillSave);
					WriteString(pFile, pProp[i].ident);
					WriteString(pFile, "; ");
					b = true;
				}
			}
			else if (pProp[i].type == CUSTOMTYPE_LONG)
			{
				WriteString(pFile, *g_pstrFillSave);
				WriteString(pFile, pProp[i].ident);
				WriteString(pFile, " ");
				WriteString(pFile, String::IntToString(pBC->GetInt32(pProp[i].id)));
				WriteString(pFile, "; ");
				b = true;
			}
			else if (pProp[i].type == CUSTOMTYPE_REAL)
			{
				WriteString(pFile, *g_pstrFillSave);
				WriteString(pFile, pProp[i].ident);
				WriteString(pFile, " ");
				WriteString(pFile, String::FloatToString(pBC->GetFloat(pProp[i].id)));
				WriteString(pFile, "; ");
				b = true;
			}
			else if (pProp[i].type == CUSTOMTYPE_STRING)
			{
				if (pBC->GetString(pProp[i].id).Content())
				{
					WriteString(pFile, *g_pstrFillSave);
					WriteString(pFile, pProp[i].ident);
					WriteString(pFile, " ");
					WriteString(pFile, pBC->GetString(pProp[i].id));
					WriteString(pFile, "; ");
					b = true;
				}
			}
			else if (pProp[i].type == CUSTOMTYPE_VECTOR)
			{
				WriteString(pFile, *g_pstrFillSave);
				WriteString(pFile, pProp[i].ident);
				WriteString(pFile, " ");
				WriteString(pFile, String::FloatToString(pBC->GetVector(pProp[i].id).x)); WriteString(pFile, " ");
				WriteString(pFile, String::FloatToString(pBC->GetVector(pProp[i].id).y)); WriteString(pFile, " ");
				WriteString(pFile, String::FloatToString(pBC->GetVector(pProp[i].id).z)); WriteString(pFile, " ");
				WriteString(pFile, "; ");
				b = true;
			}

			if (b)
				LineBreak(pFile, strFill);
		}
		WriteString(pFile, "}");
	}

	return true;
}

/*********************************************************************\
	Function name    : CDialogCustomElement::GetElementData
	Description      :
	Created at       : 02.06.02, @ 22:35:22
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 CDialogCustomElement::GetElementPluginID()
{
	if (g_pCustomElements && m_lElement >= 0 && m_lElement < g_pCustomElements->Entries())
	{
		CCustomElements* pElement = g_pCustomElements->GetItem(m_lElement);
		if (!pElement) return -1;

		return pElement->m_lID;
	}
	return -1;
}

/*********************************************************************\
	Function name    : CDialogCustomElement::ResetData
	Description      :
	Created at       : 02.06.02, @ 23:11:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogCustomElement::ResetData(Bool bResetAll)
{
	if (!m_pData) return;
	for (Int32 a = 0; a < g_pCustomElements->Entries(); a++)
	{
		if (bResetAll || (a == m_lElement))
		{
			m_pData[a] = TriState<GeData>();
		}
	}
}





CCustomElementSettings::CCustomElementSettings()
{
	m_lTabPageID = IDC_CUSTOM_ELEMENT_SETTINGS_TAB;
	m_lLastID = -1;
}

CCustomElementSettings::~CCustomElementSettings()
{

}

/*********************************************************************\
	Function name    : CCustomElementSettings::Init
	Description      :
	Created at       : 27.03.02, @ 10:35:40
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CCustomElementSettings::Init()
{
	m_wndListView.AttachListView(m_pSettingsDialog, IDC_CUSTOM_GUI_LIST);

	BaseContainer bc;
	bc.SetInt32('name', LV_COLUMN_TEXT);
	bc.SetInt32('resy', LV_COLUMN_TEXT);
	bc.SetInt32('ceid', LV_COLUMN_TEXT);
	m_wndListView.SetLayout(3, bc);
}

/*********************************************************************\
	Function name    : CCustomElementSettings::SetData
	Description      :
	Created at       : 11.08.01, @ 23:18:59
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CCustomElementSettings::SetData()
{
	_Init();

	CreateElementList();

	// fill the list view with the elements
	Int32 a = 0;
	while (m_wndListView.RemoveItem(a)) { a++; }


	if (!g_pCustomElements) return;
	Int32 lElementCount = g_pCustomElements->Entries();
	if (lElementCount == 0) return;

	CCustomElements* pFirst = g_pCustomElements->First();
	a = 0;

	while (pFirst)
	{
		BaseContainer data;
		data.SetString('name', pFirst->m_strName);
		data.SetString('resy', pFirst->m_pChResSym);
		data.SetString('ceid', String::IntToString(pFirst->m_lID));

		m_wndListView.SetItem(a, data);
		a++;
		pFirst = g_pCustomElements->Next(pFirst);
	}

	m_wndListView.DataChanged();

	AutoAlloc<BaseSelect> pSel;
	if (pSel)
	{
		pSel->Select(m_pElement->m_lElement);
		m_lLastID = -1;
		m_wndListView.SetSelection(pSel);
		m_wndListView.DataChanged();
	}

	// write the custom object properties
	if (g_pCustomElements && m_pElement->m_lElement >= 0 && m_pElement->m_lElement < g_pCustomElements->Entries())
	{
		CCustomElements* pElement = g_pCustomElements->GetItem(m_pElement->m_lElement);
		BasePlugin* pPlug = nullptr;
		if (pElement) pPlug = pElement->m_pPlug;
		if (pPlug)
		{
			CustomProperty *prop = CallCustomGuiCmd(pPlug, GetProperties)();
			m_wndSubDlg.Refresh(m_pSettingsDialog, prop, m_pElement);
		}
	}
}

/*********************************************************************\
	Function name    : CCustomElementSettings::Command
	Description      :
	Created at       : 26.03.02, @ 13:18:45
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CCustomElementSettings::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDC_CUSTOM_GUI_LIST:
		{
			Int32 lItemID = msg.GetInt32(LV_SIMPLE_ITEM_ID);
			AutoAlloc<BaseSelect> pSel;
			m_wndListView.GetSelection(pSel);
			if (!pSel) break;
			if (!pSel->IsSelected(lItemID)) break;

			if (m_lLastID != lItemID)
			{
				m_pElement->m_lElement = lItemID;
				m_lLastID = lItemID;

				CCustomElements* pElement = g_pCustomElements->GetItem(m_pElement->m_lElement);
				if (pElement)
				{
					m_pElement->m_strName = GeLoadString(IDS_CUSTOM_ELEMENT) + " [" + pElement->m_strName + "]";;
					m_pElement->SetDescr(m_pElement->m_strName);
					m_wndSubDlg.Refresh(m_pSettingsDialog, pElement->m_pProp, m_pElement);
				}
				m_pElement->ItemChanged();
				m_pElement->GetDocument()->UpdateAllViews(true, false);
			}
		}
		break;
	}
	return true;
}


/*********************************************************************\
	Function name    : CCustomSubDialog::Refresh
	Description      :
	Created at       : 27.03.02, @ 12:02:50
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CCustomSubDialog::Refresh(GeDialog* pDlg, CustomProperty *prop, CDialogCustomElement* pElement)
{
	m_pDialog = pDlg;
	m_pProp = prop;
	m_pElement = pElement;

	pDlg->AttachSubDialog(this, IDC_CUSTOM_ELEMENT_SUBDLG);
	pDlg->LayoutChanged(IDC_CUSTOM_ELEMENT_SUBDLG);
}

/*********************************************************************\
	Function name    : CCustomSubDialog::CreateLayout
	Description      :
	Created at       : 27.03.02, @ 12:04:28
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CCustomSubDialog::CreateLayout(void)
{
	Int32 i;
	GroupBegin(100, BFH_SCALEFIT | BFV_SCALEFIT, 2, 0, "", 0);
	Int32 lID = FIRST_CUSTOM_ELEMENT_ID;

	if (!m_pProp || m_pProp[0].type == CUSTOMTYPE_END)
	{
		GroupBegin(100, BFH_SCALEFIT, 1, 0, "", 0);
	}

	AddCheckbox(IDC_CUSTOM_OPEN_CLOSE, BFH_LEFT, 0, 0, "Open");
	Bool b = false;
	if (m_pElement)
	{
		CCustomElements* pElement = g_pCustomElements->GetItem(m_pElement->m_lElement);
		if (pElement)
		{
			BasePlugin *bs = (BasePlugin*)FindPlugin(pElement->m_lID, PLUGINTYPE_CUSTOMGUI);
			if (bs)
			{
				b = true;
				if (!(bs->GetInfo() & CUSTOMGUI_SUPPORT_LAYOUTSWITCH))
					Enable(IDC_CUSTOM_OPEN_CLOSE, false);
				else
					Enable(IDC_CUSTOM_OPEN_CLOSE, true);
			}
		}
	}
	if (!b)
		Enable(IDC_CUSTOM_OPEN_CLOSE, true);

	if (!m_pProp || m_pProp[0].type == CUSTOMTYPE_END)
	{
		AddStaticText(100, BFH_LEFT | BFV_CENTER, 0, 0, GeLoadString(IDS_CUSTOM_NO_PROPS), 0);
		GroupEnd();
	}

	AddStaticText(100, 0, 0, 0, "", 0);

	for (i = 0; m_pProp && m_pProp[i].type != CUSTOMTYPE_END; i++, lID++)
	{
		AddStaticText(100, BFH_LEFT | BFV_CENTER, 0, 0, m_pProp[i].ident, 0);
		switch (m_pProp[i].type)
		{
		case CUSTOMTYPE_FLAG:
			AddCheckbox(lID, BFH_LEFT | BFV_CENTER, 0, 0, "");
			break;
		case CUSTOMTYPE_LONG:
			AddEditNumber(lID, BFH_LEFT | BFV_CENTER);
			break;
		case CUSTOMTYPE_REAL:
			AddEditNumber(lID, BFH_LEFT | BFV_CENTER);
			break;
		case CUSTOMTYPE_STRING:
			AddEditText(lID, BFH_SCALEFIT | BFV_CENTER);
			break;
		case CUSTOMTYPE_VECTOR:
			AddEditNumber(lID++, BFH_LEFT | BFV_CENTER);
			AddEditNumber(lID++, BFH_LEFT | BFV_CENTER);
			AddEditNumber(lID, BFH_LEFT | BFV_CENTER);
			break;

		default:
			AddStaticText(lID, BFH_LEFT | BFV_CENTER, 0, 0, "wird auch noch ;-)", 0);
			break;
		}
	}
	/*if (!m_pProp || i == 0)
		AddStaticText(100, BFH_LEFT | BFV_CENTER, 0, 0, GeLoadString(IDS_CUSTOM_NO_PROPS), 0);*/
	return true;
}

/*********************************************************************\
	Function name    : CCustomSubDialog::InitValues
	Description      :
	Created at       : 27.03.02, @ 12:07:33
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CCustomSubDialog::InitValues(void)
{
	Int32 lID = FIRST_CUSTOM_ELEMENT_ID;
	Int32 i;

	CCustomElements* pElement = g_pCustomElements->GetItem(m_pElement->m_lElement);
	if (!pElement) return true;

	if (!m_pElement->m_pbcGUI) return true;
	BaseContainer* pBC = &m_pElement->m_pbcGUI[m_pElement->m_lElement];

	SetBool(IDC_CUSTOM_OPEN_CLOSE, pElement->m_bIsOpen);

	for (i = 0; m_pProp && m_pProp[i].type != CUSTOMTYPE_END; i++, lID++)
	{
		CustomProperty* pProp = &pElement->m_pProp[i];

		if (pProp->type == CUSTOMTYPE_FLAG) SetBool(lID, pBC, pProp->id);
		else if (pProp->type == CUSTOMTYPE_LONG) SetInt32(lID, pBC, pProp->id);
		else if (pProp->type == CUSTOMTYPE_REAL) SetFloat(lID, pBC, pProp->id);
		else if (pProp->type == CUSTOMTYPE_STRING) SetString(lID, pBC, pProp->id);
		else if (pProp->type == CUSTOMTYPE_VECTOR)
		{
			Vector v = pBC->GetVector(pProp->id);
			SetFloat(lID++,v.x);
			SetFloat(lID++,v.y);
			SetFloat(lID,v.z);
		}
	}

	return true;
}

/*********************************************************************\
	Function name    : CCustomSubDialog::Command
	Description      :
	Created at       : 27.03.02, @ 12:41:37
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CCustomSubDialog::Command(Int32 id,const BaseContainer &msg)
{
	if (!m_pElement) return true;

	CCustomElements* pElement = g_pCustomElements->GetItem(m_pElement->m_lElement);
	if (!pElement) return true;

	if (!m_pElement->m_pbcGUI) return true;
	BaseContainer* pBC = &m_pElement->m_pbcGUI[m_pElement->m_lElement];

	if (id == IDC_CUSTOM_OPEN_CLOSE)
	{
		GetBool(IDC_CUSTOM_OPEN_CLOSE, pElement->m_bIsOpen);
		m_pElement->ItemChanged();
		return true;
	}

	if (id >= FIRST_CUSTOM_ELEMENT_ID)
	{
		// find the correct property
		Int32 lID = FIRST_CUSTOM_ELEMENT_ID;
		Int32 s=0;
		CustomProperty* pProp = nullptr;
		for (Int32 i = 0; m_pProp && m_pProp[i].type != CUSTOMTYPE_END; i++)
		{
			switch (pElement->m_pProp[id - FIRST_CUSTOM_ELEMENT_ID].type)
			{
			case CUSTOMTYPE_FLAG:
			case CUSTOMTYPE_LONG:
			case CUSTOMTYPE_REAL:
			case CUSTOMTYPE_STRING:
				s = 1; break;
			case CUSTOMTYPE_VECTOR:
				s = 3; break;
			}
			if (lID <= id && id < lID + s)
			{
				pProp = &pElement->m_pProp[i];
				break;
			}
			lID += s;
		}
		if (pProp)
		{
			if (pProp->type == CUSTOMTYPE_FLAG) GetBool(lID, pBC, pProp->id);
			else if (pProp->type == CUSTOMTYPE_LONG) GetInt32(lID, pBC, pProp->id);
			else if (pProp->type == CUSTOMTYPE_REAL) GetFloat(lID, pBC, pProp->id);
			else if (pProp->type == CUSTOMTYPE_STRING) GetString(lID, pBC, pProp->id);
			else if (pProp->type == CUSTOMTYPE_VECTOR) GetVector(lID, lID + 1, lID + 2, pBC, pProp->id);
		}

		m_pElement->ItemChanged();
	}
	return true;
}
