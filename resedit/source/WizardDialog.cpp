// WizardDialog.cpp
//////////////////////////////////////////////////////////////////////

#define ID_RES_EDIT_WIZARD			450000027

#include "c4d.h"
#include "c4d_symbols.h"
#include "customgui_filename.h"
#include "WizardDialog.h"
#include "res_textparser.h"
#include "globals.h"

#define MAX_SCRIPTS						1000
#define MAX_LINES							100
#define GROUP_SIZE						1000

#define IDC_TAB_GROUP							9000
#define IDC_PATH_NAME							9001
#define IDC_FIRST_SCRIPT_GROUP		10000
#define IDC_FIRST_LINE						20000
#define IDC_FIRST_OPTION					20100

#define USERINPUT_TEXT					0
#define USERINPUT_OPTION				1

class ScriptOption
{
public:
	ScriptOption();
	virtual ~ScriptOption();

	String strID;
	String strName;
	Bool bDefault;

	Bool Modify();
	Bool UserInput(const String &str);
	Bool UserInput(Bool b);
	char* pchID;//, *pchName;

	// user input
	char* pchString;
	Bool bSet;
	Int32 lUserInput;
};

class ScriptOptions : public CDynamicObjectArray<ScriptOption>
{
public:
	virtual ~ScriptOptions()
	{
		Free();
	}

protected:
	virtual ScriptOption* AllocObject() const
	{
		return NewObjClear(ScriptOption);
	}
	virtual void FreeObject(ScriptOption *&pObj) const
	{
		DeleteObj(pObj);
	}
};

class ScriptLine
{
public:
	ScriptOptions opt;
};

class ScriptLines : public CDynamicObjectArray<ScriptLine>
{
public:
	virtual ~ScriptLines()
	{
		Free();
	}

protected:
	virtual ScriptLine* AllocObject() const
	{
		return NewObjClear(ScriptLine);
	}
	virtual void FreeObject(ScriptLine *&pObj) const
	{
		DeleteObj(pObj);
	}
};


class DataSet : public CDynamicSortArray<ScriptOption*>
{
public:
	virtual Int32 Compare(ScriptOption* const* a, ScriptOption* const* b) const
	{
		DebugAssert((*a)->pchID);
		DebugAssert((*b)->pchID);
		return strcmp((*a)->pchID, (*b)->pchID);
	};
	virtual Int32 CompareSpecial(ScriptOption* const* a, const void* pSpecial) const
	{
		DebugAssert((*a)->pchID);
		return strcmp((*a)->pchID, (char*)pSpecial);
	}
};

class ScriptData
{
public:
	String m_strName;
	ScriptLines m_Text;
	ScriptLines m_Options;

	Bool Load(const Filename &fn, String &strError);
	Bool Modify(GeDialog* pDlg, Int32 lScript);
	ScriptOption* Find(const char* pchString);

	DataSet m_Data;
};

class Scripts : public CDynamicSortObjectArray<ScriptData>
{
public:
	virtual ~Scripts()
	{
		Free();
	}

protected:
	virtual Int32 Compare(const ScriptData* a, const ScriptData* b) const
	{
		return a->m_strName.LexCompare(b->m_strName);
	}
	virtual ScriptData* AllocObject() const
	{
		return NewObjClear(ScriptData);
	}
	virtual void FreeObject(ScriptData *&pObj) const
	{
		DeleteObj(pObj);
	}

public:
	Bool AddScript(const Filename &fn, String &strError);
};

Scripts g_scripts;

String GetSuffix(const Filename &fn)
{
	Filename f = fn;
	f.ClearSuffix();
	Int32 lSuffixLen = fn.GetString().GetLength() - f.GetString().GetLength();
	if (lSuffixLen <= 1)
		return "";
	lSuffixLen--;
	return fn.GetString().SubStr(fn.GetString().GetLength() - lSuffixLen, lSuffixLen);
}

Bool Scripts::AddScript(const Filename &fn, String &strError)
{
	Filename tmp;
	tmp = fn;
	tmp.ClearSuffix();
	String str = tmp.GetFileString();
	ScriptData sd, *pScriptData;
	sd.m_strName = str;

	pScriptData = g_scripts.Add(&sd);
	if (!pScriptData)
	{
		strError += GeLoadString(IDS_WIZARD_NO_MEM) + "|";
		return false;
	}

	pScriptData->m_strName = str;
	if (!pScriptData->Load(fn, strError))
		g_scripts.RemoveObject(pScriptData);

	return true;
}

ScriptOption::ScriptOption()
{
	pchID = nullptr;
	pchString = nullptr;
	lUserInput = -1;
	//pchName = nullptr;
}

ScriptOption::~ScriptOption()
{
	DeleteMem(pchID);
	DeleteMem(pchString);
	//DeleteMem(pchName);
}

Bool ScriptOption::Modify()
{
	Int32 lLen;

	lLen = strID.GetCStringLen(STRINGENCODING_8BIT);
	pchID = NewMemClear(Char,lLen + 2);
	if (!pchID)
		return false;
	strID.GetCString(pchID, lLen + 1, STRINGENCODING_8BIT);

	/*lLen = strName.GetCStringLen(STRINGENCODING_8BIT);
	pchName = NewMemClear(Char,lLen + 1);
	if (!pchName)
		return false;
	strName.GetCString(pchName, lLen, STRINGENCODING_8BIT);*/

	return true;
}

Bool ScriptOption::UserInput(const String &str)
{
	Int32 lLen = str.GetCStringLen(STRINGENCODING_8BIT);
	pchString = NewMemClear(Char,lLen + 2);
	if (!pchString)
		return false;
	str.GetCString(pchString, lLen + 1, STRINGENCODING_8BIT);
	lUserInput = USERINPUT_TEXT;
	return true;
}

Bool ScriptOption::UserInput(Bool b)
{
	bSet = b;
	lUserInput = USERINPUT_OPTION;
	return true;
}

#define SECTION_NONE		-1
#define SECTION_TEXT		0
#define SECTION_OPTION  1

Bool ScriptData::Load(const Filename &fn, String &strError)
{
	ASCIIParser p;
	if (p.Init(fn, false, true))
	{
		strError += GeLoadString(IDS_WIZARD_FILE_ERROR) + " " + fn.GetString() + "|";
		return false;
	}

	SYMBOL sym = p.GetSymbol();
	String str, strName, strID;
	Int32 lGeschweift = 0;
	Int32 lSection = SECTION_NONE;
	Bool bDefault = false;
	ScriptLine* pLine = nullptr;
	//ScriptOption* pOption = nullptr;

	while (!p.Error())
	{
		if (sym == SYM_OPERATOR_ECKIGAUF)
		{
			sym = p.nextsym();
			if (sym != SYM_IDENT || lGeschweift != 0)
			{
				strError += GeLoadString(IDS_WIZARD_PARSE_ERROR) + " " + fn.GetString() + ", " + String::IntToString(p.GetLine()) + "|";
				return false;
			}
			str += p.GetIdent();
			sym = p.nextsym();
			if (sym != SYM_OPERATOR_ECKIGZU)
			{
				strError += GeLoadString(IDS_WIZARD_PARSE_ERROR) + " " + fn.GetString() + ", " + String::IntToString(p.GetLine()) + "|";
				return false;
			}
			sym = p.nextsym();
			while (sym == SYM_OPERATOR_NEWLINE && !p.Error())
				sym = p.nextsym();
			if (str == "Text")
				lSection = SECTION_TEXT;
			if (str == "Option")
				lSection = SECTION_OPTION;
			str = "";
			continue;
		}
		if (lSection == SECTION_NONE)
		{
			sym = p.nextsym();
			continue;
		}

		// Zeile lesen
		if (sym == SYM_OPERATOR_GESCHWEIFTAUF)
		{
			lGeschweift++;
			if (lGeschweift > 1)
			{
				strError += GeLoadString(IDS_WIZARD_PARSE_ERROR) + " " + fn.GetString() + ", " + String::IntToString(p.GetLine()) + "|";
				return false;
			}
			sym = p.nextsym();
		}
		do
		{
			if (!pLine)
			{
				if (lSection == SECTION_TEXT)
				{
					pLine = m_Text.GetNextObject();
				}
				else if (lSection == SECTION_OPTION)
				{
					pLine = m_Options.GetNextObject();
				}
				if (!pLine)
				{
					strError += GeLoadString(IDS_WIZARD_NO_MEM) + " " + fn.GetString() + ", " + String::IntToString(p.GetLine()) + "|";
					return false;
				}
			}

			if (sym == SYM_OPERATOR_GESCHWEIFTZU)
			{
				lGeschweift--;
				if (lGeschweift != 0)
				{
					strError += GeLoadString(IDS_WIZARD_PARSE_ERROR) + " " + fn.GetString() + ", " + String::IntToString(p.GetLine()) + "|";
					return false;
				}
				sym = p.nextsym();
			}
			else
			{
				if (sym == SYM_OPERATOR_NEWLINE)
				{
					sym = p.nextsym();
					continue;
				}
				if (sym == SYM_STRING)
				{
					strName = p.GetIdent();
				}
				else if (sym == SYM_IDENT)
				{
					strName = p.GetIdent();
				}
				sym = p.nextsym();
				if (sym == SYM_OPERATOR_PLUS)
				{
					bDefault = true;
					sym = p.nextsym();
				}
				else
					bDefault = false;
				if (sym != SYM_OPERATOR_EQUAL)
				{
					strError += GeLoadString(IDS_WIZARD_PARSE_ERROR) + " " + fn.GetString() + ", " + String::IntToString(p.GetLine()) + "|";
					return false;
				}
				sym = p.nextsym();
				if (sym != SYM_IDENT)
				{
					strError += GeLoadString(IDS_WIZARD_PARSE_ERROR) + " " + fn.GetString() + ", " + String::IntToString(p.GetLine()) + "|";
					return false;
				}
				strID = p.GetIdent();
				sym = p.nextsym();

				ScriptOption* pOption = pLine->opt.GetNextObject();
				if (!pOption)
				{
					strError += GeLoadString(IDS_WIZARD_NO_MEM) + " " + fn.GetString() + ", " + String::IntToString(p.GetLine()) + "|";
					return false;
				}
				pOption->bDefault = bDefault;
				pOption->strID = strID;
				pOption->strName = strName;
			}
		} while (lGeschweift > 0);
		pLine = nullptr;
	}

	return true;
}

Bool ScriptData::Modify(GeDialog* pDlg, Int32 lScript)
{
	Int32 lLines;
	ScriptLine** ppLines;
	ScriptOption** ppOptions;
	String str;
	Bool bl;
	Int32 b, c, l, lOptions;

	lLines = LMin(MAX_LINES, m_Text.GetElementCount());
	ppLines = m_Text.GetArray()->GetArray();
	if (lLines > 0)
	{
		for (b = 0; b < lLines; b++)
		{
			ppOptions = ppLines[b]->opt.GetArray()->GetArray();
			lOptions = ppLines[b]->opt.GetArray()->GetElementCount();
			if (lOptions == 1)
			{
				pDlg->GetString(IDC_FIRST_LINE + lScript * GROUP_SIZE + b, str);
				if (!ppOptions[0]->UserInput(str))
					return false;
				if (!ppOptions[0]->Modify())
					return false;
				if (!m_Data.Append(ppOptions[0]))
					return false;
			}
		}
	}

	// Options
	lLines = LMin(MAX_LINES, m_Options.GetElementCount());
	ppLines = m_Options.GetArray()->GetArray();
	if (lLines > 0)
	{
		for (b = 0; b < lLines; b++)
		{
			ppOptions = ppLines[b]->opt.GetArray()->GetArray();
			lOptions = ppLines[b]->opt.GetArray()->GetElementCount();
			if (lOptions == 1)
			{
				pDlg->GetBool(IDC_FIRST_OPTION + lScript * GROUP_SIZE + b, bl);
				if (!ppOptions[0]->UserInput(bl))
					return false;
				if (!ppOptions[0]->Modify())
					return false;
				if (!m_Data.Append(ppOptions[0]))
					return false;
			}
			else
			{
				pDlg->GetInt32(IDC_FIRST_OPTION + lScript * GROUP_SIZE + b, l);
				for (c = 0; c < lOptions; c++)
				{
					if (!ppOptions[c]->UserInput(c == l))
						return false;
					if (!ppOptions[c]->Modify())
						return false;
					if (!m_Data.Append(ppOptions[c]))
						return false;
				}
			}
		}
	}
	m_Data.Sort();
	m_Data.DeleteDuplicates();

	ScriptOption* pClassName = Find("CLASSNAME");
	if (pClassName)
	{
		ScriptLine* pLine = m_Text.GetNextObject();
		if (!pLine)
			return false;
		ScriptOption* pNewOption = pLine->opt.GetNextObject();
		if (!pNewOption)
			return false;
		pNewOption->strID = "CLASSNAMEUPPER";
		pNewOption->strName = "";
		if (!pNewOption->Modify())
			return false;
		str = pClassName->pchString;
		str = str.ToUpper();
		if (!pNewOption->UserInput(str))
			return false;
		if (!m_Data.AddElement(pNewOption))
			return false;
	}

	return true;
}

ScriptOption* ScriptData::Find(const char* pchString)
{
	Int32 l = m_Data.FindSpecial(pchString);
	if (l < 0 || l >= m_Data.GetElementCount())
		return nullptr;
	return m_Data.m_pElements[l];
}





Bool BrowseScripts(String &strError)
{
	g_scripts.Free();

	Filename fn = GeGetPluginPath() + String("scripts");
	AutoAlloc <BrowseFiles> pBrowse;
	if (!pBrowse)
	{
		strError += GeLoadString(IDS_WIZARD_NO_MEM) + "|";
		return false;
	}

	pBrowse->Init(fn, false);
	while (pBrowse->GetNext())
	{
		if (pBrowse->IsDir())
			continue;
		g_scripts.AddScript(fn + pBrowse->GetFilename(), strError);
	}

	return true;
}

WizardDialog::WizardDialog()
{

}

WizardDialog::~WizardDialog()
{

}

Bool WizardDialog::CreateLayout()
{
	if (!GeModalDialog::CreateLayout())
		return false;

	Int32 a, b, c, lElements = LMin(MAX_SCRIPTS, g_scripts.GetElementCount());
	ScriptData** ppScripts = g_scripts.GetArray()->GetArray();
	ScriptLine** ppLines;
	ScriptOption** ppOptions;
	Int32 lLines, lOptions;

	SetTitle(GeLoadString(IDS_WIZARD_TITLE));

	BaseContainer bcData;
	bcData.SetBool(FILENAME_DIRECTORY, true);
	m_FileGUI = (FilenameCustomGui*)AddCustomGui(IDC_PATH_NAME, CUSTOMGUI_FILENAME, "", BFH_SCALEFIT, 0, 0, bcData);
	if (!m_FileGUI)
		return false;

	TabGroupBegin(IDC_TAB_GROUP, BFH_SCALEFIT);
	for (a = 0; a < lElements; a++)
	{
		GroupBegin(IDC_FIRST_SCRIPT_GROUP + a, BFH_SCALEFIT | BFV_TOP, 1, 0, ppScripts[a]->m_strName, 0);
			// Text
			lLines = LMin(MAX_LINES, ppScripts[a]->m_Text.GetElementCount());
			ppLines = ppScripts[a]->m_Text.GetArray()->GetArray();
			if (lLines > 0)
			{
				GroupBegin(101, BFH_SCALEFIT, 2, 0, "", 0);
					for (b = 0; b < lLines; b++)
					{
						ppOptions = ppLines[b]->opt.GetArray()->GetArray();
						lOptions = ppLines[b]->opt.GetArray()->GetElementCount();
						if (lOptions == 1)
						{
							AddStaticText(100, BFH_LEFT, 0, 0, ppOptions[0]->strName, 0);
							AddEditText(IDC_FIRST_LINE + a * GROUP_SIZE + b, BFH_SCALEFIT, 300);
						}
					}
				GroupEnd();
			}

			// Options
			lLines = LMin(MAX_LINES, ppScripts[a]->m_Options.GetElementCount());
			ppLines = ppScripts[a]->m_Options.GetArray()->GetArray();
			if (lLines > 0)
			{
				GroupBegin(101, BFH_SCALEFIT, 1, 0, "", 0);
					for (b = 0; b < lLines; b++)
					{
						ppOptions = ppLines[b]->opt.GetArray()->GetArray();
						lOptions = ppLines[b]->opt.GetArray()->GetElementCount();
						if (lOptions == 1)
						{
							AddCheckbox(IDC_FIRST_OPTION + a * GROUP_SIZE + b, BFH_LEFT, 0, 0, ppOptions[0]->strName);
						}
						else
						{
							AddRadioGroup(IDC_FIRST_OPTION + a * GROUP_SIZE + b, BFH_SCALEFIT);
							for (c = 0; c < lOptions; c++)
							{
								AddChild(IDC_FIRST_OPTION + a * GROUP_SIZE + b, c, ppOptions[c]->strName);
							}
						}
					}
				GroupEnd();
			}

		GroupEnd();
	}
	GroupEnd();
	AddDlgGroup(DLG_OK | DLG_CANCEL);

	return true;
}

Bool WizardDialog::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDC_OK:
		DoIt();
		break;

	case IDC_PATH_NAME:
		{
			TriState<GeData> d;
			d = m_FileGUI->GetData();
			g_pResEditPrefs->strWizardPath = d.GetValue().GetFilename().GetString();
		}
		break;

	default:
		break;
	}
	return true;
}

Bool WizardDialog::InitValues()
{
	if (!GeModalDialog::InitValues()) return false;

	// set defaults
	Int32 a, b, c, lElements = LMin(MAX_SCRIPTS, g_scripts.GetElementCount());
	ScriptData** ppScripts = g_scripts.GetArray()->GetArray();
	ScriptLine** ppLines;
	ScriptOption** ppOptions;
	Int32 lLines, lOptions;

	for (a = 0; a < lElements; a++)
	{
		// Options
		lLines = LMin(MAX_LINES, ppScripts[a]->m_Options.GetElementCount());
		ppLines = ppScripts[a]->m_Options.GetArray()->GetArray();
		if (lLines > 0)
		{
			for (b = 0; b < lLines; b++)
			{
				ppOptions = ppLines[b]->opt.GetArray()->GetArray();
				lOptions = ppLines[b]->opt.GetArray()->GetElementCount();
				if (lOptions == 1)
				{
					if (ppOptions[0]->bDefault)
						SetBool(IDC_FIRST_OPTION + a * GROUP_SIZE + b, true);
				}
				else
				{
					for (c = 0; c < lOptions; c++)
					{
						if (ppOptions[c])
						{
							SetInt32(IDC_FIRST_OPTION + a * GROUP_SIZE + b, c);
							break;
						}
					}
				}
			}
		}
	}

	TriState<GeData> tri;
	tri.Add(GeData(Filename(g_pResEditPrefs->strWizardPath)));
	m_FileGUI->SetData(tri);

	return true;
}

Int32 WizardDialog::Message(const BaseContainer &msg, BaseContainer &result)
{
	/*switch (msg.GetId())
	{
	default:
		break;
	}*/
	return GeModalDialog::Message(msg, result);
}

Bool WizardDialog::AskClose()
{
	// return true, if the dialog should not be closed
	return false;
}

void WizardDialog::DoIt()
{
	Int32 lScript;
	GetInt32(IDC_TAB_GROUP, lScript);
	lScript -= IDC_FIRST_SCRIPT_GROUP;
	ScriptData* pScript = g_scripts.GetObjectAt(lScript);
	if (!pScript)
		return;
	if (!pScript->Modify(this, lScript))
		return;

	// search the template folder for all files that start with the script name
	AutoAlloc <BrowseFiles> pBrowse;
	if (!pBrowse)
		return;
	Filename fnDir = GeGetPluginPath() + String("templates");
	Filename fn, fnTest;
	pBrowse->Init(fnDir, false);
	while (pBrowse->GetNext())
	{
		if (pBrowse->IsDir())
			continue;
		fn = pBrowse->GetFilename();
		fnTest = fn;
		fnTest.ClearSuffix();
		if (pScript->m_strName.LexCompare(fnTest.GetString()) == 0)
		{
			// found the file, modify it
			ModifyFile(fnDir + fn, pScript, Filename(g_pResEditPrefs->strWizardPath));
		}
	}
}

Int32 SkipLineBreak(Int32 lCurrent, Int32 lLen, char* pFile)
{
	if (lCurrent < lLen - 3)
	{
		if (pFile[lCurrent + 1] == '\r' && pFile[lCurrent + 2] == '\n')
			return lCurrent + 2;
	}
	if (lCurrent < lLen - 2)
	{
		if (pFile[lCurrent + 1] == '\r')
			return lCurrent + 1;
	}
	return lCurrent;
}

Bool WizardDialog::ModifyFile(const Filename &fn, ScriptData* pScript, const Filename &fnDir)
{
	AutoAlloc <BaseFile> pFile;
	if (!pFile)
		return false;
	if (!pFile->Open(fn))
	{
		GeOutString(GeLoadString(IDS_WIZARD_FILE_ERROR) + " " + fn.GetString(), GEMB_OK);
		return false;
	}
	Int32 lLen = pFile->GetLength();
	Char* pChFile = NewMemClear(Char,lLen + 2);
	char* pchNext;
	if (!pChFile)
	{
		GeOutString(GeLoadString(IDS_WIZARD_NO_MEM) + " " + fn.GetString(), GEMB_OK);
		return false;
	}
	pFile->ReadBytes(pChFile, lLen);
	pFile->Close();

#define MODE_COPY		0
	Int32 lMode = MODE_COPY;

	Int32 lIDLen;
	CDynamicArray<char> arFile, arID;
	if (!arFile.SetMinSizeNoCopy(lLen))
		return false;
	char chNull = '\0';
	ScriptOption* pOption;

	Int32 a;
	for (a = 0; a < lLen; a++)
	{
		if (pChFile[a] == '#')
		{
			// check if the next char is a #, too
			if (a < lLen - 2)
			{
				if (pChFile[a + 1] == '#')
				{
					if (a < lLen - 3)
					{
						// skip this
						if (pChFile[a + 2] == '.')
						{
							pchNext = strstr(&(pChFile[a + 2]), "##");
							if (!pchNext)
								goto copy_char;
							lIDLen = ((UInt)(pchNext)) - ((UInt)(&(pChFile[a + 2])));
							if (!arID.Init(&(pChFile[a + 3]), lIDLen - 1))
								return false;
							if (!arID.Append(chNull))
								return false;
							pOption = pScript->Find(arID.GetArray());
							if (!pOption)
								goto copy_char;
							a += lIDLen + 3; // skip the next #, the ID and the last ##
							a = SkipLineBreak(a, lLen, pChFile);
							continue;
						}
					}

					// find the next ##
					pchNext = strstr(&(pChFile[a + 2]), "##");
					if (!pchNext)
						goto copy_char;
					lIDLen = ((UInt)(pchNext)) - ((UInt)(&(pChFile[a + 2])));
					if (!arID.Init(&(pChFile[a + 2]), lIDLen))
						return false;
					if (!arID.Append(chNull))
						return false;
					pOption = pScript->Find(arID.GetArray());
					if (!pOption)
						goto copy_char;
					if (pOption->lUserInput == USERINPUT_TEXT)
					{
						if (!arFile.Append(pOption->pchString, (Int32)strlen(pOption->pchString)))
							return false;
						a += lIDLen + 3; // skip the next #, the ID and the last ##
					}
					else if (pOption->lUserInput == USERINPUT_OPTION)
					{
						if (pOption->bSet)
						{
							// set the input pointer to the end of the option
							a = (((UInt)pchNext) - ((UInt)pChFile)) + 1;
							a = SkipLineBreak(a, lLen, pChFile);
						}
						else
						{
							// skip the entire option
							if (!arID.Init("##.", 3))
								return false;
							if (!arID.Append(pOption->pchID, (Int32)strlen(pOption->pchID)))
								return false;
							if (!arID.Append("##\0", 3))
								return false;
							pchNext = strstr(&(pChFile[a + 2]), arID.GetArray());
							if (!pchNext)
								goto copy_char;
							a = (((UInt)pchNext) - ((UInt)pChFile)) + arID.GetElementCount() - 2; // 2 weniger, weil Schleife inkrementiert und an ID ein \0 dran hängt
							a = SkipLineBreak(a, lLen, pChFile);
						}
					}
				}
				else
					goto copy_char;
			}
			else
				goto copy_char;
		}
		else
		{
copy_char:
			if (lMode == MODE_COPY)
			{
				if (!arFile.Append(pChFile[a], lLen))
					return false;
			}
		}
	}

	String strSuffix = GetSuffix(fn);
	Filename fnDest = fnDir;
	pOption = pScript->Find("CLASSNAME");
	if (pOption && pOption->pchString)
		fnDest += String(pOption->pchString);
	else
		fnDest += fn.GetFileString();
	fnDest.SetSuffix(strSuffix);

	DeleteMem(pChFile);

	ForceOpenFileWrite(pFile, fnDest);
	pChFile = arFile.GetArray();
	lLen = arFile.GetElementCount();
	for (a = 0; a < lLen; a++)
		pFile->WriteChar(pChFile[a]);
	pFile->Close();

	return true;
}

class WizardCommand : public CommandData
{
public:
	Bool Execute(BaseDocument *doc)
	{
		String strError;
		if (!BrowseScripts(strError) || strError.Content())
		{
			GeOutString(strError, GEMB_OK | GEMB_MULTILINE);
			return false;
		}
		WizardDialog dlg;
		dlg.Open(-2,-2);
		return true;
	}

	Int32 GetState(BaseDocument *doc)
	{
		return CMD_ENABLED;
	}
};

Bool RegisterWizardCommand()
{
	return RegisterCommandPlugin(ID_RES_EDIT_WIZARD, GeLoadString(IDS_WIZARD), 0, nullptr, GeLoadString(IDS_WIZARD), NewObjClear(WizardCommand));
}
