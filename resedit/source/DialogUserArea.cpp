/*********************************************************************\
	File name        : DialogUserArea.cpp
	Description      :
	Created at       : 15.08.01, @ 19:45:31
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

CDialogUserAreaWindow::CDialogUserAreaWindow()
{
	m_lMinSizeX = m_lMinSizeY = 0;
	m_pImage = BaseBitmap::Alloc();
}

CDialogUserAreaWindow::~CDialogUserAreaWindow()
{
	BaseBitmap::Free(m_pImage);
}

/*********************************************************************\
	Function name    : CDialogUserAreaWindow::GetMinSize
	Description      :
	Created at       : 21.11.01, @ 13:35:24
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogUserAreaWindow::GetMinSize(Int32 &w,Int32 &h)
{
	w = m_lMinSizeX; h = m_lMinSizeY;
	return true;
}

/*********************************************************************\
	Function name    : CDialogUserAreaWindow::Draw
	Description      :
	Created at       : 21.11.01, @ 13:36:07
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogUserAreaWindow::DrawMsg(Int32 x1, Int32 y1, Int32 x2, Int32 y2, const BaseContainer &bcMsg)
{
	DrawBitmap(m_pImage, 0, 0, m_pImage->GetBw(), m_pImage->GetBh(), 0, 0, m_pImage->GetBw(), m_pImage->GetBh(), BMP_NORMAL);
}

/*********************************************************************\
	Function name    : CDialogUserAreaWindow::Sized
	Description      :
	Created at       : 21.11.01, @ 14:07:01
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogUserAreaWindow::Sized(Int32 w,Int32 h)
{
	if (m_pImage->Init(w, h) != IMAGERESULT_OK) return;

	Int32 r, g, b;
	if (w <= 1) w = 2;
	if (h <= 1) h = 2;
	for (Int32 x = 0; x < w; x++)
	{
		r = 255 * x / (w - 1);
		for (Int32 y = 0; y < h; y++)
		{
			g = 255 * y / (h - 1);
			b = 255 * (x + y) / (h + w - 2);
			m_pImage->SetPixel(x, y, r, g, b);
		}
	}
}

/*********************************************************************\
	Function name    : CDialogUserAreaWindow::InputEvent
	Description      :
	Created at       : 21.11.01, @ 14:26:45
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogUserAreaWindow::InputEvent(const BaseContainer &msg)
{
	//Int32 chn = msg.GetInt32(BFM_INPUT_CHANNEL);
	Int32 dev = msg.GetInt32(BFM_INPUT_DEVICE);

	if (dev == BFM_INPUT_MOUSE)
	{
		// send this message to the preview sub
		m_pParent->Command(m_lID, msg);
	}

	return true;
}


CDialogUserArea::CDialogUserArea(CDialogDoc* pDoc)
{
	m_strName = GeLoadString(IDS_USER_AREA);
	m_strDescr = m_strName;
	m_type = UserArea;
	m_lFlags = BFH_LEFT | BFV_TOP;
	m_lInitW = 100;
	m_lInitH = 100;
	m_pDocument = pDoc;
	m_strControlID = CreateControlID();
}

Int32 GetPix(Int32 packedsize)
{
	return packedsize>>14;
}

Int32 GetChr(Int32 packedsize)
{
	return packedsize&((1<<14)-1);
}

/*********************************************************************\
	Function name    : CDialogUserArea::CreateElementBegin
	Description      :
	Created at       : 15.08.01, @ 19:47:00
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CDialogUserArea::CreateElementBegin(Int32 lID, GeDialog *pDlg)
{
	pDlg->AddUserArea(lID, m_lFlags, CONVERT_WIDTH(m_lInitW), CONVERT_HEIGHT(m_lInitH));

	pDlg->AttachUserArea(m_wndUser, lID, USERAREA_0);

	Float chr;
	Int32 w = CONVERT_WIDTH(m_lInitW);
	Int32 h = CONVERT_WIDTH(m_lInitH);
	chr = GetChr(w) / 10.0;
	m_wndUser.m_lMinSizeX = GetPix(w) + Int32(m_wndUser.DrawGetTextWidth("8") * chr + 0.5);

	chr = GetChr(h) / 10.0;
	m_wndUser.m_lMinSizeY = GetPix(h) + Int32(m_wndUser.DrawGetFontHeight() * chr + 0.5);

	m_wndUser.m_pParent = pDlg;
	m_wndUser.m_lID = lID;
}

/*********************************************************************\
	Function name    : CDialogUserArea::Load
	Description      :
	Created at       : 17.08.01, @ 12:49:32
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogUserArea::Load(BaseContainer* bc)
{
	return CDialogItem::Load(bc);
}

/*********************************************************************\
	Function name    : CDialogUserArea::Save
	Description      :
	Created at       : 30.08.01, @ 10:30:20
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CDialogUserArea::Save(BaseFile* pFile, String strFill)
{
	WriteString(pFile, "USERAREA ");
	WriteString(pFile, m_strControlID);
	WriteString(pFile, " { ");
	SaveAlignment(pFile);
	WriteString(pFile, "}");
	return true;
}




CUserAreaSettings::CUserAreaSettings()
{
	m_lTabPageID = IDC_USER_AREA_SETTINGS_TAB;
}

CUserAreaSettings::~CUserAreaSettings()
{
}

/*********************************************************************\
	Function name    : CUserAreaSettings::SetData
	Description      :
	Created at       : 15.08.01, @ 19:47:27
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void CUserAreaSettings::SetData()
{
	_Init();
}

/*********************************************************************\
	Function name    : CUserAreaSettings::Command
	Description      :
	Created at       : 15.08.01, @ 19:47:30
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool CUserAreaSettings::Command(Int32 lID, const BaseContainer &msg)
{
	return true;
}
