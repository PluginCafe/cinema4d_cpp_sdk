// DistriCompressProgr.h :  Interface definition of class CCompressProgressDlg
//
//////////////////////////////////////////////////////////////////////

#if !defined DISTRICOMPRESSPROGR_H_3fc77bfa
#define DISTRICOMPRESSPROGR_H_3fc77bfa

#include "customgui_hyperlink.h"

class CCompressProgressDlg : public GeModalDialog
{
public:
	CCompressProgressDlg(Bool* bReady, Bool* bCancel);
	virtual ~CCompressProgressDlg();

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
	virtual Bool InitValues(void);
	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);
	virtual Bool AskClose();
	virtual void Timer(const BaseContainer &msg);
	void SetStrings(const String &strSource, const String &strDest);
	void CloseDialog() { m_bClose = true; }

protected:
	Bool m_bClose;
	Bool *m_pbReady, *m_pbCancel;
	HyperLinkCustomGui *m_pSrcGUI, *m_pDestGUI;
#ifndef _WIN32
	String m_strSource, m_strDest;
#endif
};

#endif // DISTRICOMPRESSPROGR_H_3fc77bfa
