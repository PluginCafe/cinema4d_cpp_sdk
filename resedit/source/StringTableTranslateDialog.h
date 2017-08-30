// StringTableTranslateDialog.h :  Interface definition of class CStringTranslateDialog
//
//////////////////////////////////////////////////////////////////////

#if !defined STRINGTABLETRANSLATEDIALOG_H_3e284005
#define STRINGTABLETRANSLATEDIALOG_H_3e284005

#include "SearchDialog.h"

class CAvailStringTableItem : public CListItem
{
public:
	String strName;
	Filename fnAbsPath;
	Filename fnRelPath;
	Bool bIsOld, bIsNew, bModified;
	StringTableType type;
	String strType;
	Filename fnOldAbs, fnNewAbs;
	String strMissing;
	Bool bHasErrors;
};

class CStringTableItem : public CListItem
{
public:
	String strID;
	String strUSOld, strUSCommandOld, strUSNew, strUSCommandNew, strCountry, strCountryCommand;
	Bool bIsNew, bIsOld;
	Int32 lIcon;
	Int32 lIndex;
};

struct tagTreeViewSort
{
	CAvailStringTableItem* pItem;
	Int32 lIndex;
};

struct tagTreeViewStringElement
{
	Int32 lIndex;
	CStringTableItem* pItem;
};

class CStringSet;
class CStringTranslateDialog : public GeDialog
{
public:
	CStringTranslateDialog();
	virtual ~CStringTranslateDialog();

	static Int32 CompareTreeViewNameUp(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewNameDown(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewDirUp(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewDirDown(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewCreatedUp(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewCreatedDown(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewDeletedUp(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewDeletedDown(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewModifiedUp(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewModifiedDown(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewMissingUp(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewMissingDown(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewTypeUp(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareTreeViewTypeDown(const tagTreeViewSort *a, const tagTreeViewSort *b);
	static Int32 CompareStringTableItem(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);
	static Int32 CompareStringsTreeViewIDUp(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);
	static Int32 CompareStringsTreeViewIDDown(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);
	static Int32 CompareStringsTreeViewModifiedUp(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);
	static Int32 CompareStringsTreeViewModifiedDown(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);
	static Int32 CompareStringsTreeViewUSOldUp(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);
	static Int32 CompareStringsTreeViewUSOldDown(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);
	static Int32 CompareStringsTreeViewUSNewUp(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);
	static Int32 CompareStringsTreeViewUSNewDown(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);
	static Int32 CompareStringsTreeViewCountryUp(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);
	static Int32 CompareStringsTreeViewCountryDown(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);
	static Int32 CompareStringsByIndex(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b);

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);
	virtual Bool InitValues(void);
	virtual Bool AskClose();
	GeUserArea* GetTablesTreeViewArea() { return m_pAvailStringTablesTree->GetTreeViewArea(); }
	CDoubleLinkedList <CAvailStringTableItem>* GetAvailTables() { return &m_availStringTables; }
	CDoubleLinkedList <CStringTableItem>* GetTableElements() { return &m_TableElements; }
	void SelectString(CStringTableItem* pItem);
	void DeleteUnusedStringTables();
	void FindString(const String &str, RegularExprParser* pParser, Bool bOnlyCurrent, Bool bCaseSensitive);

	CAvailStringTableItem* m_pSelectedItem;
	CStringTableItem* m_pSelectedString;

	Bool LoadStringTable(CAvailStringTableItem* pTable);
	Bool LoadStringTable(Filename fn, Int32 l, CStringSet* pStringSet, CDynamicStringArray* pErrors);
	Bool AskModified(Bool bAsk = true);
	Bool SaveStringTable(Bool bOutputSaveLog);

	Int32 m_lLastSortCol, m_lLastSortDir;
	Int32 m_lLastStringSortCol, m_lLastStringSortDir;

protected:
	TreeViewCustomGui* m_pAvailStringTablesTree;
	TreeViewCustomGui* m_pStringsTree;

	Filename m_fnOld, m_fnNew;
	Bool m_bWasChanged;
	Int32 m_lSelLanguage;

	Bool CheckStringTables();
	Bool BrowseStringTables(const Filename &fnAbs, const Filename &fnRel, Bool bOld);
	Filename ModifyPath(Filename fn, Bool bIsFilename = false);
	Filename ModifyPathBack(Filename fn, String strMod);
	void CheckModified();
	void CompareStringTables(Filename &fnOld, Filename &fnNew, Bool &bModified, Bool &bHasErrors);

	CDoubleLinkedList <CAvailStringTableItem> m_availStringTables;
	CDoubleLinkedList <CStringTableItem> m_TableElements;
	String m_strStringTableName;
	CSearchDialog m_dlgSearch;
	String m_strOrigLanguage, m_strOrigLanguagePath;
};

#endif // STRINGTABLETRANSLATEDIALOG_H_3e284005
