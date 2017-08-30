/*********************************************************************\
	File name        : TreeView.cpp
	Description      : Implementation of the TreeView classes
	Created at       : 14.08.01, @ 19:13:56
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/

#if (defined _DEBUG) && (defined USE_CPP_NEW_DELETE)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "stdafx.h"
#include "globals.h"
#include "TreeView.h"
#include "TreeDialog.h"

#include "shellsort.h"
#include "DialogDoc.h"
#include "DialogItem.h"

#include "TreeDialog.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

#define DEPTH_MULTIPLY        10
#define LINE_LENGTH           9

TreeViewItem::TreeViewItem()
{
	m_ppChildren = nullptr;
	m_lChildren = 0;
	m_bIsExpanded = false;
	m_pParent = nullptr;
}

TreeViewItem::~TreeViewItem()
{
	DestroyAllChildren(nullptr);
}

/*********************************************************************\
	Function name    : TreeViewItem::AddItem
	Description      :
	Created at       : 14.08.01, @ 19:05:03
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
TreeViewItem* TreeViewItem::AddItem(const BaseContainer &data, TreeViewItem* pInsertAfter)
{
	TreeViewItem* pNewItem = NewObjClear(TreeViewItem);
	pNewItem->m_Data = data;

	TreeViewItem** pNewChildren = bNewDeprecatedUseArraysInstead<TreeViewItem*>(m_lChildren + 1);
	Int32 a, b;
	for (a = 0, b = 0; a < m_lChildren; a++, b++)
	{
		pNewChildren[b] = m_ppChildren[a];
		if ((m_ppChildren[a] == pInsertAfter) && b == a)
			pNewChildren[++b] = pNewItem;
	}

	//CopyMem(m_ppChildren, pNewChildren, sizeof(TreeViewItem*) * m_lChildren);

	bDelete(m_ppChildren);
	m_ppChildren = pNewChildren;
	if (b <= m_lChildren)
		m_ppChildren[m_lChildren] = pNewItem;
	m_lChildren++;

	return pNewItem;
}

/*********************************************************************\
	Function name    : TreeViewItem::DestroyAllChildren
	Description      :
	Created at       : 14.08.01, @ 19:11:49
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeViewItem::DestroyAllChildren(DeleteTreeViewItemCallback func)
{
	for (Int32 a = 0; a < m_lChildren; a++) {
		m_ppChildren[a]->DestroyAllChildren(func);
		if (func)
			func(m_ppChildren[a]->GetData());

		DeleteObj(m_ppChildren[a]);
	}
	bDelete(m_ppChildren);
	m_lChildren = 0;
}

/*********************************************************************\
	Function name    : TreeViewItem::ChangeParent
	Description      :
	Created at       : 15.08.01, @ 11:23:57
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeViewItem::ChangeParent(TreeViewItem* pNewParent, TreeViewItem* pInsertBefore, TreeViewItem* pInsertAfter)
{
	if (pNewParent->IsChildOf(this)) return;

	if (m_pParent->m_lChildren > 1)
	{
		TreeViewItem** ppNewChildren = bNewDeprecatedUseArraysInstead<TreeViewItem*>(m_pParent->m_lChildren - 1);
		for (Int32 s = 0, d = 0; s < m_pParent->m_lChildren; s++)
		{
			if (m_pParent->m_ppChildren[s] != this)
				ppNewChildren[d++] = m_pParent->m_ppChildren[s];
		}
		bDelete(m_pParent->m_ppChildren);
		m_pParent->m_ppChildren = ppNewChildren;
	}
	else
		bDelete(m_pParent->m_ppChildren);

	m_pParent->m_lChildren--;

	TreeViewItem** ppNewChildren = bNewDeprecatedUseArraysInstead<TreeViewItem*>(pNewParent->m_lChildren + 1);

	Bool bInserted = false;
	for (Int32 s = 0, d = 0; s < pNewParent->m_lChildren; s++, d++)
	{
		if (pNewParent->m_ppChildren[s] == pInsertBefore)
		{
			ppNewChildren[d++] = this;
			bInserted = true;
		}
		ppNewChildren[d] = pNewParent->m_ppChildren[s];
		if (pNewParent->m_ppChildren[s] == pInsertAfter)
		{
			ppNewChildren[++d] = this;
			bInserted = true;
		}
	}
	if (!bInserted)
	{
		for (Int32 a = pNewParent->m_lChildren; a >= 1; a--)
			ppNewChildren[a] = ppNewChildren[a - 1];
		ppNewChildren[0] = this;
	}

	bDelete(pNewParent->m_ppChildren);
	pNewParent->m_ppChildren = ppNewChildren;

	pNewParent->m_lChildren++;
	m_pParent = pNewParent;
}

/*********************************************************************\
	Function name    : TreeViewItem::IsChildOf
	Description      : checks, if pItem is a child of this
	Created at       : 15.08.01, @ 11:37:53
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool TreeViewItem::IsChildOf(TreeViewItem* pItem)
{
	TreeViewItem* pParent = this;
	while (pParent)
	{
		if (pParent == pItem) return true;
		pParent = pParent->GetParent();
	}
	return false;
}


/*********************************************************************\
	Function name    : TreeViewItem::GetIcon
	Description      :
	Created at       : 15.08.01, @ 09:44:05
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 TreeViewItem::GetIcon()
{
	return m_Data.GetInt32(TREEVIEW_ICON);
}


/*********************************************************************\
	Function name    : TreeViewItem::RemoveChild
	Description      :
	Created at       : 23.08.01, @ 21:37:28
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeViewItem::RemoveChild(TreeViewItem* pItem, DeleteTreeViewItemCallback cb)
{
	Int32 lItem = -1;
	for (Int32 a = 0; a < m_lChildren; a++)
	{
		if (m_ppChildren[a] == pItem)
			lItem = a;
	}
	RemoveChild(lItem, cb);
}

/*********************************************************************\
	Function name    : TreeViewItem::RemoveChild
	Description      :
	Created at       : 23.08.01, @ 21:37:30
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeViewItem::RemoveChild(Int32 lChildIndex, DeleteTreeViewItemCallback cb)
{
	if (lChildIndex < 0 || lChildIndex >= m_lChildren) return;

	if (cb)
		cb(m_ppChildren[lChildIndex]->GetData());

	DeleteObj(m_ppChildren[lChildIndex]);

	if (m_lChildren == 1)
	{
		m_lChildren = 0;
		bDelete(m_ppChildren);
		return;
	}

	TreeViewItem** ppNewChild = bNewDeprecatedUseArraysInstead<TreeViewItem*>(m_lChildren - 1);
	for (Int32 d = 0, s = 0; s < m_lChildren; s++)
	{
		if (s != lChildIndex)
			ppNewChild[d++] = m_ppChildren[s];
	}
	bDelete(m_ppChildren);
	m_ppChildren = ppNewChild;
	m_lChildren--;
}

/*********************************************************************\
	Function name    : CompareTreeViewItemSort
	Description      :
	Created at       : 06.10.01, @ 12:37:32
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Int32 CompareTreeViewItemSort(TreeViewItem* const* pItem1, TreeViewItem* const* pItem2)
{
	String str1 = (*pItem1)->GetDataC()->GetString(TREEVIEW_TEXT);
	String str2 = (*pItem2)->GetDataC()->GetString(TREEVIEW_TEXT);

	return (str1.LexCompare(str2));
}

/*********************************************************************\
	Function name    : TreeViewItem::SortChildren
	Description      :
	Created at       : 26.09.01, @ 11:55:36
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeViewItem::SortChildren()
{
	CShellSort <TreeViewItem*> sort;
	sort.Sort(m_ppChildren, CompareTreeViewItemSort, m_lChildren);

	/*Bool bAgain;
	do
	{
		bAgain = false;
		for (Int32 a = 0; a < m_lChildren - 1; a++)
		{
			for (Int32 b = a + 1; b < m_lChildren; b++)
			{
				TreeViewItem *pItem1 = m_ppChildren[a], *pItem2 = m_ppChildren[b];
				if (pItem1->m_Data.GetString(TREEVIEW_TEXT) > pItem2->m_Data.GetString(TREEVIEW_TEXT))
				{
					m_ppChildren[b] = pItem1;
					m_ppChildren[a] = pItem2;
					bAgain = true;
				}
			}
		}
	} while (bAgain);*/
}

/*********************************************************************\
	Function name    : TreeViewItem::GetNext
	Description      :
	Created at       : 26.03.02, @ 16:52:51
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
TreeViewItem* TreeViewItem::GetNext()
{
	if (!m_pParent) return nullptr;
	if (!m_pParent->m_ppChildren || m_pParent->m_lChildren <= 1) return nullptr;

	for (Int32 a = 0; a < m_pParent->m_lChildren - 1; a++)
	{
		if (m_pParent->m_ppChildren[a] == this) return m_pParent->m_ppChildren[a + 1];
	}

	return nullptr;
}

/*********************************************************************\
	Function name    : TreeViewItem::GetPred
	Description      :
	Created at       : 26.03.02, @ 16:52:52
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
TreeViewItem* TreeViewItem::GetPred()
{
	if (!m_pParent) return nullptr;
	if (!m_pParent->m_ppChildren || m_pParent->m_lChildren <= 1) return nullptr;

	for (Int32 a = 1; a < m_pParent->m_lChildren; a++)
	{
		if (m_pParent->m_ppChildren[a] == this) return m_pParent->m_ppChildren[a - 1];
	}
	return nullptr;
}





class Function2 : public TreeViewFunctions
{
	public:

		virtual void*	GetFirst(void *pRoot, void *userdata)
		{
			TreeViewItem *p = (TreeViewItem*)pRoot;
			return p->GetChild(0);
		}

		virtual void*	GetNext(void *root,void *userdata,void *obj)
		{
			TreeViewItem *p = (TreeViewItem*)obj;
			return p->GetNext();
		}

		virtual void* GetPred(void *root,void *userdata,void *obj)
		{
			TreeViewItem *p = (TreeViewItem*)obj;
			return p->GetPred();
		}

		virtual void*	GetDown(void *root,void *userdata,void *obj)
		{
			TreeViewItem *p = (TreeViewItem*)obj;
			return p->GetDown();
		}

		virtual void* GetUp(void *root,void *userdata,void *obj)
		{
			TreeViewItem *p = (TreeViewItem*)obj;
			void* pUp = p->GetUp();
			if (pUp == root) return nullptr;
			return pUp;
		}

		virtual Bool IsSelected(void *root,void *userdata,void *obj)
		{
			TreeView* pTree = (TreeView*)userdata;
			return obj == (void*)pTree->m_pCurrentItem;
		}

		virtual String GetName(void *root,void *userdata,void *obj)
		{
			TreeViewItem *p = (TreeViewItem*)obj;
			return p->m_Data.GetString(TREEVIEW_TEXT);
		}

		virtual Int GetId(void *root,void *userdata,void *obj)
		{
			return (Int)obj;
		}

		virtual Bool IsOpened(void *root,void *userdata,void *obj)
		{
			TreeViewItem *p = (TreeViewItem*)obj;
			return p->m_bIsExpanded;
		}

		virtual void Open(void *root,void *userdata,void *obj,Bool onoff)
		{
			TreeViewItem *p = (TreeViewItem*)obj;
			p->m_bIsExpanded = onoff;
			//GeEventAdd(MSG_REDRAWALL);
		}

		virtual void Select(void *root,void *userdata,void *obj,Int32 mode)
		{
			TreeView* pTree = (TreeView*)userdata;
			pTree->m_pCurrentItem = (TreeViewItem*)obj;

			BaseContainer bc;
			bc.SetInt32(TREEVIEW_MESSAGE_TYPE, TREEVIEW_SELCHANGE);
			if (mode == SELECTION_SUB) return;
				//bc.SetInt32(TREEVIEW_MESSAGE_ITEM, nullptr);
			//else
			bc.SetVoid(TREEVIEW_MESSAGE_ITEM, obj);

			pTree->m_pParent->Command(pTree->m_lTreeID, bc);
		}

		virtual Int32 AcceptDragObject(void *root,void *userdata,void *obj,Int32 dragtype,void *dragobject, Bool &bAllowCopy)
		{
			TreeView* pTree = (TreeView*)userdata;
			if (dragtype == DRAGTYPE_FILES)
			{
				return INSERT_BEFORE | INSERT_AFTER | INSERT_UNDER;
			}
			if (pTree->m_lFlags & TREE_VIEW_HAS_DRAG_DROP)
			{
				if (dragtype == pTree->m_lElementType)
				{
					Int32 lRet = INSERT_BEFORE | INSERT_AFTER;

					TreeViewItem* pDragDest = (TreeViewItem*)obj;
					CDialogItem* pDlgItem = (CDialogItem*)pDragDest->GetData()->GetVoid(TREEVIEW_DATA);
					if (pDlgItem->GetType() == Group || pDlgItem->GetType() == Dialog)
						lRet |= INSERT_UNDER;

					if (pDlgItem->GetType() == Dialog) bAllowCopy = false;
					else bAllowCopy = true;

					return lRet;
				}
			}
			return false;
		}

		virtual void InsertObject(void *root,void *userdata,void *obj,Int32 dragtype,void *dragobject,Int32 insertmode, Bool bCopy)
		{
			TreeView* pTree = (TreeView*)userdata;

			if (dragtype == DRAGTYPE_FILES)
			{
				Filename* fn = (Filename*)dragobject;
				if (!fn) return;
				((CTreeDialog*)pTree->m_pParent)->OpenDialog(*fn);
			}

			if (dragtype == pTree->m_lElementType)
			{
				TreeViewItem* pDragItem = (TreeViewItem*)dragobject;
				TreeViewItem* pDestItem = (TreeViewItem*)obj;

				if (bCopy)
				{
					if (!pDragItem || !pDestItem) return;

					CTreeDialog* pDlg = (CTreeDialog*)pTree->m_pParent;
					if (!pDlg) return;
					CDialogDoc* pDoc = pDlg->GetDocument();
					if (!pDoc) return;

					CDialogItem* pDlgItemDrag = (CDialogItem*)pDragItem->GetData()->GetVoid(TREEVIEW_DATA);
					CDialogItem* pDlgItemDrop = (CDialogItem*)pDestItem->GetData()->GetVoid(TREEVIEW_DATA);
					if (!pDlgItemDrag || !pDlgItemDrop) return;

					pDoc->ExportMacro(GeGetPluginPath() + String("DragDrop.dmf"), pDlgItemDrag);
					CDialogItem* pNewItem = pDoc->ImportMacro(GeGetPluginPath() + String("DragDrop.dmf"), pDlgItemDrop);
					GeFKill(GeGetPluginPath() + String("DragDrop.dmf"));

					if (pNewItem)
					{
						TreeViewItem* pNewTreeItem = pNewItem->m_pTreeViewItem;
						switch (insertmode)
						{
							case INSERT_BEFORE: pNewTreeItem->ChangeParent(pDestItem->GetParent(), pDestItem, nullptr); break;
							case INSERT_UNDER:  pNewTreeItem->ChangeParent(pDestItem, nullptr, nullptr); Open(root, userdata, obj, true); break;
							default:            pNewTreeItem->ChangeParent(pDestItem->GetParent(), nullptr, pDestItem); break;
						}
						Select(root, userdata, pNewTreeItem, SELECTION_NEW);
					}
				}
				else
				{
					switch (insertmode)
					{
						case INSERT_BEFORE: pDragItem->ChangeParent(pDestItem->GetParent(), pDestItem, nullptr); break;
						case INSERT_UNDER:  pDragItem->ChangeParent(pDestItem, nullptr, nullptr); Open(root, userdata, obj, true); break;
						default:            pDragItem->ChangeParent(pDestItem->GetParent(), nullptr, pDestItem); break;
					}
				}
				BaseContainer bc;
				bc.SetInt32(TREEVIEW_MESSAGE_TYPE, TREEVIEW_STRUCTURE_CHANGE);
				pTree->m_pParent->Command(pTree->m_lTreeID, bc);
				pTree->m_pTree->Refresh();
			}
		}

		virtual Int32 GetDragType(void *root,void *userdata,void *obj)
		{
			TreeView* pTree = (TreeView*)userdata;
			return pTree->m_lElementType;
		}

		virtual Bool DoubleClick(void *root,void *userdata,void *obj,Int32 col,MouseInfo *mouseinfo)
		{
			TreeView* pTree = (TreeView*)userdata;

			BaseContainer bc;
			bc.SetInt32(TREEVIEW_MESSAGE_TYPE, TREEVIEW_DOUBLECLICK);
			bc.SetVoid(TREEVIEW_MESSAGE_ITEM, obj);
			return pTree->m_pParent->Command(pTree->m_lTreeID, bc);
			//return false;
		}

		virtual Bool MouseDown(void *root,void *userdata,void *obj,Int32 col,MouseInfo *mouseinfo, Bool rightButton)
		{
			return false;
		}

		virtual Int32 GetColumnWidth(void *root,void *userdata,void *obj,Int32 col, GeUserArea* area)
		{
			if (col=='icon')
			{
				//BaseBitmap *pBmp = p->GetIcon();
				//if (pBmp) return pBmp->GetBw();
				//else return 0;
				return 20;
			}
			return 0;
		}

		virtual Int32 GetLineHeight(void *root,void *userdata,void *obj,Int32 col, GeUserArea* area)
		{
			if (col=='icon')
			{
				//BaseBitmap *pBmp = p->GetIcon();
				//if (pBmp) return pBmp->GetBh();
				//else return 0;
				return 20;
			}
			return 0;
		}

		virtual void DrawCell(void *root,void *userdata,void *obj,Int32 col,DrawInfo *drawinfo, const GeData& bgColor)
		{
			TreeViewItem* p = (TreeViewItem*)obj;
			if (col=='icon')
			{
				Int32 wx,wy,wh,ww;
				/*BaseBitmap *pBmp = p->GetIcon();
				if (pBmp)
				{
					wx = drawinfo->xpos;
					wy = drawinfo->ypos;
					ww = drawinfo->width;
					wh = drawinfo->height;

					if (ww > pBmp->GetBw()) { wx += (ww - pBmp->GetBw()) / 2; ww = pBmp->GetBw(); }
					if (wh > pBmp->GetBh()) { wy += (wh - pBmp->GetBh()) / 2; wh = pBmp->GetBh(); }

					drawinfo->frame->DrawSetPen(COLOR_BG);
					drawinfo->frame->DrawBitmap(pBmp, wx, wy, ww, wh, 0, 0, pBmp->GetBw(), pBmp->GetBh(), BMP_NORMALSCALED);
				}*/
				Int32 lWidth = 20, lHeight = 20;
				Int32 i = p->GetIcon();
				Int32 x = (i % 9) * 20;
				Int32 y = (i / 9) * 20;

				wx = drawinfo->xpos;
				wy = drawinfo->ypos;
				ww = drawinfo->width;
				wh = drawinfo->height;

				if (ww > lWidth) { wx += (ww - lWidth) / 2; ww = lWidth; }
				if (wh > lHeight) { wy += (wh - lHeight) / 2; wh = lHeight; }

				drawinfo->frame->DrawSetPen(COLOR_BG);
				drawinfo->frame->DrawBitmap(g_pControlImages, wx, wy, ww, wh, x, y, lWidth, lHeight, BMP_NORMALSCALED | BMP_ALLOWALPHA);
			}
		}
		virtual void DeletePressed(void *root, void *userdata)
		{
			TreeView* pTree = (TreeView*)userdata;
			if (!pTree) return;

			((CTreeDialog*)(pTree->m_pParent))->GetDocument()->OnEditDelete();
		}
		virtual Bool ContextMenuCall(void *root,void *userdata,void *obj, Int32 lColumn, Int32 lCommand)
		{
			if (lCommand == ID_TREEVIEW_CONTEXT_REMOVE)
				DeletePressed(root, userdata);
			if (lCommand == ID_TREEVIEW_CONTEXT_RESET)
			{
				TreeView* pTree = (TreeView*)userdata;
				if (!pTree) return false;

				((CTreeDialog*)(pTree->m_pParent))->GetDocument()->OnFileNew();
			}
			return true;
		}
} functable;









TreeView::TreeView()
{
	m_lCursor = MOUSE_SHOW;
	m_lVisibleHeight = m_lVisibleWidth = 0;
	m_RootItem.m_bIsExpanded = true;
	m_pCurrentItem = 0;
	m_lFlags = 0;
	m_pParent = 0;
	m_pDragDestination = nullptr;
	m_DeleteCallback = nullptr;
	m_bMayUpdate = true;
}

TreeView::~TreeView()
{
	m_RootItem.DestroyAllChildren(m_DeleteCallback);
}

/*********************************************************************\
	Function name    : TreeView::AddItem
	Description      :
	Created at       : 14.08.01, @ 18:33:46
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
TreeViewItem* TreeView::AddItem(String strName, TreeViewItem* pParent)
{
	BaseContainer bc;
	bc.SetString(TREEVIEW_TEXT, strName);
	return AddItem(bc, pParent);
}

/*********************************************************************\
	Function name    : TreeView::AddItem
	Description      :
	Created at       : 15.08.01, @ 09:39:49
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
TreeViewItem* TreeView::AddItem(String strName, Int32 lType, TreeViewItem* pParent)
{
	BaseContainer bc;
	bc.SetString(TREEVIEW_TEXT, strName);
	bc.SetInt32(TREEVIEW_ICON, lType);
	return AddItem(bc, pParent);
}

/*********************************************************************\
	Function name    : TreeView::AddItem
	Description      :
	Created at       : 14.08.01, @ 18:51:26
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
TreeViewItem* TreeView::AddItem(const BaseContainer &data, TreeViewItem* pParent, TreeViewItem* pInsertAfter)
{
	if (!pParent) pParent = &m_RootItem;
	TreeViewItem* pNewItem = pParent->AddItem(data, pInsertAfter);
	pNewItem->m_pParent = pParent;

	if (m_bMayUpdate)
	{
		//CreateVisibleItemList();
		Redraw();
		//UpdateParentLayout();
	}

	return pNewItem;
}

/*********************************************************************\
	Function name    : TreeView::ExpandItem
	Description      :
	Created at       : 14.08.01, @ 19:28:34
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeView::ExpandItem(TreeViewItem* pItem, Bool bExpand)
{
	pItem->m_bIsExpanded = bExpand;

	if (!bExpand && m_pCurrentItem->IsChildOf(pItem))
	{
		m_pCurrentItem = pItem;
		if (m_pParent)
		{
			BaseContainer bc;
			bc.SetInt32(TREEVIEW_MESSAGE_TYPE, TREEVIEW_SELCHANGE);
			bc.SetVoid(TREEVIEW_MESSAGE_ITEM, m_pCurrentItem);
			m_pParent->Command(m_lTreeID, bc);
		}
	}

	Redraw();
}

/*********************************************************************\
	Function name    : TreeView::Reset
	Description      :
	Created at       : 14.08.01, @ 22:28:01
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeView::Reset()
{
	m_RootItem.DestroyAllChildren(m_DeleteCallback);
	Redraw();
	m_pCurrentItem = &m_RootItem;
}

/*********************************************************************\
	Function name    : TreeView::SelectItem
	Description      :
	Created at       : 14.08.01, @ 23:17:56
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeView::SelectItem(TreeViewItem* pTreeItem)
{
	m_pCurrentItem = pTreeItem;

	TreeViewItem* pParent = pTreeItem->GetParent();
	while (pParent)
	{
		pParent->m_bIsExpanded = true;
		pParent = pParent->GetParent();
	};
	//m_VisibleItems.FreeList();
	if (m_bMayUpdate)
	{
		//CreateVisibleItemList();
		Redraw();
		//UpdateParentLayout();
	}
}

/*********************************************************************\
	Function name    : TreeView::ExpandAllItems
	Description      : expands all items that are children of pFirstItem
	Created at       : 18.08.01, @ 16:11:25
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeView::ExpandAllItems(Bool bExpand, TreeViewItem* pFirstItem)
{
	_ExpandAllItems_(bExpand, pFirstItem);
	//CreateVisibleItemList();
	Redraw();
	//UpdateParentLayout();
}

/*********************************************************************\
	Function name    : TreeView::_ExpandAllItems_
	Description      :
	Created at       : 18.08.01, @ 16:19:50
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeView::_ExpandAllItems_(Bool bExpand, TreeViewItem* pFirstItem)
{
	if (!pFirstItem) pFirstItem = &m_RootItem;

	if (pFirstItem == &m_RootItem) pFirstItem->m_bIsExpanded = true;  // the root item is always expanded
	else pFirstItem->m_bIsExpanded = bExpand;
	Int32 lChildren = pFirstItem->GetChildCount();
	for (Int32 a = 0; a < lChildren; a++)
	{
		TreeViewItem* pChild = pFirstItem->GetChild(a);
		pChild->m_bIsExpanded = bExpand;
		_ExpandAllItems_(bExpand, pChild);
	}
}

/*********************************************************************\
	Function name    : TreeView::VisitChildren
	Description      :
	Created at       : 22.08.01, @ 09:41:46
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeView::VisitChildren(VisitChildrenCallbackA cb, TreeViewItem* pStartItem, void* pData)
{
	if (!pStartItem) pStartItem = &m_RootItem;
	Int32 lChildren = pStartItem->GetChildCount();
	cb(pStartItem, pData);
	for (Int32 a = 0; a < lChildren; a++)
	{
		TreeViewItem* pChild = pStartItem->GetChild(a);
		VisitChildren(cb, pChild, pData);
	}
}

/*********************************************************************\
	Function name    : TreeView::DeleteItem
	Description      :
	Created at       : 23.08.01, @ 21:33:07
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeView::DeleteItem(TreeViewItem* pItem)
{
	if (!pItem || pItem == &m_RootItem) return;
	pItem->DestroyAllChildren(m_DeleteCallback);

	TreeViewItem* pNext = pItem->GetNext();
	if (!pNext)
		pNext = pItem->GetParent();
	pItem->GetParent()->RemoveChild(pItem, m_DeleteCallback);
	m_pCurrentItem = pNext;

	if (m_bMayUpdate)
	{
		//CreateVisibleItemList();
		Redraw();
		//UpdateParentLayout();
	}
}

/*********************************************************************\
	Function name    : TreeView::CreateTreeView
	Description      : Creates a tree view window and attaches it to the parent window
	Created at       : 26.09.01, @ 11:02:27
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool TreeView::CreateTreeView(Int32 lTreeID, GeDialog* pParent, Int32 lElementType,
		DeleteTreeViewItemCallback pDeleteCallbackFunc, Int32 lTreeViewFlags,
		Int32 lFlags)
{
	m_pParent = pParent;

	SetDeleteCallback(pDeleteCallbackFunc);

	BaseContainer treedata;
	treedata.SetBool(TREEVIEW_CTRL_DRAG, true);
	treedata.SetBool(TREEVIEW_NO_MULTISELECT, true);
	treedata.SetBool(TREEVIEW_CURSORKEYS, true);
	treedata.SetBool(TREEVIEW_ALTERNATE_BG, true);
	treedata.SetBool(TREEVIEW_FIXED_LAYOUT, true);
	//treedata.SetBool(TREEVIEW_HAS_HEADER, true);
	//treedata.SetBool(TREEVIEW_BORDER, true);
	pParent->AddCustomGui(lTreeID, CUSTOMGUI_TREEVIEW, String(), BFH_SCALEFIT | BFV_SCALEFIT, 0, 0, treedata);

	m_pTree = (TreeViewCustomGui*)pParent->FindCustomGui(lTreeID, CUSTOMGUI_TREEVIEW);
	if (!m_pTree) return false;

	BaseContainer layout;
	layout.SetInt32('tree', LV_TREE);
	layout.SetInt32('icon', LV_USER);
	m_pTree->SetLayout(2,layout);

	m_pTree->SetRoot(&m_RootItem, &functable, this);

	m_lTreeID = lTreeID;
	m_lElementType = lElementType;
	m_lFlags = lTreeViewFlags;

	return true;
}

/*********************************************************************\
	Function name    : TreeView::SortChildren
	Description      :
	Created at       : 26.09.01, @ 11:52:55
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
void TreeView::SortChildren(TreeViewItem* pStartItem)
{
	pStartItem->SortChildren();
	Redraw();
}

/*********************************************************************\
	Function name    : TreeView::MakeVisible
	Description      : makes pItem visible. It does not select the item
										 returns true if the item was visible
	Created at       : 21.10.01, @ 17:58:35
	Created by       : Thomas Kunert
	Modified by      :
\*********************************************************************/
Bool TreeView::MakeVisible(TreeViewItem* pItem)
{
	m_pTree->MakeVisible(pItem);
	return true;
}
