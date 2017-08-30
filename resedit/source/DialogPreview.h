// DialogPreview.h: Schnittstelle für die Klasse CDialogPreview.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIALOGPREVIEW_H__71133B60_8E3E_11D5_9B3B_004095418E0F__INCLUDED_)
#define AFX_DIALOGPREVIEW_H__71133B60_8E3E_11D5_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDialogDoc;
class CDialogPreview;
class CPreviewSub;
class CDialogTemplate;
class CDialogItem;

class CPreviewSub : public SubDialog
{
public:
	CPreviewSub();
	void SetDialog(CDialogPreview* pDlg, CDialogTemplate* pTempl);

	virtual Bool CreateLayout(void);
	virtual Bool InitValues();
	virtual Bool Command(Int32 lId, const BaseContainer &msg);
	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);

	CDialogPreview* m_pDialog;
	CDialogTemplate* m_pTempl;
};


class CDialogPreview : public GeDialog
{
	friend class CDialogDoc;
	friend class CPreviewSub;
public:
	Bool m_bCloseState;
	CDialogPreview(CDialogDoc* pDoc);
	virtual ~CDialogPreview();

	virtual Bool CreateLayout(void);
	virtual Bool Command(Int32 lId, const BaseContainer &msg);
	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);

	Bool AskClose();// { return true; } // user may not close the dialog
	void Update();
	void MakeVisible(CDialogItem* pItem);

protected:
	CDialogDoc* m_pDocument;
	CPreviewSub m_wndSubDlg;
};

#endif // !defined(AFX_DIALOGPREVIEW_H__71133B60_8E3E_11D5_9B3B_004095418E0F__INCLUDED_)
