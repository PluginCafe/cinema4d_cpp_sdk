/*********************************************************************\
	File name        : CResEditMenu.cpp
	Description      :
	Created at       : 26.03.02, @ 11:19:45
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "ResEditMenu.h"
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

CDialogPreview* g_pDlgPreview = nullptr;
CItemSettingsDialog* g_pSettingsDlg = nullptr;
CTreeDialog* g_pTreeDialog = nullptr;
CDialogDoc* g_pDialogDoc = nullptr;
CResEditToolBar* g_pToolbar = nullptr;
CStringTranslateDialog* g_pStringTransDialog = nullptr;
Bool g_bWasActive = false;

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CResEditMenu::CResEditMenu()
{

}

CResEditMenu::~CResEditMenu()
{

}

/*********************************************************************\
	Function name    : CResEditMenu::Execute
	Description      :
	Created at       : 26.03.02, @ 11:34:35
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CResEditMenu::Execute(BaseDocument *doc)
{
	if (!g_bWasActive) {
		g_bWasActive = true;
		/*g_pDialogDoc = NewObjClear(CDialogDoc);
		g_pDlgPreview = NewObjClear(CDialogPreview, g_pDialogDoc);
		g_pSettingsDlg = NewObjClear(CItemSettingsDialog, g_pDialogDoc);
		g_pTreeDialog = NewObjClear(CTreeDialog, g_pDialogDoc);
		g_pToolbar = NewObjClear(CResEditToolBar, g_pDialogDoc);*/

		g_pDlgPreview->m_bCloseState = false;
		g_pSettingsDlg->m_bCloseState = false;
		g_pTreeDialog->m_bCloseState = false;
		g_pToolbar->m_bCloseState = false;

		g_pDlgPreview->Open(DLG_TYPE_ASYNC, PLUGIN_ID_DIALOG_PREVIEW);
		g_pSettingsDlg->Open(DLG_TYPE_ASYNC, PLUGIN_ID_ITEM_SETTINGS);
		g_pTreeDialog->Open(DLG_TYPE_ASYNC, PLUGIN_ID_TREE_DIALOG);
		g_pToolbar->Open(DLG_TYPE_ASYNC_POPUP_RESIZEABLE, PLUGIN_ID_TOOLBAR, -1, -1, 1, 1, 1);

		g_pDialogDoc->OnFileNew();
		g_lNeedFileNew = 10;
	}
	else {
		if (g_pDialogDoc->CloseEditor())
			g_bWasActive = false;
	}

	return true;
}


/*********************************************************************\
	Funktionsname    : DeleteDialogs
	Beschreibung     : If dialogs are still open, free their memory
	Rückgabewert     : void
	Erstellt am      : 11.08.01, @ 10:55:34
\*********************************************************************/
void DeleteDialogs()
{
	DeleteObj(g_pDlgPreview);
	DeleteObj(g_pSettingsDlg);
	DeleteObj(g_pTreeDialog);
	DeleteObj(g_pToolbar);
	DeleteObj(g_pDialogDoc);
	DeleteObj(g_pStringTransDialog);
}
