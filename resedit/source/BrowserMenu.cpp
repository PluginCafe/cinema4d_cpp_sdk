/*********************************************************************\
	File name        : BrowserMenu.cpp
	Description      :
	Created at       : 26.03.02, @ 11:22:57
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "BrowserMenu.h"
#include "ResEditBrowser.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CResEditBrowser *g_pBrowserDialog;

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CBrowserMenu::CBrowserMenu()
{

}

CBrowserMenu::~CBrowserMenu()
{

}

/*********************************************************************\
	Function name    : CBrowserMenu::Execute
	Description      :
	Created at       : 26.03.02, @ 11:29:22
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CBrowserMenu::Execute(BaseDocument *doc)
{
	if (!g_pBrowserDialog) return false;
	g_pBrowserDialog->Open(DLG_TYPE_ASYNC, PLUGIN_ID_BROWSER, -1, -1);

	return true;
}

/*********************************************************************\
	Function name    : CBrowserMenu::RestoreLayout
	Description      :
	Created at       : 26.03.02, @ 11:29:24
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CBrowserMenu::RestoreLayout(void *secret)
{
	if (!g_pBrowserDialog) g_pBrowserDialog = NewObjClear(CResEditBrowser);
	if (!g_pBrowserDialog) return false;

	return g_pBrowserDialog->RestoreLayout(PLUGIN_ID_BROWSER, 0, secret);
}
