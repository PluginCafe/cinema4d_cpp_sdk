/*********************************************************************\
	File name        : Register.cpp
	Description      : Registers the plugin IDs
	Created at       : 11.08.01, @ 09:27:28
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "ResEditBrowser.h"
#include "CompareTableDialog.h"

#include "DialogDoc.h"
#include "DialogItem.h"
#include "DialogPreview.h"
#include "ItemSettingsDialog.h"
#include "TreeDialog.h"
#include "ResEditToolBar.h"
#include "StringTableTranslateDialog.h"

#include "ResEditMenu.h"
#include "BrowserMenu.h"
#include "CompareMenu.h"
#include "HelpMenu.h"
#include "ExtractZipDialog.h"
#include "MakeDistriDialog.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

String *ResEditHelp = nullptr;
String *ResBrowserHelp = nullptr;
String *ResCompareHelp = nullptr;
String *ResHelpHelp = nullptr;
CResEditBrowser *g_pBrowserDialog = nullptr;

void DeleteDialogs();

Filename *g_pfnOriginal, *g_pfnNew;
extern CDialogPreview* g_pDlgPreview;
extern CItemSettingsDialog* g_pSettingsDlg;
extern CTreeDialog* g_pTreeDialog;
extern CDialogDoc* g_pDialogDoc;
extern CResEditToolBar* g_pToolbar;
extern CStringTranslateDialog* g_pStringTransDialog;

Int32 lRestoreCount = 0;

class CPreviewLayout : public CommandData
{
public:
	virtual Bool Execute(BaseDocument *doc)
	{
		g_pDlgPreview->Open(DLG_TYPE_ASYNC, PLUGIN_ID_DIALOG_PREVIEW, -1, -1);
		return true;
	}
	virtual Int32 GetState(BaseDocument *doc) { return CMD_ENABLED; }
	virtual Bool RestoreLayout(void *secret) { return g_pDlgPreview->RestoreLayout(PLUGIN_ID_DIALOG_PREVIEW, 0, secret); }
};

class CTreeLayout : public CommandData
{
public:
	virtual Bool Execute(BaseDocument *doc)
	{
		g_pTreeDialog->Open(DLG_TYPE_ASYNC, PLUGIN_ID_TREE_DIALOG, -1, -1);
		return true;
	}
	virtual Int32 GetState(BaseDocument *doc) { return CMD_ENABLED; }
	virtual Bool RestoreLayout(void *secret) { return g_pTreeDialog->RestoreLayout(PLUGIN_ID_TREE_DIALOG, 0, secret); }
};

class CToolbarLayout : public CommandData
{
public:
	virtual Bool Execute(BaseDocument *doc)
	{
		g_pToolbar->Open(DLG_TYPE_ASYNC_POPUP_RESIZEABLE, PLUGIN_ID_TOOLBAR, -1, -1);
		return true;
	}
	virtual Int32 GetState(BaseDocument *doc) { return CMD_ENABLED; }
	virtual Bool RestoreLayout(void *secret) { return g_pToolbar->RestoreLayout(PLUGIN_ID_TOOLBAR, 0, secret); }
};

class CSettingsLayout : public CommandData
{
public:
	virtual Bool Execute(BaseDocument *doc)
	{
		g_pSettingsDlg->Open(DLG_TYPE_ASYNC, PLUGIN_ID_ITEM_SETTINGS, -1, -1);
		return true;
	}
	virtual Int32 GetState(BaseDocument *doc) { return CMD_ENABLED; }
	virtual Bool RestoreLayout(void *secret) { return g_pSettingsDlg->RestoreLayout(PLUGIN_ID_ITEM_SETTINGS, 0, secret); }
};

class CStringTableTransLayout : public CommandData
{
public:
	virtual Bool Execute(BaseDocument *doc)
	{
		g_pStringTransDialog->Open(DLG_TYPE_ASYNC, ID_STRING_TABLE_TRANS_DIALOG, -1, -1);
		return true;
	}
	virtual Int32 GetState(BaseDocument *doc) { return CMD_ENABLED; }
	virtual Bool RestoreLayout(void *secret) { return g_pStringTransDialog->RestoreLayout(ID_STRING_TABLE_TRANS_DIALOG, 0, secret); }
};

class CExtractZipFile : public CommandData
{
public:
	virtual Bool Execute(BaseDocument *doc)
	{
		Filename fn;
		if (fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_LOAD, String()))
		{
			CExtractZipDialog dlg(fn);
			dlg.Open();
			return true;
		}
		return true;
	}
};

class CMakeDistriCommand : public CommandData
{
public:
	virtual Bool Execute(BaseDocument *doc)
	{
		MakeDistriDialog dlg;
		dlg.Open(-2, -2, 800, 750);
		return true;
	}
};

class CResEditSeparator : public CommandData
{
};

/*********************************************************************\
	Function name    : RegisterResEdit
	Description      :
	Created at       : 11.08.01, @ 09:48:30
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool RegisterResEdit(void)
{
	String strError;

	do {
		// register the menu items
		do {
			if (!RegisterCommandPlugin(PLUGIN_ID_MENU_ITEM, GeLoadString(IDS_RES_EDIT_MENU_TEXT), 0,
					 nullptr, GeLoadString(IDS_RES_EDIT_MENU_TEXT), NewObjClear(CResEditMenu)))
			{
				strError = GeLoadString(IDS_FAILED_REGISTER_RES_EDIT_MENU);
				break;
			}

		} while (0);
		do {
			g_pBrowserDialog = NewObjClear(CResEditBrowser);
			if (!RegisterCommandPlugin(PLUGIN_ID_BROWSER, GeLoadString(IDS_RES_BROWSER_MENU_TEXT), 0,
					 nullptr, GeLoadString(IDS_RES_BROWSER_MENU_TEXT), NewObjClear(CBrowserMenu)))
			{
				strError = GeLoadString(IDS_FAILED_REGISTER_RES_EDIT_MENU);
				break;
			}
		} while (0);

		do {
			if (!RegisterCommandPlugin(PLUGIN_ID_COMPARE_MENU, GeLoadString(IDS_RES_COMPARE_STRINGTABLES), 0,
					 nullptr, GeLoadString(IDS_RES_COMPARE_STRINGTABLES), NewObjClear(CCompareMenu)))
			{
				strError = GeLoadString(IDS_FAILED_REGISTER_RES_EDIT_MENU);
				break;
			}
		} while (0);

		do {
			if (!RegisterCommandPlugin(PLUGIN_ID_HELP_MENU, GeLoadString(IDS_RES_HELP), 0,
					 nullptr, GeLoadString(IDS_RES_HELP), NewObjClear(CHelpMenu)))
			{
				strError = GeLoadString(IDS_FAILED_REGISTER_RES_EDIT_MENU);
				break;
			}
		} while (0);

		do
		{
			if (!RegisterCommandPlugin(ID_STRING_TABLE_TRANS_DIALOG, GeLoadString(IDS_RES_STRING_TRANSLATE_TOOL), 0,
					 nullptr, GeLoadString(IDS_RES_STRING_TRANSLATE_TOOL), NewObjClear(CStringTableTransLayout)))
			{
				strError = GeLoadString(IDS_FAILED_REGISTER_RES_EDIT_MENU);
				break;
			}
		} while (0);

		do
		{
			if (!RegisterCommandPlugin(RES_EDIT_EXTRACT_ZIP, GeLoadString(IDS_EXTRACT_ZIP_MENU), 0,
					 nullptr, GeLoadString(IDS_EXTRACT_ZIP_MENU), NewObjClear(CExtractZipFile)))
			{
				strError = GeLoadString(IDS_FAILED_REGISTER_RES_EDIT_MENU);
				break;
			}
		} while (0);

		do
		{
			if (!RegisterCommandPlugin(PLUGIN_ID_EDIT_DISTRI, GeLoadString(IDS_MAKE_DISTRI_MENU), 0,
					 nullptr, GeLoadString(IDS_MAKE_DISTRI_MENU), NewObjClear(CMakeDistriCommand)))
			{
				strError = GeLoadString(IDS_FAILED_REGISTER_RES_EDIT_MENU);
				break;
			}
		} while (0);

		// register some invisible menu items
		do {
			if (!RegisterCommandPlugin(PLUGIN_ID_DIALOG_PREVIEW, "preview", PLUGINFLAG_HIDE, nullptr, "preview", NewObjClear(CPreviewLayout)))
				break;
			if (!RegisterCommandPlugin(PLUGIN_ID_TREE_DIALOG, "tree", PLUGINFLAG_HIDE, nullptr, "tree", NewObjClear(CTreeLayout)))
				break;
			if (!RegisterCommandPlugin(PLUGIN_ID_TOOLBAR, "toolbar", PLUGINFLAG_HIDE, nullptr, "toolbar", NewObjClear(CToolbarLayout)))
				break;
			if (!RegisterCommandPlugin(PLUGIN_ID_ITEM_SETTINGS, "settings", PLUGINFLAG_HIDE, nullptr, "settings", NewObjClear(CSettingsLayout)))
				break;
		} while (0);

		RegisterCommandPlugin(RES_EDIT_SEP_1, "#$25--", 0, nullptr, "#$25--", NewObjClear(CResEditSeparator));
		RegisterCommandPlugin(RES_EDIT_SEP_2, "#$65--", 0, nullptr, "#$65--", NewObjClear(CResEditSeparator));

	} while (0);

	g_pStringCompareIcons = BaseBitmap::Alloc();
	if (!g_pStringCompareIcons || g_pStringCompareIcons->Init(GeGetPluginPath() + String("res") + String("StringCompare.tif"))!=IMAGERESULT_OK)
		strError = "Couldn't load StringCompare.tif";

	if (strError != "")
	{
		MessageDialog(strError);
		return false;
	}

	g_pDialogDoc = NewObjClear(CDialogDoc);
	g_pDlgPreview = NewObjClear(CDialogPreview, g_pDialogDoc);
	g_pSettingsDlg = NewObjClear(CItemSettingsDialog, g_pDialogDoc);
	g_pTreeDialog = NewObjClear(CTreeDialog, g_pDialogDoc);
	g_pToolbar = NewObjClear(CResEditToolBar, g_pDialogDoc);
	g_pStringTransDialog = NewObjClear(CStringTranslateDialog);
	g_pStringReplace = NewObjClear(StringReplace);
	g_lNeedFileNew = 0;

	g_LanguageList.Init();

	g_pfnOriginal = NewObjClear(Filename);
	g_pfnNew = NewObjClear(Filename);

	*g_pfnOriginal = GeGetPluginPath();

	return true;
}


/*********************************************************************\
	Function name    : FreeMenuItems
	Description      :
	Created at       : 11.08.01, @ 09:48:32
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void FreeMenuItems()
{
	DeleteObj(ResEditHelp);
	DeleteObj(ResBrowserHelp);
	DeleteObj(ResCompareHelp);
	DeleteObj(ResHelpHelp);
	DeleteObj(g_pBrowserDialog);
	DeleteObj(g_pfnOriginal);
	DeleteObj(g_pfnNew);
	DeleteObj(g_pStringReplace);
	DeleteDialogs();

	if (g_pCustomElements)
	{
		g_pCustomElements->FreeList();
		DeleteObj(g_pCustomElements);
	}
}
