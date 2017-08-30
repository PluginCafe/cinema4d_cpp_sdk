// DialogDoc.h: Schnittstelle für die Klasse CDialogDoc.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIALOGDOC_H__EF630B20_8E49_11D5_9B3B_004095418E0F__INCLUDED_)
#define AFX_DIALOGDOC_H__EF630B20_8E49_11D5_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDialogPreview;
class CItemSettingsDialog;
class CTreeDialog;
class CResEditToolBar;
class CDialogItem;
class CDialogTemplate;

#include "DialogIO.h"
#include "StringTable.h"

#define DLG_IO_GOTO_ERROR(expr) { m_lErrorID = (expr); GOTO_ERROR; }

class CDialogDoc
{
	friend class CDialogPreview;
public:
	CDialogDoc();
	virtual ~CDialogDoc();

	CDialogPreview* m_pDlgPreview;
	CItemSettingsDialog* m_pItemSettingsDlg;
	CTreeDialog* m_pTreeDlg;
	CResEditToolBar* m_pToolbar;

	void OnFileNew();
	void OnFileOpen();
	Bool OnFileSave();
	Bool OnFileSaveAs();
	void OnFileExportMacro();
	void OnFileImportMacro();

	void OnEditDelete();
	void OnEditCut();
	void OnEditCopy();
	void OnEditPaste();

	Bool LoadDialogResource(const Filename &filename);
	Bool SaveDialogResource(const Filename &filename);
	void ChangeLanguage(String strLangID, Bool bGlobalTable = false);
	Bool CloseEditor();

	void SetChanged(Bool bChanged = true);
	CDialogTemplate* GetDialog() { return m_pDialog; }
	void SelChanged(CDialogItem* pItem, Bool bUpdateTree = true, Bool bMakeVisible = true);
	void SelectItem(Int32 lID);
	CDialogItem* SearchItem(TreeViewItem* pItem, Int32 lID);
	void LoadGlobalStringTable(Filename fnProjectDir, String strTitle);
	void SaveGlobalStringTable();

	CDialogItem* AddItem(ItemType t, Bool bAddAsChild = true, Bool bAddText = false, Bool bMakeVisible = true);
	void UpdatePreview();
	void CreateElements(Int32 lID, GeDialog* pDlg);
	void CreateElements(Int32 &lID, TreeViewItem* pItem, GeDialog* pDlg);
	void InitValues(TreeViewItem* pItem);
	void UpdateAllViews(Bool bUpdateTree = false, Bool bUpdatePreview = true);
	String GetString(String strID, Bool &bFound);
	CStringTable* GetStringTable() { return &m_StringTable; }
	void StoreNamesInStringtable();
	void SetStringtableItem(String strID, String strText);
	inline Int32 GetLanguageIDList() { return m_lLanguageIDList; }
	inline Int32 GetLanguageIDTable() { return m_lLanguageIDTable; }
	inline CDialogItem* GetSelectedItem() { return m_pCurrentItem; }
	void ItemChanged();
	void ExportMacro(Filename fnMacro, CDialogItem* pExportFrom);
	CDialogItem* ImportMacro(Filename fnMacro, CDialogItem* pInsertAfter);

	void OpenDialog(Filename fn);
	void DeleteUnusedStrings();
	void ModifySymbolFile(Filename fnPath, String strSymbols, String strTemp);

	Int32 m_lLanguageIDList; // the ID of the language in g_LanguageList
	Int32 m_lLanguageIDTable; // the language ID that must be passed as a parameter to CStringTable functions

	StringTableType m_StringTableType;
	Bool m_bStringSymbolsChanged, m_bControlSymbolsChanged;

protected:
	Bool m_bIsLoading;

	Filename m_FileName, m_fnStringTableStart;
	Bool m_bChanged;

	UInt32 m_nLoadPos;

	Int32 m_lAddItemCount, m_lItemCount;

	Bool LoadDialogParts(DialogParser &p, BaseContainer *pDialogInfo, Int32 type, PROGRESS_ID	progressID);
	CDialogItem* AddItems(BaseContainer *pItemInfo, CDialogItem* pInsertAfter, PROGRESS_ID progressID, Bool bCheckNames, Bool bMakeVisible);
	Bool LoadStringTables(Filename fnStart, String strTitle, StringTableType stt);
	Bool LoadStringTable(Filename fnString, String strSuffix, String strTitle, StringTableType stt);
	Bool LoadDialogStrings(DialogParser &p, Int32 lLang, StringTableType stt);
	void LanguageChanged();
	Bool SaveStringTable(StringTableType stt);
	Bool SaveHeaderFile(Bool bGlobalStringTable);
	void WriteStringsIDs(BaseFile* pDestFile, String strStringBegin, String strStringEnd, Bool bIsGlobalTable);
	void WriteDialogIDs(BaseFile* pDestFile, String strDialogBegin, String strDialogEnd);
	Bool ImportLoadStringTable(DialogParser &p, CStringTable *pStringTable);
	Bool CheckFilename(Filename &fn);

	CDialogTemplate *m_pDialog; // holds all children
	CStringTable m_StringTable;
	CDialogItem* m_pCurrentItem;

	Int32 m_lErrorID;
	Filename m_fnErrorFile;

	String UpdateDialogCaptions();
	GEMB_R AskSave();
};

#endif // !defined(AFX_DIALOGDOC_H__EF630B20_8E49_11D5_9B3B_004095418E0F__INCLUDED_)
