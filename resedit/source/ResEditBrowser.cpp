/*********************************************************************\
	File name        : ResEditBrowser.cpp
	Description      : Implementierung der Klasse CResEditBrowser
	Created at       : 25.09.01, @ 22:19:09
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "TreeView.h"
#include "ResEditBrowser.h"
#include "TreeDialog.h"
#include "DialogDoc.h"
#include "StringtableDialog.h"
#include "ResEditMenu.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

#define ITEM_PATH_NAME      'itpn'

extern CTreeDialog* g_pTreeDialog;
extern Bool g_bWasActive;

enum
{
	IDC_BROWSE_BUTTON     = 10000,
	IDC_PATH_STATIC,
	IDC_FILE_LIST,
	IDC_DIRECTORY_TEXT,
	IDC_DIRECTORY_TREE,

	ID_GLOBAL_STRINGTABLE,

	ID_DUMMY
};

CResEditBrowser::CResEditBrowser()
{
}

CResEditBrowser::~CResEditBrowser()
{

}

/*********************************************************************\
	Function name    : CResEditBrowser::CreateLayout
	Description      :
	Created at       : 25.09.01, @ 22:33:40
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CResEditBrowser::CreateLayout(void)
{
	GroupBegin(101, BFH_SCALEFIT | BFV_SCALEFIT, 1, 0, "", 0);
		GroupBegin(201, BFH_SCALEFIT, 0, 2, "", 0);
			AddButton(IDC_BROWSE_BUTTON, BFH_LEFT, 0, 0, GeLoadString(IDS_BROWSE));
			AddStaticText(IDC_PATH_STATIC, BFH_SCALEFIT, 0, 0, "", BORDER_GROUP_IN);
		GroupEnd();

		GroupBegin(202, BFH_SCALEFIT | BFV_SCALEFIT, 1, 0, "", 0);
			if (!m_wndTreeView.CreateTreeView(IDC_DIRECTORY_TREE, this, NOTOK, nullptr, TREE_ITEM_SAME_HEIGHT, BFH_SCALEFIT | BFV_SCALEFIT)) return false;
			AddStaticText(IDC_DIRECTORY_TEXT, BFH_SCALEFIT, 0, 0, "", 0);
		GroupEnd();
	GroupEnd();
	/*m_wndListView.AttachListView(this, IDC_FILE_LIST);


	BaseContainer bc;
	bc.SetInt32('name', LV_COLUMN_TEXT);
	m_wndListView.SetLayout(1, bc);*/

	m_strPath = GeGetStartupPath() + String("resource");
	SetString(IDC_PATH_STATIC, m_strPath.GetString());

	return true;
}

/*********************************************************************\
	Function name    : CResEditBrowser::Command
	Description      :
	Created at       : 25.09.01, @ 22:33:39
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CResEditBrowser::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDC_BROWSE_BUTTON: {
		Filename fn = m_strPath;
		AutoAlloc <BrowseFiles> pFiles;
		pFiles->Init(Filename("\\"), false);
		if (!m_strPath.Content())
			fn = GeGetStartupPath() + String("resource");
		if (fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_DIRECTORY, GeLoadString(IDS_SPECIFY_RES_PATH)))
		{
			m_strPath = fn;
			FillList();
			SetString(IDC_PATH_STATIC, m_strPath.GetString());
		}
		break;
													}
	case IDC_DIRECTORY_TREE: {
		TreeViewItem* pSelItem = (TreeViewItem*)msg.GetVoid(TREEVIEW_MESSAGE_ITEM);
		Int32 lType = msg.GetInt32(TREEVIEW_MESSAGE_TYPE);
		if (!pSelItem) break;

		if (lType == TREEVIEW_DOUBLECLICK)
		{
			Filename fn = pSelItem->GetData()->GetString(ITEM_PATH_NAME);
			if (fn.CheckSuffix("res"))
			{
				// open the resource file
				if (!g_bWasActive)
				{ CResEditMenu m; m.Execute(nullptr); }
				if (!g_pTreeDialog) break;
				g_pTreeDialog->OpenDialog(fn);
			}
			else if (fn.CheckSuffix("str"))
			{
				CDialogDoc tempDoc;
				tempDoc.LoadGlobalStringTable(m_strPath, pSelItem->GetData()->GetString(TREEVIEW_TEXT));
				CStringtableDialog dlg(&tempDoc, true);
				if (dlg.Open())
				{
					if (GeOutString(GeLoadString(IDS_SAVE_STRINGTABLE), GEMB_YESNO | GEMB_ICONQUESTION) == GEMB_R_YES)
					{
						tempDoc.SaveGlobalStringTable();
					}
				}
			}
			else if (fn.Content())
			{
				// open the file (it may be an image)
				GeExecuteFile(fn);
			}
			return true;
		}
		else if (lType == TREEVIEW_SELCHANGE)
		{
			SetString(IDC_DIRECTORY_TEXT, pSelItem->GetData()->GetString(ITEM_PATH_NAME));
		}

		break;
													 }
	};
	return GeDialog::Command(lID, msg);
}

class CStringTableName : public CListItem
{
public:
	String strName;
};

/*********************************************************************\
	Function name    : CResEditBrowser::FillList
	Description      :
	Created at       : 25.09.01, @ 22:58:07
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CResEditBrowser::FillList()
{
	GeShowMouse(MOUSE_BUSY);

	m_wndTreeView.LockWindowUpdate();
	m_wndTreeView.Reset();

	m_pDialogsItem = m_wndTreeView.AddItem(GeLoadString(IDS_BROWSER_DIALOGS));
	m_pImagesItem = m_wndTreeView.AddItem(GeLoadString(IDS_BROWSER_IMAGES));
	m_pStringTablesItem = m_wndTreeView.AddItem(GeLoadString(IDS_BROWSER_STRIG_TABLES));

	BrowsePath(m_strPath);

	// add global string table(s)
	CDoubleLinkedList <CStringTableName> stringList;

	AutoAlloc <BrowseFiles> pBrowse;
	pBrowse->Init(m_strPath, false);
	while (pBrowse->GetNext())
	{
		if (!pBrowse->IsDir()) continue;

		Filename fnNew = pBrowse->GetFilename();

		if (fnNew.GetString().SubStr(0, 8).ToLower() == "strings_")
		{
			Filename fnStringTable = m_strPath + fnNew;
			TRACE("Found string table path ");
			//TRACE_STRING(fnStringTable.GetString());

			AutoAlloc <BrowseFiles> pBrowseTable;
			pBrowseTable->Init(fnStringTable, false);

			while (pBrowseTable->GetNext())
			{
				Filename fnTable = pBrowseTable->GetFilename();
				if (fnTable.CheckSuffix("str"))
				{
					TRACE("  ");
					//TRACE_STRING(fnTable.GetString());

					String strNewName = fnTable.GetString();
					// check, if we didn't find this table before

					CStringTableName *pSearch = stringList.First();
					Bool bFound = false;
					while (pSearch)
					{
						if (pSearch->strName == strNewName)
						{
							bFound = true;
							break;
						}
						pSearch = stringList.Next(pSearch);
					}
					if (bFound) continue;

					CStringTableName *pNewItem = NewObjClear(CStringTableName);
					pNewItem->strName = strNewName;
					stringList.Append(pNewItem);
				}
			}
		}
	}

	CStringTableName *pInsert = stringList.First();
	while (pInsert)
	{
		TreeViewItem* pNewItem = m_wndTreeView.AddItem(pInsert->strName, m_pStringTablesItem);
		pNewItem->GetData()->SetString(ITEM_PATH_NAME, pInsert->strName);
		pInsert = stringList.Next(pInsert);
	}
	stringList.FreeList();

	m_wndTreeView.SortChildren(m_pDialogsItem);
	m_wndTreeView.SortChildren(m_pImagesItem);
	m_wndTreeView.SortChildren(m_pStringTablesItem);

	m_wndTreeView.UnlockWindowUpdate();

	GeShowMouse(MOUSE_NORMAL);
}

/*********************************************************************\
	Function name    : CResEditBrowser::BrowsePath
	Description      :
	Created at       : 26.09.01, @ 11:59:42
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CResEditBrowser::BrowsePath(Filename fn)
{
	AutoAlloc <BrowseFiles> pBrowse;
	pBrowse->Init(fn, false);

	Filename fnBrowse;
	TreeViewItem* pNewItem;

	//TRACE_STRING(fn.GetString());

	while (pBrowse->GetNext())
	{
		fnBrowse = pBrowse->GetFilename();
		if (pBrowse->IsDir())
		{
			BrowsePath(fn + fnBrowse);
			continue;
		}
		pNewItem = nullptr;
		if (fnBrowse.CheckSuffix("res"))
			pNewItem = m_wndTreeView.AddItem(fnBrowse.GetFileString(), m_pDialogsItem);
		else if (fnBrowse.CheckSuffix("tif") || fnBrowse.CheckSuffix("jpg") || fnBrowse.CheckSuffix("bmp"))
			pNewItem = m_wndTreeView.AddItem(fnBrowse.GetFileString(), m_pImagesItem);

		if (pNewItem)
			pNewItem->GetData()->SetString(ITEM_PATH_NAME, (fn + fnBrowse).GetString());
	}
}

/*********************************************************************\
	Function name    : CResEditBrowser::InitValues
	Description      :
	Created at       : 25.09.01, @ 22:59:18
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CResEditBrowser::InitValues(void)
{
	SetTitle(GeLoadString(IDS_BROWSER_CAPT));

	//FillList();

	return true;
}

/*********************************************************************\
	Function name    : CResEditBrowser::Message
	Description      :
	Created at       : 21.10.01, @ 17:23:02
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 CResEditBrowser::Message(const BaseContainer &msg, BaseContainer &result)
{
	return GeDialog::Message(msg, result);
}

