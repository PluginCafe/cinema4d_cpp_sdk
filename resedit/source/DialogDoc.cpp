/*********************************************************************\
	File name        : DialogDoc.cpp
	Description      : Implementation of the class CDialogDoc
	Created at       : 11.08.01, @ 11:43:23
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"

#include "DialogItem.h"
#include "DialogPreview.h"
#include "ItemSettingsDialog.h"
#include "TreeDialog.h"
#include "ResEditToolBar.h"
#include "StringTableTranslateDialog.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CDialogPreview* g_pDlgPreview;
extern CItemSettingsDialog* g_pSettingsDlg;
extern CTreeDialog* g_pTreeDialog;
extern CDialogDoc* g_pDialogDoc;
extern CResEditToolBar* g_pToolbar;
extern CStringTranslateDialog* g_pStringTransDialog;

extern Bool g_bWasActive;

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CDialogDoc::CDialogDoc()
{
	m_pDlgPreview = nullptr;
	m_pItemSettingsDlg = nullptr;
	m_pTreeDlg = nullptr;
	m_pToolbar = nullptr;
	m_bChanged = false;
	m_pDialog = nullptr;
	m_bIsLoading = false;
	m_pCurrentItem = nullptr;

	m_lLanguageIDTable = -1;
	m_lLanguageIDList = -1;

	m_bStringSymbolsChanged = m_bControlSymbolsChanged = false;
}

CDialogDoc::~CDialogDoc()
{
	// DeleteObj(m_pDialog); // don't delete the dialog since it gets
												 // destroyed when the tree is destroyed
}

/*********************************************************************\
	Function name    : CDialogDoc::New
	Description      :
	Created at       : 11.08.01, @ 11:37:27
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::OnFileNew()
{
	if (m_bChanged)
	{
		GEMB_R lSave = AskSave();
		if (lSave == GEMB_R_CANCEL) return;
		if (lSave == GEMB_R_YES)
		{
			if (!OnFileSave()) return;
		}
	}

	//m_pDialog = nullptr;
	m_StringTable.Free();
	m_lLanguageIDTable = m_StringTable.AddLanguage("us");
	m_lLanguageIDList = g_LanguageList.GetLanguageID("us");
	m_pTreeDlg->GetTreeView()->Reset();
	m_pTreeDlg->SelectLanguage(m_lLanguageIDList);
	m_pDialog = (CDialogTemplate*)AddItem(Dialog, true, true, false);

	m_FileName = String("");
	m_fnStringTableStart = Filename();

	m_bChanged = false;
	m_pItemSettingsDlg->SetSelItem((CDialogItem*)m_pDialog);
	UpdateDialogCaptions();
	UpdateAllViews(true);

	m_bStringSymbolsChanged = m_bControlSymbolsChanged = false;
}

/*********************************************************************\
	Function name    : CDialogDoc::Open
	Description      :
	Created at       : 11.08.01, @ 11:37:35
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::OnFileOpen()
{
	if (m_bChanged)
	{
		GEMB_R lSave = AskSave();
		if (lSave == GEMB_R_CANCEL) return;
		if (lSave == GEMB_R_YES)
		{
			if (!OnFileSave()) return;
		}
	}

	Bool bChangeOld = m_bChanged;
	m_bChanged = false;
	Filename fn;
	if (m_FileName.Content())
		fn = m_FileName;
	else
		fn = *g_pLastOpenFile;//GeGetStartupPath();

	Bool b = fn.FileSelect(FILESELECTTYPE_ANYTHING,FILESELECT_LOAD,String());
	if (!b) { m_bChanged = bChangeOld; return; }

	OnFileNew();

	m_FileName = fn;
	*g_pLastOpenFile = fn;

	m_pTreeDlg->GetTreeView()->Reset();
	m_pDialog = nullptr;

	// now, load the resource and add the dialog items, but not here :-)
	m_pTreeDlg->GetTreeView()->LockWindowUpdate();
	LoadDialogResource(fn);
	m_pTreeDlg->GetTreeView()->UnlockWindowUpdate();
	m_pTreeDlg->GetTreeView()->ExpandAllItems(false);
	SelChanged(m_pDialog);
	m_pTreeDlg->UpdateLayout();

	//m_StringTable.Dump();

	m_bChanged = false;
	UpdateDialogCaptions();
	UpdateAllViews(true);

	m_bStringSymbolsChanged = m_bControlSymbolsChanged = false;
}

/*********************************************************************\
	Function name    : CDialogDoc::Save
	Description      :
	Created at       : 11.08.01, @ 11:37:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::OnFileSave()
{
	if (!m_FileName.Content()) return OnFileSaveAs();

	if (!CheckFilename(m_FileName)) return false;
	*g_pLastOpenFile = m_FileName;

	UpdateAllViews(true);
	SelChanged(m_pCurrentItem);
	StoreNamesInStringtable();
	SaveDialogResource(m_FileName);
	m_bChanged = false;
	UpdateDialogCaptions();

	m_bStringSymbolsChanged = m_bControlSymbolsChanged = false;

	return true;
}

/*********************************************************************\
	Function name    : CDialogDoc::SaveAs
	Description      :
	Created at       : 11.08.01, @ 11:37:44
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::OnFileSaveAs()
{
	Filename fn = m_FileName;
	if (!m_FileName.Content())
	{
		fn = Filename(m_pDialog->m_strControlID + ".res");
	}

	Bool b = fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_SAVE, String());

	if (!b) return false;

	fn.SetSuffix("res");
	if (!CheckFilename(fn)) return false;

	if (m_FileName != fn)
		m_fnStringTableStart = Filename();

	m_FileName = fn;
	*g_pLastOpenFile = m_FileName;
	UpdateAllViews(true);
	SelChanged(m_pCurrentItem);
	StoreNamesInStringtable();
	SaveDialogResource(m_FileName);

	m_bChanged = false;
	UpdateDialogCaptions();

	m_bStringSymbolsChanged = m_bControlSymbolsChanged = false;

	return true;
}

/*********************************************************************\
	Function name    : CDialogDoc::UpdateDialogCaptions
	Description      :
	Created at       : 11.08.01, @ 11:37:48
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
String CDialogDoc::UpdateDialogCaptions()
{
	String strFile;
	if (!m_FileName.Content())
		strFile = GeLoadString(IDS_UNTITLED);
	else
		strFile = m_FileName.GetFileString();

	if (m_bChanged) strFile += " *";

	if (m_pDlgPreview) m_pDlgPreview->SetTitle(GeLoadString(IDS_RES_EDIT_PREVIEW_CAPT) + " - " + strFile);
	if (m_pItemSettingsDlg) m_pItemSettingsDlg->SetTitle(GeLoadString(IDS_RES_EDIT_ITEM_SETTINGS_CAPT) + " - " + strFile);
	if (m_pTreeDlg) m_pTreeDlg->SetTitle(GeLoadString(IDS_RES_EDIT_TREEVIEW_CAPT) + " - " + strFile);

	return strFile;
}

/*********************************************************************\
	Funktionsname    : CDialogDoc::SetChanged
	Beschreibung     :
	R�ckgabewert     : void
	Erstellt am      : 11.08.01, @ 11:40:24
	Argument         : Bool bChanged
\*********************************************************************/
void CDialogDoc::SetChanged(Bool bChanged)
{
	if (m_bChanged == bChanged) return;

	m_bChanged = bChanged;
	UpdateDialogCaptions();
	UpdateAllViews();
}

/*********************************************************************\
	Function name    : CDialogDoc::AskSave
	Description      : asks if the user wants to save changes
										 return true if
	Created at       : 11.08.01, @ 11:48:14
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
GEMB_R CDialogDoc::AskSave()
{
	String strText;
	if (!m_FileName.Content())
		strText = GeLoadString(IDS_UNTITLED);
	else
		strText = m_FileName.GetFileString();

	strText += GeLoadString(IDS_WAS_CHANGED);

	return QuestionDialogYesNoCancel(strText);
}

/*********************************************************************\
	Function name    : UpdateTreeViewCallback
	Description      :
	Created at       : 27.03.02, @ 10:55:41
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void UpdateTreeViewCallback(TreeViewItem* pItem, void* pData)
{
	BaseContainer* bc = pItem->GetData();
	CDialogItem* pDlgItem = (CDialogItem*)(bc->GetVoid(TREEVIEW_DATA));
	if (pDlgItem)
	{
		if (!pDlgItem->m_strName.Content())
			bc->SetString(TREEVIEW_TEXT, pDlgItem->GetDescr());
		else
			bc->SetString(TREEVIEW_TEXT, pDlgItem->m_strName);
	}
}

/*********************************************************************\
	Funktionsname    : CDialogDoc::UpdateAllViews
	Beschreibung     :
	R�ckgabewert     : void
	Erstellt am      : 11.08.01, @ 15:07:09
\*********************************************************************/
void CDialogDoc::UpdateAllViews(Bool bUpdateTree, Bool bUpdatePreview)
{
	if (m_pDlgPreview && bUpdatePreview) m_pDlgPreview->Update();
	if (m_pItemSettingsDlg) m_pItemSettingsDlg->Update();
	if (m_pTreeDlg && bUpdateTree)
	{
		// maybe the name of a control was changed, update it
		m_pTreeDlg->GetTreeView()->VisitChildren(UpdateTreeViewCallback);

		/*CDoubleLinkedList <VisibleTreeItem>* pItems = m_pTreeDlg->GetTreeView()->GetVisibleItemList();
		VisibleTreeItem* pItem = pItems->First();
		while (pItem)
		{
			BaseContainer* pData = pItem->m_pItem->GetData();
			CDialogItem* pDlgItem = (CDialogItem*)(pData->GetInt32(TREEVIEW_DATA));
			if (pDlgItem)
			{
				if (!pDlgItem->m_strName.Content())
					pData->SetString(TREEVIEW_TEXT, pDlgItem->GetDescr());
				else
					pData->SetString(TREEVIEW_TEXT, pDlgItem->m_strName);
			}
			pItem = pItems->Next(pItem);
		};*/
		m_pTreeDlg->Update();
	}
}

/*********************************************************************\
	Function name    : CDialogDoc::AddItem
	Description      : if bAddAsChild == true, the item will be added as a
										 child of the new item, else it will be a brother
	Created at       : 11.08.01, @ 16:05:09
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogItem* CDialogDoc::AddItem(ItemType t, Bool bAddAsChild, Bool bAddText, Bool bMakeVisible)
{
	TreeViewItem* pTreeItem = m_pTreeDlg->GetSelectedItem();

	if (!pTreeItem)
	{
		pTreeItem = m_pTreeDlg->GetTreeView()->GetRootItem()->GetDown();
		if (!pTreeItem) return nullptr;
	}

	CDialogItem* pDlgItem = (CDialogItem*)pTreeItem->GetData()->GetVoid(TREEVIEW_DATA);

	CDialogItem* pNewItem = nullptr;
	if (t == Dialog) { pNewItem = (CDialogItem*)(NewObjClear(CDialogTemplate, this)); m_pDialog = (CDialogTemplate*)pNewItem; }
	else if (t == Static) pNewItem = (CDialogItem*)(NewObjClear(CDialogStatic, this));
	else if (t == Group) pNewItem = (CDialogItem*)(NewObjClear(CDialogGroup, this));
	else if (t == CheckBox) pNewItem = (CDialogItem*)(NewObjClear(CDialogCheckBox, this));
	else if (t == RadioBtn) pNewItem = (CDialogItem*)(NewObjClear(CDialogRadioBtn, this));
	else if (t == Button) pNewItem = (CDialogItem*)(NewObjClear(CDialogButton, this));
	else if (t == EditBox) pNewItem = (CDialogItem*)(NewObjClear(CDialogEditBox, this));
	else if (t == Color) pNewItem = (CDialogItem*)(NewObjClear(CDialogColor, this));
	else if (t == ArrowBtn) pNewItem = (CDialogItem*)(NewObjClear(CDialogArrowBtn, this));
	else if (t == Slider) pNewItem = (CDialogItem*)(NewObjClear(CDialogSlider, this));
	else if (t == Separator) pNewItem = (CDialogItem*)(NewObjClear(CDialogSeparator, this));
	else if (t == ListBox) pNewItem = (CDialogItem*)(NewObjClear(CDialogListBox, this));
	else if (t == ComboBox) pNewItem = (CDialogItem*)(NewObjClear(CDialogComboBox, this));
	else if (t == ComboButton) pNewItem = (CDialogItem*)(NewObjClear(CDialogComboButton, this));
	else if (t == DialogGroup) pNewItem = (CDialogItem*)(NewObjClear(CDialogDlgGroup, this));
	else if (t == UserArea) pNewItem = (CDialogItem*)(NewObjClear(CDialogUserArea, this));
	else if (t == SubDialogControl) pNewItem = (CDialogItem*)(NewObjClear(CDialogSubDlg, this));
	else if (t == CustomElement) pNewItem = (CDialogItem*)(NewObjClear(CDialogCustomElement, this));
	if (!pNewItem) return nullptr;

	pNewItem->m_pDocument = this;
	if (bAddText)
		pNewItem->AddString();

	if (pNewItem->GetControlID() != "IDC_STATIC" && pNewItem->GetControlID().Content())
		m_bControlSymbolsChanged = true;
	if (pNewItem->GetNameID().Content())
		m_bStringSymbolsChanged = true;

	if (bAddAsChild || (!bAddAsChild && pDlgItem->GetType() == Dialog))
		m_pTreeDlg->AddItem(pNewItem);
	else
	{
		m_pTreeDlg->m_wndTree.SelectItem(pDlgItem->m_pTreeViewItem->GetParent());
		m_pTreeDlg->AddItem(pNewItem, pDlgItem);
	}

	if (m_pDlgPreview && !m_bIsLoading) m_pDlgPreview->Update();
	if (m_pItemSettingsDlg && !m_bIsLoading) m_pItemSettingsDlg->Update();
	SelChanged(pNewItem, true, bMakeVisible);
	SetChanged();

	return pNewItem;
}

/*********************************************************************\
	Function name    : CDialogDoc::SelChanged
	Description      : Updates the settings dialog
	Created at       : 11.08.01, @ 19:07:24
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::SelChanged(CDialogItem* pItem, Bool bUpdateTree, Bool bMakeVisible)
{
	if (!pItem) return;

	m_pCurrentItem = pItem;
	if (m_pItemSettingsDlg && !m_bIsLoading) m_pItemSettingsDlg->SetSelItem(pItem);
	if (m_pTreeDlg && bUpdateTree)
	{
		m_pTreeDlg->SetSelItem(pItem, bMakeVisible);
		m_pTreeDlg->Update();
	}
	m_pDlgPreview->MakeVisible(pItem);
}

/*********************************************************************\
	Function name    : CDialogDoc::UpdatePreview
	Description      :
	Created at       : 11.08.01, @ 20:30:49
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::UpdatePreview()
{
	if (m_pDlgPreview) m_pDlgPreview->Update();
}

/*********************************************************************\
	Function name    : CDialogDoc::CreateElements
	Description      :
	Created at       : 15.08.01, @ 10:09:52
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::CreateElements(Int32 lID, GeDialog* pDlg)
{
	TreeView* pTree = m_pTreeDlg->GetTreeView();
	TreeViewItem* pRoot = pTree->GetRootItem();
	if (pRoot->GetChildCount() < 1) return;

	TreeViewItem* pDialog = pRoot->GetChild(0);
	CreateElements(lID, pDialog, pDlg);
}

/*********************************************************************\
	Function name    : CDialogDoc::CreateElements
	Description      :
	Created at       : 11.08.01, @ 20:36:44
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::CreateElements(Int32 &lID, TreeViewItem* pItem, GeDialog* pDlg)
{
	Int32 lElementID = lID++;
	CDialogItem* pDlgItem = (CDialogItem*)pItem->GetData()->GetVoid(TREEVIEW_DATA);
	if (!pDlgItem) return;

	pDlgItem->m_lID = lElementID;
	pDlgItem->m_pDialog = pDlg;

	pDlgItem->CreateElementBegin(lElementID, pDlg);
	Int32 lChildren = pItem->GetChildCount();
	for (Int32 a = 0; a < lChildren; a++)
	{
		CreateElements(lID, pItem->GetChild(a), pDlg);
	}
	pDlgItem->CreateElementEnd(lElementID, pDlg);
}

/*********************************************************************\
	Function name    : CDialogDoc::InitValues
	Description      :
	Created at       : 27.04.02, @ 16:43:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::InitValues(TreeViewItem* pItem)
{
	if (!pItem)
	{
		TreeView* pTree = m_pTreeDlg->GetTreeView();
		TreeViewItem* pRoot = pTree->GetRootItem();
		if (pRoot->GetChildCount() < 1) return;

		pItem = pRoot->GetChild(0);
	}

	CDialogItem* pDlgItem = (CDialogItem*)pItem->GetData()->GetVoid(TREEVIEW_DATA);
	if (!pDlgItem) return;

	Int32 lChildren = pItem->GetChildCount();
	pDlgItem->InitValues();
	for (Int32 a = 0; a < lChildren; a++)
	{
		InitValues(pItem->GetChild(a));
	}
}

/*********************************************************************\
	Function name    : CDialogDoc::SelectItem
	Description      : searches for the item with lID and selects it
	Created at       : 11.08.01, @ 21:31:52
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::SelectItem(Int32 lID)
{
	CDialogItem* pItem = SearchItem(m_pTreeDlg->GetTreeView()->GetRootItem(), lID);
	if (pItem)
		SelChanged(pItem);
}

/*********************************************************************\
	Function name    : CDialogDoc::SearchItem
	Description      :
	Created at       : 15.08.01, @ 10:16:34
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogItem* CDialogDoc::SearchItem(TreeViewItem* pItem, Int32 lID)
{
	CDialogItem* pDlgItem = (CDialogItem*)(pItem->GetData()->GetVoid(TREEVIEW_DATA));
	if (pDlgItem && pDlgItem->m_lID == lID) return pDlgItem;

	Int32 lChildren = pItem->GetChildCount();
	for (Int32 a = 0; a < lChildren; a++)
	{
		CDialogItem* pFound = SearchItem(pItem->GetChild(a), lID);
		if (pFound) return pFound;
	}

	return nullptr;
}

/*********************************************************************\
	Function name    : StoreInStringtableCallback
	Description      :
	Created at       : 23.08.01, @ 17:48:17
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void StoreInStringtableCallback(TreeViewItem* pItem, void* pData)
{
	CDialogItem* pDlgItem = (CDialogItem*)pItem->GetData()->GetVoid(TREEVIEW_DATA);
	if (!pDlgItem) return;
	CDialogDoc* pDoc = (CDialogDoc*)pData;

	String strID = pDlgItem->GetNameID();
	if (strID.Content())
		pDoc->SetStringtableItem(strID, pDlgItem->m_strName);
}

/*********************************************************************\
	Function name    : CDialogDoc::StoreNamesInStringtable
	Description      : Since the string table is not changed when editing an item's name
										 we need to call this function to stores the item's names back
										 in the stringtable.
	Created at       : 23.08.01, @ 17:47:37
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::StoreNamesInStringtable()
{
	m_pTreeDlg->GetTreeView()->VisitChildren(StoreInStringtableCallback, nullptr, (void*)this);
}

/*********************************************************************\
	Function name    : CDialogDoc::ChangeLanguage
	Description      : Changes the language of the dialog
	Created at       : 21.08.01, @ 18:28:34
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::ChangeLanguage(String strSuffix, Bool bGlobalTable)
{
	Int32 lNewLang = g_LanguageList.GetLanguageID(strSuffix);
	if (lNewLang == m_lLanguageIDList) return;

	if (bGlobalTable) return;

	// first, store all modifications at the item's names back to the string table
	StoreNamesInStringtable();

	// and then change the language
	m_lLanguageIDList = lNewLang;
	m_lLanguageIDTable = m_StringTable.GetLanguageID(strSuffix);
	LanguageChanged();
}

/*********************************************************************\
	Function name    : ChangeLanguageCallback
	Description      :
	Created at       : 22.08.01, @ 11:39:08
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void ChangeLanguageCallback(TreeViewItem* pItem, void* pData)
{
	CDialogItem* pDlgItem = (CDialogItem*)pItem->GetData()->GetVoid(TREEVIEW_DATA);
	if (!pDlgItem) return;

	CDialogDoc* pDoc = (CDialogDoc*)pData;
	Bool bFound;
	String str = pDoc->GetString(pDlgItem->GetNameID(), bFound);
	pDlgItem->m_strName = str;
	//TRACE_STRING(str);
}

/*********************************************************************\
	Function name    : CDialogDoc::LanguageChanged
	Description      :
	Created at       : 22.08.01, @ 11:34:22
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::LanguageChanged()
{
	m_pTreeDlg->GetTreeView()->VisitChildren(ChangeLanguageCallback, nullptr, (void*)this);

	UpdateAllViews(true);
	SelChanged(m_pCurrentItem, false);
}

/*********************************************************************\
	Function name    : CDialogDoc::GetString
	Description      :
	Created at       : 22.08.01, @ 11:44:02
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
String CDialogDoc::GetString(String strID, Bool &bFound)
{
	return m_StringTable.GetString(strID, m_lLanguageIDTable, bFound);
}

/*********************************************************************\
	Function name    : CDialogDoc::SetStringtableItem
	Description      :
	Created at       : 23.08.01, @ 17:50:31
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::SetStringtableItem(String strID, String strText)
{
	m_StringTable.SetItemText(strID, m_lLanguageIDTable, strText);
}

/*********************************************************************\
	Function name    : CDialogDoc::RemoveCurrentItem
	Description      :
	Created at       : 23.08.01, @ 21:29:48
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::OnEditDelete()
{
	if (!m_pCurrentItem) return;
	if (m_pCurrentItem->GetType() == Dialog) return;

	m_pTreeDlg->RemoveItem(m_pCurrentItem);
	m_pCurrentItem = (CDialogItem*)m_pTreeDlg->GetSelectedItem()->GetData()->GetVoid(TREEVIEW_DATA);
	m_pDlgPreview->Update();
	m_pItemSettingsDlg->Update();
	m_pItemSettingsDlg->SetSelItem(m_pCurrentItem);
	SetChanged();
	m_bControlSymbolsChanged = true;
}

/*********************************************************************\
	Function name    : CDialogDoc::ItemChanged
	Description      :
	Created at       : 09.09.01, @ 09:41:15
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::ItemChanged()
{
	SetChanged();
	if (g_bAutoUpdate)
	{
		m_pDlgPreview->Update();
		m_pDlgPreview->MakeVisible(m_pCurrentItem);
	}
}

/*********************************************************************\
	Function name    : CDialogDoc::Copy
	Description      :
	Created at       : 09.09.01, @ 14:02:26
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::OnEditCopy()
{
	UpdateAllViews(true);
	ExportMacro(GeGetPluginPath() + String("Clipboard.dmf"), m_pCurrentItem);
}

/*********************************************************************\
	Function name    : CDialogDoc::Paste
	Description      :
	Created at       : 09.09.01, @ 14:02:27
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::OnEditPaste()
{
	ImportMacro(GeGetPluginPath() + String("Clipboard.dmf"), m_pCurrentItem);
}

/*********************************************************************\
	Function name    : CDialogDoc::Cut
	Description      :
	Created at       : 09.09.01, @ 14:29:57
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::OnEditCut()
{
	OnEditCopy();
	OnEditDelete();
}

/*********************************************************************\
	Function name    : CDialogDoc::OnFileImportMacro
	Description      :
	Created at       : 10.09.01, @ 13:33:00
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::OnFileImportMacro()
{
	Filename fn = GeGetPluginPath();
	if (!fn.FileSelect(FILESELECTTYPE_ANYTHING,FILESELECT_LOAD,String())) return;

	if (!GeFExist(fn)) return;

	ImportMacro(fn, m_pCurrentItem);
}

/*********************************************************************\
	Function name    : CDialogDoc::OnFileExportMacro
	Description      :
	Created at       : 10.09.01, @ 13:33:02
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::OnFileExportMacro()
{
	Filename fn = GeGetPluginPath();
	fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_SAVE, String());

	UpdateAllViews(true);
	ExportMacro(fn, m_pCurrentItem);
}

/*********************************************************************\
	Function name    : CDialogDoc::OpenDialog
	Description      :
	Created at       : 30.09.01, @ 10:17:34
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::OpenDialog(Filename fn)
{
	if (m_bChanged)
	{
		GEMB_R lSave = AskSave();
		if (lSave == GEMB_R_CANCEL) return;
		if (lSave == GEMB_R_YES)
		{
			if (!OnFileSave()) return;
		}
	}

	//m_FileName = fn;

	m_bChanged = false;
	OnFileNew();
	m_FileName = fn;

	m_pTreeDlg->GetTreeView()->Reset();
	m_pDialog = nullptr;

	// now, load the resource and add the dialog items, but not here :-)
	m_pTreeDlg->GetTreeView()->LockWindowUpdate();
	LoadDialogResource(fn);
	m_pTreeDlg->GetTreeView()->UnlockWindowUpdate();
	m_pTreeDlg->GetTreeView()->ExpandAllItems(false);
	if (!m_pDialog)
	{
		m_pTreeDlg->GetTreeView()->Reset();
		OnFileNew();
	}
	SelChanged(m_pDialog);

	m_pTreeDlg->UpdateLayout();

	//m_StringTable.Dump();

	m_bChanged = false;
	UpdateDialogCaptions();
	UpdateAllViews(true);
}

class CStringTableName : public CListItem
{
	String strName;
};

/*********************************************************************\
	Function name    : CDialogDoc::LoadGlobalStringTable
	Description      :
	Created at       : 03.10.01, @ 17:46:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::LoadGlobalStringTable(Filename fnProjectDir, String strTitle)
{
	if (!LoadStringTables(fnProjectDir, strTitle, typeStringTable))
	{
		if (!LoadStringTables(fnProjectDir, strTitle, typeCommandStrings))
		{
			if (!LoadStringTables(fnProjectDir, strTitle, typeDialogStrings))
			{
				GeOutString(GeLoadString(IDS_DIALOG_RESOURCE_CORRUPTED), GEMB_ICONEXCLAMATION);
				return;
			}
			else
				m_StringTableType = typeDialogStrings;
		}
		else
			m_StringTableType = typeCommandStrings;
	}
	else
		m_StringTableType = typeStringTable;

	m_FileName = fnProjectDir + strTitle;


	//TRACE_STRING(fnProjectDir.GetFileString());
	//TRACE_STRING(strTitle);
}

/*********************************************************************\
	Function name    : CDialogDoc::SaveGlobalStringTable
	Description      :
	Created at       : 06.10.01, @ 17:12:11
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::SaveGlobalStringTable()
{
	SaveStringTable(m_StringTableType/*CStringTable::typeStringTable*/);

	// ... and create a header file
	if (m_bStringSymbolsChanged)
		SaveHeaderFile(true);
}

/*********************************************************************\
	Function name    : CDialogDoc::CloseEditor
	Description      :
	Created at       : 11.10.01, @ 21:14:03
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::CloseEditor()
{
	Bool bRet = false;
	if (m_bChanged)
	{
		GEMB_R lSave = AskSave();
		if (lSave == GEMB_R_CANCEL) goto _exit;
		if (lSave == GEMB_R_YES)
		{
			if (!OnFileSave()) goto _exit;
		}
	}

	g_bWasActive = false;
	m_bChanged = false;
	OnFileNew();

	if (!g_pDlgPreview->m_bCloseState) { g_pDlgPreview->m_bCloseState = true; g_pDlgPreview->Close(); }
	if (!g_pSettingsDlg->m_bCloseState) { g_pSettingsDlg->m_bCloseState = true; g_pSettingsDlg->Close(); }
	if (!g_pTreeDialog->m_bCloseState) { g_pTreeDialog->m_bCloseState = true; g_pTreeDialog->Close(); }
	if (!g_pToolbar->m_bCloseState) { g_pToolbar->m_bCloseState = true; g_pToolbar->Close(); }

	bRet = true;

_exit:
	g_pDlgPreview->m_bCloseState = false;
	g_pSettingsDlg->m_bCloseState = false;
	g_pTreeDialog->m_bCloseState = false;
	g_pToolbar->m_bCloseState = false;

	return bRet;
}

/*********************************************************************\
	Function name    : CDialogDoc::CheckFilename
	Description      : returns true, if file save is allowed, false otherwise
	Created at       : 12.11.01, @ 18:10:16
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::CheckFilename(Filename &fn)
{
	String str = fn.GetFileString();
	String strSuffix;

	Int32 lPos = -1;
	if (str.FindLast(".", &lPos))
	{
		strSuffix = str.SubStr(lPos, str.GetLength() - lPos);
		str.Delete(lPos, str.GetLength() - lPos);
	}
	if (str.LexCompare(m_pDialog->m_strControlID) != 0)
	{
		Filename fnOld = fn;
		Filename fnNew = fn.GetDirectory();
		fnNew += (m_pDialog->m_strControlID + strSuffix);
		String strOut = GeLoadString(IDS_ID_MATCH_NAME);
		strOut += "\n";
		strOut += fnOld.GetString();
		strOut += "\n-->\n";
		strOut += fnNew.GetString();
		GEMB_R lOut = QuestionDialogYesNoCancel(strOut);
		if (lOut == GEMB_R_YES) fn = fnNew;
		else if (lOut == GEMB_R_NO) { }
		else return false;
	}
	return true;
}

typedef struct
{
	CStringTable* pStringTable;
	Int32 lElements;
	Char* pchExists;
} tagDeleteUnusedStrings;

/*********************************************************************\
	Function name    : DeleteUnusedCallback
	Description      : Deletes unused strings from the string table
	Created at       : 05.01.02, @ 23:07:07
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void DeleteUnusedCallback(TreeViewItem* pItem, void* pData)
{
	tagDeleteUnusedStrings* ds = (tagDeleteUnusedStrings*)pData;
	CDialogItem* pDlgItem = (CDialogItem*)(pItem->GetData()->GetVoid(TREEVIEW_DATA));
	if (!pDlgItem || !ds) return;
	Int32 a;

	ItemType type = pDlgItem->GetType();
	a = ds->pStringTable->GetElementIndex(pDlgItem->GetNameID());
	if (a >= 0 && a < ds->lElements)
		ds->pchExists[a] = 1;

	if (type == ComboBox || type == ComboButton || type == ArrowBtn)
	{
		CChildItems* pItems;
		if (type == ComboBox) pItems = ((CDialogComboBox*)pDlgItem)->GetChildren();
		else if (type == ComboButton) pItems = ((CDialogComboButton*)pDlgItem)->GetChildren();
		else pItems = ((CDialogArrowBtn*)pDlgItem)->GetChildren();

		String strID, strStringID;
		Int32 lChildren = pItems->GetItemCount();
		for (Int32 b = 0; b < lChildren; b++)
		{
			pItems->GetElement(b, strID, strStringID);
			/*a = ds->pStringTable->GetElementIndex(strID);
			if (a >= 0 && a < ds->lElements)
				ds->pchExists[a] = 1;*/
			a = ds->pStringTable->GetElementIndex(strStringID);
			if (a >= 0 && a < ds->lElements)
				ds->pchExists[a] = 1;
		}
	}
}

/*********************************************************************\
	Function name    : CDialogDoc::DeleteUnusedStrings
	Description      :
	Created at       : 05.01.02, @ 23:01:34
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::DeleteUnusedStrings()
{
	tagDeleteUnusedStrings ds;
	ds.pStringTable = &m_StringTable;
	ds.lElements = m_StringTable.GetNumElements();

	ds.pchExists = bNewDeprecatedUseArraysInstead<Char>(ds.lElements);
	Int32 lDeleteCount = 0;

	m_pTreeDlg->GetTreeView()->VisitChildren(DeleteUnusedCallback, nullptr, &ds);
	for (Int32 a = ds.lElements - 1; a >= 0; a--)
	{
		if (!ds.pchExists[a])
		{
			m_StringTable.DeleteElement(a);
			lDeleteCount++;
		}
	}

	bDelete(ds.pchExists);

	String str = GeLoadString(IDS_NUM_DELETED);
	str += String::IntToString(lDeleteCount);

	GeOutString(str, GEMB_OK | GEMB_ICONASTERISK);
}
