/*********************************************************************\
	File name        : HelpMenu.cpp
	Description      :
	Created at       : 26.03.02, @ 11:22:16
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "HelpMenu.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CHelpMenu::CHelpMenu()
{

}

CHelpMenu::~CHelpMenu()
{

}

/*********************************************************************\
	Function name    : CHelpMenu::Execute
	Description      :
	Created at       : 26.03.02, @ 11:41:38
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CHelpMenu::Execute(BaseDocument *doc)
{
	Filename fn = GeGetPluginPath();
	fn += Filename("Doc");
	fn += GeLoadString(IDS_HELP_FILE);

	GeExecuteFile(fn);

	return true;
}
