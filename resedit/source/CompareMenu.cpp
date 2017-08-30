/*********************************************************************\
	File name        : CompareMenu.cpp
	Description      :
	Created at       : 26.03.02, @ 11:20:57
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "CompareMenu.h"
#include "CompareTableDialog.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

extern Filename *g_pfnOriginal, *g_pfnNew;

CCompareMenu::CCompareMenu()
{

}

CCompareMenu::~CCompareMenu()
{

}

/*********************************************************************\
	Function name    : CCompareMenu::RestoreLayout
	Description      :
	Created at       : 26.03.02, @ 11:31:52
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CCompareMenu::Execute(BaseDocument *doc)
{
	String str;

	//g_pfnOriginal = GeGetPluginPath();
	if (!g_pfnOriginal->FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_LOAD, GeLoadString(IDS_SELECT_ORIGINAL_TABLE))) return true;
	//fnOriginal = Filename("h:") + String("cinema7201") + String("resource") + String("modules") + String("MonFilter") + String("res") + String("strings_de") + String("c4d_strings.str");

	if (!g_pfnNew->Content())
		*g_pfnNew = g_pfnOriginal->GetDirectory();

	if (!g_pfnNew->FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_LOAD, GeLoadString(IDS_SELECT_NEW_TABLE))) return true;
	//fnNew = Filename("h:") + String("cinema7201") + String("resource") + String("modules") + String("MonFilter") + String("res") + String("strings_de") + String("c4d_strings1.str");

	CCompareTableDialog dlg(*g_pfnOriginal, *g_pfnNew);
	dlg.Open();

	return true;
}
