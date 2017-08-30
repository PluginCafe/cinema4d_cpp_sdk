 /*********************************************************************\
	File name        : TreeDialog.cpp
	Description      : Implementation of the class CTreeDialog
	Created at       : 11.08.01, @ 10:25:26
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "TreeDialog.h"
#include "DialogDoc.h"
#include "DialogItem.h"
#include "StringtableDialog.h"
#include "OptionsDialog.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

enum {
	ID_FILE_NEW = 100,
	ID_FILE_OPEN,
	ID_FILE_SAVE,
	ID_FILE_SAVE_AS,
	ID_FILE_IMPORT_MACRO,
	ID_FILE_EXPORT_MACRO,

	ID_LANG_EDIT_ST,
	ID_LANG_MAKE_EMPTY_ST,

	ID_EDIT_CUT,
	ID_EDIT_COPY,
	ID_EDIT_PASTE,
	ID_EDIT_DELETE,

	ID_OPTIONS,
	ID_OPTIONS_DELETE_UNUSDED_STRINGS,

	IDC_LIST_VIEW,
	IDC_SCROLL_GROUP,
	IDC_USER_AREA,

	IDC_FIRST_LANGUAGE = 1000,
	IDC_FIRST_LANGUAGE_NEW = 5000,

	ID_DUMMY // :-)
};

/*********************************************************************\
	Function name    : DestroyTreeItem
	Description      :
	Created at       : 15.08.01, @ 15:40:32
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void DestroyTreeItem(BaseContainer* pData)
{
	CDialogItem* pItem = (CDialogItem*)(pData->GetVoid(TREEVIEW_DATA));
	DeleteObj(pItem);
}

/*********************************************************************\
	Function name    : CResEditTreeView::IsDragAllowed
	Description      :
	Created at       : 24.11.01, @ 15:29:10
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CResEditTreeView::IsDragAllowed(TreeViewItem* pDragDest, TreeViewItem* pInsBefore, TreeViewItem* pInsAfter)
{
	if (pDragDest && !pInsBefore && !pInsAfter)
	{
		CDialogItem* pDlgItem = (CDialogItem*)pDragDest->GetData()->GetVoid(TREEVIEW_DATA);
		if ((pDlgItem->GetType() != Group) && (pDlgItem->GetType() != Dialog))
			return false;
	}

	return true;
}

CTreeDialog::CTreeDialog(CDialogDoc* pDoc)
{
	m_pDocument = pDoc;
	m_pDocument->m_pTreeDlg = this;
	m_bCloseState = false;
}

CTreeDialog::~CTreeDialog()
{

}

/*********************************************************************\
	Funktionsname    : CTreeDialog::CreateLayout
	Beschreibung     :
	Rückgabewert     : Bool
	Erstellt am      : 11.08.01, @ 10:42:29
	Argument         : void
\*********************************************************************/
Bool CTreeDialog::CreateLayout(void)
{
	Bool bRes = GeDialog::CreateLayout();

	Int32 a;

	// add a menu
	MenuSubBegin(GeLoadString(IDS_FILE));
		MenuAddString(ID_FILE_NEW, GeLoadString(IDS_FILE_NEW));
		MenuAddString(ID_FILE_OPEN, GeLoadString(IDS_FILE_OPEN));
		MenuAddString(ID_FILE_SAVE, GeLoadString(IDS_FILE_SAVE));
		MenuAddString(ID_FILE_SAVE_AS, GeLoadString(IDS_FILE_SAVE_AS));
		MenuAddSeparator();
		MenuAddString(ID_FILE_IMPORT_MACRO, GeLoadString(IDS_FILE_IMPORT_MACRO));
		MenuAddString(ID_FILE_EXPORT_MACRO, GeLoadString(IDS_FILE_EXPORT_MACRO));
	MenuSubEnd();

	MenuSubBegin(GeLoadString(IDS_EDIT));
		MenuAddString(ID_EDIT_CUT, GeLoadString(IDS_EDIT_CUT));
		MenuAddString(ID_EDIT_COPY, GeLoadString(IDS_EDIT_COPY));
		MenuAddString(ID_EDIT_PASTE, GeLoadString(IDS_EDIT_PASTE));
		MenuAddString(ID_EDIT_DELETE, GeLoadString(IDS_EDIT_DELETE));
	MenuSubEnd();

	MenuSubBegin(GeLoadString(IDS_LANGUAGE));
		for (a = 0; a < g_LanguageList.GetNumLanguages(); a++)
			MenuAddString(IDC_FIRST_LANGUAGE + a, g_LanguageList.GetName(a));
		MenuAddSeparator();

		MenuSubBegin(GeLoadString(IDS_LANGUAGE_NEW));
			for (a = 0; a < g_LanguageList.GetNumLanguages(); a++)
				MenuAddString(IDC_FIRST_LANGUAGE_NEW + a, g_LanguageList.GetName(a));
		MenuSubEnd();

		MenuAddString(ID_LANG_EDIT_ST, GeLoadString(IDS_LANGUAGE_EDIT_ST));
		MenuAddString(ID_LANG_MAKE_EMPTY_ST, GeLoadString(IDS_MAKE_EMPTY_ST));
	MenuSubEnd();

	MenuSubBegin(GeLoadString(IDS_OPTIONS_CAPT));
		MenuAddString(ID_OPTIONS, GeLoadString(IDS_OPTIONS));
		MenuAddString(ID_OPTIONS_DELETE_UNUSDED_STRINGS, GeLoadString(IDS_DELETE_UNUSED));
	MenuSubEnd();

	MenuFinished();

	//AddListView(IDC_LIST_VIEW, BFH_SCALE | BFV_SCALE, 0, 0);
	//m_wndListView.AttachListView(this, IDC_LIST_VIEW);
	if (!m_wndTree.CreateTreeView(IDC_USER_AREA, this, PLUGIN_ID_TREE_DIALOG, DestroyTreeItem, TREE_VIEW_HAS_DRAG_DROP | TREE_ITEM_SAME_HEIGHT)) return false;

	if (++g_lNeedFileNew == 4) m_pDocument->OnFileNew();

	return bRes;
}

/*********************************************************************\
	Funktionsname    : CTreeDialog::InitValues
	Beschreibung     :
	Rückgabewert     : Bool
	Erstellt am      : 11.08.01, @ 16:33:35
	Argument         : void
\*********************************************************************/
Bool CTreeDialog::InitValues(void)
{
	if (!GeDialog::InitValues()) return false;

	//MenuInitString(ID_EDIT_CUT, 0, 1);
	//Enable(ID_EDIT_CUT, false);

	return true;
}

/*********************************************************************\
	Funktionsname    : CTreeDialog::Command
	Beschreibung     :
	Rückgabewert     : Bool
	Erstellt am      : 11.08.01, @ 16:14:33
	Argumente        : Int32 id
										 const BaseContainer &msg
\*********************************************************************/
Bool CTreeDialog::Command(Int32 id,const BaseContainer &msg)
{
	switch (id)
	{
	case IDC_USER_AREA: {
		TreeViewItem* pSelItem = (TreeViewItem*)msg.GetVoid(TREEVIEW_MESSAGE_ITEM);
		Int32 lType = msg.GetInt32(TREEVIEW_MESSAGE_TYPE);

		if (pSelItem && (lType == TREEVIEW_SELCHANGE))
		{
			m_pDocument->SelChanged((CDialogItem*)(pSelItem->GetData()->GetVoid(TREEVIEW_DATA)));
			m_pDocument->UpdateAllViews(true, false);
		}
		if (!pSelItem)
		{
			m_pDocument->SelChanged(nullptr);
			m_pDocument->UpdateAllViews(true, false);
		}
		if (lType == TREEVIEW_STRUCTURE_CHANGE)
		{
			m_pDocument->SetChanged();
			m_pDocument->UpdateAllViews();
		}
		if (lType == TREEVIEW_DOUBLECLICK && pSelItem)
		{
			CDialogItem* pItem = (CDialogItem*)(pSelItem->GetData()->GetVoid(TREEVIEW_DATA));
			m_pDocument->SelChanged(pItem);
			if (RenameDialog(&pItem->m_strName))
			{
				m_pDocument->SelChanged((CDialogItem*)(pSelItem->GetData()->GetVoid(TREEVIEW_DATA)));
				m_pDocument->UpdateAllViews(true, true);
			}
		}

		//AttachUserArea(m_wndTree, IDC_USER_AREA);
		//LayoutChanged(IDC_USER_AREA);
		break;
											}
	case ID_FILE_NEW:
		m_pDocument->OnFileNew();
		break;
	case ID_FILE_OPEN:
		m_pDocument->OnFileOpen();
		break;
	case ID_FILE_SAVE:
		m_pDocument->OnFileSave();
		break;
	case ID_FILE_SAVE_AS:
		m_pDocument->OnFileSaveAs();
		break;
	case ID_EDIT_CUT:
		m_pDocument->OnEditCut();
		break;
	case ID_EDIT_COPY:
		m_pDocument->OnEditCopy();
		break;
	case ID_EDIT_PASTE:
		m_pDocument->OnEditPaste();
		break;
	case ID_EDIT_DELETE:
		m_pDocument->OnEditDelete();
		break;
	case ID_FILE_EXPORT_MACRO:
		m_pDocument->OnFileExportMacro();
		break;
	case ID_FILE_IMPORT_MACRO:
		m_pDocument->OnFileImportMacro();
		break;
	case ID_LANG_EDIT_ST: {
		m_pDocument->StoreNamesInStringtable();
		CStringtableDialog dlg(m_pDocument);
		dlg.Open();
		break;
												}

	case ID_LANG_MAKE_EMPTY_ST:
		{
			Filename fn;
			fn = GeGetStartupPath() + String("resource");
			if (fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_DIRECTORY, GeLoadString(IDS_SPECIFY_RES_PATH)))
			{
				AutoAlloc <BaseFile> pFile;
				if (pFile)
				{
					for (Int32 a = 0; a < g_LanguageList.GetNumLanguages(); a++)
					{
						Filename fnNew = fn + Filename("strings_" + g_LanguageList.GetLanguageSuffix(a));
						if (!GeFExist(fnNew, true))
							GeFCreateDir(fnNew);

						fnNew += String("c4d_strings.str");
						if (!GeFExist(fnNew))
						{
							ForceOpenFileWrite(pFile, fnNew);
							WriteString(pFile, "// C4D-StringResource"); LineBreak(pFile, "");
							WriteString(pFile, "// Identifier	Text "); LineBreak(pFile, ""); LineBreak(pFile, "");
							WriteString(pFile, "STRINGTABLE"); LineBreak(pFile, "");
							WriteString(pFile, "{"); LineBreak(pFile, "");
							WriteString(pFile, "}"); LineBreak(pFile, "");
							pFile->Close();
						}
					}
				}
			}
		}
		break;

	case ID_OPTIONS : {
		COptionsDialog dlg;
		dlg.Open();
		break;
										}

	case ID_OPTIONS_DELETE_UNUSDED_STRINGS:
		m_pDocument->DeleteUnusedStrings();
		break;

	case IDC_OK:
	case IDC_CANCEL:
		{
			int a = 1;
			a = a;
			break;
		}

	};

	/*case IDC_SCROLL_GROUP : {
		BFM_SCROLLAREA

		break;
				 }*/

	if (id >= IDC_FIRST_LANGUAGE && id < IDC_FIRST_LANGUAGE + g_LanguageList.GetNumLanguages())
	{
		Int32 lSel = id - IDC_FIRST_LANGUAGE;
		m_pDocument->ChangeLanguage(g_LanguageList.GetLanguageSuffix(lSel));
		SelectLanguage(lSel);
	}

	if (id >= IDC_FIRST_LANGUAGE_NEW && id < IDC_FIRST_LANGUAGE_NEW + g_LanguageList.GetNumLanguages())
	{
		Int32 lSel = id - IDC_FIRST_LANGUAGE_NEW;
		m_pDocument->GetStringTable()->AddLanguage(g_LanguageList.GetLanguageSuffix(lSel));
		m_pDocument->ChangeLanguage(g_LanguageList.GetLanguageSuffix(lSel));
		SelectLanguage(lSel);
	}


	return true;
}

/*********************************************************************\
	Function name    : CTreeDialog::Message
	Description      :
	Created at       : 23.08.01, @ 21:29:01
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 CTreeDialog::Message(const BaseContainer &msg, BaseContainer &result)
{
	Int32 lID = msg.GetId();
	Int32 lDevice;
	switch (lID)
	{
		case BFM_INPUT:
			lDevice = msg.GetInt32(BFM_INPUT_DEVICE);
			if (lDevice == BFM_INPUT_KEYBOARD)
			{
				Int32 lValue = msg.GetInt32(BFM_INPUT_CHANNEL);
				Bool bCtrl = (msg.GetInt32(BFM_INPUT_QUALIFIER) & QCTRL) != 0;

				if (bCtrl) // the strg key was pressed
				{
					if (lValue == 78) // N
						m_pDocument->OnFileNew();
					else if (lValue == 83) // s
						m_pDocument->OnFileSave();
					else if (lValue == 79) // o
						m_pDocument->OnFileOpen();
					else if (lValue == 88) // x
						m_pDocument->OnEditCut();
					else if (lValue == 67) // c
						m_pDocument->OnEditCopy();
					else if (lValue == 86) // v
						m_pDocument->OnEditPaste();
					return true;
				}
				/*else
				{
					if (lValue == 32777) // delete
						m_pDocument->OnEditDelete();
				}*/

				TRACE("Key pressed : %d\n", lValue);
				//GeOutString("Taste gedrückt", 0);
				//return m_wndTree.Message(msg, result);

				//
				return true;
			}
			break;
	};

	return GeDialog::Message(msg, result);
}
/*
Bool CTreeDialog::InputEvent(const BaseContainer &msg)
{
	return true;
}*/


/*********************************************************************\
	Function name    : CTreeDialog::SetSelItem
	Description      :
	Created at       : 12.08.01, @ 09:59:26
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CTreeDialog::SetSelItem(CDialogItem* pItem, Bool bMakeVisible)
{
	// first, expand all parent items
	TreeViewItem* pTreeItem = pItem->m_pTreeViewItem;
	m_wndTree.SelectItem(pTreeItem);
	if (bMakeVisible)
		m_wndTree.MakeVisible(pTreeItem);

	UpdateLayout();
}

/*********************************************************************\
	Function name    : CTreeDialog::AddItem
	Description      :
	Created at       : 14.08.01, @ 22:52:19
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CTreeDialog::AddItem(CDialogItem* pItem, CDialogItem* pInsertAfter)
{
	BaseContainer bc;
	bc.SetString(TREEVIEW_TEXT, pItem->m_strName);
	bc.SetInt32(TREEVIEW_ICON, (Int32)pItem->GetType());
	bc.SetVoid(TREEVIEW_DATA, pItem);
	pItem->m_pTreeViewItem = m_wndTree.AddItem(bc, m_wndTree.GetCurrentItem(), pInsertAfter ? pInsertAfter->m_pTreeViewItem : nullptr);
}

/*********************************************************************\
	Function name    : CTreeDialog::RemoveItem
	Description      :
	Created at       : 23.08.01, @ 22:01:15
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CTreeDialog::RemoveItem(CDialogItem* pItem)
{
	m_wndTree.DeleteItem(pItem->m_pTreeViewItem);
	UpdateLayout();
}

/*********************************************************************\
	Function name    : CTreeDialog::Update
	Description      :
	Created at       : 15.08.01, @ 11:51:37
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CTreeDialog::Update()
{
	m_wndTree.Redraw();
}

/*********************************************************************\
	Function name    : CTreeDialog::UpdateLayout
	Description      :
	Created at       : 15.08.01, @ 15:26:16
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CTreeDialog::UpdateLayout()
{
	//AttachUserArea(m_wndTree, IDC_USER_AREA);
	//LayoutChanged(IDC_USER_AREA);
}

/*********************************************************************\
	Function name    : CTreeDialog::SelectLanguage
	Description      : checks the menu item for lLanguage lLang
	Created at       : 20.09.01, @ 10:09:50
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CTreeDialog::SelectLanguage(Int32 lLang)
{
	for (Int32 l = 0; l < g_LanguageList.GetNumLanguages(); l++)
	{
		Bool bEnabled;
		bEnabled = (m_pDocument->GetStringTable()->GetLanguageID(g_LanguageList.GetLanguageSuffix(l))) >= 0;
		MenuInitString(l + IDC_FIRST_LANGUAGE, bEnabled, lLang == l);
		MenuInitString(l + IDC_FIRST_LANGUAGE_NEW, !bEnabled, 0);
	}
}

/*********************************************************************\
	Function name    : CTreeDialog::OpenDialog
	Description      : opens a dialog file
	Created at       : 30.09.01, @ 10:15:58
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CTreeDialog::OpenDialog(Filename fn)
{
	m_pDocument->OpenDialog(fn);
}

/*********************************************************************\
	Function name    : CTreeDialog::AskClose
	Description      :
	Created at       : 11.10.01, @ 21:38:20
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CTreeDialog::AskClose()
{
	if (m_bCloseState) return false;

	m_bCloseState = true;
	if (!m_pDocument->CloseEditor()) return true;

	return false;
}
