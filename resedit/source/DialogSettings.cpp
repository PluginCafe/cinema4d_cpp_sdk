/*********************************************************************\
	File name        : DialogSettings.cpp
	Description      : Implementation of the class CDialogSettings
	Created at       : 11.08.01, @ 23:27:45
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"
#include "DialogItem.h"
#include "DialogItemSettings.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CDialogSettings::CDialogSettings()
{
	//m_lTabPageID = IDC_DIALOG_SETTINGS_TAB;
}

CDialogSettings::~CDialogSettings()
{

}

/*********************************************************************\
	Function name    : CDialogSettings::SetData
	Description      :
	Created at       : 11.08.01, @ 23:24:52
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogSettings::SetData()
{
	_Init();
}
