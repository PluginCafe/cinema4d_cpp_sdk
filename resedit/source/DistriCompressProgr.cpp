// DistriCompressProgr.cpp
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "globals.h"
#include "DistriCompressProgr.h"

CCompressProgressDlg::CCompressProgressDlg(Bool* bReady, Bool* bCancel)
{
	m_pbReady = bReady;
	m_pbCancel = bCancel;
	m_bClose = false;
}

CCompressProgressDlg::~CCompressProgressDlg()
{

}

Bool CCompressProgressDlg::CreateLayout()
{
	if (!GeDialog::CreateLayout() || !LoadDialogResource(IDD_DISTRI_COMPR_DLG, nullptr, 0))
		return false;

	m_pSrcGUI = (HyperLinkCustomGui*)FindCustomGui(IDC_DISTRI_COMPRESS_SRC_FILE, CUSTOMGUI_HYPER_LINK_STATIC);
	m_pDestGUI = (HyperLinkCustomGui*)FindCustomGui(IDC_DISTRI_COMPRESS_DST_FILE, CUSTOMGUI_HYPER_LINK_STATIC);
	if (!m_pSrcGUI || !m_pDestGUI)
		return false;

	return true;
}

Bool CCompressProgressDlg::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDC_CANCEL:
		*m_pbCancel = true;
		break;
	default:
		break;
	}
	return true;
}

Bool CCompressProgressDlg::InitValues()
{
	if (!GeDialog::InitValues()) return false;

	*m_pbReady = true;
	SetTimer(150);

	return true;
}

Int32 CCompressProgressDlg::Message(const BaseContainer &msg, BaseContainer &result)
{
/*
	switch (msg.GetId())
	{
	default:
		break;
	}
*/
	return GeDialog::Message(msg, result);
}

Bool CCompressProgressDlg::AskClose()
{
	// return true, if the dialog should not be closed
	return false;
}

void RedrawWindow(void* pHandle);

void CCompressProgressDlg::SetStrings(const String &strSource, const String &strDest)
{
	if (!(GeGetSystemInfo()&SYSTEMINFO_COMMANDLINE))
	{
#ifdef _WIN32
		m_pSrcGUI->SetLinkStringThreaded(nullptr, &strSource);
		m_pDestGUI->SetLinkStringThreaded(nullptr, &strDest);
		RedrawWindow(GetWindowHandle());
#else
		m_strSource = strSource;
		m_strDest = strDest;
		//GeSleep(1);
#endif
	}
	else
	{
		GeConsoleOut("copy "+strSource+" -> "+strDest);
	}
}

void CCompressProgressDlg::Timer(const BaseContainer &msg)
{
#ifndef _WIN32
	m_pSrcGUI->SetLinkString(nullptr, &m_strSource);
	m_pDestGUI->SetLinkString(nullptr, &m_strDest);
#endif
	if (!m_bClose)
		return;
	Close(true);
}

