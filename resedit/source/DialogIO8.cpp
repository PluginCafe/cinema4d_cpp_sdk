/*********************************************************************\
	File name        : DialogIO8.cpp
	Description      :
	Created at       : 02.05.02, @ 17:29:04
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"
#include "DialogItem.h"

#ifdef VERSION_8

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

Bool ReadValue(ASCIIParser &p, /*Resource *strings, */CUSTOMTYPE type, GeData &res);

/*********************************************************************\
	Function name    : DialogResource::LoadDialogParts
	Description      :
	Created at       : 02.05.02, @ 17:29:27
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogDoc::LoadDialogParts(DialogParser &p,BaseContainer *bc,Int32 type, PROGRESS_ID	progressID)
{
	SYMBOL sym = p.nextsym();
	Bool needsemi;
	Bool done;
	Int32 w;

	BfProgressUpdate(nullptr, progressID, p.GetCurrentReadPosition() * 100 / p.GetTotalFileSize(), 100, "");

	if (sym!=SYM_OPERATOR_GESCHWEIFTAUF)
		GOTO_ERROR;

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
			GOTO_ERROR;
		needsemi = false; done = false;

		if (p.CheckIdent("NAME"))
		{
			String name;
			sym = p.nextsym(); if (sym!=SYM_IDENT) DLG_IO_GOTO_ERROR(ERR_EXPECTED_IDENT);
			name = p.GetIdent();

			/*ResourceObj *strobj = strings.FindObj(p.GetIdent());
			String name;
			if (!strobj)
			{
				GeOutString(GeLoadString(IDS_SYMBOL_NOT_FOUND,p.GetIdent(),p.GetFilename().GetString(),String::IntToString(p.GetLine())),GEMB_OK+GEMB_ICONEXCLAMATION);
			}
			else
			{
				name = strobj->data.GetString(0);
			}*/

			if (name.Content()) bc->SetData(DR_NAME, name);
			needsemi++; done++;
		}
		else if (p.CheckIdent("FIT_V"))
		{
			Int32 flags = bc->GetData(DR_ALIGNMENT).GetInt32();
			flags &= ~BFV_MASK;
			flags |= BFV_FIT;
			bc->SetData(DR_ALIGNMENT,flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("FIT_H"))
		{
			Int32 flags = bc->GetData(DR_ALIGNMENT).GetInt32();
			flags &= ~BFH_MASK;
			flags |= BFH_FIT;
			bc->SetData(DR_ALIGNMENT,flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("ALIGN_LEFT"))
		{
			Int32 flags = bc->GetData(DR_ALIGNMENT).GetInt32();
			flags &= ~BFH_MASK;
			flags |= BFH_LEFT;
			bc->SetData(DR_ALIGNMENT,flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("ALIGN_RIGHT"))
		{
			Int32 flags = bc->GetData(DR_ALIGNMENT).GetInt32();
			flags &= ~BFH_MASK;
			flags |= BFH_RIGHT;
			bc->SetData(DR_ALIGNMENT,flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("ALIGN_TOP"))
		{
			Int32 flags = bc->GetData(DR_ALIGNMENT).GetInt32();
			flags &= ~BFV_MASK;
			flags |= BFV_TOP;
			bc->SetData(DR_ALIGNMENT,flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("ALIGN_BOTTOM"))
		{
			Int32 flags = bc->GetData(DR_ALIGNMENT).GetInt32();
			flags &= ~BFV_MASK;
			flags |= BFV_BOTTOM;
			bc->SetData(DR_ALIGNMENT,flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("CENTER_H"))
		{
			Int32 flags = bc->GetData(DR_ALIGNMENT).GetInt32();
			flags &= ~BFH_MASK;
			flags |= BFH_CENTER;
			bc->SetData(DR_ALIGNMENT,flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("CENTER_V"))
		{
			Int32 flags = bc->GetData(DR_ALIGNMENT).GetInt32();
			flags &= ~BFV_MASK;
			flags |= BFV_CENTER;
			bc->SetData(DR_ALIGNMENT,flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("SCALE_H"))
		{
			Int32 flags = bc->GetData(DR_ALIGNMENT).GetInt32();
			flags &= ~BFH_MASK;
			flags |= BFH_SCALEFIT;
			bc->SetData(DR_ALIGNMENT,flags);
			needsemi++; done++;
		}
		else if (p.CheckIdent("SCALE_V"))
		{
			Int32 flags = bc->GetData(DR_ALIGNMENT).GetInt32();
			flags &= ~BFV_MASK;
			flags |= BFV_SCALEFIT;
			bc->SetData(DR_ALIGNMENT,flags);
			needsemi++; done++;
		}
		else switch (type)
		{
			case DRTYPE_SCROLLGROUP:
				if      (p.CheckIdent("SCROLL_V")) { bc->SetData(DR_SCROLL_V,true); needsemi++; done++; }
				else if (p.CheckIdent("SCROLL_H")) { bc->SetData(DR_SCROLL_H,true); needsemi++; done++; }
				else if (p.CheckIdent("SCROLL_BORDER")) { bc->SetData(DR_SCROLL_BORDERIN,true); needsemi++; done++; }
				else if (p.CheckIdent("SCROLL_STATUSBAR")) { bc->SetData(DR_SCROLL_STATUSBAR,true); needsemi++; done++; }
				else if (p.CheckIdent("SCROLL_AUTO_H")) { /*bc->SetData(DR_SCROLL_AUTO_H,true); */bc->SetData(DR_SCROLL_AUTO_H,true); needsemi++; done++; }
				else if (p.CheckIdent("SCROLL_AUTO_V")) { /*bc->SetData(DR_SCROLL_AUTO_V,true); */bc->SetData(DR_SCROLL_AUTO_V,true); needsemi++; done++; }
				else if (p.CheckIdent("SCROLL_NOBLIT")) { bc->SetData(DR_SCROLL_NOBLIT,true); needsemi++; done++; }
				else if (p.CheckIdent("SCROLL_LEFT")) { bc->SetData(DR_SCROLL_LEFT,true); needsemi++; done++; }				else goto trygadgets;
				break;

			case DROLDWIN_ARROWBUTTON:
				if      (p.CheckIdent("ARROW_LEFT"))  { bc->SetData(DR_ARROWTYPE,(Int32)ARROW_LEFT);  needsemi++; done++; }
				else if (p.CheckIdent("ARROW_RIGHT")) { bc->SetData(DR_ARROWTYPE,(Int32)ARROW_RIGHT); needsemi++; done++; }
				else if (p.CheckIdent("ARROW_UP"))    { bc->SetData(DR_ARROWTYPE,(Int32)ARROW_UP);    needsemi++; done++; }
				else if (p.CheckIdent("ARROW_DOWN"))  { bc->SetData(DR_ARROWTYPE,(Int32)ARROW_DOWN);  needsemi++; done++; }

				else if (p.CheckIdent("ARROW_SMALL_LEFT"))  { bc->SetData(DR_ARROWTYPE,(Int32)ARROW_SMALL_LEFT);  needsemi++; done++; }
				else if (p.CheckIdent("ARROW_SMALL_RIGHT")) { bc->SetData(DR_ARROWTYPE,(Int32)ARROW_SMALL_RIGHT); needsemi++; done++; }
				else if (p.CheckIdent("ARROW_SMALL_UP"))    { bc->SetData(DR_ARROWTYPE,(Int32)ARROW_SMALL_UP);    needsemi++; done++; }
				else if (p.CheckIdent("ARROW_SMALL_DOWN"))  { bc->SetData(DR_ARROWTYPE,(Int32)ARROW_SMALL_DOWN);  needsemi++; done++; }
				else goto readgadgets;
				break;

			case DRTYPE_DIALOG:
				if (p.CheckIdent("WINRES"))
				{
					// old windows resource
					/*BaseContainer sub(DRTYPE_OLDWIN);
					BaseContainer *s = bc->SetContainer(DR_CHILD, sub);
					if (!LoadDialogParts(p,s,DRTYPE_OLDWIN, progressID)) goto error;*/
					BaseContainer sub;
					if (!LoadDialogParts(p, &sub, DRTYPE_OLDWIN, progressID)) DLG_IO_GOTO_ERROR(0);
					Int32 lChildren = bc->GetInt32(DR_NUMCHILDREN);
					bc->SetContainer(DR_CHILD + lChildren, sub);
					bc->SetInt32(DR_NUMCHILDREN, lChildren + 1);

					done++;
				}
				else goto trygadgets;
				break;

			case DRTYPE_TAB:
				if (p.CheckIdent("SELECTION_NONE"))
				{
					bc->SetData(DR_TAB_SELECTBY,(Int32)TAB_NOSELECT);
					needsemi++; done++;
				}
				else if (p.CheckIdent("SELECTION_TABS"))
				{
					bc->SetData(DR_TAB_SELECTBY,(Int32)TAB_TABS);
					needsemi++; done++;
				}
				else if (p.CheckIdent("SELECTION_VLTABS"))
				{
					bc->SetData(DR_TAB_SELECTBY,(Int32)TAB_VLTABS);
					needsemi++; done++;
				}
				else if (p.CheckIdent("SELECTION_VRTABS"))
				{
					bc->SetData(DR_TAB_SELECTBY,(Int32)TAB_VRTABS);
					needsemi++; done++;
				}
				else if (p.CheckIdent("SELECTION_CYCLE"))
				{
					bc->SetData(DR_TAB_SELECTBY,(Int32)TAB_CYCLE);
					needsemi++; done++;
				}
				else if (p.CheckIdent("SELECTION_RADIO"))
				{
					bc->SetData(DR_TAB_SELECTBY,(Int32)TAB_RADIO);
					needsemi++; done++;
				}
				else goto trygadgets;
				break;

			case DRTYPE_WINDOWPIN:
				goto readgadgets;

			case DRTYPE_SEPARATOR:
				goto readgadgets;

			case DRTYPE_DLGGROUP:
				if (p.CheckIdent("OK"))
				{
					bc->SetData(DR_DLGGROUP_OK,true);
					needsemi++; done++;
				}
				else if (p.CheckIdent("CANCEL"))
				{
					bc->SetData(DR_DLGGROUP_CANCEL,true);
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
					sym = p.nextsym(); if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(w); bc->SetData(DR_COLUMNS,w);
					needsemi++; done++;
				}
				else if (p.CheckIdent("ROWS"))
				{
					sym = p.nextsym(); if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(w); bc->SetData(DR_ROWS,w);
					needsemi++; done++;
				}
				else if (p.CheckIdent("EQUAL_ROWS"))
				{
					bc->SetData(DR_EQUALROWS,true);
					needsemi++; done++;
				}
				else if (p.CheckIdent("EQUAL_COLS"))
				{
					bc->SetData(DR_EQUALCOLS,true);
					needsemi++; done++;
				}
				else if (p.CheckIdent("ALLOW_WEIGHTS"))
				{
					bc->SetData(DR_ALLOW_WEIGHTS,true);
					needsemi++; done++;
				}
				else if (p.CheckIdent("TITLECHECKBOX"))
				{
					bc->SetData(BFV_BORDERGROUP_CHECKBOX,true);
					needsemi++; done++;
				}
				else if (p.CheckIdent("BORDERSTYLE"))
				{
					Int32 type;
					sym = p.nextsym();
					if (sym!=SYM_IDENT)
						GOTO_ERROR;
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
					else
						GOTO_ERROR;

					bc->SetData(DR_BORDERSTYLE,type);

					needsemi++; done++;
				}
				else if (p.CheckIdent("BORDERSIZE"))
				{
					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(w); bc->SetData(DR_BORDERSIZE_L,w);
					sym = p.nextsym();
					if (sym!=SYM_OPERATOR_KOMMA)
						GOTO_ERROR;

					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(w); bc->SetData(DR_BORDERSIZE_T,w);
					sym = p.nextsym();
					if (sym!=SYM_OPERATOR_KOMMA)
						GOTO_ERROR;

					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(w); bc->SetData(DR_BORDERSIZE_R,w);
					sym = p.nextsym();
					if (sym!=SYM_OPERATOR_KOMMA)
						GOTO_ERROR;

					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(w); bc->SetData(DR_BORDERSIZE_B,w);

					needsemi++; done++;
				}
				else if (p.CheckIdent("SPACE"))
				{
					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(w); bc->SetData(DR_SPACE_X,w);
					sym = p.nextsym();
					if (sym!=SYM_OPERATOR_KOMMA)
						GOTO_ERROR;

					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(w); bc->SetData(DR_SPACE_Y,w);
					needsemi++; done++;
				}
				else goto trygadgets;
				break;

			case DROLDWIN_SDK:
			case DROLDWIN_SUBDIALOG:
			case DRTYPE_OLDWIN:
trygadgets:

				if (p.CheckIdent("SIZE"))
				{
					Int32 n;

					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(n); /*if (n<0) n=(-n)<<14; */
					bc->SetData(DROLDWIN_POS_W,n);
					sym = p.nextsym();
					if (sym!=SYM_OPERATOR_KOMMA)
						GOTO_ERROR;

					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(n); /*if (n<0) n=(-n)<<14; */
					bc->SetData(DROLDWIN_POS_H,n);
					needsemi++; done++;
				}
				else
				{
					Int32 typeNew = 0, flags=BFH_FIT|BFV_CENTER;
					Int32 pluginid = 0;

					if (type == DRTYPE_DIALOG || type == DRTYPE_GROUP || type == DRTYPE_RADIOGROUP || type == DRTYPE_SCROLLGROUP || type == DRTYPE_TAB)
					{
						if (p.CheckIdent("GROUP"))            { typeNew = DRTYPE_GROUP;       flags = BFH_LEFT|BFV_TOP; }
						else if (p.CheckIdent("DLGGROUP"))    { typeNew = DRTYPE_DLGGROUP;    flags = BFH_LEFT|BFV_TOP; }
						else if (p.CheckIdent("RADIOGROUP"))  { typeNew = DRTYPE_RADIOGROUP;  flags = BFH_LEFT|BFV_TOP; }
						else if (p.CheckIdent("WINDOWPIN"))   { typeNew = DRTYPE_WINDOWPIN;   flags = BFH_CENTER|BFV_CENTER; }
						else if (p.CheckIdent("SEPARATOR"))   { typeNew = DRTYPE_SEPARATOR;   flags = BFH_FIT|BFV_CENTER; }
						else if (p.CheckIdent("TAB"))         { typeNew = DRTYPE_TAB;         flags = BFH_LEFT|BFV_TOP; }
						else if (p.CheckIdent("SCROLLGROUP")) { typeNew = DRTYPE_SCROLLGROUP; flags = BFH_FIT|BFV_FIT; }
						else if (p.CheckIdent("EDITTEXT"))         typeNew = DROLDWIN_EDITTEXT;
						else if (p.CheckIdent("MULTILINEEDIT"))    typeNew = DROLDWIN_MULTILINETEXT;
						else if (p.CheckIdent("EDITNUMBER"))       typeNew = DROLDWIN_EDITNUMBER;
						else if (p.CheckIdent("EDITNUMBERARROWS")) typeNew = DROLDWIN_EDITNUMBERARROWS;
						else if (p.CheckIdent("COMBOBOX"))         typeNew = DROLDWIN_COMBOBOX;
						else if (p.CheckIdent("COMBOBUTTON"))      typeNew = DROLDWIN_COMBOBUTTON;
						else if (p.CheckIdent("POPUPBUTTON"))      typeNew = DROLDWIN_POPUPBUTTON;
						else if (p.CheckIdent("CHECKBOX"))         typeNew = DROLDWIN_CHECKBOX;
						else if (p.CheckIdent("STATICTEXT"))       typeNew = DROLDWIN_STATICTEXT;
						else if (p.CheckIdent("SLIDER"))           typeNew = DROLDWIN_SLIDER;
						else if (p.CheckIdent("EDITSLIDER"))       typeNew = DROLDWIN_SLIDERNUM;
						else if (p.CheckIdent("BUTTON"))           typeNew = DROLDWIN_BUTTON;
						else if (p.CheckIdent("USERAREA"))         typeNew = DROLDWIN_OWNERDRAW;
						else if (p.CheckIdent("COLORFIELD"))       typeNew = DROLDWIN_COLORFIELD;
						else if (p.CheckIdent("RADIOGADGET"))      typeNew = DROLDWIN_RADIOGADGET;
						else if (p.CheckIdent("SPECIALRADIO"))     typeNew = DROLDWIN_SPECIALRADIO;
						else if (p.CheckIdent("ARROWBUTTON"))    { typeNew = DROLDWIN_ARROWBUTTON; flags = BFH_LEFT|BFV_TOP; }
						else if (p.CheckIdent("LISTVIEW"))       { typeNew = DROLDWIN_LISTVIEW;    flags = BFH_SCALEFIT|BFV_SCALEFIT; }
						else if (p.CheckIdent("SUBDIALOG"))      { typeNew = DROLDWIN_SUBDIALOG;   flags = BFH_SCALEFIT|BFV_SCALEFIT; }
						else
						{
							BasePlugin *plug = CheckCustomGuiPlugins(GetFirstPlugin(), p.GetIdent());
							if (plug)
							{
								pluginid = plug->GetID();
								typeNew = DROLDWIN_SDK;
							}
						}
						//flags = 0;BFH_SCALEFIT|BFV_SCALEFIT;
					}

					if (typeNew)
					{
						sym = p.nextsym();
						BaseContainer sub(typeNew);

						if (sym==SYM_OPERATOR_GESCHWEIFTAUF)
							p.rewindsym();
						else
						{
							if (sym==SYM_NUMBER)
							{
								Int32 id;
								if (!p.GetInt32(id))
									GOTO_ERROR;
								sub.SetData(DR_ID,String::IntToString(id));
							}
							else
							{
								if (sym!=SYM_IDENT)
									GOTO_ERROR;
								sub.SetData(DR_ID,String(p.GetIdent()));
							}
						}

						sub.SetData(DR_ALIGNMENT,flags);

						//BaseContainer *s = bc->InsContainer(DR_CHILD,sub);

						if (typeNew==DRTYPE_RADIOGROUP || typeNew==DRTYPE_GROUP)
						{
							sub.SetData(DR_SPACE_X,4);
							sub.SetData(DR_SPACE_Y,4);
						}

						if (typeNew==DROLDWIN_SDK)
						{
							sub.SetData(DROLDWIN_SDK,pluginid);
						}

						if (!LoadDialogParts(p, &sub, typeNew, progressID))
							GOTO_ERROR;

						if (typeNew==DRTYPE_GROUP)
						{
							if (sub.GetData(DR_SPACE_X)==0 && sub.GetData(DR_SPACE_Y)==0)
								typeNew = typeNew; // s = s;
						}
						Int32 lChildren = bc->GetInt32(DR_NUMCHILDREN);
						bc->SetContainer(DR_CHILD + lChildren, sub);
						bc->SetInt32(DR_NUMCHILDREN, lChildren + 1);

						done++;
					}
				}
				break;

			case DROLDWIN_MULTILINETEXT:
				if (p.CheckIdent("MONOSPACED"))
				{
					bc->SetData(DR_MULTILINE_MONOSPACED,true);
					needsemi++; done++;
				}
				if (p.CheckIdent("SYNTAXCOLOR"))
				{
					bc->SetData(DR_MULTILINE_SYNTAXCOLOR,true);
					needsemi++; done++;
				}
				if (p.CheckIdent("STATUSBAR"))
				{
					bc->SetData(DR_MULTILINE_STATUSBAR,true);
					needsemi++; done++;
				}
				if (p.CheckIdent("PYTHON"))
				{
					bc->SetData(DR_MULTILINE_PYTHON,true);
					needsemi++; done++;
				}
				if (p.CheckIdent("WORDWRAP"))
				{
					bc->SetData(DR_MULTILINE_WORDWRAP,true);
					needsemi++; done++;
				}
				goto readgadgets;

			case DROLDWIN_COLORFIELD:
				if (p.CheckIdent("NOBRIGHTNESS"))
				{
					bc->SetData(DR_COLORFIELD_NO_BRIGHTNESS,true);
					needsemi++; done++;
				}
				if (p.CheckIdent("NOCOLOR"))
				{
					bc->SetData(DR_COLORFIELD_NO_COLOR,true);
					needsemi++; done++;
				}
				if (p.CheckIdent("ICC_BASEDOCUMENT"))
				{
					bc->SetBool(DR_COLORFIELD_ICC_BASEDOC, true);
					needsemi++; done++;
				}
				if (p.CheckIdent("ICC_BPTEXTURE"))
				{
					bc->SetBool(DR_COLORFIELD_ICC_BPTEX, true);
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

							bc->SetData(DR_BORDERSTYLE, type);

							needsemi++; done++;
						}
					}
readgadgets:
				if (p.CheckIdent("POS"))
				{
					Int32 n;

					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(n); bc->SetData(DROLDWIN_POS_X,n);

					sym = p.nextsym();
					if (sym!=SYM_OPERATOR_KOMMA)
						GOTO_ERROR;
					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(n); bc->SetData(DROLDWIN_POS_Y,n);

					sym = p.nextsym();
					if (sym!=SYM_OPERATOR_KOMMA)
						GOTO_ERROR;
					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(n); bc->SetData(DROLDWIN_POS_W,n);

					sym = p.nextsym();
					if (sym!=SYM_OPERATOR_KOMMA)
						GOTO_ERROR;
					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(n); bc->SetData(DROLDWIN_POS_H,n);

					needsemi++; done++;
				}
				else if (p.CheckIdent("SIZE"))
				{
					Int32 n;

					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(n); /*if (n<0) n=(-n)<<14; */bc->SetData(DROLDWIN_POS_W,n);
					sym = p.nextsym();
					if (sym!=SYM_OPERATOR_KOMMA)
						GOTO_ERROR;

					sym = p.nextsym();
					if (sym!=SYM_NUMBER)
						GOTO_ERROR;
					p.GetInt32(n); /*if (n<0) n=(-n)<<14; */bc->SetData(DROLDWIN_POS_H,n);
					needsemi++; done++;
				}
				else if ((type == DROLDWIN_COMBOBOX || type == DROLDWIN_COMBOBUTTON || type == DROLDWIN_POPUPBUTTON) && p.CheckIdent("CHILDS"))
				{
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
							//id = SymbolTab_FindEntry(symbols,p.GetIdent(),false);
							strID = p.GetIdent();
						}
						else
						{
							if (sym!=SYM_NUMBER)
								GOTO_ERROR;
							if (p.GetInt32(id))
								strID = String::IntToString(id);
							else
								strID = "-1";
						}

						/*sym = p.nextsym();
						if (sym!=SYM_OPERATOR_KOMMA) goto error;

						sym = p.nextsym();
						if (sym!=SYM_IDENT) goto error;

						ResourceObj *strobj = strings.FindObj(p.GetIdent());
						String name;
						if (!strobj)
						{
							GeOutString(GeLoadString(IDS_SYMBOL_NOT_FOUND,p.GetIdent(),p.GetFilename().GetString(),String::IntToString(p.GetLine())),GEMB_OK+GEMB_ICONEXCLAMATION);
						}
						else
						{
							name = strobj->data.GetString(0);
						}

						childs.InsData(id,name);
						sym = p.nextsym();
						if (sym!=SYM_OPERATOR_SEMIKOLON) goto error;*/
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
					lChildren = bc->GetInt32(DR_NUMCHILDREN);
					bc->SetContainer(DR_CHILD + lChildren, childs);
					bc->SetInt32(DR_NUMCHILDREN, lChildren + 1);
					done++;
				}
				break;

		}
		if (!done && type == DROLDWIN_SDK)
		{
			Int32 pluginid = bc->GetInt32(DROLDWIN_SDK);
			// check additional defines
			BasePlugin *bs = (BasePlugin*)FindPlugin(pluginid, PLUGINTYPE_CUSTOMGUI);
			if (bs)
			{
				Int32 i;
				GeData val;

				CustomProperty *prop = CallCustomGuiCmd(bs,GetProperties)();
				if (p.CheckIdent("OPEN"))
				{
					bc->SetBool(DR_CUSTOMGUI_OPEN, true);
					done++;
					needsemi = true;
				}
				else
				{
					if (prop)
					{
						for (i=0;!done && prop[i].type!=CUSTOMTYPE_END;i++)
						{
							if (p.CheckIdent(prop[i].ident))
							{
								switch (prop[i].type)
								{
									case CUSTOMTYPE_FLAG:
										bc->SetData(prop[i].id,true);
										done = true;
										needsemi = true;
										break;

									case CUSTOMTYPE_LONG:
										ReadValue(p, /*&strings, */CUSTOMTYPE_LONG, val);
										bc->SetData(prop[i].id,val);
										done     = true;
										needsemi = true;
										break;

									case CUSTOMTYPE_REAL:
										ReadValue(p, /*&strings, */CUSTOMTYPE_REAL, val);
										bc->SetData(prop[i].id, val);
										done     = true;
										needsemi = true;
										break;

									case CUSTOMTYPE_STRING:
										ReadValue(p, /*&strings, */CUSTOMTYPE_STRING, val);
										bc->SetData(prop[i].id, val);
										done     = true;
										needsemi = true;
										break;

									case CUSTOMTYPE_VECTOR:
										ReadValue(p, /*&strings, */CUSTOMTYPE_VECTOR, val);
										bc->SetData(prop[i].id,val);
										done     = true;
										needsemi = true;
										break;

									default:
										GOTO_ERROR;
								}
							}
						}
					}
				}
			}
		}

		if (!done)
			GOTO_ERROR;
		if (needsemi)
		{
			sym = p.nextsym();
			if (sym!=SYM_OPERATOR_SEMIKOLON)
				GOTO_ERROR;
		}
		sym = p.nextsym();
	}
	return !p.Error();

error:
	p.SetError();
	return false;
}

Bool ReadValue(ASCIIParser &p,/*Resource *strings,*/CUSTOMTYPE type,GeData &res)
{
	SYMBOL sym;

	if (type==CUSTOMTYPE_REAL)
	{
		Float w;
		sym = p.nextsym();
		if (sym!=SYM_NUMBER)
			GOTO_ERROR;
		p.GetFloat(w);
		res = GeData(w);
	}
/*	else if (type==DESCRIPTIONTYPE_TIME)
	{
		Float w;
		sym = p.nextsym(); if (sym!=SYM_NUMBER) goto error;
		p.GetFloat(w);
		res = GeData(BaseTime(w));
	}*/
	else if (type == CUSTOMTYPE_LONG/* || type == DESCRIPTIONTYPE_BOOL || type == DESCRIPTIONTYPE_GROUP*/)
	{
		Int32 w;
		sym = p.nextsym();
		if (sym!=SYM_NUMBER)
			GOTO_ERROR;
		p.GetInt32(w);
		res = GeData(w);
	}
	else if (type == CUSTOMTYPE_VECTOR/* || type==DESCRIPTIONTYPE_COLOR*/)
	{
		Vector v;

		sym = p.nextsym();
		if (sym!=SYM_NUMBER)
			GOTO_ERROR;
		p.GetFloat(v.x);

		sym = p.nextsym();
		if (sym==SYM_OPERATOR_SEMIKOLON)
		{
			v.y = v.z = v.x;
			p.rewindsym();
		}
		else
		{
			if (sym!=SYM_NUMBER)
				GOTO_ERROR;
			p.GetFloat(v.y);

			sym = p.nextsym();
			if (sym!=SYM_NUMBER)
				GOTO_ERROR;
			p.GetFloat(v.z);
		}
		res = GeData(v);
	}
	else if (type == CUSTOMTYPE_STRING/* || type==DESCRIPTIONTYPE_FILENAME*/)
	{
		String str;
		sym = p.nextsym();
		if (sym!=SYM_IDENT)
			GOTO_ERROR;
		res = GeData(String(p.GetIdent()));
	}

	return true;
error:
	return false;
}

/*Bool ReadValue(ASCIIParser &p,Resource *strings,Int32 type,GeData &res)
{
	SYMBOL sym;

	if (type==DESCRIPTIONTYPE_REAL)
	{
		Float w;
		sym = p.nextsym(); if (sym!=SYM_NUMBER) goto error;
		p.GetFloat(w);
		res = GeData(w);
	}
	else if (type==DESCRIPTIONTYPE_TIME)
	{
		Float w;
		sym = p.nextsym(); if (sym!=SYM_NUMBER) goto error;
		p.GetFloat(w);
		res = GeData(BaseTime(w));
	}
	else if (type==DESCRIPTIONTYPE_LONG || type==DESCRIPTIONTYPE_BOOL || type==DESCRIPTIONTYPE_GROUP)
	{
		Int32 w;
		sym = p.nextsym(); if (sym!=SYM_NUMBER) goto error;
		p.GetInt32(w);
		res = GeData(w);
	}
	else if (type==DESCRIPTIONTYPE_VECTOR || type==DESCRIPTIONTYPE_COLOR)
	{
		Vector v;

		sym = p.nextsym();
		if (sym!=SYM_NUMBER) goto error;
		p.GetFloat(v.x);

		sym = p.nextsym();
		if (sym==SYM_OPERATOR_SEMIKOLON)
		{
			v.y = v.z = v.x;
			p.rewindsym();
		}
		else
		{
			if (sym!=SYM_NUMBER) goto error;
			p.GetFloat(v.y);

			sym = p.nextsym();
			if (sym!=SYM_NUMBER) goto error;
			p.GetFloat(v.z);
		}
		res = GeData(v);
	}
	else if (type==DESCRIPTIONTYPE_STRING || type==DESCRIPTIONTYPE_FILENAME)
	{
		ResourceObj *strobj;
		String str;
		sym = p.nextsym(); if (sym!=SYM_IDENT) goto error;
		strobj = strings->FindObj(p.GetIdent());
		if (!strobj)
		{
			GeOutString(GeLoadString(IDS_SYMBOL_NOT_FOUND,p.GetIdent(),p.GetFilename().GetString(),String::IntToString(p.GetLine())),GEMB_OK+GEMB_ICONEXCLAMATION);
		}
		else
		{
			str = strobj->data.GetString(0);
		}
		res = GeData(str);
	}

	return true;
error:
	return false;
}

					for (i=0;!done && prop[i].type!=CUSTOMTYPE_END;i++)
					{
						if (p.CheckIdent(prop[i].ident))
						{
							GeData val;
							switch (prop[i].type)
							{
								case CUSTOMTYPE_FLAG:
									bc->SetData(prop[i].id,true);
									done = true;
									needsemi = true;
									break;

								case CUSTOMTYPE_LONG:
									ReadValue(p,&strings,DESCRIPTIONTYPE_LONG,val);
									bc->SetData(prop[i].id,val);
									done     = true;
									needsemi = true;
									break;

								case CUSTOMTYPE_REAL:
									ReadValue(p,&strings,DESCRIPTIONTYPE_REAL,val);
									bc->SetData(prop[i].id,val);
									done     = true;
									needsemi = true;
									break;

								case CUSTOMTYPE_STRING:
									ReadValue(p,&strings,DESCRIPTIONTYPE_STRING,val);
									bc->SetData(prop[i].id,val);
									done     = true;
									needsemi = true;
									break;

								case CUSTOMTYPE_VECTOR:
									ReadValue(p,&strings,DESCRIPTIONTYPE_VECTOR,val);
									bc->SetData(prop[i].id,val);
									done     = true;
									needsemi = true;
									break;

								default: goto error;
							}
						}
					}
					*/

#endif
