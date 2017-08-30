// OptionsDialog.h: Schnittstelle für die Klasse COptionsDialog.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONSDIALOG_H__5DA75860_A507_11D5_9B3B_004095418E0F__INCLUDED_)
#define AFX_OPTIONSDIALOG_H__5DA75860_A507_11D5_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class COptionsDialog : public GeModalDialog
{
public:
	COptionsDialog();
	virtual ~COptionsDialog();

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 id,const BaseContainer &msg);
	virtual Bool InitValues(void);
};

#endif // !defined(AFX_OPTIONSDIALOG_H__5DA75860_A507_11D5_9B3B_004095418E0F__INCLUDED_)
