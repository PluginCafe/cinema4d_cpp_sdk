// TreeDialog.h: Schnittstelle für die Klasse CTreeDialog.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TREEDIALOG_H__71133B62_8E3E_11D5_9B3B_004095418E0F__INCLUDED_)
#define AFX_TREEDIALOG_H__71133B62_8E3E_11D5_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CDialogDoc;
class CDialogItem;

class CResEditTreeView : public TreeView
{
public:
	Bool IsDragAllowed(TreeViewItem* pDragDest, TreeViewItem* pInsBefore, TreeViewItem* pInsAfter);
};

class CTreeDialog : public GeDialog
{
	friend class CDialogDoc;
public:
	Bool m_bCloseState, m_bClose1;
	CTreeDialog(CDialogDoc* pDoc);
	virtual ~CTreeDialog();

	virtual Bool CreateLayout(void);
	Bool AskClose();// { return true; } // user may not close the dialog
	virtual Bool Command(Int32 id,const BaseContainer &msg);
	virtual Bool InitValues(void);
	//Bool InputEvent(const BaseContainer &msg);
	virtual Int32 Message(const BaseContainer &msg, BaseContainer &result);

	TreeViewItem* GetSelectedItem() { return m_wndTree.GetCurrentItem(); }
	void SetSelItem(CDialogItem* pItem, Bool bMakeVisible = true);
	void AddItem(CDialogItem* pNewItem, CDialogItem* pInsertAfter = nullptr);
	void RemoveItem(CDialogItem* pItem);
	TreeView* GetTreeView() { return &m_wndTree; }
	void Update();
	void SelectLanguage(Int32 lLang);
	void OpenDialog(Filename fn);
	CDialogDoc* GetDocument() { return m_pDocument; }

protected:
	void UpdateLayout();

	CDialogDoc* m_pDocument;
	CResEditTreeView m_wndTree;
};

#endif // !defined(AFX_TREEDIALOG_H__71133B62_8E3E_11D5_9B3B_004095418E0F__INCLUDED_)
