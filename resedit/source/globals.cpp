/*********************************************************************\
	File name        : globals.cpp
	Description      :
	Created at       : 11.08.01, @ 09:35:25
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "shellsort.h"
#include "customgui_datetime.h"
#include "dynarray.h"

#include <stdio.h>

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BaseBitmap *g_pControlImages = nullptr;
Bool g_bAutoUpdate = true;
char g_chFill;
CLanguageList g_LanguageList;
Int32 g_lNeedFileNew;
Filename* g_pLastOpenFile = nullptr;
CDoubleLinkedList <CCustomElements> *g_pCustomElements;
StringReplace* g_pStringReplace = nullptr;

// global settings
String* g_pstrFillSave = nullptr; // the complete fill string
Int32 g_lFillChars; // the number of characters
Bool g_bSaveStringIDs = false; // true, if the strings should be stored in c4d_symbols.h
BaseBitmap* g_pStringCompareIcons = nullptr;
ResEditPrefs *g_pResEditPrefs = nullptr;

/*********************************************************************\
	Funktionsname    : QuestionDialogYesNoCancel
	Beschreibung     :
	Rückgabewert     : Long
	Erstellt am      : 11.08.01, @ 13:05:30
	Argument         : String str
\*********************************************************************/
GEMB_R QuestionDialogYesNoCancel(String str)
{
	return GeOutString(str, GEMB_YESNOCANCEL | GEMB_ICONQUESTION);
}

#ifdef OWN_TRACE
#ifdef _DEBUG

#include <stdarg.h>

void TRACE(const char* lpszFormat, ...)
{
	// some parts of this code were taken from ::AfxTrace
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	char szBuffer[512];

	nBuf = _vsnprintf(szBuffer, sizeof(szBuffer), lpszFormat, args);
	OutputDebugString(szBuffer);

	va_end(args);
}

/*********************************************************************\
	Function name    : TRACE_STRING
	Description      :
	Created at       : 21.08.01, @ 17:36:17
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TRACE_STRING(String str)
{
	Int32 lLen = str.GetCStringLen();
	char* pCh = bNewDeprecatedUseArraysInstead<char>(lLen + 2);
	str.GetCString(pCh, lLen + 1);
	TRACE("%s\n", pCh);
	delete [] pCh;
}

#endif // _DEBUG
#endif // OWN_TRACE

/*********************************************************************\
	Function name    : AddChildMultilineEditBox
	Description      :
	Created at       : 03.10.01, @ 18:22:10
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void AddChildMultilineEditBox(Int32 lIDText, Int32 lIDEditBox, GeDialog* pDlg)
{
	if (lIDText != 0)
		pDlg->AddStaticText(lIDText, BFH_LEFT | BFV_CENTER, 0, 0, GeLoadString(IDS_CHILD_ELEMENTS), 0);
	pDlg->AddMultiLineEditText(lIDEditBox, BFH_FIT, 300, 0, DR_MULTILINE_MONOSPACED);
}

Bool LineBreak(BaseFile* pFile, String strFill) { return (WriteString(pFile, "\r\n") && WriteString(pFile, strFill)); }
Bool WriteInt32(BaseFile* pFile, Int32 l) { return WriteString(pFile, String::IntToString(l)); }

/*********************************************************************\
	Function name    : WriteString
	Description      : writes a string without length information into a file
										 bReplace == true : ä ö ü will be replaced
	Created at       : 30.08.01, @ 10:35:54
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool WriteString(BaseFile* pFile, String str, Bool bReplace)
{
	Int32 lPos, lStart = 0;
	Int32 lLen = str.GetLength();
	if (bReplace)
	{
		while (str.FindFirst('\"', &lPos, lStart))
		{
			str = str.SubStr(0, lPos) + "$\"" + str.SubStr(lPos + 1, lLen - lPos - 1);
			lStart = lPos + 2;
			lLen++;
		}
	}

	lLen = str.GetCStringLen(STRINGENCODING_7BITHEX);
	char* pCh = bNewDeprecatedUseArraysInstead<char>(lLen + 2);
	str.GetCString(pCh, lLen + 1, STRINGENCODING_7BITHEX);

	for (Int32 l = 0; l < lLen; l++)
		pFile->WriteChar(pCh[l]);

	bDelete(pCh);

	return true;
}

/*********************************************************************\
	Function name    : ReadString
	Description      :
	Created at       : 05.09.01, @ 16:33:33
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool ReadString(BaseFile* pFile, String &str)
{
	char ch[2], chLast = 0;
	str = "";
	ch[1] = 0;

	while (pFile->ReadChar(ch))
	{
		if ((chLast == '\r' && *ch == '\n') || (chLast == '\n' && *ch == '\r'))
			return true;

		if (*ch != '\r' && *ch != '\n')
			str += String(ch);

		chLast = *ch;
	}

	return false;
}

/*********************************************************************\
	Function name    : TrimNumbers
	Description      : deletes the numbers at the end of a string
	Created at       : 05.01.02, @ 10:33:17
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
String TrimNumbers(String str)
{
	String strRet;
	String str1;

	Int32 lLen = str.GetLength();
	Int32 l;
	for (l = lLen - 1; l >= 0; l--)
	{
		str1 = str.SubStr(l, lLen - l);
		if (!IsInteger(str1)) break;
	}

	if (l == -1) strRet = str1;
	else strRet = str.SubStr(0, l + 1);

	return strRet;
}

/*********************************************************************\
	Function name    : IsInteger
	Description      : checks, if a string consits only of 0...9
	Created at       : 05.01.02, @ 12:04:32
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool IsInteger(String str)
{
	Int32 lLen = str.GetLength();
	Char pCh[1000];
	str.GetCString(pCh, 1000);
	for (Int32 l = 0; l < lLen; l++)
	{
		if (pCh[l] < '0' || pCh[l] > '9') return false;
	}

	return true;
}

/*********************************************************************\
	Function name    : SkipChar
	Description      :
	Created at       : 06.01.02, @ 19:18:34
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void SkipChar(String &str, UInt16 ch)
{
	Int32 lPos;
	while (str.FindFirst(ch, &lPos) && lPos == 0)
		str = str.SubStr(1, str.GetLength() - 1);
}

BasePlugin *CheckCustomGuiPlugins(BasePlugin *plug, Char* pCh)
{
	for (; plug; plug = (BasePlugin*)plug->GetNext())
	{
		if (plug->GetPluginType() == PLUGINTYPE_CUSTOMGUI)
		{
			const Char *ressym = CallCustomGuiCmd(plug, GetResourceSym)();
			if (!ressym) return nullptr;

			if (String(ressym) == String(pCh)) return plug;
		}
		BasePlugin* pl = CheckCustomGuiPlugins((BasePlugin*)plug->GetDown(), pCh);
		if (pl) return pl;
	}

	/*if (!plug) return nullptr;

	if (plug->GetType() == PLUGINTYPE_CUSTOMGUI)
	{
		const Char *ressym = CallCustomGuiCmd(plug, GetResourceSym)();
		if (!ressym) return nullptr;

		if (String(ressym) == String(pCh)) return plug;
	}

	return CheckCustomGuiPlugins((BasePlugin*)plug->GetDown(), pCh);*/
	return nullptr;
}

BasePlugin* FindCustomGUIPlugin(CCustomElements* pPlug, Int32 id)
{
	if (!pPlug) return nullptr;
	if (pPlug->m_lID == id) return pPlug->m_pPlug;
	return FindCustomGUIPlugin((CCustomElements*)g_pCustomElements->Next(pPlug), id);
}


void RecSearchPlugin(BasePlugin *plug)
{
	for (; plug; plug = (BasePlugin*)plug->GetNext())
	{
		if (plug->GetPluginType() == PLUGINTYPE_CUSTOMGUI)
		{
			Int32 id = plug->GetID();
			const Char *ressym = CallCustomGuiCmd(plug, GetResourceSym)();
			CustomProperty *prop = CallCustomGuiCmd(plug, GetProperties)();

			CCustomElements* pNew = NewObjClear(CCustomElements);
			pNew->m_pPlug = plug;
			pNew->m_strName = plug->GetName();
			pNew->m_pChResSym = ressym;
			pNew->m_pProp = prop;
			pNew->m_lID = id;
			g_pCustomElements->Append(pNew);

			/*for (i = 0; prop && prop[i].type != CUSTOMTYPE_END; i++)
			{
			}*/
		}
		RecSearchPlugin((BasePlugin*)plug->GetDown());
	}
}

Bool bCreatedElementList = false;

Int32 SortCustomGuiByName(const Int32 a, const Int32 b)
{
	CCustomElements* pa = g_pCustomElements->GetItem(a);
	CCustomElements* pb = g_pCustomElements->GetItem(b);
	if (!pa || !pb) return 0;
	return pa->m_strName.Compare(pb->m_strName);
}

/*********************************************************************\
	Function name    : CreateElementList
	Description      :
	Created at       : 26.03.02, @ 15:33:10
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CreateElementList()
{
	Int32 a;
	if (bCreatedElementList) return;
	bCreatedElementList = true;

	g_pCustomElements = NewObjClear(CDoubleLinkedList<CCustomElements>);
	if (!g_pCustomElements) return;

	RecSearchPlugin(GetFirstPlugin());

	Int32 lElementCount = g_pCustomElements->Entries();
	if (!lElementCount) return;
	AutoArray(plElements, Int32, lElementCount);
	AutoArray(plCustomElements, CCustomElements*, lElementCount);
	//AutoArray <Int32> plElements(lElementCount);
	//AutoArray <CCustomElements*> plCustomElements(lElementCount);
	if (!plElements || !plCustomElements) return;

	for (a = 0; a < lElementCount; a++)
		plElements[a] = a;

	CShellSort <Int32> s;
	s.Sort(plElements, SortCustomGuiByName, lElementCount);

	for (a = 0; a < lElementCount; a++)
		plCustomElements[a] = g_pCustomElements->GetItem(plElements[a]);
	g_pCustomElements->UnlinkList();
	g_pCustomElements->FromArray(plCustomElements, lElementCount);


	/*BasePlugin* plug = FindPlugin(DESCRIPTIONCUSTOMGUIDATA, PLUGINTYPE_CUSTOMGUI);

	if (plug)
	{
		Int32 i;
		const Char *ressym = CallCustomGuiCmd(plug, GetResourceSym)();
		CustomGuiProperty *prop = CallCustomGuiCmd(plug, GetProperties)();
		for (i = 0; prop && prop[i].type != CUSTOMTYPE_END; i++)
		{
		}
	}*/
}

class CFilenameArray : public CDynamicObjectArray<Filename>
{
public:
	virtual ~CFilenameArray() { Free(); }
protected:
	virtual Filename* AllocObject() const { return NewObjClear(Filename); }
	virtual void FreeObject(Filename *&pObj) const { DeleteObj(pObj); }
};

Bool AssertPathExists(const Filename &fn)
{
	if (GeFExist(fn, true))
		return true;

	CFilenameArray pfnDir;
	Filename fnDir = fn;
	Bool bRet = false;
	Int32 a;
	Filename** ppfn;

	while (!GeFExist(fnDir, true))
	{
		Filename* pfn = pfnDir.InsertAt(0);
		if (!pfn)
			goto _error;
		*pfn = fnDir.GetFile();
		fnDir = fnDir.GetDirectory();
		if (!fnDir.Content())
		{
			return false;
		}
	}

	ppfn = pfnDir.GetArray()->GetArray();
	for (a = 0; a < pfnDir.GetElementCount(); a++)
	{
		fnDir += fnDir + *(ppfn[a]);
		if (GeFExist(fnDir, true))
			continue;
		if (!GeFCreateDir(fnDir))
			goto _error;
	}

	bRet = true;
_error:
	return bRet;
}

void AddFilename1(Filename &fn, const Filename fnDir, const Filename fnFile)
{
	if (fnDir.Content())
		AddFilename1(fn, fnDir.GetDirectory(), fnDir.GetFile());
	fn += fnFile;
}

void AddFilename(Filename &fn, const Filename fnDest)
{
	if (!fnDest.Content())
		return;
	AddFilename1(fn, fnDest.GetDirectory(), fnDest.GetFile());
}



void StringReplace::Init(const String &strBuildVersion)
{
	DateTime dt;
	BaseContainer bc;
	char ch[100];

	GetDateTimeNow(dt);
	sprintf_safe(ch, sizeof(ch), "%4d%02d%02d", (int)dt.year, (int)dt.month, (int)dt.day);
	m_strDate = ch;
	sprintf_safe(ch, sizeof(ch), "%02d%02d%02d", (int)dt.hour, (int)dt.minute, (int)dt.second);
	m_strTime = ch;
	m_strDateTime = m_strDate + "_" + m_strTime;
	Int32 lVersion = GetC4DVersion();
	sprintf_safe(ch, sizeof(ch), "%.3f", ((Float32)lVersion) / 1000.0f);
	m_strCurrentVersion = ch;
	m_strStartupPath = GeGetStartupPath().GetString();
	m_strPluginPath = GeGetPluginPath().GetString();
	m_strBuildVersion = strBuildVersion;
	bc = GetMachineFeatures();
	m_strReleaseLine = bc.GetString(1000001);
	m_strBuildID = bc.GetString(1000002);
}

void StringReplace::Replace(String &str)
{
	// test for environment variables
	Int32 lStart, lEnd, lStartSearch = 0;
	Int32 lStringLen;
	String strExpr;
	char* pchEnvVar;
	char pszVar[1000];
	BaseContainer bc;

	while (str.FindFirst("($", &lStart, lStartSearch))
	{
		if (!str.FindFirst("$)", &lEnd, lStart))
			return;
		lStartSearch = lStart + 1;

		strExpr = str.SubStr(lStart + 2, lEnd - lStart - 2);
		lStringLen = strExpr.GetCStringLen(STRINGENCODING_8BIT);
		Char* pchString = NewMem(Char,lStringLen + 2);
		if (!pchString)
			return;
		strExpr.GetCString(pchString, lStringLen + 1, STRINGENCODING_8BIT);
		pchEnvVar = GetOSEnvironmentVariable(pchString, pszVar, 1000);
		DeleteMem(pchString);
		if (pchEnvVar)
			str = str.SubStr(0, lStart) + String(pchEnvVar) + str.SubStr(lEnd + 2, str.GetLength() - lEnd - 2);
	}

	FindAndReplace(str, String("($CINEMA_4D_ROOT$)"), m_strStartupPath);
	FindAndReplace(str, String("($CINEMA_4D_PLUGIN$)"), m_strPluginPath);
	FindAndReplace(str, String("($CINEMA_4D_VERSION$)"), m_strCurrentVersion);
	FindAndReplace(str, String("($CURRENT_DATE$)"), m_strDate);
	FindAndReplace(str, String("($CURRENT_TIME$)"), m_strTime);
	FindAndReplace(str, String("($CURRENT_DATE_TIME$)"), m_strDateTime);
	FindAndReplace(str, String("($BUILD_VERSION$)"), m_strBuildVersion);
	FindAndReplace(str, String("($RELEASE_LINE$)"), m_strReleaseLine);
	FindAndReplace(str, String("($BUILD_ID$)"), m_strBuildID);
}

void StringReplace::FindAndReplace(String &str, const String &strFind, const String &strReplace)
{
	Int32 lStart = 0;
	Int32 lPos;
	while (str.FindFirst(strFind, &lPos, lStart))
	{
		str = str.SubStr(0, lPos) + strReplace + str.SubStr(lPos + strFind.GetLength(), str.GetLength() - lPos - strFind.GetLength());
		lStart = lPos;
	}
}

Bool ForceOpenFileWrite(BaseFile* pFile, const Filename &fn)
{
	if (pFile->Open(fn, FILEOPEN_WRITE))
		return true;
	String str = GeLoadString(IDS_WRITE_PROTECTION, fn.GetFileString());
	if (GeOutString(str, GEMB_ICONQUESTION | GEMB_YESNO) != GEMB_R_YES)
		return false;
	GeFKill(fn, GE_FKILL_FORCE);
	return pFile->Open(fn, FILEOPEN_WRITE);
}
