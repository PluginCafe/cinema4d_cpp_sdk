/*********************************************************************\
	File name        : DialogSlider.cpp
	Description      :
	Created at       : 14.08.01, @ 10:43:28
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

CDialogSlider::CDialogSlider(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_SLIDER);
	m_strDescr = m_strName;
	m_type = Slider;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 90;
	m_lInitH = 0;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

/*********************************************************************\
	Function name    : CDialogSlider::CreateElementBegin
	Description      :
	Created at       : 14.08.01, @ 10:44:06
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogSlider::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	pDlg->AddSlider(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));
}

/*********************************************************************\
	Function name    : CDialogSlider::Load
	Description      :
	Created at       : 17.08.01, @ 12:48:57
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogSlider::Load(BaseContainer* bc)
{
	return CDialogItem::Load(bc);
}

/*********************************************************************\
	Function name    : CDialogSlider::Save
	Description      :
	Created at       : 30.08.01, @ 10:29:54
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogSlider::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "SLIDER ");
	WriteString(pFile, m_strControlID);
	WriteString(pFile, " { ");
	SaveAlignment(pFile);
	WriteString(pFile, "}");
	return true;
}



CSliderSettings::CSliderSettings()
{
	m_lTabPageID = IDC_SLIDER_SETTINGS_TAB;
}

CSliderSettings::~CSliderSettings()
{
}

/*********************************************************************\
	Function name    : CGroupSettings::SetData
	Description      :
	Created at       : 12.08.01, @ 09:24:31
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CSliderSettings::SetData()
{
	_Init();
}

/*********************************************************************\
	Function name    : CSliderSettings::Command
	Description      :
	Created at       : 14.08.01, @ 10:45:25
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CSliderSettings::Command(Int32 lID, const BaseContainer &msg)
{
	return true;
}
