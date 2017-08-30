// TreeView.h: Schnittstelle für die Klasse TreeView1.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TREEVIEW_H__55612DC1_90DA_11D5_9B3B_004095418E0F__INCLUDED_)
#define AFX_TREEVIEW_H__55612DC1_90DA_11D5_9B3B_004095418E0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CListItem;
template <class ListItemType> class CDoubleLinkedList;

#include "doublelinklist.h"

#define TREEVIEW_TEXT       1     // String
#define TREEVIEW_ICON       2     // type of the element
// messages
#define TREEVIEW_MESSAGE_TYPE       'tvmt'
#define TREEVIEW_SELCHANGE          'tvsc'
#define TREEVIEW_STRUCTURE_CHANGE   'tvst'
#define TREEVIEW_DOUBLECLICK        'tvdc'
// msg constants
#define TREEVIEW_MESSAGE_ITEM       'tvmi'

#define TREE_ITEM_SAME_HEIGHT     1
#define TREE_VIEW_HAS_DRAG_DROP   2

typedef void (*DeleteTreeViewItemCallback)(BaseContainer* pData);

class TreeViewItem
{
//  friend class TreeViewItem;
	friend class TreeView;
	friend class VisibleTreeItem;
public:
	TreeViewItem();
	~TreeViewItem();

	Int32 GetChildCount() { return m_lChildren; }
	TreeViewItem* GetChild(Int32 l) { return (m_ppChildren != nullptr && l >= 0 && l < m_lChildren) ? m_ppChildren[l] : nullptr; }
	BaseContainer* GetData() { return &m_Data; }
	const BaseContainer* GetDataC() const { return &m_Data; }
	TreeViewItem* GetParent() { return m_pParent; }
	Bool IsChildOf(TreeViewItem* pItem);

	TreeViewItem* GetNext();
	TreeViewItem* GetPred();
	TreeViewItem* GetDown() { return GetChild(0); }
	TreeViewItem* GetUp() { return m_pParent; }

	void DestroyAllChildren(DeleteTreeViewItemCallback func);
	TreeViewItem* AddItem(const BaseContainer &data, TreeViewItem* pInsertAfter = nullptr);
	void RemoveChild(TreeViewItem* pItem, DeleteTreeViewItemCallback cb = nullptr);
	void RemoveChild(Int32 lChildIndex, DeleteTreeViewItemCallback cb = nullptr);
	void SortChildren();

	Int32 GetIcon();
	void ChangeParent(TreeViewItem* pNewParent, TreeViewItem* pInsertBefore, TreeViewItem* pInsertAfter);
	TreeViewItem **m_ppChildren;
	TreeViewItem *m_pParent;
	Int32 m_lChildren;

	BaseContainer m_Data;

	Bool m_bIsExpanded;
};
typedef void (*VisitChildrenCallbackA)(TreeViewItem* pItem, void* pData);

class TreeView
{
public:
	TreeView();
	virtual ~TreeView();
	Bool CreateTreeView(Int32 lTreeID, GeDialog* pParent, Int32 lElementType,
		DeleteTreeViewItemCallback pDeleteCallbackFunc = nullptr, Int32 lTreeViewFlags = TREE_ITEM_SAME_HEIGHT,
		Int32 lFlags = BFH_SCALEFIT | BFV_SCALEFIT);


	//virtual Int32 Message(const BaseContainer &msg,BaseContainer &result);
	/*virtual Bool InputEvent(const BaseContainer &msg);
	virtual Bool GetMinSize(Int32 &w,Int32 &h);
	virtual void Sized(Int32 w, Int32 h);*/
	TreeViewItem* GetRootItem() { return &m_RootItem; }
	void ExpandItem(TreeViewItem* pItem, Bool bExpand = true);
	Bool IsExpanded(TreeViewItem* pItem) { return pItem->m_bIsExpanded; }
	TreeViewItem* GetCurrentItem() { return m_pCurrentItem; }
	void VisitChildren(VisitChildrenCallbackA cb, TreeViewItem* pStartItem = nullptr, void* pData = nullptr);
	void SortChildren(TreeViewItem* pStartItem);

	// overridables
	//virtual Int32 GetTextHeight(TreeViewItem* pItem);
	//virtual Bool IsDragAllowed(TreeViewItem* pDragDest, TreeViewItem* pInsBefore, TreeViewItem* pInsAfter);

	TreeViewItem* AddItem(String strName, TreeViewItem* pParent = nullptr);
	TreeViewItem* AddItem(String strName, Int32 lType, TreeViewItem* pParent = nullptr);
	TreeViewItem* AddItem(const BaseContainer &data, TreeViewItem* pParent = nullptr, TreeViewItem* pInsertAfter = nullptr);
	void DeleteItem(TreeViewItem* pItem);
	void Reset();

	void SelectItem(TreeViewItem* pTreeItem);
	void SetFlags(Int32 lFlags) { m_lFlags = lFlags; }

	void SetDeleteCallback(DeleteTreeViewItemCallback func = nullptr) { m_DeleteCallback = func; }
	void LockWindowUpdate() { m_bMayUpdate = false; }
	void UnlockWindowUpdate() { m_bMayUpdate = true; Redraw(); }
	void ExpandAllItems(Bool bExpand, TreeViewItem* pFirstItem = nullptr);
	Bool MakeVisible(TreeViewItem* pItem);

	GeDialog* m_pParent;
	TreeViewCustomGui* m_pTree;

	void Redraw() { if (m_bMayUpdate) m_pTree->Refresh(); }
	void GetDragDestination(Int32 xClick, Int32 yClick);
	void _ExpandAllItems_(Bool bExpand, TreeViewItem* pFirstItem = nullptr);
	//void UpdateParentLayout();

	Int32 m_lCursor;
	TreeViewItem m_RootItem;
	TreeViewItem* m_pCurrentItem;
	TreeViewItem* m_pDragDestination;
	TreeViewItem* m_pInsertBefore, *m_pInsertAfter;

	Int32 m_lSizeX, m_lSizeY;
	Int32 m_lFlags;
	Int32 m_lItemHeight;

	Int32 m_lVisibleHeight; // the height of all elements that may be visible
	Int32 m_lVisibleWidth;

	Int32 m_lTreeID;//, m_lScrollGroupID;

	DeleteTreeViewItemCallback m_DeleteCallback;
	Bool m_bMayUpdate;

	Int32 m_lElementType;
};

#endif // !defined(AFX_TREEVIEW_H__55612DC1_90DA_11D5_9B3B_004095418E0F__INCLUDED_)
