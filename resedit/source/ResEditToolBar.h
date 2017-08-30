/*********************************************************************\
	File name        : ResEditToolBar.h
	Description      : Interface for the class CResEditToolBar
	Created at       : 11.08.01, @ 12:53:50
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/


#if !defined(AFX_RESEDITTOOLBAR_H__A55F8FE0_8E57_11D5_9B3B_004095418E0F__INCLUDED_)
#define AFX_RESEDITTOOLBAR_H__A55F8FE0_8E57_11D5_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDialogDoc;
class BitmapButtonCustomGui;

class CResEditToolBar : public GeDialog
{
public:
	Bool m_bCloseState;
	CResEditToolBar(CDialogDoc* pDoc);
	virtual ~CResEditToolBar();

	virtual Bool CreateLayout(void);
	Bool AskClose();// { return true; } // user may not close the dialog
	virtual Bool Command(Int32 lID, const BaseContainer &msg);
	virtual Bool InitValues(void);

protected:
	CDialogDoc* m_pDocument;
};

#endif // !defined(AFX_RESEDITTOOLBAR_H__A55F8FE0_8E57_11D5_9B3B_004095418E0F__INCLUDED_)
