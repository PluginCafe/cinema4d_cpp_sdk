/*********************************************************************\
	File name        : DialogTemplate.cpp
	Description      :
	Created at       : 11.08.01, @ 15:26:29
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#include "stdafx.h"
#include "globals.h"
#include "DialogDoc.h"
#include "DialogItem.h"

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*********************************************************************\
	Function name    : CDialogTemplate
	Description      :
	Created at       : 11.08.01, @ 18:41:28
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
CDialogTemplate::CDialogTemplate(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_DIALOG);
	m_strDescr = m_strName;
	m_type = Dialog;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogTemplate::Save
	Description      :
	Created at       : 11.08.01, @ 15:42:57
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogTemplate::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "// C4D-DialogResource");
	LineBreak(pFile, strFill);
	WriteString(pFile, "DIALOG ");
	WriteString(pFile, m_strControlID);

	LineBreak(pFile, strFill);
	WriteString(pFile, "{");
	LineBreak(pFile, strFill + SPACE_NEW_LINE);
	SaveAlignment(pFile, false, false);
	LineBreak(pFile, strFill + SPACE_NEW_LINE);
	LineBreak(pFile, strFill + SPACE_NEW_LINE);

	// store the children
	CDialogItem::Save(pFile, strFill + SPACE_NEW_LINE);

	LineBreak(pFile, strFill);
	WriteString(pFile, "}");

	return true;
}

/*********************************************************************\
	Function name    : CDialogTemplate::Load
	Description      :
	Created at       : 11.08.01, @ 15:42:55
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogTemplate::Load(BaseContainer* bc)
{
	return CDialogItem::Load(bc);
}

/*********************************************************************\
	Function name    : CDialogTemplate::CreateElement
	Description      :
	Created at       : 11.08.01, @ 16:06:12
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogTemplate::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
}
