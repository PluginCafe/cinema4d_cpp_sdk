/*********************************************************************\
	File name        : CompareTableDialog.cpp
	Description      :
	Created at       : 09.10.01, @ 20:48:26
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

// CompareTableDialog.cpp: Implementierung der Klasse CCompareTableDialog.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "globals.h"

#include "CompareTableDialog.h"

enum
{
	IDC_OUTPUT_LIST = 1000,
	IDS_SAVE_BTN,

	IDC_DUMMY
};

#define COMPARE_LIST_NAME       'name'
#define COMPARE_LIST_DATA       'data'

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CCompareTableDialog::CCompareTableDialog(Filename fnOriginal, Filename fnNew)
{
	fnOriginal.SetSuffix("");
	fnNew.SetSuffix("");

	String str;
	str = fnNew.GetString();
	fnNew = Filename(str.SubStr(0, str.GetLength() - 1));

	str = fnOriginal.GetString();
	fnOriginal = Filename(str.SubStr(0, str.GetLength() - 1));

	String strOriginalFileString = fnOriginal.GetFileString();
	String strNewFileString = fnNew.GetFileString();

	Filename fn;

	Bool bOriginalDiag = false, bNewDiag = false;

	fn = fnOriginal.GetDirectory(); // fn contains strings_* or dialogs
	if (fn.GetFileString().LexCompare("dialogs") == 0)
	{
		fn = fn.GetDirectory(); // now, we are in the strings_* path
		bOriginalDiag = true;
	}
	fn = fn.GetDirectory();
	fnOriginal = fn;

	fn = fnNew.GetDirectory(); // fn contains strings_* or dialogs
	if (fn.GetFileString().LexCompare("dialogs") == 0)
	{
		fn = fn.GetDirectory(); // now, we are in the strings_* path
		bNewDiag = true;
	}
	fn = fn.GetDirectory();
	fnNew = fn;

	TRACE_STRING(fnOriginal.GetString());
	TRACE_STRING(fnNew.GetString());

	m_OriginalDoc.LoadGlobalStringTable(fnOriginal, strOriginalFileString);
	m_NewDoc.LoadGlobalStringTable(fnNew, strNewFileString);

	TRACE_STRING(fnNew.GetDirectory().GetFileString());

	m_pOriginalTable = m_OriginalDoc.GetStringTable();
	m_pNewTable = m_NewDoc.GetStringTable();

	if (bOriginalDiag)
		m_fnOriginal = fnOriginal + String("strings_*") + String("dialogs") + (strOriginalFileString + ".str");
	else
		m_fnOriginal = fnOriginal + String("strings_*") + (strOriginalFileString + ".str");

	if (bNewDiag)
		m_fnNew = fnNew + String("strings_*") + String("dialogs") + (strNewFileString + ".str");
	else
		m_fnNew = fnNew + String("strings_*") + (strNewFileString + ".str");

	//TRACE_STRING(m_fnNew.GetString());
	//TRACE_STRING(m_fnOriginal.GetString());

	m_bIsCommandTable = (m_OriginalDoc.m_StringTableType == typeCommandStrings) &&
											(m_NewDoc.m_StringTableType == typeCommandStrings);
}

CCompareTableDialog::~CCompareTableDialog()
{

}

/*********************************************************************\
	Function name    : CCompareTableDialog::CreateLayout
	Description      :
	Created at       : 09.10.01, @ 21:07:14
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CCompareTableDialog::CreateLayout(void)
{
	SetTitle(GeLoadString(IDS_COMPARE_TABLE_CAPTION));

	GroupBegin(101, BFH_FIT, 0, 1, "", 0);
		//AddStaticText(201, BFH_LEFT, 0, 0, GeLoadString(IDS_ORIGINAL_TABLE), 0);
		AddListView(IDC_OUTPUT_LIST, BFH_FIT, 800, 300);
		//AddStaticText(202, BFH_LEFT, 0, 0, GeLoadString(IDS_NEW_TABLE), 0);
		//AddListView(IDC_NEW_LIST, BFH_FIT, 400, 10);
	GroupEnd();

	AddButton(IDS_SAVE_BTN, BFH_LEFT, 0, 0, GeLoadString(IDS_SAVE_RESULT));

	m_wndOutputList.AttachListView(this, IDC_OUTPUT_LIST);

	AddDlgGroup(DLG_OK | DLG_CANCEL);

	return true;
}

/*********************************************************************\
	Function name    : CCompareTableDialog::Command
	Description      :
	Created at       : 09.10.01, @ 21:07:17
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CCompareTableDialog::Command(Int32 id,const BaseContainer &msg)
{
	switch (id)
	{
	case IDS_SAVE_BTN:
		SaveResult();
		break;
	}
	return true;
}

/*********************************************************************\
	Function name    : CCompareTableDialog::InitValues
	Description      :
	Created at       : 09.10.01, @ 21:07:22
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CCompareTableDialog::InitValues(void)
{
	BaseContainer bcLayout;
	Int32 a, b;

	bcLayout.SetInt32(COMPARE_LIST_NAME, LV_COLUMN_TEXT);
	bcLayout.SetInt32(COMPARE_LIST_DATA, LV_COLUMN_TEXT);
	m_wndOutputList.SetLayout(2, bcLayout);

	// the tbles are loaded, so write some important information
	Int32 lItemCount = 0;
	BaseContainer data;

	CStringTable* pTable;
	StringTableType stt;

	for (b = 0; b < 2; b++)
	{
		if (b == 0)
		{
			data.SetString(COMPARE_LIST_NAME, GeLoadString(IDS_COMPARE_ORIGINAL_FILE)); data.SetString(COMPARE_LIST_DATA, m_fnOriginal.GetString()); m_wndOutputList.SetItem(lItemCount++, data);
			pTable = m_pOriginalTable;
			stt = m_OriginalDoc.m_StringTableType;
		}
		else
		{
			data.SetString(COMPARE_LIST_NAME, GeLoadString(IDS_COMPARE_NEW_FILE)); data.SetString(COMPARE_LIST_DATA, m_fnNew.GetString()); m_wndOutputList.SetItem(lItemCount++, data);
			pTable = m_pNewTable;
			stt = m_NewDoc.m_StringTableType;
		}

		data.SetString(COMPARE_LIST_NAME, GeLoadString(IDS_COMPARE_STT));
		if (stt == typeDialogStrings)
			data.SetString(COMPARE_LIST_DATA, GeLoadString(IDS_COMPARE_TYPE_DLG));
		else if (stt == typeStringTable)
			data.SetString(COMPARE_LIST_DATA, GeLoadString(IDS_COMPARE_TYPE_STT));
		else if (stt == typeCommandStrings)
			data.SetString(COMPARE_LIST_DATA, GeLoadString(IDS_COMPARE_TYE_COMMAND));
		m_wndOutputList.SetItem(lItemCount++, data);


		data.SetString(COMPARE_LIST_NAME, GeLoadString(IDS_COMPARE_NUM_LANG)); data.SetString(COMPARE_LIST_DATA, String::IntToString(pTable->GetUsedLanguages())); m_wndOutputList.SetItem(lItemCount++, data);

		for (a = 0; a < pTable->GetUsedLanguages(); a++)
		{
			if (a == 0)
			{
				data.SetString(COMPARE_LIST_NAME, GeLoadString(IDS_COMPARE_LANGUAGES));
			}
			else
			{
				data.SetString(COMPARE_LIST_NAME, "");
			}

			String strSuffix = pTable->GetSuffix(a);
			data.SetString(COMPARE_LIST_DATA, String("") + g_LanguageList.GetName(strSuffix) + String("(") + strSuffix + String(")")); m_wndOutputList.SetItem(lItemCount++, data);
		}
		data.SetString(COMPARE_LIST_DATA, String("")); data.SetString(COMPARE_LIST_NAME, String("")); m_wndOutputList.SetItem(lItemCount++, data);
	}

	// search all strings that are in the original table, but not in the new table
	data.SetString(COMPARE_LIST_NAME, GeLoadString(IDS_COMPARE_REMOVED_ITEMS)); data.SetString(COMPARE_LIST_DATA, String("")); m_wndOutputList.SetItem(lItemCount++, data);
	data.SetString(COMPARE_LIST_NAME, String(""));
	for (a = 0; a < m_pOriginalTable->GetNumElements(); a++)
	{
		String strID = m_pOriginalTable->GetElementID(a);
		if (!m_pNewTable->ContainsElement(strID))
		{
			data.SetString(COMPARE_LIST_DATA, strID); m_wndOutputList.SetItem(lItemCount++, data);
		}
	}

	// now, find all items that were added in the new table
	data.SetString(COMPARE_LIST_NAME, GeLoadString(IDS_COMPARE_ADDED_ITEMS)); data.SetString(COMPARE_LIST_DATA, String("")); m_wndOutputList.SetItem(lItemCount++, data);
	data.SetString(COMPARE_LIST_NAME, String(""));
	for (a = 0; a < m_pNewTable->GetNumElements(); a++)
	{
		String strID = m_pNewTable->GetElementID(a);
		if (!m_pOriginalTable->ContainsElement(strID))
		{
			data.SetString(COMPARE_LIST_DATA, strID); m_wndOutputList.SetItem(lItemCount++, data);
		}
	}

	// search for all modified languages
	data.SetString(COMPARE_LIST_NAME, GeLoadString(IDS_COMPARE_MODIFIED_ITEMS)); data.SetString(COMPARE_LIST_DATA, String("")); m_wndOutputList.SetItem(lItemCount++, data);
	data.SetString(COMPARE_LIST_NAME, String(""));

	Int32 lLanguages = m_pOriginalTable->GetUsedLanguages();

	Int32 lBothLang = 0;
	Int32 *plLangIDO = bNewDeprecatedUseArraysInstead<Int32>(lLanguages);
	Int32 *plLangIDN = bNewDeprecatedUseArraysInstead<Int32>(lLanguages);
	Bool *pbChanged = bNewDeprecatedUseArraysInstead<Bool>(lLanguages);

	// check for all languages that are available in both tables
	for (a = 0; a < lLanguages; a++)
	{
		String strLangID = m_pOriginalTable->GetSuffix(a);
		Int32 lLangID = m_pNewTable->GetLanguageID(strLangID);
		if (lLangID < 0) continue;

		plLangIDO[lBothLang] = a;
		plLangIDN[lBothLang] = lLangID;

		lBothLang++;
	}

	for (a = 0; a < m_pOriginalTable->GetNumElements(); a++)
	{
		String strID = m_pOriginalTable->GetElementID(a);
		if (m_pNewTable->ContainsElement(strID))
		{
			Bool bChanged = false;
			ClearMemType(pbChanged, lLanguages);

			for (b = 0; b < lBothLang; b++)
			{
				if (m_bIsCommandTable)
				{
					String str1, str2, strCommand1, strCommand2;
					Bool bFound;

					str1 = m_pOriginalTable->GetString(a, plLangIDO[b], &strCommand1);
					str2 = m_pNewTable->GetString(m_pOriginalTable->GetElementID(a), plLangIDN[b], bFound, &strCommand2);

					if ((str1 != str2) || (strCommand1 != strCommand2))
					{
						pbChanged[b] = true;
						bChanged = true;
					}
				}
				else
				{
					String str = m_pOriginalTable->GetString(a, plLangIDO[b]);
					Bool bFound;
					if (str != m_pNewTable->GetString(m_pOriginalTable->GetElementID(a), plLangIDN[b], bFound))
					{
						pbChanged[b] = true;
						bChanged = true;
					}
				}
			}
			if (bChanged)
			{
				String str = m_pOriginalTable->GetElementID(a);
				for (b = 0; b < lBothLang; b++)
				{
					if (pbChanged[b])
						str += String("  ") + m_pOriginalTable->GetSuffix(plLangIDO[b]);
				}
				data.SetString(COMPARE_LIST_DATA, str); m_wndOutputList.SetItem(lItemCount++, data);
			}
		}
	}

	bDelete(plLangIDO);
	bDelete(plLangIDN);
	bDelete(pbChanged);

	m_wndOutputList.DataChanged();

	return true;
}

/*********************************************************************\
	Function name    : CCompareTableDialog::SaveResult
	Description      :
	Created at       : 10.10.01, @ 22:16:07
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CCompareTableDialog::SaveResult()
{
	Filename fn;
	if (!fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_SAVE, String())) return;

	Int32 lMaxLen = 0;

	Int32 a, lNumItems;
	lNumItems = m_wndOutputList.GetItemCount();

	for (a = 0; a < lNumItems; a++)
	{
		BaseContainer data;
		m_wndOutputList.GetItem(a, &data);
		lMaxLen = LMax(lMaxLen, data.GetString(COMPARE_LIST_NAME).GetLength());
	}
	lMaxLen += 2;

	AutoAlloc <BaseFile> pFile;
	if (!pFile)
		return;

	pFile->Open(fn, FILEOPEN_WRITE);
	for (a = 0; a < lNumItems; a++)
	{
		BaseContainer data;
		m_wndOutputList.GetItem(a, &data);
		String str = data.GetString(COMPARE_LIST_NAME);
		WriteString(pFile, str);
		Int32 b = lMaxLen - str.GetLength();
		while (b--) WriteString(pFile, " ");
		WriteString(pFile, data.GetString(COMPARE_LIST_DATA));
		LineBreak(pFile, "");
	}
}
