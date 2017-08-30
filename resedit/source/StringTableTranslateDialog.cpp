// StringTableTranslateDialog.cpp
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "globals.h"
#include "StringTableTranslateDialog.h"
#include "res_textparser.h"
#include "shellsort.h"

#define STRING_TABLE_NAME           'stna'
#define STRING_TABLE_LOCATIION      'stlo'
#define STRING_TABLE_DELETED        'stde'
#define STRING_TABLE_CREATED        'stcr'
#define STRING_TABLE_MODIFIED       'stmo'
#define STRING_TABLE_MISSING        'stmi'
#define STRING_TABLE_TYPE           'stty'
#define STRING_TABLE_ERROR          'ster'

#define ST_ELEMENT_ID               'seid'
#define ST_ELEMENT_STATE            'sees'
#define ST_ELEMENT_US_OLD           'seuo'
#define ST_ELEMENT_US_NEW           'seun'
#define ST_ELEMENT_COUNTRY          'secn'

#define CHECK_ICON_WIDTH            15
#define CHECK_ICON_HEIGHT           15

#define IDC_TRANSLATE_FIRST_LANG    1000
#define IDC_BASE_LANGUAGE						2000

void FreeListCallback(CListItem* pItem)
{
	CStringTableItem* pTableItem = (CStringTableItem*)pItem;
	DeleteObj(pTableItem);
}

class CStringSet : public CDynamicSortSet<tagTreeViewStringElement>
{
protected:
	Int32 Compare(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b) const
	{
		return a->pItem->strID.Compare(b->pItem->strID);
	}
public:
	virtual ~CStringSet()
	{
		Free();
	}

	void FreeItems()
	{
		Int32 a;
		for (a = 0; a < m_lElements; a++)
		{
			DeleteObj(m_pElements[a].pItem);
		}
	}
};

class CAvailStringsFuncs : public TreeViewFunctions
{
public:
	virtual void*		GetFirst(void *root,void *userdata)
	{
		CDoubleLinkedList <CAvailStringTableItem>* pItems = (CDoubleLinkedList <CAvailStringTableItem>*)root;
		return pItems->First();
	}
	virtual void*		GetDown(void *root,void *userdata,void *obj)
	{
		return nullptr;
	}
	virtual void*		GetNext(void *root,void *userdata,void *obj)
	{
		CDoubleLinkedList <CAvailStringTableItem>* pItems = (CDoubleLinkedList <CAvailStringTableItem>*)root;
		CAvailStringTableItem* pElement = (CAvailStringTableItem*)obj;

		return pItems->Next(pElement);
	}
	virtual Int32		GetColumnWidth(void *root, void *userdata, void *obj, Int32 col, GeUserArea* area)
	{
		GeUserArea* pArea = ((CStringTranslateDialog*)userdata)->GetTablesTreeViewArea();
		if (!pArea)
			return 50;

		CAvailStringTableItem* pElement = (CAvailStringTableItem*)obj;
		if (col == STRING_TABLE_LOCATIION)
		{
			return pArea->DrawGetTextWidth(pElement->fnRelPath.GetString()) + 4;
		}
		else if (col == STRING_TABLE_TYPE)
		{
			return pArea->DrawGetTextWidth(pElement->strType) + 4;
		}
		return 50;
	}
	virtual Bool		IsSelected(void *root,void *userdata,void *obj)
	{
		CStringTranslateDialog* pDlg = (CStringTranslateDialog*)userdata;
		return obj == pDlg->m_pSelectedItem;
	}
	virtual Bool		IsOpened(void *root,void *userdata,void *obj)
	{
		return false;
	}
	virtual String	GetName(void *root,void *userdata,void *obj)
	{
		CAvailStringTableItem* pElement = (CAvailStringTableItem*)obj;

		return pElement->strName;
	}
	virtual Int		GetId(void *root,void *userdata,void *obj)
	{
		return 1;
	}
	virtual Int32		GetDragType(void *root,void *userdata,void *obj)
	{
		return NOTOK;
	}

	virtual void		Select(void *root,void *userdata,void *obj,Int32 mode)
	{
		CStringTranslateDialog* pDlg = (CStringTranslateDialog*)userdata;
		Bool bLoadNew = false;

		pDlg->SelectString(pDlg->m_pSelectedString);

		if (pDlg->m_pSelectedItem == nullptr)
			bLoadNew = true;
		else if (pDlg->m_pSelectedItem != obj)
		{
			if (pDlg->AskModified())
				bLoadNew = true;
		}
		if (bLoadNew)
			pDlg->LoadStringTable((CAvailStringTableItem*)obj);
	}

	virtual void		DrawCell(void *root,void *userdata,void *obj,Int32 col,DrawInfo *drawinfo, const GeData& bgColor)
	{
		CAvailStringTableItem* pElement = (CAvailStringTableItem*)obj;

		drawinfo->frame->SetClippingRegion(drawinfo->xpos, drawinfo->ypos, drawinfo->width, drawinfo->height);
		drawinfo->frame->DrawSetTextCol(COLOR_TEXT, COLOR_TRANS);
		if (col == STRING_TABLE_LOCATIION)
		{
			drawinfo->frame->DrawText(pElement->fnRelPath.GetString(), drawinfo->xpos, drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2);
		}
		else if (col == STRING_TABLE_DELETED)
		{
			// alten Pfad gibts, neuen nicht
			if (pElement->bIsOld && !pElement->bIsNew)
			{
				drawinfo->frame->DrawSetPen(bgColor);
				drawinfo->frame->DrawBitmap(g_pStringCompareIcons, (drawinfo->width - CHECK_ICON_WIDTH) / 2 + drawinfo->xpos,
					(drawinfo->height - CHECK_ICON_HEIGHT) / 2 + drawinfo->ypos, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT,
					2 * CHECK_ICON_WIDTH, 0, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT, BMP_ALLOWALPHA);
			}
		}
		else if (col == STRING_TABLE_CREATED)
		{
			if (!pElement->bIsOld && pElement->bIsNew)
			{
				drawinfo->frame->DrawSetPen(bgColor);
				drawinfo->frame->DrawBitmap(g_pStringCompareIcons, (drawinfo->width - CHECK_ICON_WIDTH) / 2 + drawinfo->xpos,
					(drawinfo->height - CHECK_ICON_HEIGHT) / 2 + drawinfo->ypos, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT,
					1 * CHECK_ICON_WIDTH, 0, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT, BMP_ALLOWALPHA);
			}
		}
		else if (col == STRING_TABLE_MODIFIED)
		{
			if (pElement->bModified)
			{
				drawinfo->frame->DrawSetPen(bgColor);
				drawinfo->frame->DrawBitmap(g_pStringCompareIcons, (drawinfo->width - CHECK_ICON_WIDTH) / 2 + drawinfo->xpos,
					(drawinfo->height - CHECK_ICON_HEIGHT) / 2 + drawinfo->ypos, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT,
					0 * CHECK_ICON_WIDTH, 0, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT, BMP_ALLOWALPHA);
			}
		}
		else if (col == STRING_TABLE_ERROR)
		{
			if (pElement->bHasErrors)
			{
				drawinfo->frame->DrawSetPen(bgColor);
				drawinfo->frame->DrawBitmap(g_pStringCompareIcons, (drawinfo->width - CHECK_ICON_WIDTH) / 2 + drawinfo->xpos,
					(drawinfo->height - CHECK_ICON_HEIGHT) / 2 + drawinfo->ypos, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT,
					4 * CHECK_ICON_WIDTH, 0, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT, BMP_ALLOWALPHA);
			}
		}
		else if (col == STRING_TABLE_MISSING)
		{
			drawinfo->frame->DrawSetTextCol(COLOR_TEXT, COLOR_TRANS);
			drawinfo->frame->DrawText(pElement->strMissing, drawinfo->xpos, drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2);
			/*if (pElement->strMissing.Content())
			{
				drawinfo->frame->DrawSetPen(bgColor);
				drawinfo->frame->DrawBitmap(g_pStringCompareIcons, (drawinfo->width - CHECK_ICON_WIDTH) / 2 + drawinfo->xpos,
					(drawinfo->height - CHECK_ICON_HEIGHT) / 2 + drawinfo->ypos, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT,
					0 * CHECK_ICON_WIDTH, 0, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT, BMP_ALLOWALPHA);
			}*/
		}
		else if (col == STRING_TABLE_TYPE)
		{
			drawinfo->frame->DrawSetTextCol(COLOR_TEXT, COLOR_TRANS);
			drawinfo->frame->DrawText(pElement->strType, drawinfo->xpos, drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2);
		}
		drawinfo->frame->ClearClippingRegion();
	}
	virtual Int32 GetHeaderSortArrow(void *root, void *userdata, Int32 lColID)
	{
		CStringTranslateDialog* pDlg = (CStringTranslateDialog*)userdata;
		if (lColID == pDlg->m_lLastSortCol)
			return pDlg->m_lLastSortDir;

		return 0;
	}

	virtual Bool HeaderClick(void *root, void *userdata, Int32 lColID, Int32 lChannel, Bool bDbl)
	{
		CStringTranslateDialog* pDlg = (CStringTranslateDialog*)userdata;
		if (pDlg->m_lLastSortCol != lColID)
			pDlg->m_lLastSortDir = 1;
		else
		{
			if (pDlg->m_lLastSortDir != 0)
				pDlg->m_lLastSortDir *= -1;
			else
				pDlg->m_lLastSortDir = 1;
		}

		CDoubleLinkedList <CAvailStringTableItem>* pTables = pDlg->GetAvailTables();

		Int32 lElements = pTables->Entries();

		tagTreeViewSort* pItems = bNewDeprecatedUseArraysInstead<tagTreeViewSort>(lElements);
		Int32 a = 0;
		if (pItems)
		{
			CAvailStringTableItem* pItem = pTables->First();
			a = 0;
			while (pItem)
			{
				pItems[a].pItem = pItem;
				pItems[a].lIndex = a;
				a++;
				pItem = pTables->Next(pItem);
			}

			CShellSort <tagTreeViewSort> s;
			if (lColID == STRING_TABLE_NAME && pDlg->m_lLastSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewNameDown, lElements);
			else if (lColID == STRING_TABLE_NAME && pDlg->m_lLastSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewNameUp, lElements);

			if (lColID == STRING_TABLE_LOCATIION && pDlg->m_lLastSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewDirDown, lElements);
			else if (lColID == STRING_TABLE_LOCATIION && pDlg->m_lLastSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewDirUp, lElements);

			if (lColID == STRING_TABLE_DELETED && pDlg->m_lLastSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewDeletedDown, lElements);
			else if (lColID == STRING_TABLE_DELETED && pDlg->m_lLastSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewDeletedUp, lElements);

			if (lColID == STRING_TABLE_CREATED && pDlg->m_lLastSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewCreatedDown, lElements);
			else if (lColID == STRING_TABLE_CREATED && pDlg->m_lLastSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewCreatedUp, lElements);

			if (lColID == STRING_TABLE_MODIFIED && pDlg->m_lLastSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewModifiedDown, lElements);
			else if (lColID == STRING_TABLE_MODIFIED && pDlg->m_lLastSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewModifiedUp, lElements);

			if (lColID == STRING_TABLE_MISSING && pDlg->m_lLastSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewMissingDown, lElements);
			else if (lColID == STRING_TABLE_MISSING && pDlg->m_lLastSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewMissingUp, lElements);

			if (lColID == STRING_TABLE_TYPE && pDlg->m_lLastSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewTypeDown, lElements);
			else if (lColID == STRING_TABLE_TYPE && pDlg->m_lLastSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareTreeViewTypeUp, lElements);

			pTables->UnlinkList();
			for (a = 0; a < lElements; a++)
				pTables->Append(pItems[a].pItem);
		}
		bDelete(pItems);

		pDlg->m_lLastSortCol = lColID;

		return true;
	}

	Bool   DoubleClick(void *root,void *userdata,void *obj,Int32 col,MouseInfo *mouseinfo)
	{
		return true;
	}

	Bool    IsMoveColAllowed(void *root, void *userdata, Int32 lColID)
	{
		if (lColID == STRING_TABLE_NAME)
			return false;
		return true;
	}

	Bool    ColumnMoved(void *root, void *userdata, Int32 lColID, Int32 lInsertBeforeCol, Int32* plIndexMap, const BaseContainer* pbcLayout)
	{
		if (lInsertBeforeCol == 0)
			return false;
		return true;
	}

} availStringsFuncs;

class CStringTableFuncs : public TreeViewFunctions
{
public:
	virtual void*		GetFirst(void *root,void *userdata)
	{
		CDoubleLinkedList <CStringTableItem>* pItems = (CDoubleLinkedList <CStringTableItem>*)root;
		return pItems->First();
	}

	virtual void*		GetDown(void *root,void *userdata,void *obj)
	{
		return nullptr;
	}
	virtual void*		GetNext(void *root,void *userdata,void *obj)
	{
		CDoubleLinkedList <CStringTableItem>* pItems = (CDoubleLinkedList <CStringTableItem>*)root;
		CStringTableItem* pElement = (CStringTableItem*)obj;

		return pItems->Next(pElement);
	}
	virtual Int32		GetColumnWidth(void *root,void *userdata,void *obj,Int32 col, GeUserArea* area)
	{
		GeUserArea* pArea = ((CStringTranslateDialog*)userdata)->GetTablesTreeViewArea();
		if (!pArea)
			return 50;

		CStringTableItem* pElement = (CStringTableItem*)obj;
		if (col == ST_ELEMENT_ID)
		{
			return pArea->DrawGetTextWidth(pElement->strID) + 4;
		}
		else if (col == ST_ELEMENT_STATE)
		{
			return 15;
		}
		else if (col == ST_ELEMENT_US_OLD)
		{
			return pArea->DrawGetTextWidth(pElement->strUSOld) + 4;
		}
		else if (col == ST_ELEMENT_US_NEW)
		{
			return pArea->DrawGetTextWidth(pElement->strUSNew) + 4;
		}
		else if (col == ST_ELEMENT_COUNTRY)
		{
			return pArea->DrawGetTextWidth(pElement->strCountry) + 4;
		}
		return 50;
	}
	virtual Bool		IsSelected(void *root,void *userdata,void *obj)
	{
		CStringTranslateDialog* pDlg = (CStringTranslateDialog*)userdata;
		return pDlg->m_pSelectedString == obj;
	}
	virtual void		Select(void *root,void *userdata,void *obj,Int32 mode)
	{
		CStringTranslateDialog* pDlg = (CStringTranslateDialog*)userdata;
		pDlg->SelectString((CStringTableItem*)obj);
	}
	virtual Bool		IsOpened(void *root,void *userdata,void *obj)
	{
		return false;
	}
	virtual String	GetName(void *root,void *userdata,void *obj)
	{
		CStringTableItem* pElement = (CStringTableItem*)obj;
		return pElement->strID;
	}
	virtual Int		GetId(void *root,void *userdata,void *obj)
	{
		return 1;
	}
	virtual Int32		GetDragType(void *root,void *userdata,void *obj)
	{
		return NOTOK;
	}

	virtual void		DrawCell(void *root,void *userdata,void *obj,Int32 col,DrawInfo *drawinfo, const GeData& bgColor)
	{
		CStringTableItem* pElement = (CStringTableItem*)obj;
		drawinfo->frame->SetClippingRegion(drawinfo->xpos, drawinfo->ypos, drawinfo->width, drawinfo->height);
		if (col == ST_ELEMENT_ID)
		{
			drawinfo->frame->DrawSetTextCol(COLOR_TEXT, COLOR_TRANS);
			drawinfo->frame->DrawText(pElement->strID, drawinfo->xpos, drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2);
		}
		else if (col == ST_ELEMENT_STATE)
		{
			if (pElement->lIcon >= 0)
			{
				drawinfo->frame->DrawSetPen(bgColor);
				drawinfo->frame->DrawBitmap(g_pStringCompareIcons, (drawinfo->width - CHECK_ICON_WIDTH) / 2 + drawinfo->xpos,
					(drawinfo->height - CHECK_ICON_HEIGHT) / 2 + drawinfo->ypos, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT,
					pElement->lIcon * CHECK_ICON_WIDTH, 0, CHECK_ICON_WIDTH, CHECK_ICON_HEIGHT, BMP_ALLOWALPHA);
			}
		}
		else if (col == ST_ELEMENT_US_OLD)
		{
			drawinfo->frame->DrawSetTextCol(COLOR_TEXT, COLOR_TRANS);
			drawinfo->frame->DrawText(pElement->strUSOld, drawinfo->xpos, drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2);
		}
		else if (col == ST_ELEMENT_US_NEW)
		{
			drawinfo->frame->DrawSetTextCol(COLOR_TEXT, COLOR_TRANS);
			drawinfo->frame->DrawText(pElement->strUSNew, drawinfo->xpos, drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2);
		}
		else if (col == ST_ELEMENT_COUNTRY)
		{
			drawinfo->frame->DrawSetTextCol(COLOR_TEXT, COLOR_TRANS);
			drawinfo->frame->DrawText(pElement->strCountry, drawinfo->xpos, drawinfo->ypos + (drawinfo->height - drawinfo->frame->DrawGetFontHeight()) / 2);
		}
		drawinfo->frame->ClearClippingRegion();
	}
	virtual void    GetColors(void *root,void *userdata,void *obj, GeData* pNormal, GeData* pSelected)
	{
		CStringTableItem* pElement = (CStringTableItem*)obj;
		if (pElement->bIsOld && !pElement->bIsNew)
		{
			*pNormal = COLOR_EDGEDK;
			*pSelected = COLOR_EDGEDK;
		}
		else if (!pElement->bIsOld && pElement->bIsNew)
			*pNormal = COLOR_EDGEWH;
	}
	virtual Bool    IsResizeColAllowed(void *root, void *userdata, Int32 lColID)
	{
		if (lColID != ST_ELEMENT_STATE)
			return true;
		return false;
	}

	virtual Int32 GetHeaderSortArrow(void *root, void *userdata, Int32 lColID)
	{
		CStringTranslateDialog* pDlg = (CStringTranslateDialog*)userdata;
		if (lColID == pDlg->m_lLastStringSortCol)
			return pDlg->m_lLastStringSortDir;

		return 0;
	}
	virtual Bool HeaderClick(void *root, void *userdata, Int32 lColID, Int32 lChannel, Bool bDbl)
	{
		CStringTranslateDialog* pDlg = (CStringTranslateDialog*)userdata;
		if (pDlg->m_lLastStringSortCol != lColID)
			pDlg->m_lLastStringSortDir = 1;
		else
		{
			if (pDlg->m_lLastStringSortDir != 0)
				pDlg->m_lLastStringSortDir *= -1;
			else
				pDlg->m_lLastStringSortDir = 1;
		}

		CDoubleLinkedList <CStringTableItem>* pTables = pDlg->GetTableElements();
		Int32 lElements = pTables->Entries();
		tagTreeViewStringElement* pItems = bNewDeprecatedUseArraysInstead<tagTreeViewStringElement>(lElements);
		Int32 a = 0;
		if (pItems)
		{
			CStringTableItem* pItem = pTables->First();
			a = 0;
			while (pItem)
			{
				pItems[a].pItem = pItem;
				pItems[a].lIndex = a;
				a++;
				pItem = pTables->Next(pItem);
			}

			CShellSort <tagTreeViewStringElement> s;
			if (lColID == ST_ELEMENT_ID && pDlg->m_lLastStringSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareStringsTreeViewIDDown, lElements);
			else if (lColID == ST_ELEMENT_ID && pDlg->m_lLastStringSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareStringsTreeViewIDUp, lElements);

			if (lColID == ST_ELEMENT_STATE && pDlg->m_lLastStringSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareStringsTreeViewModifiedDown, lElements);
			else if (lColID == ST_ELEMENT_STATE && pDlg->m_lLastStringSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareStringsTreeViewModifiedUp, lElements);

			if (lColID == ST_ELEMENT_US_OLD && pDlg->m_lLastStringSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareStringsTreeViewUSOldDown, lElements);
			else if (lColID == ST_ELEMENT_US_OLD && pDlg->m_lLastStringSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareStringsTreeViewUSOldUp, lElements);

			if (lColID == ST_ELEMENT_US_NEW && pDlg->m_lLastStringSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareStringsTreeViewUSNewDown, lElements);
			else if (lColID == ST_ELEMENT_US_NEW && pDlg->m_lLastStringSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareStringsTreeViewUSNewUp, lElements);

			if (lColID == ST_ELEMENT_COUNTRY && pDlg->m_lLastStringSortDir < 0)
				s.Sort(pItems, CStringTranslateDialog::CompareStringsTreeViewCountryDown, lElements);
			else if (lColID == ST_ELEMENT_COUNTRY && pDlg->m_lLastStringSortDir > 0)
				s.Sort(pItems, CStringTranslateDialog::CompareStringsTreeViewCountryUp, lElements);

			pTables->UnlinkList();
			for (a = 0; a < lElements; a++)
				pTables->Append(pItems[a].pItem);
		}
		bDelete(pItems);

		pDlg->m_lLastStringSortCol = lColID;

		return true;
	}
	Bool   DoubleClick(void *root,void *userdata,void *obj,Int32 col,MouseInfo *mouseinfo)
	{
		return true;
	}
	Bool    IsMoveColAllowed(void *root, void *userdata, Int32 lColID)
	{
		if (lColID == ST_ELEMENT_ID)
			return false;

		return true;
	}
	Bool    ColumnMoved(void *root, void *userdata, Int32 lColID, Int32 lInsertBeforeCol, Int32* plIndexMap, const BaseContainer* pbcLayout)
	{
		if (lInsertBeforeCol == 0)
			return false;
		return true;
	}

} StringTableFuncs;

// name
Int32 CStringTranslateDialog::CompareTreeViewNameUp(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	Int32 l = a->pItem->strName.LexCompare(b->pItem->strName);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareTreeViewNameDown(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	Int32 l = b->pItem->strName.LexCompare(a->pItem->strName);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

// directory
Int32 CStringTranslateDialog::CompareTreeViewDirUp(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	Int32 l = a->pItem->fnRelPath.GetString().LexCompare(b->pItem->fnRelPath.GetString());
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareTreeViewDirDown(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	Int32 l = b->pItem->fnRelPath.GetString().LexCompare(a->pItem->fnRelPath.GetString());
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

// created
Int32 CStringTranslateDialog::CompareTreeViewCreatedUp(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	if ((!a->pItem->bIsOld && a->pItem->bIsNew) && !(!b->pItem->bIsOld && b->pItem->bIsNew))
		return -1;
	else if (!(!a->pItem->bIsOld && a->pItem->bIsNew) && (!b->pItem->bIsOld && b->pItem->bIsNew))
		return 1;

	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareTreeViewCreatedDown(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	if ((!a->pItem->bIsOld && a->pItem->bIsNew) && !(!b->pItem->bIsOld && b->pItem->bIsNew))
		return 1;
	else if (!(!a->pItem->bIsOld && a->pItem->bIsNew) && (!b->pItem->bIsOld && b->pItem->bIsNew))
		return -1;

	return a->lIndex < b->lIndex ? -1 : 1;
}

// deleted
Int32 CStringTranslateDialog::CompareTreeViewDeletedUp(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	if ((a->pItem->bIsOld && !a->pItem->bIsNew) && !(b->pItem->bIsOld && !b->pItem->bIsNew))
		return -1;
	else if (!(a->pItem->bIsOld && !a->pItem->bIsNew) && (b->pItem->bIsOld && !b->pItem->bIsNew))
		return 1;

	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareTreeViewDeletedDown(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	if ((a->pItem->bIsOld && !a->pItem->bIsNew) && !(b->pItem->bIsOld && !b->pItem->bIsNew))
		return 1;
	else if (!(a->pItem->bIsOld && !a->pItem->bIsNew) && (b->pItem->bIsOld && !b->pItem->bIsNew))
		return -1;

	return a->lIndex < b->lIndex ? -1 : 1;
}

// modified
Int32 CStringTranslateDialog::CompareTreeViewModifiedUp(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	if (a->pItem->bModified && !b->pItem->bModified)
		return -1;
	else if (!a->pItem->bModified && b->pItem->bModified)
		return 1;

	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareTreeViewModifiedDown(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	if (a->pItem->bModified && !b->pItem->bModified)
		return 1;
	else if (!a->pItem->bModified && b->pItem->bModified)
		return -1;

	return a->lIndex < b->lIndex ? -1 : 1;
}

// missing
Int32 CStringTranslateDialog::CompareTreeViewMissingUp(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	Int32 c = a->pItem->strMissing.LexCompare(b->pItem->strMissing);
	if (c != 0)
		return c;
	/*if (a->pItem->bMissing && !b->pItem->bMissing)
		return -1;
	else if (!a->pItem->bMissing && b->pItem->bMissing)
		return 1;*/

	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareTreeViewMissingDown(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	Int32 c = a->pItem->strMissing.LexCompare(b->pItem->strMissing);
	if (c != 0)
		return -c;
	/*if (a->pItem->bMissing && !b->pItem->bMissing)
		return 1;
	else if (!a->pItem->bMissing && b->pItem->bMissing)
		return -1;*/

	return a->lIndex < b->lIndex ? -1 : 1;
}

// type
Int32 CStringTranslateDialog::CompareTreeViewTypeUp(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	Int32 l = a->pItem->strType.LexCompare(b->pItem->strType);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareTreeViewTypeDown(const tagTreeViewSort *a, const tagTreeViewSort *b)
{
	Int32 l = b->pItem->strType.LexCompare(a->pItem->strType);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

// String ID
Int32 CStringTranslateDialog::CompareStringsTreeViewIDUp(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	Int32 l = a->pItem->strID.LexCompare(b->pItem->strID);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareStringsTreeViewIDDown(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	Int32 l = b->pItem->strID.LexCompare(a->pItem->strID);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

// state
Int32 CStringTranslateDialog::CompareStringsTreeViewModifiedUp(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	if (a->pItem->lIcon < b->pItem->lIcon)
		return -1;
	else if (a->pItem->lIcon > b->pItem->lIcon)
		return 1;
	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareStringsTreeViewModifiedDown(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	if (a->pItem->lIcon < b->pItem->lIcon)
		return 1;
	else if (a->pItem->lIcon > b->pItem->lIcon)
		return -1;
	return a->lIndex < b->lIndex ? -1 : 1;
}

// US old
Int32 CStringTranslateDialog::CompareStringsTreeViewUSOldUp(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	Int32 l = a->pItem->strUSOld.LexCompare(b->pItem->strUSOld);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareStringsTreeViewUSOldDown(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	Int32 l = b->pItem->strUSOld.LexCompare(a->pItem->strUSOld);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

// US new
Int32 CStringTranslateDialog::CompareStringsTreeViewUSNewUp(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	Int32 l = a->pItem->strUSNew.LexCompare(b->pItem->strUSNew);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareStringsTreeViewUSNewDown(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	Int32 l = b->pItem->strUSNew.LexCompare(a->pItem->strUSNew);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

// country
Int32 CStringTranslateDialog::CompareStringsTreeViewCountryUp(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	Int32 l = a->pItem->strCountry.LexCompare(b->pItem->strCountry);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareStringsTreeViewCountryDown(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	Int32 l = b->pItem->strCountry.LexCompare(a->pItem->strCountry);
	if (l != 0)
		return l;
	return a->lIndex < b->lIndex ? -1 : 1;
}

Int32 CStringTranslateDialog::CompareStringTableItem(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	if (a->lIndex < b->lIndex)
		return -1;
	else if (a->lIndex > b->lIndex)
		return 1;
	return 0;
}

Int32 CStringTranslateDialog::CompareStringsByIndex(const tagTreeViewStringElement* a, const tagTreeViewStringElement* b)
{
	if (a->pItem->lIndex < b->pItem->lIndex)
		return -1;
	else if (a->pItem->lIndex > b->pItem->lIndex)
		return 1;
	return 0;
}


CStringTranslateDialog::CStringTranslateDialog()
{
	m_lLastSortCol = -1;
	m_lLastSortDir = 0;
	m_lLastStringSortCol = -1;
	m_lLastStringSortDir = 0;
	m_bWasChanged = false;
	m_dlgSearch.m_pDlg = this;
	m_strOrigLanguagePath = "strings_us";
	m_strOrigLanguage = "US";
}

CStringTranslateDialog::~CStringTranslateDialog()
{
	m_availStringTables.FreeList();
	m_TableElements.FreeList();
}

Bool CStringTranslateDialog::CreateLayout()
{
	if (!GeDialog::CreateLayout() ||
					!LoadDialogResource(IDC_STRING_TRANS_DIALOG, nullptr, 0))
		return false;

	m_pAvailStringTablesTree = (TreeViewCustomGui*)FindCustomGui(IDC_TABLES_TREE, CUSTOMGUI_TREEVIEW);
	m_pStringsTree = (TreeViewCustomGui*)FindCustomGui(IDC_STRINGS_TREE, CUSTOMGUI_TREEVIEW);
	if (!m_pAvailStringTablesTree || !m_pStringsTree)
		return false;

	Int32 a;
	m_lSelLanguage = -1;
	for (a = 0; a < g_LanguageList.GetNumLanguages(); a++)
	{
		//if ((m_pDocument->GetStringTable()->GetLanguageID(g_LanguageList.GetLanguageSuffix(a))) >= 0)
		{
			AddChild(IDC_TRANSLATE_LANG_ARROW, IDC_TRANSLATE_FIRST_LANG + a, g_LanguageList.GetName(a));
			if (m_lSelLanguage == -1) m_lSelLanguage = a;
			AddChild(IDC_BASE_LANG_ARROW, IDC_BASE_LANGUAGE + a, g_LanguageList.GetName(a));
		}
	}

	BaseContainer bcLayoutTables, bcLayoutStrings;

	bcLayoutTables.SetInt32(STRING_TABLE_NAME, LV_TREE);
	bcLayoutTables.SetInt32(STRING_TABLE_LOCATIION, LV_USER);
	bcLayoutTables.SetInt32(STRING_TABLE_DELETED, LV_USER);
	bcLayoutTables.SetInt32(STRING_TABLE_CREATED, LV_USER);
	bcLayoutTables.SetInt32(STRING_TABLE_MODIFIED, LV_USER);
	bcLayoutTables.SetInt32(STRING_TABLE_MISSING, LV_USER);
	bcLayoutTables.SetInt32(STRING_TABLE_TYPE, LV_USER);
	bcLayoutTables.SetInt32(STRING_TABLE_ERROR, LV_USER);
	m_pAvailStringTablesTree->SetLayout(8, bcLayoutTables);
	m_pAvailStringTablesTree->SetRoot(&m_availStringTables, &availStringsFuncs, this);

	m_pAvailStringTablesTree->SetHeaderText(STRING_TABLE_NAME, GeLoadString(IDS_TABLE_NAME));
	m_pAvailStringTablesTree->SetHeaderText(STRING_TABLE_LOCATIION, GeLoadString(IDS_TABLE_LOCATION));
	m_pAvailStringTablesTree->SetHeaderText(STRING_TABLE_DELETED, GeLoadString(IDS_TABLE_WAS_DELETED));
	m_pAvailStringTablesTree->SetHeaderText(STRING_TABLE_CREATED, GeLoadString(IDS_TABLE_WAS_CREATED));
	m_pAvailStringTablesTree->SetHeaderText(STRING_TABLE_MODIFIED, GeLoadString(IDS_TABLE_WAS_MODIFIED));
	m_pAvailStringTablesTree->SetHeaderText(STRING_TABLE_MISSING, GeLoadString(IDS_TABLE_MISSING));
	m_pAvailStringTablesTree->SetHeaderText(STRING_TABLE_TYPE, GeLoadString(IDS_TABLE_TYPE));
	m_pAvailStringTablesTree->SetHeaderText(STRING_TABLE_ERROR, GeLoadString(IDS_TABLE_ERROR));
	bcLayoutTables.SetInt32(STRING_TABLE_TYPE, LV_USER);

	bcLayoutStrings.SetInt32(ST_ELEMENT_ID, LV_TREE);
	bcLayoutStrings.SetInt32(ST_ELEMENT_STATE, LV_USER);
	bcLayoutStrings.SetInt32(ST_ELEMENT_US_OLD, LV_USER);
	bcLayoutStrings.SetInt32(ST_ELEMENT_US_NEW, LV_USER);
	bcLayoutStrings.SetInt32(ST_ELEMENT_COUNTRY, LV_USER);
	m_pStringsTree->SetLayout(5, bcLayoutStrings);
	m_pStringsTree->SetRoot(&m_TableElements, &StringTableFuncs, this);

	m_pStringsTree->SetHeaderText(ST_ELEMENT_ID, GeLoadString(IDS_ST_STRING_ID));
	m_pStringsTree->SetHeaderText(ST_ELEMENT_STATE, GeLoadString(IDS_ST_STRING_STATE));
	m_pStringsTree->SetHeaderText(ST_ELEMENT_US_OLD, m_strOrigLanguage + GeLoadString(IDS_ST_STRING_US_OLD));
	m_pStringsTree->SetHeaderText(ST_ELEMENT_US_NEW, m_strOrigLanguage + GeLoadString(IDS_ST_STRING_US_NEW));
	m_pStringsTree->SetHeaderText(ST_ELEMENT_COUNTRY, g_LanguageList.GetLanguageSuffix(m_lSelLanguage) + "- " + g_LanguageList.GetLanguageName(m_lSelLanguage));

	return true;
}

Bool CStringTranslateDialog::Command(Int32 lID, const BaseContainer &msg)
{
	switch (lID)
	{
	case IDC_ORIG_PATH_BTN:
		{
			m_fnOld = g_pResEditPrefs->strOrigPath;
			m_fnOld.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_DIRECTORY, String());
			Enable(IDC_ORIG_PATH_BTN, false);
			Enable(IDC_ORIG_PATH_EDIT, false);
			Enable(IDC_ORIG_PATH_BTN, true);
			Enable(IDC_ORIG_PATH_EDIT, true);
			SetString(IDC_ORIG_PATH_EDIT, m_fnOld.GetString());
			GetString(IDC_ORIG_PATH_EDIT, g_pResEditPrefs->strOrigPath);
		}
		break;
	case IDC_NEW_PATH_BTN:
		{
			m_fnNew = g_pResEditPrefs->strNewPath;
			m_fnNew.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_DIRECTORY, String());
			Enable(IDC_NEW_PATH_BTN, true);
			Enable(IDC_NEW_PATH_EDIT, true);
			SetString(IDC_NEW_PATH_EDIT, m_fnNew.GetString());
			GetString(IDC_NEW_PATH_EDIT, g_pResEditPrefs->strNewPath);
		}
		break;
	case IDC_ORIG_PATH_EDIT:
	case IDC_NEW_PATH_EDIT:
		GetString(IDC_ORIG_PATH_EDIT, g_pResEditPrefs->strOrigPath); m_fnOld = g_pResEditPrefs->strOrigPath;
		GetString(IDC_NEW_PATH_EDIT, g_pResEditPrefs->strNewPath); m_fnNew = g_pResEditPrefs->strNewPath;
		break;
	case IDC_ST_TRANS_COMPARE_BTN:
		{
			GeShowMouse(MOUSE_BUSY);
			Bool b = CheckStringTables();
			GeShowMouse(MOUSE_NORMAL);
			if (!b)
				GeOutString(GeLoadString(IDS_ST_COMPARE_FAILED), GEMB_ICONEXCLAMATION);
		}
		break;
	case IDC_STRING_TRANS_COUNTRY_EDIT:
	case IDC_STRING_TRANS_COUNTRY_COMMAD_EDIT:
		m_bWasChanged = true;
		break;
	case IDC_ST_TRANS_DEL_UNUSED_BTN:
		DeleteUnusedStringTables();
		break;
	case IDC_OK:
		return false;
		break;
	case IDC_OK_BTN:
		SelectString(m_pSelectedString);
		Close();
		break;
	case IDC_ST_TRANS_SAVE_TABLE_BTN:
		SelectString(m_pSelectedString);
		AskModified(false);
		break;
	case IDC_TRANSLATE_LANG_ARROW:
		SelectString(m_pSelectedString);
		if (AskModified())
		{
			Int32 l;
			GetInt32(IDC_TRANSLATE_LANG_ARROW, l);
			m_lSelLanguage = l - IDC_TRANSLATE_FIRST_LANG;
			LoadStringTable(m_pSelectedItem);
			//m_pStringsTree->SetHeaderText(ST_ELEMENT_COUNTRY, GeLoadString(IDS_ST_STRING_COUNTRY) + "- " + g_LanguageList.GetLanguageName(m_lSelLanguage));
			m_pStringsTree->SetHeaderText(ST_ELEMENT_COUNTRY, g_LanguageList.GetLanguageSuffix(m_lSelLanguage) + "- " + g_LanguageList.GetLanguageName(m_lSelLanguage));
			m_pStringsTree->Refresh();
		}
		break;
	case IDC_BASE_LANG_ARROW:
		SelectString(m_pSelectedString);
		if (AskModified())
		{
			Int32 l;
			GetInt32(IDC_BASE_LANG_ARROW, l);
			l -= IDC_BASE_LANGUAGE;
			//m_pStringsTree->SetHeaderText(ST_ELEMENT_COUNTRY, GeLoadString(IDS_ST_STRING_COUNTRY) + "- " + g_LanguageList.GetLanguageName(m_lSelLanguage));
			m_pStringsTree->SetHeaderText(ST_ELEMENT_COUNTRY, g_LanguageList.GetLanguageSuffix(m_lSelLanguage) + "- " + g_LanguageList.GetLanguageName(m_lSelLanguage));
			m_strOrigLanguage = g_LanguageList.GetLanguageSuffix(l);
			m_strOrigLanguagePath = "strings_" + m_strOrigLanguage;
			LoadStringTable(m_pSelectedItem);
			m_pStringsTree->SetHeaderText(ST_ELEMENT_US_OLD, m_strOrigLanguage + GeLoadString(IDS_ST_STRING_US_OLD));
			m_pStringsTree->SetHeaderText(ST_ELEMENT_US_NEW, m_strOrigLanguage + GeLoadString(IDS_ST_STRING_US_NEW));
			m_pStringsTree->Refresh();
			m_pAvailStringTablesTree->Refresh();
		}
		break;
	case IDC_ST_TRANS_SEARCH_BTN:
		m_dlgSearch.Open(DLG_TYPE_ASYNC, 0);
		break;
	case IDC_TRANSLATE_LANG_GOTO_EMPTY_BTN:
		{
			CStringTableItem* pEmpty = m_pSelectedString ? (CStringTableItem*)m_pSelectedString->m_pNext : m_TableElements.First();
			CStringTableItem* pStartItem = pEmpty;
			Bool bSkipEmptyBase;
			GetBool(IDC_TRANSLATE_LANG_SKIP_EMPTY_BASE_CHK, bSkipEmptyBase);
			while (pEmpty)
			{
				if (bSkipEmptyBase)
				{
					if ((pEmpty->strUSNew.Content() && !pEmpty->strCountry.Content()) ||
							(pEmpty->strUSCommandNew.Content() && !pEmpty->strCountryCommand.Content()))
						break;
				}
				else
				{
					if ((pEmpty->strUSNew.Content()	^ pEmpty->strCountry.Content()) ||
							(pEmpty->strUSCommandNew.Content() ^ pEmpty->strCountryCommand.Content()))
						break;
				}
				pEmpty = (CStringTableItem*)pEmpty->m_pNext;
				if (!pEmpty)
					pEmpty = m_TableElements.First();
				if (pStartItem == pEmpty)
					break;
			}
			if (pEmpty && pEmpty != m_pSelectedString)
			{
				SelectString(pEmpty);
				m_pStringsTree->Refresh();
			}
			m_pStringsTree->MakeVisible(pEmpty);
		}
		break;
	case IDC_TRANSLATE_LANG_GOTO_EMPTY_ALL_BTN:
		{
			CStringTableItem* pEmpty = m_pSelectedString ? (CStringTableItem*)m_pSelectedString->m_pNext : m_TableElements.First();
			CAvailStringTableItem* pTable = m_pSelectedItem;
			Bool bSkipEmptyBase;
			GetBool(IDC_TRANSLATE_LANG_SKIP_EMPTY_BASE_CHK, bSkipEmptyBase);
			BaseContainer bcKey;
			while (pTable)
			{
				if (GetInputState(BFM_INPUT_KEYBOARD, KEY_ESC, bcKey) && bcKey.GetInt32(BFM_INPUT_VALUE) != 0)
					goto _search_escape;
				while (pEmpty)
				{
					if (bSkipEmptyBase)
					{
						if ((pEmpty->strUSNew.Content()	&& !pEmpty->strCountry.Content()) ||
								(pEmpty->strUSCommandNew.Content() && !pEmpty->strCountryCommand.Content()))
							goto _found_it;
					}
					else
					{
						if ((pEmpty->strUSNew.Content()	^ pEmpty->strCountry.Content()) ||
								(pEmpty->strUSCommandNew.Content() ^ pEmpty->strCountryCommand.Content()))
							goto _found_it;
					}
					pEmpty = (CStringTableItem*)pEmpty->m_pNext;
				}
				if (!pEmpty)
				{
					pTable = m_availStringTables.Next(pTable);
					//pEmpty = m_TableElements.First();
				}
				if (pTable != m_pSelectedItem)
				{
					m_pSelectedItem = pTable;
					if (pTable)
						LoadStringTable(pTable);
					pEmpty = m_TableElements.First();
					m_pAvailStringTablesTree->MakeVisible(pTable);
					m_pAvailStringTablesTree->Refresh();
				}
			}
_found_it:
			if (pEmpty && pEmpty != m_pSelectedString)
			{
				SelectString(pEmpty);
				m_pStringsTree->Refresh();
			}
			m_pStringsTree->MakeVisible(pEmpty);
_search_escape: ;
		}
		break;
	default:
		break;
	}
	return true;
}

Bool CStringTranslateDialog::InitValues()
{
	if (!GeDialog::InitValues()) return false;

	SetString(IDC_ORIG_PATH_EDIT, g_pResEditPrefs->strOrigPath);
	SetString(IDC_NEW_PATH_EDIT, g_pResEditPrefs->strNewPath);

	if (m_pSelectedString)
	{
		SetString(IDC_STRING_TRANS_COUNTRY_EDIT, m_pSelectedString->strCountry);
		SetString(IDC_STRING_TRANS_COUNTRY_COMMAD_EDIT, m_pSelectedString->strCountryCommand);
	}

	return true;
}

Int32 CStringTranslateDialog::Message(const BaseContainer &msg, BaseContainer &result)
{
	Int32 lID = msg.GetId();
	Int32 lDevice;
	switch (lID)
	{
		case BFM_INPUT:
			lDevice = msg.GetInt32(BFM_INPUT_DEVICE);
			if (lDevice == BFM_INPUT_KEYBOARD)
			{
				Int32 lValue = msg.GetInt32(BFM_INPUT_CHANNEL);
				lValue = lValue;
				CStringTableItem* pSelString = m_pSelectedString;
				if (m_pSelectedString)
				{
					if (lValue == 32773 || lValue == 61829 /*Bild ab*/)
					{
						pSelString = m_TableElements.Next(m_pSelectedString);
					}
					else if (lValue == 32772 || lValue == 61828 /*Bild auf*/)
					{
						pSelString = m_TableElements.Prev(m_pSelectedString);
					}
				}
				if (pSelString && pSelString != m_pSelectedString)
				{
					SelectString(pSelString);
					m_pStringsTree->MakeVisible(pSelString);
				}

				return true;
			}
			break;
	}
	return GeDialog::Message(msg, result);
}

Bool CStringTranslateDialog::AskClose()
{
	// return true, if the dialog should not be closed
	if (!AskModified())
		return true;
	m_dlgSearch.Close();

	return false;
}

Bool CStringTranslateDialog::CheckStringTables()
{
	String str;
	Bool bOK = false;

	if (!GetString(IDC_ORIG_PATH_EDIT, str))
		goto _error;
	m_fnOld = str;

	if (!GetString(IDC_NEW_PATH_EDIT, str))
		goto _error;
	m_fnNew = str;

	if (!m_fnNew.Content() || !m_fnOld.Content() || !GeFExist(m_fnOld, true) || !GeFExist(m_fnNew, true))
		goto _error;

	m_availStringTables.FreeList();
	m_TableElements.FreeList();
	m_pSelectedItem = nullptr;
	m_pSelectedString = nullptr;

	if (!BrowseStringTables(m_fnOld, Filename(""), true))
		goto _error;

	if (!BrowseStringTables(m_fnNew, Filename(""), false))
		goto _error;

	CheckModified();

	m_pAvailStringTablesTree->Refresh();
	m_pStringsTree->Refresh();

	bOK = true;
_error: ;
	return bOK;
}

Bool CStringTranslateDialog::BrowseStringTables(const Filename &fnAbs, const Filename &fnRel, Bool bOld)
{
	AutoAlloc <BrowseFiles> browse;

	if (!browse)
		return false;

	Filename fn, fnMod;

	browse->Init(fnAbs, false);

	String strName;
	while (browse->GetNext())
	{
		fn = browse->GetFilename();
		if (browse->IsDir())
		{
			Int32 lPos;
			String str = fn.GetString();
			if (str.FindFirst("strings_", &lPos))
			{
				if (str.FindFirst(m_strOrigLanguagePath, &lPos))
				{
					if (!BrowseStringTables(fnAbs + fn, fnRel + fn, bOld))
						return false;
				}
			}
			else
			{
				if (!BrowseStringTables(fnAbs + fn, fnRel + fn, bOld))
					return false;
			}
		}
		else
		{
			if (fn.CheckSuffix("str"))
			{
				fnMod = ModifyPath(fnRel);

				strName = fn.GetString();

				Bool bExists = false;
				CAvailStringTableItem* pFirst = m_availStringTables.First();
				CAvailStringTableItem* pItem=nullptr;
				while (pFirst && !bExists)
				{
					if (pFirst->fnRelPath.GetString().LexCompare(fnMod.GetString()) == 0 && pFirst->strName.LexCompare(strName) == 0)
					{
						bExists = true;
						pItem = pFirst;
					}
					pFirst = m_availStringTables.Next(pFirst);
				}

				// String table gefunden
				if (!bExists)
				{
					pItem = NewObjClear(CAvailStringTableItem);
					if (!pItem)
						return false;
					pItem->strName = fn.GetString();
					pItem->fnAbsPath = fnAbs + fn;
					pItem->fnRelPath = fnMod;
					m_availStringTables.Append(pItem);
				}
				if (bOld)
				{
					pItem->bIsOld = true;
					pItem->fnOldAbs = ModifyPath(fnAbs + fn, true);
				}
				else
				{
					pItem->bIsNew = true;
					pItem->fnNewAbs = ModifyPath(fnAbs + fn, true);
				}
			}
		}
	}

	return true;
}

Filename CStringTranslateDialog::ModifyPath(Filename fn, Bool bIsFilename)
{
	String str;
	Int32 lPos, lStart = 0;

	Filename fnDir = fn.GetDirectory();
	Filename fnFile = fn.GetFile();

	str = bIsFilename ? fnDir.GetString() : fn.GetString();

	while (str.FindFirst("strings_", &lPos, lStart))
	{
		str = str.SubStr(0, lPos) + "strings_??" + str.SubStr(lPos + 10, str.GetLength() - lPos - 10);
		lStart = lPos + 10;
	}
	fn = str;
	return bIsFilename ? fn + fnFile : fn;
}

Filename CStringTranslateDialog::ModifyPathBack(Filename fn, String strMod)
{
	String str;
	Int32 lPos, lStart = 0;

	Filename fnDir = fn.GetDirectory();
	Filename fnFile = fn.GetFile();
	str = fnDir.GetString();

	while (str.FindFirst("strings_??", &lPos, lStart))
	{
		str = str.SubStr(0, lPos) + "strings_" + strMod + str.SubStr(lPos + 10, str.GetLength() - lPos - 10);
		lStart = lPos + 10;
	}
	fn = str;
	return fn + fnFile;
}

void CStringTranslateDialog::CheckModified()
{
	CAvailStringTableItem* pItem = m_availStringTables.First();
	Int32 a;
	Bool bLoop = false;
	SYMBOL sym;
	Filename fnOld, fnNew;
	//Int32 lReadOld, lReadNew;
	//UChar pbBufferOld[1024], pbBufferNew[1024];
	Bool bModified;

	while (pItem)
	{
		// try to load the US version
		Filename fn;

		pItem->type = typeUnknown;

		fn = ModifyPathBack(pItem->fnOldAbs, m_strOrigLanguage);
		bLoop = false;
		fnOld = fnNew = String("");
		for (a = 0; a < 2; a++)
		{
			if ((a == 0 && pItem->bIsOld) || (a == 1 && pItem->bIsNew))
			{
				if (a == 0)
					fnOld = fn;
				else if (a == 1)
					fnNew = fn;

				// first original version
				ASCIIParser *p = NewObjClear(ASCIIParser);
				if (p && !p->Init(fn, false, false))
				{
					sym = p->GetSymbol();
					while (!p->Error())
					{
						if (sym==SYM_OPERATOR_GESCHWEIFTZU) break;
						//if (sym != SYM_IDENT)

						if (p->CheckIdent("DIALOGSTRINGS"))
							pItem->type = (!bLoop || pItem->type == typeDialogStrings || !pItem->bIsOld) ? typeDialogStrings : typeUnknown;
						else if (p->CheckIdent("STRINGTABLE"))
							pItem->type = (!bLoop || pItem->type == typeStringTable || !pItem->bIsOld) ? typeStringTable : typeUnknown;
						else if (p->CheckIdent("COMMANDSTRINGS"))
							pItem->type = (!bLoop || pItem->type == typeCommandStrings || !pItem->bIsOld) ? typeCommandStrings : typeUnknown;
						break;
					}
				}
				else
				{
					pItem = pItem;
				}
				p->Close();
				DeleteObj(p);
			}
			fn = ModifyPathBack(pItem->fnNewAbs, m_strOrigLanguage);
			bLoop = true;
		}

		if (pItem->bIsNew && pItem->bIsOld)
		{
			// gibts den String in den US tables aber nicht in einer der Landessprachen?
			for (a = 0; a < g_LanguageList.GetNumLanguages(); a++)
			{
				fn = ModifyPathBack(pItem->fnNewAbs, g_LanguageList.GetLanguageSuffix(a));
				if (!GeFExist(fn))
				{
					pItem->strMissing += g_LanguageList.GetLanguageSuffix(a) + "  ";
				}
			}
		}

		bModified = false;
		/*if (fnOld.Content() && fnNew.Content())
		{
			AutoAlloc <BaseFile> pOldFile;
			AutoAlloc <BaseFile> pNewFile;

			if (pOldFile && pNewFile)
			{
				if (pOldFile->Open(fnOld, FILEOPEN_READ, FILEDIALOG_NONE) && pNewFile->Open(fnNew, FILEOPEN_READ, FILEDIALOG_NONE))
				{
					if (pOldFile->GetLength() != pNewFile->GetLength())
						bModified = true;
					while (!bModified)
					{
						lReadOld = pOldFile->TryReadBytes(pbBufferOld, 1024);
						lReadNew = pNewFile->TryReadBytes(pbBufferNew, 1024);

						if (lReadOld != lReadNew)
							bModified = true;
						else
						{
							if (lReadOld <= 0)
								break;

							if (memcmp(pbBufferOld, pbBufferNew, lReadOld))
								bModified = true;
						}
					}
				}
				pOldFile->Close();
				pNewFile->Close();
			}
		}*/
		CompareStringTables(fnOld, fnNew, pItem->bModified, pItem->bHasErrors);

		if (pItem->type == typeDialogStrings)
			pItem->strType = GeLoadString(IDS_TABLE_TYPE_DIALOG);
		else if (pItem->type == typeStringTable)
			pItem->strType = GeLoadString(IDS_TABLE_TYPE_STRINGTAB);
		else if (pItem->type == typeCommandStrings)
			pItem->strType = GeLoadString(IDS_TABLE_TYPE_COMMAND);
		else
			pItem->strType = GeLoadString(IDS_TABLE_TYPE_UNKNOWN);

		pItem = m_availStringTables.Next(pItem);
	}
}

Bool CStringTranslateDialog::AskModified(Bool bAsk)
{
	if (!m_bWasChanged || !m_pSelectedItem)
		return true;

	if (bAsk)
	{
		GEMB_R lRet = GeOutString(m_pSelectedItem->strName + GeLoadString(IDS_WAS_CHANGED), GEMB_YESNOCANCEL | GEMB_ICONQUESTION);
		if (lRet == GEMB_R_CANCEL)
			return false;
		else if (lRet == GEMB_R_NO)
			return true;
	}

	if (!SaveStringTable(true))
		return false;

	return true;
}

Bool CStringTranslateDialog::LoadStringTable(CAvailStringTableItem* pTable)
{
	m_pSelectedItem = pTable;
	if (!pTable)
		return true;

	m_TableElements.FreeList();

	CDynamicStringArray arErrors;

	// load new US string table
	CStringSet strSet;
	Filename fn = ModifyPathBack(pTable->fnNewAbs, m_strOrigLanguage);
	if (fn.GetString().GetLength() > 0)
		LoadStringTable(fn, 0, &strSet, &arErrors);
	//else
		//GeOutString((pTable->fnRelPath + pTable->strName).GetString() + "(us)" + GeLoadString(IDS_ST_LOAD_NOT_EXIST_NEW), GEMB_OK | GEMB_ICONEXCLAMATION);

	// load old US string
	fn = ModifyPathBack(pTable->fnOldAbs, m_strOrigLanguage);
	if (fn.GetString().GetLength() > 0)
		LoadStringTable(fn, 1, &strSet, &arErrors);
	//else
		//GeOutString((pTable->fnRelPath + pTable->strName).GetString() + "(us)" + GeLoadString(IDS_ST_LOAD_NOT_EXIST_OLD), GEMB_OK | GEMB_ICONEXCLAMATION);

	// load new country strings
	fn = ModifyPathBack(pTable->fnNewAbs, g_LanguageList.GetLanguageSuffix(m_lSelLanguage));
	if (fn.GetString().GetLength() > 0)
		LoadStringTable(fn, 2, &strSet, &arErrors);
	//else
		//GeOutString((pTable->fnRelPath + pTable->strName).GetString() + "(" + g_LanguageList.GetLanguageSuffix(m_lSelLanguage) + ")" + GeLoadString(IDS_ST_LOAD_NOT_EXIST_NEW), GEMB_OK | GEMB_ICONEXCLAMATION);

	// change the array to a list
	tagTreeViewStringElement* pTreeSort = strSet.GetArray();

	Int32 lElements = strSet.GetElementCount();
	Int32 a;
	CShellSort <tagTreeViewStringElement> s;
	s.Sort(pTreeSort, CStringTranslateDialog::CompareStringTableItem, lElements);

	for (a = 0; a < lElements; a++)
	{
		CStringTableItem* pElement = pTreeSort[a].pItem;
		m_TableElements.Append(pElement);
		pElement->lIcon = -1;
		if (!pElement->bIsOld && pElement->bIsNew)
			pElement->lIcon = 1;
		else if (pElement->bIsOld && !pElement->bIsNew)
			pElement->lIcon = 2;
		else
		{
			if (pElement->strUSOld != pElement->strUSNew || (m_pSelectedItem->type == typeCommandStrings && (pElement->strUSCommandOld != pElement->strUSCommandNew)))
				pElement->lIcon = 0;
		}
	}

	pTable->bHasErrors = arErrors.GetElementCount() > 0;
	for (Int32 a = 0; a < arErrors.GetElementCount(); a++)
		GePrint(*(arErrors[a]));

	m_pSelectedString = nullptr;
	m_bWasChanged = false;
	m_lLastStringSortCol = -1;
	m_lLastStringSortDir = 0;
	SetString(IDC_STRING_TRANS_COUNTRY_EDIT, "");
	SetString(IDC_STRING_TRANS_COUNTRY_COMMAD_EDIT, "");

	m_pStringsTree->Refresh();

	return true;
}

Bool CStringTranslateDialog::SaveStringTable(Bool bOutputSaveLog)
{
	Bool bRet = false;
	AutoAlloc <BaseFile> pFile;
	if (!pFile)
		return false;

	Filename fn = ModifyPathBack(m_pSelectedItem->fnNewAbs, g_LanguageList.GetLanguageSuffix(m_lSelLanguage));
	if (!AssertPathExists(fn.GetDirectory()))
		return false;

	if (!ForceOpenFileWrite(pFile, fn))
		return false;

	// sort by the string IDs
	String strMissingTranslatedString = GeLoadString(IDS_ST_MISSING_TRANSLATED_STRING);
	String strMissingTranslatedCommand = GeLoadString(IDS_ST_MISSING_TRANSLATED_COMMAND);
	const Char* pChIdent=nullptr;
	CDoubleLinkedList <CStringTableItem>* pTables = GetTableElements();
	Int32 lElements = pTables->Entries();
	tagTreeViewStringElement* pItems = bNewDeprecatedUseArraysInstead<tagTreeViewStringElement>(lElements);
	Int32 a = 0, b;
	Int32 lMaxLen;
	Int32 lCount = 0, lSkipped = 0;
#define MAX_LOG_LENGTH 20

	if (bOutputSaveLog)
		GePrint(GeLoadString(IDS_ST_SAVING_TABLE, fn.GetString()));

	if (pItems)
	{
		CStringTableItem* pItem = pTables->First();
		a = 0;
		while (pItem)
		{
			pItems[a].pItem = pItem;
			pItems[a].lIndex = a;
			a++;
			pItem = pTables->Next(pItem);
		}

		CShellSort <tagTreeViewStringElement> s;
		s.Sort(pItems, CStringTranslateDialog::CompareStringsByIndex, lElements);

		pTables->UnlinkList();
		for (a = 0; a < lElements; a++)
			pTables->Append(pItems[a].pItem);
	}

	if (m_pSelectedItem->type == typeDialogStrings) pChIdent = "DIALOGSTRINGS";
	else if (m_pSelectedItem->type == typeStringTable) pChIdent = "STRINGTABLE";
	else if (m_pSelectedItem->type == typeCommandStrings) pChIdent = "COMMANDSTRINGS";

	if (!WriteString(pFile, pChIdent))
		goto _error;
	if (!WriteString(pFile, " "))
		goto _error;
	if (!WriteString(pFile, m_strStringTableName))
		goto _error;
	if (!LineBreak(pFile, ""))
		goto _error;
	if (!WriteString(pFile, "{"))
		goto _error;

	lMaxLen = 0;
	for (a = 0; a < lElements; a++)
	{
		lMaxLen = LMax(lMaxLen, pItems[a].pItem->strID.GetLength());
	}
	lMaxLen += 3;

	for (a = 0; a < lElements; a++)
	{
		// skip deleted
		if (!pItems[a].pItem->bIsNew)
			continue;

		if (!LineBreak(pFile, "  "))
			goto _error;

		if (!WriteString(pFile, pItems[a].pItem->strID))
			goto _error;

		b = lMaxLen - pItems[a].pItem->strID.GetLength();
		while (--b > 0)
		{
			if (!pFile->WriteChar(' '))
				goto _error;
		}

		if (!pFile->WriteChar('\"'))
			goto _error;
		if (!WriteString(pFile, pItems[a].pItem->strCountry, true))
			goto _error;
		if (!pFile->WriteChar('\"'))
			goto _error;

		if (/*m_pSelectedItem->type == typeCommandStrings*/pItems[a].pItem->strCountryCommand.Content())
		{
			if (!WriteString(pFile, "   "))
				goto _error;
			if (!pFile->WriteChar('\"'))
				goto _error;
			if (!WriteString(pFile, pItems[a].pItem->strCountryCommand, true))
				goto _error;
			if (!pFile->WriteChar('\"'))
				goto _error;
		}
		if (!pFile->WriteChar(';'))
			goto _error;

		if (bOutputSaveLog)
		{
			if (!pItems[a].pItem->strCountry.Content())
			{
				lCount++;
				if (lCount < MAX_LOG_LENGTH)
					GePrint(strMissingTranslatedString + pItems[a].pItem->strID);
				else
					lSkipped++;
			}
			if (m_pSelectedItem->type == typeCommandStrings && pItems[a].pItem->strUSCommandNew.Content() && !pItems[a].pItem->strCountryCommand.Content())
			{
				lCount++;
				if (lCount < MAX_LOG_LENGTH)
					GePrint(strMissingTranslatedCommand + pItems[a].pItem->strID);
				else
					lSkipped++;
			}
		}
	}

	if (!LineBreak(pFile, ""))
		goto _error;
	if (!WriteString(pFile, "}"))
		goto _error;

	if (bOutputSaveLog && lSkipped++)
		GePrint(GeLoadString(IDS_ST_SAVE_TABLE_SKIPPED, String::IntToString(lSkipped)));

	//m_pSelectedItem->fnNewAbs;
	m_bWasChanged = false;
	bRet = true;
_error:
	bDelete(pItems);
	pFile->Close();
	return bRet;
}

Bool CStringTranslateDialog::LoadStringTable(Filename fn, Int32 l, CStringSet* pStringSet, CDynamicStringArray* pErrors)
{
	const char* pChIdent = nullptr;
	if (m_pSelectedItem)
	{
		if (m_pSelectedItem->type == typeDialogStrings) pChIdent = "DIALOGSTRINGS";
		else if (m_pSelectedItem->type == typeStringTable) pChIdent = "STRINGTABLE";
		else if (m_pSelectedItem->type == typeCommandStrings) pChIdent = "COMMANDSTRINGS";
	}
	else
	{
		// sonst laden wir die String table nur zum Testen, ob string tables identisch sind
		pChIdent = nullptr;
	}

	ASCIIParser p;
	SYMBOL sym;
	String strID, strString, strCommand;

	if (p.Init(fn, false, false))
	{
	 // GeOutString(fn.GetString() + GeLoadString(IDS_ST_LOAD_ERROR), GEMB_OK | GEMB_ICONEXCLAMATION);
		//return false;
	}

	sym = p.GetSymbol();
	Int32 lCount=0;

	CStringTableItem tempItem;
	tagTreeViewStringElement tmp;

	if (l == 0)
		lCount = 0;
	else if (l == 1)
		lCount = pStringSet->GetElementCount();

	if (!p.Error())
	{
		if (sym != SYM_IDENT)
			return false;

		if (!pChIdent)
		{
			String str = p.GetIdent();
			if (str == "DIALOGSTRINGS")
				pChIdent = "DIALOGSTRINGS";
			else if (str == "STRINGTABLE")
				pChIdent = "STRINGTABLE";
			else if (str == "COMMANDSTRINGS")
				pChIdent = "COMMANDSTRINGS";
			else
				return false;
		}
		if (p.CheckIdent(pChIdent))
		{
			sym = p.nextsym();
			if (sym == SYM_IDENT)
			{
				m_strStringTableName = p.GetIdent();
				sym = p.nextsym();
			}
			else
				m_strStringTableName = "";

			if (sym != SYM_OPERATOR_GESCHWEIFTAUF)
				return false;
			sym = p.nextsym();

			while (!p.Error())
			{
				strString = "";
				strCommand = "";
				if (sym == SYM_OPERATOR_GESCHWEIFTZU)
					break;

				if (sym != SYM_IDENT)
					return false;
				strID = p.GetIdent();
				sym = p.nextsym();

				if (sym != SYM_STRING)
					return false;
				strString = p.GetIdent();
				sym = p.nextsym();

				if (sym == SYM_STRING)
				{
					strCommand = p.GetIdent();
					sym = p.nextsym();
				}
				/*if (m_pSelectedItem->type == typeCommandStrings)
				{
					if (sym != SYM_STRING)
						return false;
					strCommand = p.GetIdent();
					sym = p.nextsym();
				}*/

				if (sym != SYM_OPERATOR_SEMIKOLON)
					return false;
				sym = p.nextsym();

				if (l == 0)
				{
					CStringTableItem* pNewItem = NewObjClear(CStringTableItem);
					if (!pNewItem)
						return false;
					pNewItem->strID = strID;
					pNewItem->strUSNew = strString;
					pNewItem->strUSCommandNew = strCommand;
					pNewItem->bIsNew = true;
					pNewItem->bIsOld = false;

					tmp.pItem = pNewItem;
					pNewItem->lIndex = tmp.lIndex = lCount++;

					Bool bExisted;
					tagTreeViewStringElement* pNew = pStringSet->AddElement(&tmp, bExisted);
					if (bExisted)
					{
						if (pErrors)
							pErrors->AddString(fn.GetString() + ": ID exists twice: " + strID + "(" + pNew->pItem->strUSNew + " / " + pNewItem->strUSNew + ")");
					}
					if (bExisted || !pNew)
						DeleteObj(pNewItem);
					else
						*pNew = tmp;
				}
				else if (l == 1)
				{
					tmp.pItem = &tempItem;
					tempItem.strID = strID;
					tagTreeViewStringElement* pStringElement = pStringSet->ElementExists(&tmp);
					if (pStringElement)
					{
						pStringElement->pItem->strUSOld = strString;
						pStringElement->pItem->strUSCommandOld = strCommand;
						pStringElement->pItem->bIsOld = true;
					}
					else
					{
						CStringTableItem* pNewItem = NewObjClear(CStringTableItem);
						if (!pNewItem)
							return false;
						pNewItem->strID = strID;
						pNewItem->strUSOld = strString;
						pNewItem->strUSCommandOld = strCommand;
						pNewItem->bIsNew = false;
						pNewItem->bIsOld = true;

						tmp.pItem = pNewItem;
						pNewItem->lIndex = tmp.lIndex = lCount++;

						pStringSet->AddElement(tmp);
					}
				}
				else if (l == 2)
				{
					tmp.pItem = &tempItem;
					tempItem.strID = strID;
					tagTreeViewStringElement* pStringElement = pStringSet->ElementExists(&tmp);
					if (pStringElement)
					{
						if (pErrors && pStringElement->pItem->strCountry.Content())
							pErrors->AddString(fn.GetString() + ": ID exists twice: " + strID + "(" + pStringElement->pItem->strCountry + " / " + strString + ")");
						pStringElement->pItem->strCountry = strString;
						pStringElement->pItem->strCountryCommand = strCommand;
					}
				}
			}
		}
		else
			return false;
	}

	return true;
}

void CStringTranslateDialog::SelectString(CStringTableItem* pItem)
{
	if (m_pSelectedString != nullptr)
	{
		GetString(IDC_STRING_TRANS_COUNTRY_EDIT, m_pSelectedString->strCountry);
		GetString(IDC_STRING_TRANS_COUNTRY_COMMAD_EDIT, m_pSelectedString->strCountryCommand);
	}
	m_pSelectedString = pItem;
	if (m_pSelectedString != nullptr)
	{
		SetString(IDC_STRING_TRANS_COUNTRY_EDIT, m_pSelectedString->strCountry);
		SetString(IDC_STRING_TRANS_COUNTRY_COMMAD_EDIT, m_pSelectedString->strCountryCommand);
		SetString(IDC_ST_TRANS_US_NEW_STATIC, m_pSelectedString->strUSNew);
		SetString(IDC_ST_TRANS_US_CMD_STATIC, m_pSelectedString->strUSCommandNew);
	}

	m_pAvailStringTablesTree->Refresh();
	m_pStringsTree->GetTreeViewArea()->Redraw();
	m_pStringsTree->GetHeaderArea()->Redraw();
}

void CStringTranslateDialog::CompareStringTables(Filename &fnOld, Filename &fnNew, Bool &bModified, Bool &bHasErrors)
{
	bHasErrors = false;
	bModified = false;
	if (!fnOld.Content() || !fnNew.Content())
		return;

	CDynamicStringArray arErrors;

	CStringSet oldSet, newSet;
	if (LoadStringTable(fnOld, 0, &oldSet, &arErrors) && LoadStringTable(fnNew, 0, &newSet, &arErrors))
	{
		bModified = true;

		Int32 lOldCount = oldSet.GetElementCount();
		Int32 lNewCount = newSet.GetElementCount();
		if (lOldCount != lNewCount)
			goto _exit;
		for (Int32 a = 0; a < lOldCount; a++)
		{
			tagTreeViewStringElement* pOld = oldSet.GetElement(a);
			tagTreeViewStringElement* pNew = newSet.GetElement(a);
			if (!pOld || !pNew)
				goto _exit;
			if (!pOld->pItem || !pNew->pItem)
				goto _exit;
			if (pOld->pItem->strID != pNew->pItem->strID)
				goto _exit;
			if (pOld->pItem->strUSNew != pNew->pItem->strUSNew)
				goto _exit;
			if (pOld->pItem->strUSCommandNew != pNew->pItem->strUSCommandNew)
				goto _exit;
		}
	}

	bModified = false;
_exit:
	for (Int32 a = 0; a < arErrors.GetElementCount(); a++)
		GePrint(*(arErrors[a]));
	bHasErrors = arErrors.GetElementCount() > 0;

	oldSet.FreeItems();
	newSet.FreeItems();
}

void CStringTranslateDialog::DeleteUnusedStringTables()
{
	if (GeOutString(GeLoadString(IDS_DELETE_ALL_UNUSED), GEMB_ICONQUESTION | GEMB_YESNO) != GEMB_R_YES)
		return;

	CAvailStringTableItem* pTableItem = m_availStringTables.First();
	Int32 lLanguages = g_LanguageList.GetNumLanguages();
	Int32 a;
	while (pTableItem)
	{
		if (pTableItem->bIsOld && !pTableItem->bIsNew)
		{
			// wurde gelöscht
			for (a = 0; a < lLanguages; a++)
			{
				String strLang = g_LanguageList.GetLanguageSuffix(a);
				if (strLang == m_strOrigLanguage)
					continue;

				Filename fn = g_pResEditPrefs->strNewPath;
				Filename fnRelDir = pTableItem->fnRelPath + pTableItem->strName;
				fnRelDir = ModifyPathBack(fnRelDir, strLang);
				AddFilename(fn, fnRelDir);
				if (!GeFExist(fn))
					goto _next_delete;
				//Char *cstr(Filename &fn);
				//GeDebugOut("deleting string table %s", cstr(fn));
				GeFKill(fn);
			}
		}
_next_delete:
		pTableItem = m_availStringTables.Next(pTableItem);
	}
}

void CStringTranslateDialog::FindString(const String &str, RegularExprParser* pParser, Bool bOnlyCurrent, Bool bCaseSensitive)
{
	CAvailStringTableItem* pItem = m_pSelectedItem;
	if (!pItem)
		return;

	// search elements  of the selected item
	Filename fnTable;
	Int32 lPos;
	String strSearch = bCaseSensitive ? str : str.ToLower();
	String strCurrent;
	BaseContainer bcKey;
	while (pItem)
	{
		if (GetInputState(BFM_INPUT_KEYBOARD, KEY_ESC, bcKey) && bcKey.GetInt32(BFM_INPUT_VALUE) != 0)
			goto _exit;

		CStringTableItem* pCurrent = nullptr;
		if (m_pSelectedString)
		{
			pCurrent = m_TableElements.Next(m_pSelectedString);
			if (!pCurrent)
				pCurrent = m_TableElements.First();
		}
		else
			pCurrent = m_TableElements.First();
		CStringTableItem* pStartElement = pCurrent;

		while (pCurrent)
		{
			if (pParser)
			{
				String strPattern;
				if (pParser->FindFirst(pCurrent->strCountry, RegParseContains, true, lPos, strPattern))
				{
					SelectString(pCurrent);
					m_pStringsTree->MakeVisible(pCurrent);
					m_pAvailStringTablesTree->MakeVisible(pItem);
					goto _exit;
				}
			}
			else
			{
				String* ptrlist[] = { &pCurrent->strUSOld, &pCurrent->strUSCommandOld,
														&pCurrent->strUSNew, &pCurrent->strUSCommandNew,
														&pCurrent->strCountry, &pCurrent->strCountryCommand };
				for (Int32 l = 0; l < sizeof(ptrlist) / sizeof(ptrlist[0]); l++)
				{
					strCurrent = bCaseSensitive ? *(ptrlist[l]) : ptrlist[l]->ToLower();
					if (strCurrent.FindFirst(strSearch, &lPos))
					{
						SelectString(pCurrent);
						m_pStringsTree->MakeVisible(pCurrent);
						m_pAvailStringTablesTree->MakeVisible(pItem);
						goto _exit;
					}
				}
			}
			pCurrent = m_TableElements.Next(pCurrent);
			if (bOnlyCurrent)
			{
				// continue searching in other tables
				if (!pCurrent)
					pCurrent = m_TableElements.First();
				if (pCurrent == pStartElement)
					pCurrent = nullptr;
			}
		}

		if (bOnlyCurrent)
		{
			GeOutString(GeLoadString(IDS_NO_MORE_MATCHES_TABLE), GEMB_ICONASTERISK);
			goto _exit;
		}

		CAvailStringTableItem* pNext = m_availStringTables.Next(pItem);
		if (pNext)
		{
			if (AskModified())
			{
				LoadStringTable(pNext);
			}
			else
				goto _exit;
		}
		else
		{
			GeOutString(GeLoadString(IDS_NO_MORE_MATCHES_GLOBAL), GEMB_ICONASTERISK);
			goto _exit;
		}
		pItem = pNext;
	}
_exit: ;
}
