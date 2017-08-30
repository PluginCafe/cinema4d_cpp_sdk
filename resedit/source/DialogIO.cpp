/*********************************************************************\
	File name        : DialogIO.cpp
	Description      :
	Created at       : 16.08.01, @ 18:08:51
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"
#include "DialogItem.h"

#include "TreeDialog.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*********************************************************************\
	Function name    : CDialogDoc::LoadDialogResource
	Description      :
	Created at       : 16.08.01, @ 18:09:58
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::LoadDialogResource(const Filename &filename)
{
	DialogParser p;
	SYMBOL sym;
	Int32 lLangID;

	p.Init(filename,true,false);


	PROGRESS_ID	progressID = BfProgressNew(nullptr);

	m_lErrorID = 0;
	BaseContainer DialogContainer;

	String strTitle = filename.GetFile().GetString();
	Int32 lPos;
	if (strTitle.FindLast('.', &lPos))
		strTitle = strTitle.SubStr(0, lPos);
	//TRACE_STRING(strTitle);

	sym = p.GetSymbol();
	while (!p.Error())
	{
		if (sym==SYM_OPERATOR_GESCHWEIFTZU) break;

		if (sym!=SYM_IDENT)
			DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);

		if (p.CheckIdent("DIALOG"))
		{
			sym = p.nextsym();
			if (sym!=SYM_IDENT)
				DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);

			String strDialog = p.GetIdent();
			if (strDialog.LexCompare(strTitle))
			{
				GeOutString(GeLoadString(IDS_DIALOG_RESOURCE_CORRUPTED),GEMB_OK|GEMB_ICONEXCLAMATION);
				DLG_IO_GOTO_ERROR(ERR_FILENAME_DIALOGNAME_NO_MATCH);
			}

			//obj = AddEntry(symbols,menuname,BaseContainer(DRTYPE_DIALOG));

			DialogContainer.SetId(DRTYPE_DIALOG);/*.SetInt32(DR_TYPE, DRTYPE_DIALOG);*/
			DialogContainer.SetString(DR_ID, p.GetIdent());
			m_lItemCount = 0;
			if (!LoadDialogParts(p, &DialogContainer, DRTYPE_DIALOG, progressID))
				GOTO_ERROR;
		}
		else
			GOTO_ERROR;
		sym = p.GetSymbol();
	}

	m_StringTable.Free();
	// try to find the string table
	{
		Filename fnDir = filename.GetDirectory();
		//TRACE("Dialog directory: ");
		//TRACE_STRING(fnDir.GetString());
		Filename fnRes = fnDir.GetDirectory();
		//TRACE("Resource directory: ");
		//TRACE_STRING(fnRes.GetString());
		Filename fnPlug = fnRes.GetDirectory();
		//TRACE("Plugin directory: ");
		//TRACE_STRING(fnPlug.GetString());

		// first, search for string tables in the resource directory
		m_StringTableType = typeDialogStrings;
		if (!LoadStringTables(fnRes, strTitle, typeDialogStrings))
		{
			// look in the plugin directory
			LoadStringTables(fnPlug, strTitle, typeDialogStrings);
		}
	}

	lLangID = g_LanguageList.GetLanguageID("us");
	if (lLangID < 0)
	{
		lLangID = 0;
		m_lLanguageIDTable = m_StringTable.GetLanguageID(g_LanguageList.GetLanguageSuffix(lLangID));
	}
	else
	{
		m_lLanguageIDTable = m_StringTable.GetLanguageID("us");
	}
	m_pTreeDlg->SelectLanguage(lLangID);
	m_lLanguageIDList = lLangID;


	// now, put the infos in DialogContainer into the tree view
	m_bIsLoading = true;
	m_pDialog = (CDialogTemplate*)AddItem(Dialog);
	m_pDialog->Load(&DialogContainer);
	AddItems(&DialogContainer, (CDialogItem*)m_pDialog, progressID, false, false);

	if (m_StringTable.GetUsedLanguages() == 0)
		m_lLanguageIDTable = m_StringTable.AddLanguage("us");

	LanguageChanged();
	//m_StringTable.Dump();

	m_bIsLoading = false;

	BfProgressDelete(nullptr, progressID);

	m_bControlSymbolsChanged = false;
	m_bStringSymbolsChanged = false;

	return true;

error:
	BfProgressDelete(nullptr, progressID);
	//GeOutString("uuups", GEMB_OK + GEMB_ICONEXCLAMATION);
	String str = GeLoadString(IDS_ERROR_LOADING_FILE);
	str += m_fnErrorFile.GetString();
	str += "\n";
	str += GeLoadString(IDS_ERROR_LOADING_FILE_LINE);
	str += String::IntToString(p.GetLine());
	str += "\n";
	str += GeLoadString(IDS_ERROR_LOADING_FILE_SYMBOL);

	sym = p.GetSymbol();
	if (sym == SYM_NULL) str += GeLoadString(IDS_ERROR_LOADING_FILE_SYM_NULL);
	else if (sym == SYM_IDENT || sym == SYM_STRING || sym == SYM_STRINGCHR) str += p.GetIdent();
	else if (sym == SYM_NUMBER) { Float r; p.GetFloat(r); str += String::FloatToString(r); }
	else if (SYM_OPERATOR_NEWLINE) str += GeLoadString(IDS_ERROR_LOADING_FILE_NEWLINE);
	else if (SYM_OPERATOR_GESCHWEIFTAUF) str += "{";
	else if (SYM_OPERATOR_GESCHWEIFTZU) str += "}";
	else if (SYM_OPERATOR_COMMENT) str += GeLoadString(IDS_ERROR_LOADING_FILE_COMMENT);
	else if (SYM_OPERATOR_EQUAL) str += "=";
	else if (SYM_OPERATOR_KOMMA) str += ",";
	else if (SYM_OPERATOR_SEMIKOLON) str += ";";
	else if (SYM_OPERATOR_PLUS) str += "+";
	else if (SYM_OPERATOR_TILDE) str += "~";
	else if (SYM_OPERATOR_ILLEGAL) str += GeLoadString(IDS_ERROR_LOADING_FILE_ILLEGAL_OPERATOR);
	str += "\n";
	if (m_lErrorID == ERR_EXPECTED_IDENT) str += GeLoadString(IDS_ERROR_LOADING_FILE_EXPECTED_IDENT);
	else if (m_lErrorID == ERR_EXPECTED_GESCHWEIFT_AUF) str += GeLoadString(IDS_ERROR_LOADING_FILE_EXPECTED_GA);
	else if (m_lErrorID == ERR_EXPECTED_STRING) str += GeLoadString(IDS_ERROR_LOADING_FILE_EXPECTED_STRING);
	else if (m_lErrorID == ERR_EXPECTED_SEMIKOLON) str += GeLoadString(IDS_ERROR_LOADING_FILE_EXPECTED_SEMI);
	else if (m_lErrorID == ERR_EXPECTED_NUMBER) str += GeLoadString(IDS_ERROR_LOADING_FILE_EXPECTED_NUMBER);
	else if (m_lErrorID == ERR_EXPECTED_KOMMA) str += GeLoadString(IDS_ERROR_LOADING_FILE_EXPECTED_KOMMA);
	else if (m_lErrorID == ERR_UNKNOWN_ID) str += GeLoadString(IDS_ERROR_LOADING_FILE_UNKNOWN_ID);
	else if (m_lErrorID == ERR_FILENAME_DIALOGNAME_NO_MATCH) str += GeLoadString(IDS_ERROR_LOADING_FILE_FILENAME_ID_EQUAL);

	GeOutString(str, GEMB_OK | GEMB_ICONEXCLAMATION);
	return false;
}

/*********************************************************************\
	Function name    : CDialogDoc::LoadDialogParts
	Description      :
	Created at       : 16.08.01, @ 23:20:46
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
#ifdef VERSION_7
Bool CDialogDoc::LoadDialogParts(DialogParser &p, BaseContainer *pDialogInfo, Int32 type, PROGRESS_ID	progressID)
{
	SYMBOL sym = p.nextsym();
	Bool needsemi;
	Bool done;
	Int32 w;

	Int32 lPos = p.GetCurrentReadPosition();

	BfProgressUpdate(progressID, p.GetCurrentReadPosition() * 100 / p.GetTotalFileSize(), 100, "");

	if (sym!=SYM_OPERATOR_GESCHWEIFTAUF) DLG_IO_GOTO_ERROR(ERR_EXPECTED_GESCHWEIFT_AUF);

	sym = p.nextsym();

	while (!p.Error())
	{
		if (sym==SYM_OPERATOR_GESCHWEIFTZU) break;
		if (sym==SYM_OPERATOR_SEMIKOLON)
		{
			sym=p.nextsym();
			continue;
		}

		if (sym!=SYM_IDENT) DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);
		needsemi = false; done = false;

		if (p.CheckIdent("NAME"))
		{
			String name;
			sym = p.nextsym(); if (sym!=SYM_IDENT) DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);
			name = p.GetIdent();
			/*TODO*/
			/*ResourceObj *strobj = strings.FindObj(p.GetIdent());
			if (!strobj)
			{
				GeOutString(GeLoadString(IDS_SYMBOL_NOT_FOUND,p.GetIdent(),p.GetFilename().GetString(),String::IntToString(p.GetLine())),GEMB_OK+GEMB_ICONEXCLAMATION);
			}
			else
			{
				name = strobj->data.GetString(0);
			}*/

			if (name.Content())
			{
				//TRACE_STRING(name);
				pDialogInfo->SetString(DR_NAME, name);
			}

			needsemi++; done++;
		}
		else if (p.CheckIdent("FIT_V"))
		{
			Int32 flags = pDialogInfo->GetInt32(DR_ALIGNMENT);
			flags &= ~BFV_MASK;
			flags |= BFV_FIT;
			pDialogInfo->SetInt32(DR_ALIGNMENT, flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("FIT_H"))
		{
			Int32 flags = pDialogInfo->GetInt32(DR_ALIGNMENT);
			flags &= ~BFH_MASK;
			flags |= BFH_FIT;
			pDialogInfo->SetInt32(DR_ALIGNMENT, flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("ALIGN_LEFT"))
		{
			Int32 flags = pDialogInfo->GetInt32(DR_ALIGNMENT);
			flags &= ~BFH_MASK;
			flags |= BFH_LEFT;
			pDialogInfo->SetInt32(DR_ALIGNMENT, flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("ALIGN_RIGHT"))
		{
			Int32 flags = pDialogInfo->GetInt32(DR_ALIGNMENT);
			flags &= ~BFH_MASK;
			flags |= BFH_RIGHT;
			pDialogInfo->SetInt32(DR_ALIGNMENT, flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("ALIGN_TOP"))
		{
			Int32 flags = pDialogInfo->GetInt32(DR_ALIGNMENT);
			flags &= ~BFV_MASK;
			flags |= BFV_TOP;
			pDialogInfo->SetInt32(DR_ALIGNMENT, flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("ALIGN_BOTTOM"))
		{
			Int32 flags = pDialogInfo->GetInt32(DR_ALIGNMENT);
			flags &= ~BFV_MASK;
			flags |= BFV_BOTTOM;
			pDialogInfo->SetInt32(DR_ALIGNMENT, flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("CENTER_H"))
		{
			Int32 flags = pDialogInfo->GetInt32(DR_ALIGNMENT);
			flags &= ~BFH_MASK;
			flags |= BFH_CENTER;
			pDialogInfo->SetInt32(DR_ALIGNMENT, flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("CENTER_V"))
		{
			Int32 flags = pDialogInfo->GetInt32(DR_ALIGNMENT);
			flags &= ~BFV_MASK;
			flags |= BFV_CENTER;
			pDialogInfo->SetInt32(DR_ALIGNMENT, flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("SCALE_H"))
		{
			Int32 flags = pDialogInfo->GetInt32(DR_ALIGNMENT);
			flags &= ~BFH_MASK;
			flags |= BFH_SCALE;
			pDialogInfo->SetInt32(DR_ALIGNMENT, flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("SCALE_V"))
		{
			Int32 flags = pDialogInfo->GetInt32(DR_ALIGNMENT);
			flags &= ~BFV_MASK;
			flags |= BFV_SCALE;
			pDialogInfo->SetInt32(DR_ALIGNMENT, flags);
			needsemi++; done++;
		}
		else
		{
			switch (type) {
				case DRTYPE_SCROLLGROUP:
					if      (p.CheckIdent("SCROLL_V")) { pDialogInfo->SetBool(DR_SCROLL_V, true); needsemi++; done++; }
					else if (p.CheckIdent("SCROLL_H")) { pDialogInfo->SetBool(DR_SCROLL_H, true); needsemi++; done++; }
					else if (p.CheckIdent("SCROLL_BORDER")) { pDialogInfo->SetBool(DR_SCROLL_BORDERIN, true); needsemi++; done++; }
					else if (p.CheckIdent("SCROLL_STATUSBAR")) { pDialogInfo->SetBool(DR_SCROLL_STATUSBAR, true); needsemi++; done++; }
					/*else if (p.CheckIdent("SIZE"))
					{
						Int32 n;

						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(n); if (n<0) n=(-n)<<14; pDialogInfo->SetInt32(DROLDWIN_POS_W, n);
						sym = p.nextsym(); if (sym!=SYM_OPERATOR_KOMMA) DLG_IO_GOTO_ERROR(ERR_EXPECTED_KOMMA);

						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(n); if (n<0) n=(-n)<<14; pDialogInfo->SetInt32(DROLDWIN_POS_H, n);
						needsemi++; done++;
					}*/
					else goto trygadgets;
					break;

				case DROLDWIN_ARROWBUTTON:
					if (p.CheckIdent("ARROW_LEFT"))  { pDialogInfo->SetInt32(DR_ARROWTYPE, ARROW_LEFT);  needsemi++; done++; }
					else if (p.CheckIdent("ARROW_RIGHT")) { pDialogInfo->SetInt32(DR_ARROWTYPE, ARROW_RIGHT); needsemi++; done++; }
					else if (p.CheckIdent("ARROW_UP"))    { pDialogInfo->SetInt32(DR_ARROWTYPE, ARROW_UP);    needsemi++; done++; }
					else if (p.CheckIdent("ARROW_DOWN"))  { pDialogInfo->SetInt32(DR_ARROWTYPE, ARROW_DOWN);  needsemi++; done++; }

					else if (p.CheckIdent("ARROW_SMALL_LEFT"))  { pDialogInfo->SetInt32(DR_ARROWTYPE, ARROW_SMALL_LEFT);  needsemi++; done++; }
					else if (p.CheckIdent("ARROW_SMALL_RIGHT")) { pDialogInfo->SetInt32(DR_ARROWTYPE, ARROW_SMALL_RIGHT); needsemi++; done++; }
					else if (p.CheckIdent("ARROW_SMALL_UP"))    { pDialogInfo->SetInt32(DR_ARROWTYPE, ARROW_SMALL_UP);    needsemi++; done++; }
					else if (p.CheckIdent("ARROW_SMALL_DOWN"))  { pDialogInfo->SetInt32(DR_ARROWTYPE, ARROW_SMALL_DOWN);  needsemi++; done++; }
					else goto readgadgets;
					break;

				case DRTYPE_DIALOG:
					if (p.CheckIdent("WINRES"))
					{
						// old windows resource
						BaseContainer sub;
						if (!LoadDialogParts(p, &sub, DRTYPE_OLDWIN, progressID)) DLG_IO_GOTO_ERROR(0);
						Int32 lChildren = pDialogInfo->GetInt32(DR_NUMCHILDREN);
						pDialogInfo->SetContainer(DR_CHILD + lChildren, sub);
						pDialogInfo->SetInt32(DR_NUMCHILDREN, lChildren + 1);
						sub.SetId(DRTYPE_OLDWIN);

						done++;
					}
					else goto trygadgets;
					break;

				case DRTYPE_TAB:
					if (p.CheckIdent("SELECTION_NONE"))
					{
						pDialogInfo->SetInt32(DR_TAB_SELECTBY, TAB_NOSELECT);
						needsemi++; done++;
					}
					else if (p.CheckIdent("SELECTION_TABS"))
					{
						pDialogInfo->SetInt32(DR_TAB_SELECTBY, TAB_TABS);
						needsemi++; done++;
					}
					else if (p.CheckIdent("SELECTION_VLTABS"))
					{
						pDialogInfo->SetInt32(DR_TAB_SELECTBY, TAB_VLTABS);
						needsemi++; done++;
					}
					else if (p.CheckIdent("SELECTION_VRTABS"))
					{
						pDialogInfo->SetInt32(DR_TAB_SELECTBY, TAB_VRTABS);
						needsemi++; done++;
					}
					else if (p.CheckIdent("SELECTION_CYCLE"))
					{
						pDialogInfo->SetInt32(DR_TAB_SELECTBY, TAB_CYCLE);
						needsemi++; done++;
					}
					else if (p.CheckIdent("SELECTION_RADIO"))
					{
						pDialogInfo->SetInt32(DR_TAB_SELECTBY, TAB_RADIO);
						needsemi++; done++;
					}
					else goto trygadgets;
					break;

				case DRTYPE_WINDOWPIN:
					break;

				case DRTYPE_SEPARATOR:
					break;

				case DRTYPE_DLGGROUP:
					if (p.CheckIdent("OK"))
					{
						pDialogInfo->SetBool(DR_DLGGROUP_OK, true);
						needsemi++; done++;
					}
					else if (p.CheckIdent("CANCEL"))
					{
						pDialogInfo->SetBool(DR_DLGGROUP_CANCEL, true);
						needsemi++; done++;
					}
					else if (p.CheckIdent("HELP"))
					{
						needsemi++; done++;
					}
					break;


				case DRTYPE_RADIOGROUP:
				case DRTYPE_GROUP:
					if (p.CheckIdent("COLUMNS"))
					{
						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(w); pDialogInfo->SetInt32(DR_COLUMNS, w);
						needsemi++; done++;
					}
					else if (p.CheckIdent("ROWS"))
					{
						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(w); pDialogInfo->SetInt32(DR_ROWS, w);
						needsemi++; done++;
					}
					else if (p.CheckIdent("EQUAL_ROWS"))
					{
						pDialogInfo->SetBool(DR_ALLOW_EIGHTS, true);
						needsemi++; done++;
					}
					else if (p.CheckIdent("EQUAL_COLS"))
					{
						pDialogInfo->SetBool(DR_EQUALCOLS, true);
						needsemi++; done++;
					}
					else if (p.CheckIdent("TITLECHECKBOX"))
					{
						pDialogInfo->SetBool(BFV_BORDERGROUP_CHECKBOX,true);
						needsemi++; done++;
					}
					else if (p.CheckIdent("ALLOW_WEIGHTS"))
					{
						bc->SetData(DR_ALLOW_WEIGHTS,true);
						needsemi++; done++;
					}
					else if (p.CheckIdent("BORDERSTYLE"))
					{
						Int32 type;
						sym = p.nextsym(); if (sym!=SYM_IDENT) DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);
						if (p.CheckIdent("BORDER_NONE"))           type = BORDER_NONE;
						else if (p.CheckIdent("BORDER_THIN_IN"))   type = BORDER_THIN_IN;
						else if (p.CheckIdent("BORDER_THIN_OUT"))  type = BORDER_THIN_OUT;
						else if (p.CheckIdent("BORDER_IN"))        type = BORDER_IN;
						else if (p.CheckIdent("BORDER_OUT"))       type = BORDER_OUT;
						else if (p.CheckIdent("BORDER_GROUP_IN"))  type = BORDER_GROUP_IN;
						else if (p.CheckIdent("BORDER_GROUP_TOP")) type = BORDER_GROUP_TOP;
						else if (p.CheckIdent("BORDER_GROUP_OUT")) type = BORDER_GROUP_OUT;
						else if (p.CheckIdent("BORDER_OUT2"))      type = BORDER_OUT2;
						else if (p.CheckIdent("BORDER_OUT3"))      type = BORDER_OUT3;
						else if (p.CheckIdent("BORDER_OUT3l"))     type = BORDER_OUT3l;
						else if (p.CheckIdent("BORDER_OUT3r"))     type = BORDER_OUT3r;
						else if (p.CheckIdent("BORDER_BLACK"))     type = BORDER_BLACK;
						else DLG_IO_GOTO_ERROR(ERR_UNKNOWN_ID);

						pDialogInfo->SetInt32(DR_BORDERSTYLE, type);

						needsemi++; done++;
					}
					else if (p.CheckIdent("BORDERSIZE"))
					{
						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(w); pDialogInfo->SetInt32(DR_BORDERSIZE_L, w);
						sym = p.nextsym(); if (sym!=SYM_OPERATOR_KOMMA) DLG_IO_GOTO_ERROR(ERR_EXPECTED_KOMMA);

						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(w); pDialogInfo->SetInt32(DR_BORDERSIZE_T, w);
						sym = p.nextsym(); if (sym!=SYM_OPERATOR_KOMMA) DLG_IO_GOTO_ERROR(ERR_EXPECTED_KOMMA);

						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(w); pDialogInfo->SetInt32(DR_BORDERSIZE_R, w);
						sym = p.nextsym(); if (sym!=SYM_OPERATOR_KOMMA) DLG_IO_GOTO_ERROR(ERR_EXPECTED_KOMMA);

						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(w); pDialogInfo->SetInt32(DR_BORDERSIZE_B, w);

						needsemi++; done++;
					}
					else if (p.CheckIdent("SPACE"))
					{
						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(w); pDialogInfo->SetInt32(DR_SPACE_X, w);
						sym = p.nextsym(); if (sym!=SYM_OPERATOR_KOMMA) DLG_IO_GOTO_ERROR(ERR_EXPECTED_KOMMA);

						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(w); pDialogInfo->SetInt32(DR_SPACE_Y, w);
						needsemi++; done++;
					}
					else if (p.CheckIdent("SIZE"))
					{
						goto trygadgets;
					}
					else goto trygadgets;
	/*
				//DR_CHILD
				//DR_NAME
				DR_BORDERSTYLE						= 'bdst', // type of the border
					BORDER_NONE								= 0,
					BORDER_THIN_IN						= 1,
					BORDER_THIN_OUT						= 2,
					BORDER_IN									= 3,
					BORDER_OUT								= 4,
					BORDER_GROUP_IN						= 5,
					BORDER_GROUP_OUT					= 6,
					BORDER_OUT2								= 7,
					BORDER_OUT3								= 8,
					BORDER_BLACK							= 9,

				DR_BORDERSIZE							= 'bdsz', // space around the group
				DR_SPACE									= 'spce', // space between elements
	*/
					break;

				case DRTYPE_OLDWIN:
	trygadgets:
					if (p.CheckIdent("SIZE"))
					{
						Int32 w,h;

						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(w); pDialogInfo->SetInt32(DROLDWIN_POS_W, w);

						sym = p.nextsym(); if (sym!=SYM_OPERATOR_KOMMA) DLG_IO_GOTO_ERROR(ERR_EXPECTED_KOMMA);

						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(h); pDialogInfo->SetInt32(DROLDWIN_POS_H, h);

						needsemi++; done++;
					}
					else
					{
						Int32 type = 0, flags=BFH_FIT|BFV_CENTER;

						/**/if (p.CheckIdent("GROUP"))            { type = DRTYPE_GROUP;       flags = BFH_LEFT|BFV_TOP; }
						/**/else if (p.CheckIdent("DLGGROUP"))    { type = DRTYPE_DLGGROUP;    flags = BFH_LEFT|BFV_TOP; }
						/**/else if (p.CheckIdent("RADIOGROUP"))  { type = DRTYPE_RADIOGROUP;  flags = BFH_LEFT|BFV_TOP; }
						else if (p.CheckIdent("WINDOWPIN"))   { type = DRTYPE_WINDOWPIN;   flags = BFH_CENTER|BFV_CENTER; }
						/**/else if (p.CheckIdent("SEPARATOR"))   { type = DRTYPE_SEPARATOR;   flags = BFH_FIT|BFV_CENTER; }
						/**/else if (p.CheckIdent("TAB"))         { type = DRTYPE_TAB;         flags = BFH_LEFT|BFV_TOP; }
						/**/else if (p.CheckIdent("SCROLLGROUP")) { type = DRTYPE_SCROLLGROUP; flags = BFH_FIT|BFV_FIT; }
						/**/else if (p.CheckIdent("EDITTEXT"))         type = DROLDWIN_EDITTEXT;
						/**/else if (p.CheckIdent("MULTILINEEDIT"))    type = DROLDWIN_MULTILINETEXT;
						/**/else if (p.CheckIdent("EDITNUMBER"))       type = DROLDWIN_EDITNUMBER;
						/**/else if (p.CheckIdent("EDITNUMBERARROWS")) type = DROLDWIN_EDITNUMBERARROWS;
						/**/else if (p.CheckIdent("COMBOBOX"))         type = DROLDWIN_COMBOBOX;
						/**/else if (p.CheckIdent("COMBOBUTTON"))      type = DROLDWIN_COMBOBUTTON;
						/**/else if (p.CheckIdent("POPUPBUTTON"))      type = DROLDWIN_POPUPBUTTON;
						/**/else if (p.CheckIdent("CHECKBOX"))         type = DROLDWIN_CHECKBOX;
						/**/else if (p.CheckIdent("STATICTEXT"))       type = DROLDWIN_STATICTEXT;
						/**/else if (p.CheckIdent("SLIDER"))           type = DROLDWIN_SLIDER;
						/**/else if (p.CheckIdent("EDITSLIDER"))       type = DROLDWIN_SLIDERNUM;
						/**/else if (p.CheckIdent("BUTTON"))           type = DROLDWIN_BUTTON;
						/**/else if (p.CheckIdent("USERAREA"))         type = DROLDWIN_OWNERDRAW;
						/**/else if (p.CheckIdent("COLORFIELD"))       type = DROLDWIN_COLORFIELD;
						/**/else if (p.CheckIdent("RADIOGADGET"))      type = DROLDWIN_RADIOGADGET;
						/**/else if (p.CheckIdent("SPECIALRADIO"))     type = DROLDWIN_SPECIALRADIO;
						/**/else if (p.CheckIdent("ARROWBUTTON"))    { type = DROLDWIN_ARROWBUTTON; flags = BFH_LEFT|BFV_TOP; }
						/**/else if (p.CheckIdent("LISTVIEW"))       { type = DROLDWIN_LISTVIEW;    flags = BFH_SCALEFIT|BFV_SCALEFIT; }
						/**/else if (p.CheckIdent("SUBDIALOG"))      {
						type = DROLDWIN_SUBDIALOG;   flags = BFH_SCALEFIT|BFV_SCALEFIT;
						}

						if (type)
						{
							m_lItemCount++;

							sym = p.nextsym();
							BaseContainer sub;
							sub.SetId(type);/*.SetInt32(DR_TYPE, type);*/

							if (sym==SYM_OPERATOR_GESCHWEIFTAUF)
								p.rewindsym();
							/*TODO*/
							else
							{
								if (sym==SYM_NUMBER)
								{
									Int32 id;
									if (!p.GetInt32(id)) DLG_IO_GOTO_ERROR(0);
									sub.SetString(DR_ID, String::IntToString(id));
								}
								else
								{
									if (sym!=SYM_IDENT) DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);
									sub.SetString(DR_ID, String(p.GetIdent()));
								}
							}

							/*TODO*/
							sub.SetInt32(DR_ALIGNMENT, flags);

							//GeContainer *s = bc->InsContainer(DR_CHILD,sub);
							if (type == DRTYPE_RADIOGROUP || type == DRTYPE_GROUP)
							{
								sub.SetInt32(DR_SPACE_X, 4);
								sub.SetInt32(DR_SPACE_Y, 4);
							}
							if (!LoadDialogParts(p, &sub, type, progressID)) DLG_IO_GOTO_ERROR(0);

							Int32 lChildren = pDialogInfo->GetInt32(DR_NUMCHILDREN);
							pDialogInfo->SetContainer(DR_CHILD + lChildren, sub);
							pDialogInfo->SetInt32(DR_NUMCHILDREN, lChildren + 1);

						/*{
							BaseContainer childs = sub.GetContainer(DR_CHILD);
							Int32 lChildren = childs.GetInt32(0);
							for (Int32 o = 0; o < lChildren; o++)
							{
								String str;
								str = childs.GetString(2 * o + 3);
								str = str;
							}
						}*/


							if (type==DRTYPE_GROUP)
							{
								/*if (s->GetData(DR_SPACE_X)==0 && s->GetData(DR_SPACE_Y)==0)
									s = s;*/
							}

							done++;
						}
						else DLG_IO_GOTO_ERROR(ERR_UNKNOWN_ID);
					}
					break;

				case DROLDWIN_MULTILINETEXT:
					if (p.CheckIdent("MONOSPACED"))
					{
						pDialogInfo->SetBool(DR_MULTILINE_MONOSPACED, true);
						needsemi++; done++;
					}
					if (p.CheckIdent("SYNTAXCOLOR"))
					{
						pDialogInfo->SetBool(DR_MULTILINE_SYNTAXCOLOR, true);
						needsemi++; done++;
					}
					if (p.CheckIdent("STATUSBAR"))
					{
						pDialogInfo->SetBool(DR_MULTILINE_STATUSBAR, true);
						needsemi++; done++;
					}
					if (p.CheckIdent("PYTHON"))
					{
						pDialogInfo->SetBool(DR_MULTILINE_PYTHON, true);
						needsemi++; done++;
					}
					if (p.CheckIdent("WORDWRAP"))
					{
						pDialogInfo->SetBool(DR_MULTILINE_WORDWRAP, true);
						needsemi++; done++;
					}

					goto readgadgets;

				case DROLDWIN_COLORFIELD:
					if (p.CheckIdent("NOBRIGHTNESS"))
					{
						pDialogInfo->SetBool(DR_COLORFIELD_NO_BRIGHTNESS, true);
						needsemi++; done++;
					}
					if (p.CheckIdent("NOCOLOR"))
					{
						pDialogInfo->SetBool(DR_COLORFIELD_NO_COLOR, true);
						needsemi++; done++;
					}
					if (p.CheckIdent("ICC_BASEDOCUMENT"))
					{
						pDialogInfo->SetBool(DR_COLORFIELD_ICC_BASEDOC, true);
						needsemi++; done++;
					}
					if (p.CheckIdent("ICC_BPTEXTURE"))
					{
						pDialogInfo->SetBool(DR_COLORFIELD_ICC_BPTEX, true);
						needsemi++; done++;
					}
					goto readgadgets;

				case DROLDWIN_EDITTEXT:
				case DROLDWIN_EDITNUMBER:
				case DROLDWIN_EDITNUMBERARROWS:
				case DROLDWIN_COMBOBOX:
				case DROLDWIN_COMBOBUTTON:
				case DROLDWIN_POPUPBUTTON:
				case DROLDWIN_CHECKBOX:
				case DROLDWIN_STATICTEXT:
				case DROLDWIN_SLIDER:
				case DROLDWIN_SLIDERNUM:
				case DROLDWIN_GROUPBOX:
				case DROLDWIN_BUTTON:
				case DROLDWIN_OWNERDRAW:
				case DROLDWIN_RADIOGADGET:
				case DROLDWIN_SPECIALRADIO:
				case DROLDWIN_LISTVIEW:
					if (type == DROLDWIN_STATICTEXT)
					{
						if (p.CheckIdent("BORDERSTYLE"))
						{
							Int32 type;
							sym = p.nextsym(); if (sym!=SYM_IDENT) DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);
							if (p.CheckIdent("BORDER_NONE"))           type = BORDER_NONE;
							else if (p.CheckIdent("BORDER_THIN_IN"))   type = BORDER_THIN_IN;
							else if (p.CheckIdent("BORDER_THIN_OUT"))  type = BORDER_THIN_OUT;
							else if (p.CheckIdent("BORDER_IN"))        type = BORDER_IN;
							else if (p.CheckIdent("BORDER_OUT"))       type = BORDER_OUT;
							else if (p.CheckIdent("BORDER_GROUP_IN"))  type = BORDER_GROUP_IN;
							else if (p.CheckIdent("BORDER_GROUP_TOP")) type = BORDER_GROUP_TOP;
							else if (p.CheckIdent("BORDER_GROUP_OUT")) type = BORDER_GROUP_OUT;
							else if (p.CheckIdent("BORDER_OUT2"))      type = BORDER_OUT2;
							else if (p.CheckIdent("BORDER_OUT3"))      type = BORDER_OUT3;
							else if (p.CheckIdent("BORDER_OUT3l"))     type = BORDER_OUT3l;
							else if (p.CheckIdent("BORDER_OUT3r"))     type = BORDER_OUT3r;
							else if (p.CheckIdent("BORDER_BLACK"))     type = BORDER_BLACK;
							else DLG_IO_GOTO_ERROR(ERR_UNKNOWN_ID);

							pDialogInfo->SetInt32(DR_BORDERSTYLE, type);

							needsemi++; done++;
						}
					}
	readgadgets:
					if (p.CheckIdent("POS"))
					{
						Int32 n;

						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(n); pDialogInfo->SetInt32(DROLDWIN_POS_X, n);

						sym = p.nextsym(); if (sym!=SYM_OPERATOR_KOMMA) DLG_IO_GOTO_ERROR(ERR_EXPECTED_KOMMA);
						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(n); pDialogInfo->SetInt32(DROLDWIN_POS_Y, n);

						sym = p.nextsym(); if (sym!=SYM_OPERATOR_KOMMA) DLG_IO_GOTO_ERROR(ERR_EXPECTED_KOMMA);
						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(n); pDialogInfo->SetInt32(DROLDWIN_POS_W, n);

						sym = p.nextsym(); if (sym!=SYM_OPERATOR_KOMMA) DLG_IO_GOTO_ERROR(ERR_EXPECTED_KOMMA);
						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(n); pDialogInfo->SetInt32(DROLDWIN_POS_H, n);

						needsemi++; done++;
					}
					else if (p.CheckIdent("SIZE"))
					{
						Int32 n;

						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(n); if (n<0) n=(-n)<<14; pDialogInfo->SetInt32(DROLDWIN_POS_W, n);
						sym = p.nextsym(); if (sym!=SYM_OPERATOR_KOMMA) DLG_IO_GOTO_ERROR(ERR_EXPECTED_KOMMA);

						sym = p.nextsym(); if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
						p.GetInt32(n); if (n<0) n=(-n)<<14; pDialogInfo->SetInt32(DROLDWIN_POS_H, n);
						needsemi++; done++;
					}
					else if ((type == DROLDWIN_COMBOBOX || type == DROLDWIN_COMBOBUTTON || type == DROLDWIN_POPUPBUTTON) && p.CheckIdent("CHILDS"))
					{
						/*TODO*/
						Int32 id;
						BaseContainer childs;
						childs.SetId(DRTYPE_COMBO_POPUP_CHILDREN);/*.SetInt32(DR_TYPE, DRTYPE_COMBO_POPUP_CHILDREN);*/
						sym = p.nextsym(); if (sym!=SYM_OPERATOR_GESCHWEIFTAUF) DLG_IO_GOTO_ERROR(ERR_EXPECTED_GESCHWEIFT_AUF);
						Int32 lChildren = 1;
						String strID;
						while (true)
						{
							sym = p.nextsym();
							if (sym==SYM_OPERATOR_GESCHWEIFTZU) break;

							if (sym==SYM_IDENT)
							{
								strID = p.GetIdent();
							}
							else
							{
								if (sym!=SYM_NUMBER) DLG_IO_GOTO_ERROR(ERR_EXPECTED_NUMBER);
								p.GetInt32(id);
								strID = String::IntToString(id);
							}

							sym = p.nextsym();
							if (sym != SYM_OPERATOR_KOMMA) DLG_IO_GOTO_ERROR(ERR_EXPECTED_KOMMA);

							sym = p.nextsym();
							if (sym != SYM_IDENT) DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);

							String name = p.GetIdent();

							childs.SetString(2 * lChildren, strID);
							childs.SetString(2 * lChildren + 1, name);
							sym = p.nextsym();
							if (sym != SYM_OPERATOR_SEMIKOLON) DLG_IO_GOTO_ERROR(ERR_EXPECTED_SEMIKOLON);

							lChildren++;
						}

						childs.SetInt32(0, --lChildren);
						lChildren = pDialogInfo->GetInt32(DR_NUMCHILDREN);
						pDialogInfo->SetContainer(DR_CHILD + lChildren, childs);
						pDialogInfo->SetInt32(DR_NUMCHILDREN, lChildren + 1);
						done++;
					}
					break;

				default:
					DLG_IO_GOTO_ERROR(ERR_UNKNOWN_ID);
					break;
			}
		}

		if (!done)
		{
			TRACE("Warning : Found unknown object type\n");
			//goto error;
		}
		if (needsemi)
		{
			sym = p.nextsym();
			if (sym!=SYM_OPERATOR_SEMIKOLON)
			{
				if (sym == SYM_IDENT)
					TRACE("expected semi, found identifier %s\n", p.GetIdent());
				DLG_IO_GOTO_ERROR(ERR_EXPECTED_SEMIKOLON);
			}
		}
		sym = p.nextsym();
	}
	return !p.Error();

error:
	p.SetError();
	return false;
}
#endif //  VERSION_7

typedef struct
{
	BaseContainer* bc;
	Int32 lMinID;
	String strID;
} tagAddCheckName;

/*********************************************************************\
	Function name    : AddItemCheckNameCallback
	Description      :
	Created at       : 04.01.02, @ 23:25:40
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void AddItemCheckNameCallback(TreeViewItem* pItem, void* pData)
{
	tagAddCheckName* cn = (tagAddCheckName*)pData;
	CDialogItem* pDlgItem = (CDialogItem*)pItem->GetData()->GetVoid(TREEVIEW_DATA);

	if (!cn || !pDlgItem) return;

	String str = cn->strID;
	String strID = pDlgItem->m_strControlID;

	Int32 lPos;
	if (strID.FindFirst(str, &lPos) && lPos == 0)
	{
		String strNewID = strID.SubStr(str.GetLength(), strID.GetLength() - str.GetLength());
		Int32 lError;
		Int32 lMin = strNewID.ParseToInt32(&lError);
		if (lError == 0)
			cn->lMinID = LMax(cn->lMinID + 1, lMin);
		else
			cn->lMinID = LMax(cn->lMinID, 1);
	}
}

/*********************************************************************\
	Function name    : AddItems
	Description      :
	Created at       : 17.08.01, @ 10:48:20
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogItem* CDialogDoc::AddItems(BaseContainer *pItemInfo, CDialogItem* pInsertAfter, PROGRESS_ID progressID,
																	Bool bCheckNames, Bool bMakeVisible)
{
	m_pTreeDlg->SetSelItem(pInsertAfter, false);

	BfProgressUpdate(nullptr, progressID, m_lAddItemCount, m_lItemCount, "");

	Int32 lType = pItemInfo->GetId();//>GetInt32(DR_TYPE);
	CDialogItem* pNewItem = nullptr;
	if (lType != DRTYPE_DIALOG)
	{
		//if (lType == Dialog) pNewItem = AddItem(Template);
		if (lType == DROLDWIN_STATICTEXT) pNewItem = AddItem(Static, true, false, bMakeVisible);
		else if (lType == DRTYPE_GROUP || lType == DRTYPE_TAB || lType == DRTYPE_SCROLLGROUP ||
						 lType == DRTYPE_RADIOGROUP) pNewItem = AddItem(Group, true, false, bMakeVisible);
		else if (lType == DROLDWIN_CHECKBOX) pNewItem = AddItem(CheckBox, true, false, bMakeVisible);
		else if (lType == DROLDWIN_RADIOGADGET || lType == DROLDWIN_SPECIALRADIO) pNewItem = AddItem(RadioBtn, true, false, bMakeVisible);
		else if (lType == DROLDWIN_BUTTON) pNewItem = AddItem(Button, true, false, bMakeVisible);
		else if (lType == DROLDWIN_EDITTEXT || lType == DROLDWIN_EDITNUMBER ||
						 lType == DROLDWIN_EDITNUMBERARROWS || lType == DROLDWIN_MULTILINETEXT ||
						 lType == DROLDWIN_SLIDERNUM)
			pNewItem = AddItem(EditBox, true, false, bMakeVisible);
		else if (lType == DROLDWIN_COLORFIELD) pNewItem = AddItem(Color, true, false, bMakeVisible);
		else if (lType == DROLDWIN_ARROWBUTTON || lType == DROLDWIN_POPUPBUTTON) pNewItem = AddItem(ArrowBtn, true, false, bMakeVisible);
		else if (lType == DROLDWIN_SLIDER) pNewItem = AddItem(Slider, true, false, bMakeVisible);
		else if (lType == DRTYPE_SEPARATOR) pNewItem = AddItem(Separator, true, false, bMakeVisible);
		else if (lType == DROLDWIN_LISTVIEW) pNewItem = AddItem(ListBox, true, false, bMakeVisible);
		else if (lType == DROLDWIN_COMBOBOX) pNewItem = AddItem(ComboBox, true, false, bMakeVisible);
		else if (lType == DROLDWIN_COMBOBUTTON) pNewItem = AddItem(ComboButton, true, false, bMakeVisible);
		else if (lType == DRTYPE_DLGGROUP) pNewItem = AddItem(DialogGroup, true, false, bMakeVisible);
		else if (lType == DROLDWIN_OWNERDRAW) pNewItem = AddItem(UserArea, true, false, bMakeVisible);
		else if (lType == DROLDWIN_SUBDIALOG) pNewItem = AddItem(SubDialogControl, true, false, bMakeVisible);
		else if (lType == DROLDWIN_SDK) pNewItem = AddItem(CustomElement, true, false, bMakeVisible);

		if (!pNewItem)
			return nullptr;

		m_lAddItemCount++;

		if (bCheckNames && pItemInfo->GetString(DR_ID) != "IDC_STATIC")
		{
			tagAddCheckName cn;
			cn.bc = pItemInfo;
			cn.lMinID = -1;
			cn.strID = TrimNumbers(pItemInfo->GetString(DR_ID));
			m_pTreeDlg->GetTreeView()->VisitChildren(AddItemCheckNameCallback, nullptr, &cn);
			if (cn.lMinID >= 0)
				pItemInfo->SetString(DR_ID, cn.strID + String::IntToString(cn.lMinID));
			else
				pItemInfo->SetString(DR_ID, cn.strID);
		}

		pNewItem->Load(pItemInfo);
	}
	else
		pNewItem = pInsertAfter;

	Int32 lChildren = pItemInfo->GetInt32(DR_NUMCHILDREN);
	for (Int32 l = 0; l < lChildren; l++)
	{
		BaseContainer bc = pItemInfo->GetContainer(DR_CHILD + l);
		AddItems(&bc, pNewItem, progressID, bCheckNames, bMakeVisible);
	}
	return pNewItem;
}

/*********************************************************************\
	Function name    : CDialogDoc::LoadStringTables
	Description      :
	Created at       : 22.08.01, @ 10:04:11
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::LoadStringTables(Filename fnStart, String strTitle, StringTableType stt)
{
	Bool bRes = false;

	Filename fnRes;
	m_StringTable.Free();

	AutoAlloc<BrowseFiles> pBrowse;
	pBrowse->Init(fnStart, false);
	while (pBrowse->GetNext())
	{
		if (pBrowse->IsDir())
		{
			Filename fn = pBrowse->GetFilename();
			if (fn.GetString().SubStr(0, 8).ToLower() == "strings_")
			{
				String idx = fn.GetString();
				idx.Delete(0, 8);


				TRACE_STRING(strTitle);
				if (strTitle.SubStr(strTitle.GetLength() - 4, 4) != ".str")
					strTitle += ".str";

				String strTitleA = strTitle;
				Int32 lPos;
				if (strTitleA.FindLast(".str", &lPos))
					strTitleA = strTitleA.SubStr(0, lPos);

				Filename fnRes = fnStart + fn + strTitle;
				TRACE_STRING(fnRes.GetString());

				if (GeFExist(fnRes))
					bRes |= LoadStringTable(fnRes, idx, strTitleA, stt);
				else
				{
					fnRes = fnStart + fn + String("dialogs") + strTitle;
					if (GeFExist(fnRes))
						bRes |= LoadStringTable(fnRes, idx, strTitleA, stt);
				}
			}
		}
	}

	if (bRes) m_fnStringTableStart = fnStart;
	return bRes;
}

/*********************************************************************\
	Function name    : CDialogDoc::LoadStringTable
	Description      :
	Created at       : 22.08.01, @ 10:14:11
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::LoadStringTable(Filename fnString, String strSuffix, String strTitle, StringTableType stt)
{
	//TRACE("Loading string table ");
	//TRACE_STRING(fnString.GetString());

	const char* pChIdent=nullptr;
	if (stt == typeDialogStrings) pChIdent = "DIALOGSTRINGS";
	else if (stt == typeStringTable) pChIdent = "STRINGTABLE";
	else if (stt == typeCommandStrings) pChIdent = "COMMANDSTRINGS";
	else
		return false;

	DialogParser p;
	SYMBOL sym;
	if (!p.Init(fnString, true, false))
	{
		sym = p.GetSymbol();
		while (!p.Error())
		{
			if (sym==SYM_OPERATOR_GESCHWEIFTZU) break;

			if (sym!=SYM_IDENT)
				DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);

			if (p.CheckIdent(pChIdent))
			{
				if (stt == typeDialogStrings)
				{
					sym = p.nextsym();
					if (sym!=SYM_IDENT) DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);

					String strDialog = p.GetIdent();
					if (strDialog.LexCompare(strTitle))
					{
						GeOutString(GeLoadString(IDS_DIALOG_RESOURCE_CORRUPTED), GEMB_OK | GEMB_ICONEXCLAMATION);
						GOTO_ERROR;
					}
				}

				Int32 lLang = m_StringTable.AddLanguage(strSuffix);
				//TRACE_STRING(strSuffix);
				if (!LoadDialogStrings(p, lLang, stt))
					GOTO_ERROR;
			}
			else
				GOTO_ERROR;

			sym = p.GetSymbol();
		}
	}

	return true;
error:
	return false;
}

/*********************************************************************\
	Function name    : CDialogDoc::LoadDialogStrings
	Description      :
	Created at       : 22.08.01, @ 10:51:40
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::LoadDialogStrings(DialogParser &p, Int32 lLang, StringTableType stt)
{
	SYMBOL sym = p.nextsym();

	if (sym!=SYM_OPERATOR_GESCHWEIFTAUF) DLG_IO_GOTO_ERROR(ERR_EXPECTED_GESCHWEIFT_AUF);

	sym = p.nextsym();

	while (!p.Error())
	{
		if (sym==SYM_OPERATOR_GESCHWEIFTZU) break;
		if (sym==SYM_OPERATOR_SEMIKOLON)
		{
			sym=p.nextsym();
			continue;
		}

		if (sym!=SYM_IDENT)
			DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);

		String ident = p.GetIdent();

		sym=p.nextsym();
		if (sym!=SYM_STRING)
			DLG_IO_GOTO_ERROR(ERR_EXPECTED_STRING);


		if (stt == typeCommandStrings)
		{
			String strText = p.GetIdent();
			sym = p.nextsym();
			if (sym!=SYM_STRING)
				DLG_IO_GOTO_ERROR(ERR_EXPECTED_STRING);

			String strCommand = p.GetIdent();
			m_StringTable.SetItemText(ident, lLang, strText, &strCommand);
		}
		else
		{
			m_StringTable.SetItemText(ident, lLang, p.GetIdent());
		}

		sym = p.nextsym();
		if (sym!=SYM_OPERATOR_SEMIKOLON)
			DLG_IO_GOTO_ERROR(ERR_EXPECTED_SEMIKOLON);

		sym = p.nextsym();
	}
	return !p.Error();

error:
	p.SetError();
	return false;
}

/*********************************************************************\
	Function name    : CDialogDoc::SaveStringTable
	Description      : stores the string table on the disk, uses the directory from
										 where it was loaded. If a new dialog was created the user will be asked
										 for the path name
	Created at       : 31.08.01, @ 11:15:41
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::SaveStringTable(StringTableType stt)
{
	//TRACE_STRING(m_fnStringTableStart.GetString());

	if (!m_fnStringTableStart.Content())
	{
		// at this point, we know the filename of the dialog resource
		Filename fnDir = m_FileName.GetDirectory(); // should be the dialog path
		//TRACE_STRING(fnDir.GetString());
		fnDir = fnDir.GetDirectory();  // should be the res path
		//TRACE_STRING(fnDir.GetString());

		if (!fnDir.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_DIRECTORY, GeLoadString(IDS_SPECIFY_RES_PATH))) return false;
		m_fnStringTableStart = fnDir;
	}

	//TRACE_STRING(m_fnStringTableStart.GetString());

	Int32 lItems = m_StringTable.GetNumElements();
	// get the length of the longest item
	Int32 lMaxIDLen = 0;
	Int32 l;
	for (l = 0; l < lItems; l++)
	{
		String strName = m_StringTable.GetElementID(l);
		lMaxIDLen = LMax(lMaxIDLen, strName.GetLength());
	}

	for (Int32 lLanguageCount = 0; lLanguageCount < g_LanguageList.GetNumLanguages(); lLanguageCount++) {
		Filename strDir = m_fnStringTableStart;
		strDir += ("strings_" + g_LanguageList.GetLanguageSuffix(lLanguageCount));
		Int32 lLangID = m_StringTable.GetLanguageID(g_LanguageList.GetLanguageSuffix(lLanguageCount));
		if (lLangID < 0) continue;

		if (!GeFExist(strDir, true))
			GeFCreateDir(strDir);
		if (!GeFExist(strDir, true)) return false;

		if (stt == typeDialogStrings)
		{
			strDir += String("dialogs");
			if (!GeFExist(strDir, true))
				GeFCreateDir(strDir);
		}

		if (!GeFExist(strDir, true)) return false;

		String strTitle = m_FileName.GetFile().GetString();
		Int32 lPos;
		if (strTitle.FindLast('.', &lPos))
			strTitle = strTitle.SubStr(0, lPos);
		//TRACE_STRING(strTitle);

		strDir += (strTitle + String(".str"));
		TRACE_STRING(strDir.GetString());

		AutoAlloc <BaseFile> pFile;
		if (!pFile)
			return false;

		if (!ForceOpenFileWrite(pFile, strDir))
		{
			return false;
		}

		// write the resource
		if (stt == typeDialogStrings)
		{
			WriteString(pFile, "// C4D-DialogResource"); LineBreak(pFile, ""); LineBreak(pFile, "");
			WriteString(pFile, "DIALOGSTRINGS ");
			WriteString(pFile, m_pDialog->m_strControlID);
			LineBreak(pFile, "");
		}
		else if (stt == typeStringTable)
		{
			WriteString(pFile, "// C4D-StringResource"); LineBreak(pFile, "");
			WriteString(pFile, "// Identifier	Text "); LineBreak(pFile, ""); LineBreak(pFile, "");
			WriteString(pFile, "STRINGTABLE"); LineBreak(pFile, "");
		}
		else if (stt == typeCommandStrings)
		{
			WriteString(pFile, "// C4D-StringResource"); LineBreak(pFile, "");
			WriteString(pFile, "// Identifier	Text "); LineBreak(pFile, ""); LineBreak(pFile, "");
			WriteString(pFile, "COMMANDSTRINGS"); LineBreak(pFile, "");
		}

		WriteString(pFile, "{");
		for (l = 0; l < lItems; l++)
		{
			String strCommand;
			LineBreak(pFile, SPACE_NEW_LINE);
			String strID = m_StringTable.GetElementID(l);
			String strName = m_StringTable.GetString(l, lLangID, &strCommand);
			WriteString(pFile, strID);
			Int32 b = 2 + lMaxIDLen - strID.GetLength();
			while (b-- > 0) WriteString(pFile, " ");
			WriteString(pFile, "\"");
			WriteString(pFile, strName, true);

			if (stt == typeCommandStrings)
			{
				WriteString(pFile, "\"   \"");
				//String strName = m_StringTable.GetCommandString(l, lLangID);
				WriteString(pFile, strCommand, true);
			}

			WriteString(pFile, "\";");
		}
		LineBreak(pFile, "");
		WriteString(pFile, "}");
		LineBreak(pFile, "");
	}

	return true;
}

/*********************************************************************\
	Function name    : WriteDialogItemIDsCallback
	Description      :
	Created at       : 05.09.01, @ 16:45:05
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void WriteDialogItemIDsCallback(TreeViewItem* pItem, void* pData)
{
	CDialogItem* pDlgItem = (CDialogItem*)pItem->GetData()->GetVoid(TREEVIEW_DATA);
	if (!pDlgItem) return;

	BaseFile* pFile = (BaseFile*)pData;

	String str = pDlgItem->m_strControlID;

	if (str.Content() && (str != "IDC_STATIC") && (!IsInteger(str)))
	{
		LineBreak(pFile, SPACE_NEW_LINE);
		WriteString(pFile, str);
		WriteString(pFile, ",");
	}

	//TODO
	// also write element IDs of arrow buttons and combo boxes
	ItemType type = pDlgItem->GetType();
	if (type == ComboBox || type == ComboButton || type == ArrowBtn)
	{
		CChildItems* pItems;
		if (type == ComboBox) pItems = ((CDialogComboBox*)pDlgItem)->GetChildren();
		else if (type == ComboButton) pItems = ((CDialogComboButton*)pDlgItem)->GetChildren();
		else pItems = ((CDialogArrowBtn*)pDlgItem)->GetChildren();

		String strID, strStringID;
		Int32 lChildren = pItems->GetItemCount();
		for (Int32 b = 0; b < lChildren; b++)
		{
			pItems->GetElement(b, strID, strStringID);
			if (IsInteger(strID)) continue;

			LineBreak(pFile, SPACE_NEW_LINE);
			WriteString(pFile, strID);
			WriteString(pFile, ",");
		}
	}
}

/*********************************************************************\
	Function name    : CDialogDoc::WriteStringsIDs
	Description      :
	Created at       : 05.09.01, @ 19:41:24
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::WriteStringsIDs(BaseFile* pDestFile, String strStringBegin, String strStringEnd, Bool bIsGlobalTable)
{
	// just write the strings if the user wants it
	if (g_bSaveStringIDs || bIsGlobalTable)
	{
		WriteString(pDestFile, SPACE_NEW_LINE);
		// we didn't find the string table yet, so write it now
		/*LineBreak(pDestFile, SPACE_NEW_LINE); */WriteString(pDestFile, strStringBegin);
		Int32 lElements = m_StringTable.GetNumElements();
		for (Int32 l = 0; l < lElements; l++)
		{
			LineBreak(pDestFile, SPACE_NEW_LINE);
			WriteString(pDestFile, m_StringTable.GetElementID(l));
			WriteString(pDestFile, ",");
		}
		LineBreak(pDestFile, SPACE_NEW_LINE); WriteString(pDestFile, strStringEnd);
		LineBreak(pDestFile, ""); LineBreak(pDestFile, "");
	}
}

/*********************************************************************\
	Function name    : CDialogDoc::WriteDialogIDs
	Description      :
	Created at       : 05.09.01, @ 19:45:41
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::WriteDialogIDs(BaseFile* pDestFile, String strDialogBegin, String strDialogEnd)
{
	// we didn't find the string table yet, so write it now
	WriteString(pDestFile, SPACE_NEW_LINE);
	/*LineBreak(pDestFile, SPACE_NEW_LINE); */WriteString(pDestFile, strDialogBegin);
	m_pTreeDlg->GetTreeView()->VisitChildren(WriteDialogItemIDsCallback, nullptr, (void*)pDestFile);
	LineBreak(pDestFile, SPACE_NEW_LINE); WriteString(pDestFile, strDialogEnd);
	LineBreak(pDestFile, ""); LineBreak(pDestFile, "");
}

/*********************************************************************\
	Function name    : CDialogDoc::SaveHeaderFile
	Description      :
	Created at       : 05.09.01, @ 13:46:42
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::SaveHeaderFile(Bool bGlobalStringTable)
{
	// and our modified data
	// now, we know the resource start directory : m_fnStringTableStart

	Filename fn;
	AutoAlloc <BaseFile> pSrcFile, pDestFile;
	if (!pSrcFile || !pDestFile)
		return false;

	// open the header file we still have
	fn = m_fnStringTableStart + String("c4d_symbols.h");
	if (!pSrcFile->Open(fn, FILEOPEN_READ))
	{
		// it does not exist yet, so let's create a default file
		if (!ForceOpenFileWrite(pSrcFile, fn))
		{
			return false;
		}
		WriteString(pSrcFile, "//*********************************************************************\\"); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, "// File name        : c4d_symbols.h"); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, "// Description      : symbol definition file for the plugin "); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, "// Created at       : "); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, "// Created by       : Resource editor"); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, "// Modified by      : "); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, "//*********************************************************************/"); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, "// WARNING : Only edit this file, if you exactly know what you are doing."); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, "// WARNING : The comments are important, too."); LineBreak(pSrcFile, ""); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, "enum"); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, "{"); LineBreak(pSrcFile, SPACE_NEW_LINE);
		WriteString(pSrcFile, "_FIRST_ELEMENT_      = 10000,"); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, "// End of symbol definition"); LineBreak(pSrcFile, "");
		WriteString(pSrcFile, SPACE_NEW_LINE); WriteString(pSrcFile, "_DUMMY_ELEMENT_");
		LineBreak(pSrcFile, ""); WriteString(pSrcFile, "}");
		LineBreak(pSrcFile, "");
		pSrcFile->Close();

		// try to open the original header, again
		if (!pSrcFile->Open(fn, FILEOPEN_READ, FILEDIALOG_NONE))
		{
			return false;
		}
	}

	// open a temporary file where we copy the original c4d_symbols.h file
	fn = m_fnStringTableStart + String("c4d_symbols.tmp");
	if (!ForceOpenFileWrite(pDestFile, fn))
	{
		return false;
	}

	// copy the contents of the original file to the temporary file,
	// replace old dialog and string definitions

	Int32 lPos;
	String strTitle = m_FileName.GetFile().GetString();
	if (strTitle.FindLast('.', &lPos)) strTitle = strTitle.SubStr(0, lPos);
	//TRACE_STRING(strTitle);

	String strStringBegin = String("// String table definition of ") + strTitle + " starts here";
	String strStringEnd = String("// String table definition of ") + strTitle + " ends here";
	String strDialogBegin = String("// Dialog definitions of ") + strTitle + " start here";
	String strDialogEnd = String("// Dialog definitions of ") + strTitle + " end here";
	String strGlobalBegin = String("// Global string definitions start here");
	String strGlobalEnd = String("// Global string definitions end here");

	Bool bWroteString = false, bWroteDialog = false, bWroteGlobal = false;
	String str;
	Bool bIgnoreString = false, bIgnoreDialog = false, bIgnoreGlobal = false;
	Bool bIgnoreNextEmptyLine = false;
	while (ReadString(pSrcFile, str))
	{
		Int32 lPos;
		if (str.FindFirst("// End of symbol definition", &lPos)) break;
		if (str.FindFirst("}", &lPos)) break;

		if (bGlobalStringTable)
		{
			if (str.FindFirst(strGlobalBegin, &lPos))
				bIgnoreGlobal = true;
			if (str.FindFirst(strGlobalEnd, &lPos) && bIgnoreGlobal)
			{
				bIgnoreGlobal = false;
				WriteStringsIDs(pDestFile, strGlobalBegin, strGlobalEnd, true);
				bWroteGlobal = true;
				bIgnoreNextEmptyLine = true;
				continue;
			}
		}
		else
		{
			if (str.FindFirst(strStringBegin, &lPos))
				bIgnoreString = true;
			if (str.FindFirst(strStringEnd, &lPos) && bIgnoreString)
			{
				bIgnoreString = false;
				WriteStringsIDs(pDestFile, strStringBegin, strStringEnd, false);
				bWroteString = true;
				bIgnoreNextEmptyLine = true;
				continue;
			}

			if (str.FindFirst(strDialogBegin, &lPos))
				bIgnoreDialog = true;
			if (str.FindFirst(strDialogEnd, &lPos) && bIgnoreDialog)
			{
				bIgnoreDialog = false;
				WriteDialogIDs(pDestFile, strDialogBegin, strDialogEnd);
				bWroteDialog = true;
				bIgnoreNextEmptyLine = true;
				continue;
			}
		}

		if (!bIgnoreString && !bIgnoreDialog && !bIgnoreGlobal)
		{
			//TRACE_STRING(str);
			if ((str.GetLength() > 0) || (!bIgnoreNextEmptyLine && str.GetLength() == 0))
			{
				WriteString(pDestFile, str);
				LineBreak(pDestFile, "");
				if (str.GetLength() == 0)
					bIgnoreNextEmptyLine = false;
			}
		}
	}

	if (bGlobalStringTable)
	{
		if (!bWroteGlobal)
			WriteStringsIDs(pDestFile, strGlobalBegin, strGlobalEnd, true);
	}
	else
	{
		if (!bWroteString)
			WriteStringsIDs(pDestFile, strStringBegin, strStringEnd, false);

		if (!bWroteDialog)
			WriteDialogIDs(pDestFile, strDialogBegin, strDialogEnd);
	}

	LineBreak(pDestFile, ""); WriteString(pDestFile, "// End of symbol definition");
	LineBreak(pDestFile, "");
	WriteString(pDestFile, SPACE_NEW_LINE); WriteString(pDestFile, "_DUMMY_ELEMENT_");
	LineBreak(pDestFile, ""); WriteString(pDestFile, "};"); LineBreak(pDestFile, "");

	pSrcFile->Close();
	pDestFile->Close();

	// make a backup copy of the symbol file
	GeFKill(m_fnStringTableStart + String("c4d_symbols.bak"), GE_FKILL_FORCE);
	GeFCopyFile(m_fnStringTableStart + String("c4d_symbols.h"),
							m_fnStringTableStart + String("c4d_symbols.bak"),
							GE_FCOPY_OVERWRITE);

	/*// then, delete the c4d_symbols.h file and replace it by the temporary file
	if (GeFCopyFile(m_fnStringTableStart + String("c4d_symbols.tmp"),
									m_fnStringTableStart + String("c4d_symbols.h"),
									GE_FCOPY_OVERWRITE))
		// delete the temporary file only if updating the header file was successful
		GeFKill(m_fnStringTableStart + String("c4d_symbols.tmp"));*/

	m_bStringSymbolsChanged = false;
	m_bControlSymbolsChanged = false;

	ModifySymbolFile(m_fnStringTableStart, "c4d_symbols.tmp", "c4d_symbols.mod");
	if (!GeFKill(m_fnStringTableStart + String("c4d_symbols.h")))
	{
		String str = GeLoadString(IDS_WRITE_PROTECTION, (m_fnStringTableStart + String("c4d_symbols.h")).GetString());
		if (GeOutString(str, GEMB_ICONQUESTION | GEMB_YESNO) == GEMB_R_YES)
			GeFKill(m_fnStringTableStart + String("c4d_symbols.h"), GE_FKILL_FORCE);
	}

	if (GeFCopyFile(m_fnStringTableStart + String("c4d_symbols.mod"),
									m_fnStringTableStart + String("c4d_symbols.h"),
									GE_FCOPY_OVERWRITE))
	{
		// delete the temporary file only if updating the header file was successful
		GeFKill(m_fnStringTableStart + String("c4d_symbols.mod"), GE_FKILL_FORCE);
		GeFKill(m_fnStringTableStart + String("c4d_symbols.tmp"), GE_FKILL_FORCE);
 }

	return true;
}


/*********************************************************************\
	Function name    : CDialogDoc::SaveDialogResource
	Description      :
	Created at       : 30.08.01, @ 09:48:10
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::SaveDialogResource(const Filename &strFileName)
{
	AutoAlloc <BaseFile> pFile;
	if (!pFile) return false;

	if (!ForceOpenFileWrite(pFile, strFileName))
	{
		return false;
	}

	m_pDialog->Save(pFile, "");
	pFile->Close();

	// store the string resource...
	SaveStringTable(typeDialogStrings);

	// ... and create a header file
	if ((m_bStringSymbolsChanged && g_bSaveStringIDs) || m_bControlSymbolsChanged)
		SaveHeaderFile(false);

	return true;
}

typedef struct
{
	Int32 lLanguage;
	String strFill;
	BaseFile* pFile;
	CStringTable* pStringTable;
} tagExportMacro;

/*********************************************************************\
	Function name    : ExportMacroWriteStringCallback
	Description      :
	Created at       : 10.09.01, @ 14:22:27
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void ExportMacroWriteStringCallback(TreeViewItem* pItem, void* pData)
{
	CDialogItem* pDlgItem = (CDialogItem*)pItem->GetData()->GetVoid(TREEVIEW_DATA);
	if (!pDlgItem) return;

	tagExportMacro* pExport = (tagExportMacro*)pData;
	BaseFile* pFile = pExport->pFile;
	CStringTable* pStringTable = pExport->pStringTable;

	String strID = pDlgItem->GetNameID();
	if (strID.Content())
	{
		LineBreak(pFile, pExport->strFill);
		if (pExport->lLanguage == -1)
			WriteString(pFile, strID);
		else
		{
			Bool bFound;
			WriteString(pFile, "\"");
			WriteString(pFile, pStringTable->GetString(strID, pExport->lLanguage, bFound));
			WriteString(pFile, "\"");
		}
		WriteString(pFile, ";");
	}

	// also write the StringIDs of combo boxes and arrow buttons
	if (pDlgItem->GetType() != ComboBox && pDlgItem->GetType() != ComboButton && pDlgItem->GetType() != ArrowBtn)
		return;

	CChildItems* pChildren;
	if (pDlgItem->GetType() == ComboBox)
		pChildren = ((CDialogComboBox*)pDlgItem)->GetChildren();
	else if (pDlgItem->GetType() == ComboButton)
		pChildren = ((CDialogComboButton*)pDlgItem)->GetChildren();
	else
		pChildren = ((CDialogArrowBtn*)pDlgItem)->GetChildren();

	String strStringID;
	for (Int32 a = 0; a < pChildren->GetItemCount(); a++)
	{
		pChildren->GetElement(a, strID, strStringID);

		if (pExport->lLanguage == -1)
		{
			LineBreak(pFile, pExport->strFill);
			WriteString(pFile, strStringID);
			WriteString(pFile, ";");
		}
		else
		{
			LineBreak(pFile, pExport->strFill);
			Bool bFound;
			WriteString(pFile, "\"");
			WriteString(pFile, pStringTable->GetString(strStringID, pExport->lLanguage, bFound));
			WriteString(pFile, "\"");
			WriteString(pFile, ";");
		}
	}
}

/*********************************************************************\
	Function name    : CDialogDoc::ExportMacro
	Description      :
	Created at       : 10.09.01, @ 13:35:55
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::ExportMacro(Filename fnMacro, CDialogItem* pExportFrom)
{
	Int32 l;
	AutoAlloc <BaseFile> pFile;
	if (!pFile)
		return;
	if (!ForceOpenFileWrite(pFile, fnMacro))
		return;

	WriteString(pFile, "// Dialog macro file");
	LineBreak(pFile, ""); LineBreak(pFile, "");

	// write the string table
	WriteString(pFile, "STRINGTABLE"); LineBreak(pFile, "");
	WriteString(pFile, "{"); LineBreak(pFile, SPACE_NEW_LINE);
	// write all string IDs
	WriteString(pFile, "IDLIST"); LineBreak(pFile, SPACE_NEW_LINE);
	WriteString(pFile, "{");
	tagExportMacro ExportMacro;
	ExportMacro.lLanguage = -1;
	ExportMacro.pFile = pFile;
	ExportMacro.strFill = SPACE_NEW_LINE + SPACE_NEW_LINE;
	ExportMacro.pStringTable = &m_StringTable;

	m_pTreeDlg->GetTreeView()->VisitChildren(ExportMacroWriteStringCallback, pExportFrom->m_pTreeViewItem, &ExportMacro);

	LineBreak(pFile, SPACE_NEW_LINE);
	WriteString(pFile, "}"); LineBreak(pFile, SPACE_NEW_LINE);

	// now, write the strings
	WriteString(pFile, "STRINGS"); LineBreak(pFile, SPACE_NEW_LINE);
	WriteString(pFile, "{");

	for (l = 0; l < g_LanguageList.GetNumLanguages(); l++)
	{
		Int32 lLangID = m_StringTable.GetLanguageID(g_LanguageList.GetLanguageSuffix(l));
		if (lLangID < 0) continue;

		LineBreak(pFile, SPACE_NEW_LINE + SPACE_NEW_LINE);
		WriteString(pFile, "LANGUAGE "); WriteString(pFile, g_LanguageList.GetLanguageSuffix(l)); LineBreak(pFile, SPACE_NEW_LINE + SPACE_NEW_LINE);
		WriteString(pFile, "{");
		ExportMacro.lLanguage = lLangID;
		ExportMacro.strFill = SPACE_NEW_LINE + SPACE_NEW_LINE + SPACE_NEW_LINE;

		m_pTreeDlg->GetTreeView()->VisitChildren(ExportMacroWriteStringCallback, pExportFrom->m_pTreeViewItem, &ExportMacro);

		LineBreak(pFile, SPACE_NEW_LINE + SPACE_NEW_LINE);
		WriteString(pFile, "}");
	}

	LineBreak(pFile, SPACE_NEW_LINE);
	WriteString(pFile, "}"); LineBreak(pFile, "");
	WriteString(pFile, "}"); LineBreak(pFile, ""); LineBreak(pFile, "");

	WriteString(pFile, "DIALOGELEMENTS"); LineBreak(pFile, "");
	WriteString(pFile, "{"); LineBreak(pFile, SPACE_NEW_LINE);
	pExportFrom->Save(pFile, SPACE_NEW_LINE);
	LineBreak(pFile, ""); WriteString(pFile, "}");
}

typedef struct
{
	CStringTable* pStringTable, *pDocStringTable;
	Int32 lLanguage;
} tagMacroSetName;

/*********************************************************************\
	Function name    : ImportMacroSetNameIDCallback
	Description      :
	Created at       : 10.09.01, @ 15:56:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void ImportMacroSetNameIDCallback(TreeViewItem* pItem, void* pData)
{
	tagMacroSetName* pSetName = (tagMacroSetName*)pData;
	CDialogItem* pDlgItem = (CDialogItem*)pItem->GetData()->GetVoid(TREEVIEW_DATA);
	if (!pDlgItem) return;

	String strNameID = pDlgItem->GetNameID();
	if (strNameID.Content())
	{
		pDlgItem->m_strNameID = pSetName->pDocStringTable->CopyElement(pSetName->pStringTable, strNameID);
		Bool bFound;
		pDlgItem->m_strName = pSetName->pDocStringTable->GetString(pDlgItem->m_strNameID, pSetName->lLanguage, bFound);
	}

	if (pDlgItem->GetType() != ComboBox && pDlgItem->GetType() != ComboButton && pDlgItem->GetType() != ArrowBtn)
		return;

	CChildItems* pChildren;
	if (pDlgItem->GetType() == ComboBox)
		pChildren = ((CDialogComboBox*)pDlgItem)->GetChildren();
	else if (pDlgItem->GetType() == ComboButton)
		pChildren = ((CDialogComboButton*)pDlgItem)->GetChildren();
	else
		pChildren = ((CDialogArrowBtn*)pDlgItem)->GetChildren();

	String strID, strStringID;
	for (Int32 a = 0; a < pChildren->GetItemCount(); a++)
	{
		pChildren->GetElement(a, strID, strStringID);
		strStringID = pSetName->pDocStringTable->CopyElement(pSetName->pStringTable, strStringID);
		pChildren->SetElement(a, strID, strStringID);
	}
}

/*********************************************************************\
	Function name    : CDialogDoc::ImportMacro
	Description      :
	Created at       : 10.09.01, @ 13:35:56
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogItem* CDialogDoc::ImportMacro(Filename fnMacro, CDialogItem* pInsertAfter)
{
	//if (!GeFExist(fn)) return;

	DialogParser p;
	p.Init(fnMacro, true, false);
	SYMBOL sym;
	Int32 lChildren, l;
	CDialogItem* pFirstNew = nullptr;

	CDialogItem* pParent = (CDialogItem*)(pInsertAfter->m_pTreeViewItem->GetParent()->GetData()->GetVoid(TREEVIEW_DATA));
	if (pParent) pInsertAfter = pParent;

	sym = p.GetSymbol();

	BaseContainer DialogContainer;
	CStringTable StringTable;

	PROGRESS_ID	progressID = BfProgressNew(nullptr);

	while (!p.Error())
	{
		if (sym==SYM_OPERATOR_GESCHWEIFTZU) break;

		if (sym!=SYM_IDENT)
			DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);

		if (p.CheckIdent("STRINGTABLE"))
		{
			sym = p.nextsym();
			if (sym != SYM_OPERATOR_GESCHWEIFTAUF)
				DLG_IO_GOTO_ERROR(ERR_EXPECTED_GESCHWEIFT_AUF);

			if (!ImportLoadStringTable(p, &StringTable))
				GOTO_ERROR;

			//StringTable.Dump();
			sym = sym;
		}
		else if (p.CheckIdent("DIALOGELEMENTS"))
		{
			m_lItemCount = 0;
			if (!LoadDialogParts(p, &DialogContainer, DRTYPE_DIALOG, progressID))
				GOTO_ERROR;
		}
		else DLG_IO_GOTO_ERROR(ERR_UNKNOWN_ID);
		sym = p.nextsym();
	}

	m_pTreeDlg->GetTreeView()->LockWindowUpdate();
	m_bIsLoading = true;
	lChildren = DialogContainer.GetInt32(DR_NUMCHILDREN);
	for (l = 0; l < lChildren; l++)
	{
		BaseContainer bc = DialogContainer.GetContainer(DR_CHILD + l);
		CDialogItem* pNewItem = AddItems(&bc, pInsertAfter, progressID, true, false);
		if (!pFirstNew) pFirstNew = pNewItem;

		//m_StringTable.Dump();

		tagMacroSetName MacroSetName;
		MacroSetName.lLanguage = m_lLanguageIDTable;
		MacroSetName.pDocStringTable = &m_StringTable;
		MacroSetName.pStringTable = &StringTable;
		m_pTreeDlg->GetTreeView()->VisitChildren(ImportMacroSetNameIDCallback, pNewItem->m_pTreeViewItem, &MacroSetName);
		m_pTreeDlg->GetTreeView()->ExpandAllItems(false, pNewItem->m_pTreeViewItem);
	}
	m_bIsLoading = false;
	m_pTreeDlg->GetTreeView()->UnlockWindowUpdate();

	SelChanged(pInsertAfter);
	m_pTreeDlg->UpdateLayout();

	m_bChanged = true;
	UpdateAllViews(true);

	//m_StringTable.Dump();

	m_bControlSymbolsChanged = true;
	m_bStringSymbolsChanged = true;

	BfProgressDelete(nullptr, progressID);
	return pFirstNew;

error:
	BfProgressDelete(nullptr, progressID);
	GeOutString(GeLoadString(IDS_DIALOG_RESOURCE_CORRUPTED), GEMB_OK | GEMB_ICONEXCLAMATION);
	return nullptr;
}

class CStringID : public CListItem
{
public:
	String m_str;
};
/*********************************************************************\
	Function name    : CDialogDoc::ImportLoadStringTable
	Description      :
	Created at       : 10.09.01, @ 14:45:36
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::ImportLoadStringTable(DialogParser &p, CStringTable *pStringTable)
{
	SYMBOL sym = p.nextsym();
	String str;
	Bool bRet = false;

	CDoubleLinkedList <CStringID> StringIDs;

	while (sym != SYM_OPERATOR_GESCHWEIFTZU)
	{
		if (p.CheckIdent("IDLIST"))
		{
			sym = p.nextsym();
			if (sym != SYM_OPERATOR_GESCHWEIFTAUF) goto _exit;
			sym = p.nextsym();
			while (sym != SYM_OPERATOR_GESCHWEIFTZU)
			{
				if (sym != SYM_IDENT) goto _exit;

				pStringTable->AddElement(p.GetIdent());
				{
				CStringID* pID = NewObjClear(CStringID);
				pID->m_str = p.GetIdent();
				StringIDs.Append(pID);
				}

				//TRACE_STRING(p.GetIdent());
				sym = p.nextsym();

				if (sym != SYM_OPERATOR_SEMIKOLON) goto _exit;
				sym = p.nextsym();
			}
			sym = p.nextsym();
		}
		else if (p.CheckIdent("STRINGS"))
		{
			sym = p.nextsym();
			if (sym != SYM_OPERATOR_GESCHWEIFTAUF) goto _exit;
			sym = p.nextsym();

			while (sym != SYM_OPERATOR_GESCHWEIFTZU)
			{
				if (sym != SYM_IDENT) goto _exit;

				if (p.CheckIdent("LANGUAGE"))
				{
					sym = p.nextsym();
					if (sym != SYM_IDENT) goto _exit;

					{
					String strLangSuffix = p.GetIdent();

					Int32 lLang;
					lLang = pStringTable->GetLanguageID(strLangSuffix);
					if (lLang < 0)
						lLang = pStringTable->AddLanguage(strLangSuffix);

					Int32 lItemCount = 0;
					CStringID* pStringID = StringIDs.First();

					sym = p.nextsym();
					if (sym != SYM_OPERATOR_GESCHWEIFTAUF) goto _exit;

					sym = p.nextsym();
					while (sym != SYM_OPERATOR_GESCHWEIFTZU)
					{
						if (sym != SYM_STRING) goto _exit;

						str = p.GetIdent();

						sym = p.nextsym();
						if (sym != SYM_OPERATOR_SEMIKOLON) goto _exit;
						sym = p.nextsym();

						if (lLang >= 0)
						{
							if (pStringID)
								pStringTable->SetItemText(pStringID->m_str, lLang, str); // add a new element
							else
								pStringID = pStringID;

							lItemCount++;
							pStringID = StringIDs.Next(pStringID);
						}
					}
					}
					sym = p.nextsym();
				}
				else
					goto _exit;
			}
			sym = p.nextsym();
		}
		else
			goto _exit;
	}

	bRet = true;
_exit:
	StringIDs.FreeList();

	//pStringTable->Dump();

	return bRet;
}

class CSymbolListItem : public CListItem
{
public:
	String str;
};

/*********************************************************************\
	Function name    : CDialogDoc::ChangeSymbolFile
	Description      : modifies the symbol file that we have no duplicate strings
	Created at       : 06.01.02, @ 17:13:57
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogDoc::ModifySymbolFile(Filename fnPath, String strSymbols, String strTemp)
{
	CDoubleLinkedList <CSymbolListItem> stringList;

	AutoAlloc <BaseFile> pFile;
	if (!pFile)
		return;
	Filename fnSymbols = fnPath + strSymbols;
	if (!pFile->Open(fnSymbols, FILEOPEN_READ, FILEDIALOG_NONE))
		return;

	String str;
	Bool bCopyInList = false;
	Int32 lPos;

	CStringTable table;

	while (ReadString(pFile, str))
	{
		if (str.FindFirst('{', &lPos) && lPos >= 0)
		{
			bCopyInList = true; continue;
		}
		if (str.FindFirst('}', &lPos) && lPos >= 0)
		{
			bCopyInList = false; continue;
		}

		if (bCopyInList)
		{
			Int32 lPos1, lPos2;
			if (str.FindFirst("//", &lPos1))
			{
				// delete the // only if it is not followed by a ' '
				if (!str.FindFirst("// ", &lPos2))
				{
					str = str.SubStr(0, lPos1) + str.SubStr(lPos1 + 2, str.GetLength() - lPos1 - 2);
				}
			}

			CSymbolListItem* pNewItem = NewObjClear(CSymbolListItem);
			pNewItem->str = str;
			stringList.Append(pNewItem);
		}

		// copy every symbol in our list
		if (table.GetElementIndex(str) < 0)
			table.AddElement(str);
	}
	if (!pFile->Close())
		return;

	if (!pFile->Open(fnPath + strSymbols, FILEOPEN_READ, FILEDIALOG_NONE))
		return;

	bCopyInList = false;

	AutoAlloc <BaseFile> pOutFile;
	if (!pOutFile)
		return;
	if (!ForceOpenFileWrite(pOutFile, fnPath + strTemp))
		return;

	while (ReadString(pFile, str))
	{
		if (str.FindFirst('}', &lPos) && lPos >= 0)
		{
			bCopyInList = false;
		}

		if (!bCopyInList)
		{
			WriteString(pOutFile, str);
			LineBreak(pOutFile, "");
		}

		if (bCopyInList) continue;

		if (str.FindFirst('{', &lPos) && lPos >= 0)
		{
			// copy our symbol table in the file
			bCopyInList = true;

			Int32 lElements = table.GetNumElements();
			Char* pChUsed = bNewDeprecatedUseArraysInstead<Char>(lElements);

			CSymbolListItem* pStringItem = (CSymbolListItem*)stringList.First();
			while (pStringItem)
			{
				String &str = pStringItem->str;

				Int32 lIndex = table.GetElementIndex(str);
				ASSERT(lIndex >= 0 && lIndex < lElements);
				if (pChUsed[lIndex] && str.Content())
				{
					SkipChar(str, ' ');
					WriteString(pOutFile, String("  //") + str);
				}
				else
					WriteString(pOutFile, str);

				LineBreak(pOutFile, "");

				pChUsed[lIndex] = 1;

				pStringItem = (CSymbolListItem*)stringList.Next(pStringItem);
			}

			bDelete(pChUsed);
		}
	}

	pOutFile->Close();
	pFile->Close();

	stringList.FreeList();
}
