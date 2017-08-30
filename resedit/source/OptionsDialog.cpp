/*********************************************************************\
	File name        : OptionsDialog.cpp
	Description      : Implementation of the class COptionsDialog
	Created at       : 09.09.01, @ 14:41:33
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "OptionsDialog.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

enum
{
	ID_AUTO_UPDATE_CHK = 1000,
	IDC_TAB_WIDTH_EDIT,
	IDC_TAB_TYPE_COMBO,
	IDC_SAVE_STRING_ID_CHK,

	ID_DUMMY
};

COptionsDialog::COptionsDialog()
{

}

COptionsDialog::~COptionsDialog()
{

}

/*********************************************************************\
	Function name    : COptionsDialog::CreateLayout
	Description      :
	Created at       : 09.09.01, @ 14:35:56
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool COptionsDialog::CreateLayout(void)
{
	Bool bRes = GeDialog::CreateLayout();

	SetTitle(GeLoadString(IDS_OPTIONS_CAPT));

	GroupBegin(100, BFH_FIT, 1, 0, "", 0);
		GroupBegin(101, BFH_FIT, 1, 0, GeLoadString(IDS_GENERAL_SETTINGS), 0);
			GroupBorder(BORDER_GROUP_IN);
			GroupBorderSpace(4, 4, 4, 4);
			AddCheckbox(ID_AUTO_UPDATE_CHK, BFH_LEFT, 0, 0, GeLoadString(IDS_AUTO_UPDATE));
		GroupEnd();
		GroupBegin(201, BFH_FIT, 1, 0, GeLoadString(IDS_SAVE_SETTINGS), 0);
			GroupBorder(BORDER_GROUP_IN);
			GroupBorderSpace(4, 4, 4, 4);
			GroupBegin(211, BFH_LEFT, 4, 0, "", 0);
				AddStaticText(210, BFH_LEFT, 0, 0, GeLoadString(IDS_SAVE_TAB_WIDTH), 0);
				AddEditNumber(IDC_TAB_WIDTH_EDIT, BFH_LEFT);
				AddStaticText(210, BFH_LEFT, 0, 0, GeLoadString(IDS_SAVE_TAB_TYPE), 0);
				AddComboBox(IDC_TAB_TYPE_COMBO, BFH_LEFT);
					AddChild(IDC_TAB_TYPE_COMBO, 0, GeLoadString(IDS_SAVE_TAB_TAB));
					AddChild(IDC_TAB_TYPE_COMBO, 1, GeLoadString(IDS_SAVE_TAB_SPACE));
			GroupEnd();
			AddCheckbox(IDC_SAVE_STRING_ID_CHK, BFH_LEFT | BFV_CENTER, 0, 0, GeLoadString(IDS_SAVE_STRING_ID));
		GroupEnd();
	GroupEnd();

	AddDlgGroup(DLG_OK | DLG_CANCEL);

	return bRes;
}

/*********************************************************************\
	Function name    : COptionsDialog::Command
	Description      :
	Created at       : 09.09.01, @ 14:35:58
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool COptionsDialog::Command(Int32 id,const BaseContainer &msg)
{
	switch (id)
	{
	case IDC_OK:
		GetBool(ID_AUTO_UPDATE_CHK, g_bAutoUpdate);
		GetBool(IDC_SAVE_STRING_ID_CHK, g_bSaveStringIDs);

		GetInt32(IDC_TAB_WIDTH_EDIT, g_lFillChars);
		Int32 lChar;
		GetInt32(IDC_TAB_TYPE_COMBO, lChar);
		if (lChar == 0) g_chFill = '\t';
		else if (lChar == 1) g_chFill = ' ';
		*g_pstrFillSave = "";
		char ch[2];
		ch[0] = g_chFill;
		ch[1] = '\0';
		for (Int32 a = 0; a < g_lFillChars; a++)
			*g_pstrFillSave += ch;

		break;
	}
	return true;
}

/*********************************************************************\
	Function name    : COptionsDialog::InitValues
	Description      :
	Created at       : 09.09.01, @ 14:36:00
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool COptionsDialog::InitValues(void)
{
	SetBool(ID_AUTO_UPDATE_CHK, g_bAutoUpdate);
	SetBool(IDC_SAVE_STRING_ID_CHK, g_bSaveStringIDs);

	SetInt32(IDC_TAB_WIDTH_EDIT, g_lFillChars);
	Int32 lChar=0;
	if (g_chFill == '\t') lChar = 0;
	else if (g_chFill == ' ') lChar = 1;
	SetInt32(IDC_TAB_TYPE_COMBO, lChar);

	return true;
}
