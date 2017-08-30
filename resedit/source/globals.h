/*********************************************************************\
	File name        : globals.h
	Description      :
	Created at       : 11.08.01, @ 09:30:43
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#ifndef GLOBALS_H__
#define GLOBALS_H__

#define PLUGIN_ID_HELP_MENU           1007376
#define PLUGIN_ID_COMPARE_MENU        1006667
#define PLUGIN_ID_BROWSER             1006445
#define PLUGIN_ID_TOOLBAR             1006046
#define PLUGIN_ID_MENU_ITEM           1006045
#define PLUGIN_ID_DIALOG_PREVIEW      1006044
#define PLUGIN_ID_ITEM_SETTINGS       1006043
#define PLUGIN_ID_TREE_DIALOG         1006042
#define PLUGIN_ID_EDIT_DISTRI         1011154
#define ID_STRING_TABLE_TRANS_DIALOG  1010990
#define RES_EDIT_EXTRACT_ZIP          1011159
#define RES_EDIT_SEP_1                1011174
#define RES_EDIT_SEP_2                1011176
#define SPACE_NEW_LINE                *g_pstrFillSave//"  " // the spaces that will be added to each new line

#define TREEVIEW_DATA       10

#include "c4d_customguidata.h"
#include "TreeView.h"
#include "LanguageList.h"
#include "lib_regexpr.h"
#include "dynarray.h"

#define GOTO_ERROR { goto error; }

String OldFormatString(const char* pChFormat, ...);

#define VERSION_8

class CustomGuiPlugin;
class StringReplace;
enum ItemType { unknown = -1, Dialog = 0, Static, Group, CheckBox, RadioBtn, Button, ArrowBtn, EditBox, Slider,
								Color, Separator, ListBox, ComboBox, DialogGroup, UserArea, SubDialogControl, CustomElement, ComboButton,
								LastControlElement }; // just a dummy value
extern BaseBitmap *g_pControlImages;
extern Bool g_bAutoUpdate;
extern String* g_pstrFillSave; // the complete fill string
extern Int32 g_lFillChars; // the number of characters
extern char g_chFill;
extern CLanguageList g_LanguageList;
extern Int32 g_lNeedFileNew;
extern Filename* g_pLastOpenFile;
extern Bool g_bSaveStringIDs;
extern BaseBitmap* g_pStringCompareIcons;
extern StringReplace* g_pStringReplace;

enum StringTableType { typeDialogStrings, typeStringTable, typeCommandStrings, typeUnknown };

GEMB_R QuestionDialogYesNoCancel(String str);
void AddBorderComboBox(Int32 lIDText, Int32 lIDCombo, GeDialog* pDlg);
Bool WriteString(BaseFile* pFile, String str, Bool bReplace = false);
Bool ReadString(BaseFile* pFile, String &str);
Bool LineBreak(BaseFile* pFile, String strFill);
Bool WriteInt32(BaseFile* pFile, Int32 l);
void AddChildMultilineEditBox(Int32 lIDText, Int32 lIDEditBox, GeDialog* pDlg);
String TrimNumbers(String str);
Bool IsInteger(String str);
void SkipChar(String &str, UInt16 ch);
void CreateElementList();
BasePlugin *CheckCustomGuiPlugins(BasePlugin *plug, Char* pCh);
void SelectPath(Filename* pfn);
Bool AssertPathExists(const Filename &fn);
void AddFilename(Filename &fn, const Filename fnDest);
char* GetOSEnvironmentVariable(const char* pszVarName, char* pszVar, unsigned int nVarLen);
Bool ForceOpenFileWrite(BaseFile* pFile, const Filename &fn);
Bool ReadPreset(const Filename& fnPreset);

struct ResEditPrefs
{
	String strOrigPath, strNewPath, strExtractPath, strWizardPath;
	Filename fnNewZip, fnDestZip;
	CDynamicFilenameSet arOldFiles;
};
extern ResEditPrefs *g_pResEditPrefs;

#define CONVERT_WIDTH(value)  ((value < 0) ? SizePix(-value) : SizeChr(value))
#define CONVERT_HEIGHT(value) ((value < 0) ? SizePix(-value) : SizeChr(value))

class StringReplace
{
public:
	void Init(const String &strBuildVersion);
	void Replace(String &str);
protected:
	void FindAndReplace(String &str, const String &strFind, const String &strReplace);
	String m_strDate, m_strTime, m_strDateTime, m_strCurrentVersion;
	String m_strStartupPath, m_strPluginPath, m_strBuildVersion, m_strReleaseLine, m_strBuildID;
};


class CCustomElements : public CListItem
{
public:
	BasePlugin* m_pPlug;
	String m_strName;
	const Char* m_pChResSym;
	CustomProperty *m_pProp;
	Int32 m_lID;

	Bool m_bIsOpen;
};
extern CDoubleLinkedList <CCustomElements> *g_pCustomElements;

enum ErrorCodes
{
	ERR_EXPECTED_IDENT = 1,
	ERR_EXPECTED_GESCHWEIFT_AUF,
	ERR_EXPECTED_STRING,
	ERR_EXPECTED_SEMIKOLON,
	ERR_EXPECTED_NUMBER,
	ERR_EXPECTED_KOMMA,
	ERR_UNKNOWN_ID,
	ERR_FILENAME_DIALOGNAME_NO_MATCH,

	ERR_DUMMY
};

class CSubSettingsDialog
{
public:
	CSubSettingsDialog() { m_lTabPageID = -1; m_bInit = false; }
	virtual void SetData() = 0;
	void _Init() { if (!m_bInit) { m_bInit = true; Init(); } }
	GeDialog* m_pSettingsDialog;
	Int32 m_lTabPageID;
	void Invalidate() { m_bInit = false; }

	inline Bool SetBool(Int32 id, Bool value,Int32 tristate=0) { return m_pSettingsDialog->SetBool(id, value, tristate); }
	inline Bool SetInt32(Int32 id, Int32 value,Int32 min=-LIMIT<Int32>::MAX,Int32 max=LIMIT<Int32>::MAX,Int32 step=1,Int32 tristate=0) { return m_pSettingsDialog->SetInt32(id, value, min, max, step, tristate); }
	inline Bool SetString(Int32 id, const String &text,Int32 tristate=0) { return m_pSettingsDialog->SetString(id, text, tristate); }

	inline Bool GetBool(Int32 id, Bool &value) { return m_pSettingsDialog->GetBool(id, value); }
	inline Bool GetInt32(Int32 id, Int32 &value) { return m_pSettingsDialog->GetInt32(id, value); }
	inline Bool GetString(Int32 id, String &value) { return m_pSettingsDialog->GetString(id, value); }

	inline Bool Enable(Int32 id, Bool enabled) { return m_pSettingsDialog->Enable(id, enabled); }

	virtual void Init() { }

	Bool m_bInit;
};

#if (!defined TRACE) && defined _WINDOWS
	#if !defined(_KERNEL32_)
		#define WINBASEAPI __declspec(dllimport)
	#else
		#define WINBASEAPI
	#endif

	extern "C" {
		// we get OutputDebugStringA from kernel32.lib
		WINBASEAPI void __stdcall OutputDebugStringA(const char* pchOutputString);
		#define OutputDebugString OutputDebugStringA
	}

	// for _ASSERT macros
	//#include <crtdbg.h>
#define ASSERT(expr)    DebugAssert(expr)

	#define OWN_TRACE
	#ifdef _DEBUG
		void TRACE(const char* lpszFormat, ...);
		void TRACE_STRING(String str);
	#else
		inline void TRACE(const char* lpszFormat, ...) { }
		inline void TRACE_STRING(String str) { }
	#endif
#else
	inline void TRACE(const char* lpszFormat, ...) { }
	inline void TRACE_STRING(String str) { }
	#define ASSERT(expr) { }
#endif

#endif	// GLOBALS_H__
