// Thomas Kunert
// 21.11.2000

#ifndef _LIST_H_
#define _LIST_H_

class CListItem;
template <class ListItemType> class CDoubleLinkedList;

typedef void (*LIST_DELETE_CALLBACK)(CListItem* pItem);

class CListItem
{
public :
	CListItem() {}
	virtual ~CListItem() {} // make it virtual that we destroy derived classes, too

	CListItem *m_pPrev, *m_pNext;

	//friend  class CDoubleLinkedList;

//protected:
	//CListItem *m_pPrev, *m_pNext;
};

template <class ListItemType> class CDoubleLinkedList
{
public:
	CDoubleLinkedList();
	~CDoubleLinkedList();

	void AddAtHead(ListItemType* pNew) { AddAfter(&m_Head, pNew); }
	void Append(ListItemType* pNew) { AddAtTail(pNew); }
	void AddAtTail(ListItemType* pNew) { AddBefore(&m_Tail, pNew); }
	void AddAfter(ListItemType* pItem, ListItemType* pNew);
	void AddBefore(ListItemType* pItem, ListItemType* pNew);

	Bool UnlinkItem(ListItemType* pItem);
	void FreeItem(ListItemType* pItem, LIST_DELETE_CALLBACK FreeFunc = nullptr);
	void UnlinkList();
	void FreeList(LIST_DELETE_CALLBACK FreeFunc = nullptr);
	void FromArray(ListItemType** pArray, Int32 lSize);

	inline ListItemType* First() { return (m_Head.m_pNext != &m_Tail) ? (ListItemType*)m_Head.m_pNext : nullptr; }
	inline ListItemType* Last() { return (m_Tail.m_pPrev != &m_Head) ? (ListItemType*)m_Tail.m_pPrev : nullptr; }
	inline ListItemType* Next(ListItemType* pItem) { return (pItem->m_pNext->m_pNext != nullptr) ? (ListItemType*)pItem->m_pNext : nullptr; }
	inline ListItemType* Prev(ListItemType* pItem) { return (pItem->m_pPrev->m_pPrev != nullptr) ? (ListItemType*)pItem->m_pPrev : nullptr; }
	ListItemType* GetItem(Int32 nItem);

	inline Int32 Entries() { return(m_nNrOfEntries); }

protected:
	ListItemType m_Head, m_Tail;
	Int32 m_nNrOfEntries;
};



template<class T> CDoubleLinkedList<T>::CDoubleLinkedList()
{
	m_Head.m_pPrev = nullptr;
	m_Head.m_pNext = &m_Tail;
	m_Tail.m_pPrev = &m_Head;
	m_Tail.m_pNext = nullptr;
	m_nNrOfEntries  = 0;
}

template<class T> CDoubleLinkedList<T>::~CDoubleLinkedList()
{
	UnlinkList();
}

template<class T> void CDoubleLinkedList<T>::UnlinkList()
{
	while (m_Head.m_pNext != &m_Tail) {
		UnlinkItem((T*)m_Head.m_pNext);
	}
}

template<class T> void CDoubleLinkedList<T>::FreeList(LIST_DELETE_CALLBACK FreeFunc)
{
	while (m_Head.m_pNext != &m_Tail) {
		FreeItem((T*)m_Head.m_pNext, FreeFunc);
	}
}

template <class T> void CDoubleLinkedList<T>::AddAfter(T* pItem, T* pNew)
{
	if (pItem != nullptr) {
		if (pNew != nullptr) {
			if (pItem != &m_Tail) {
				pNew->m_pNext = pItem->m_pNext;
				pItem->m_pNext = pNew;
				pNew->m_pPrev = pItem;
				pItem->m_pNext->m_pPrev = pItem;
				m_nNrOfEntries++;
			}
		}
	}
	else AddAtTail(pNew);
}

template<class T> void CDoubleLinkedList<T>::AddBefore(T* pItem, T* pNew)
{
	if (pItem != nullptr) {
		if (pNew != nullptr) {
			if (pItem != &m_Head) {
				pNew->m_pPrev = pItem->m_pPrev;
				pItem->m_pPrev = pNew;
				pNew->m_pNext = pItem;
				pNew->m_pPrev->m_pNext = pNew;
				m_nNrOfEntries++;
			}
		}
	}
	else AddAtHead(pNew);
}

template<class T> Bool CDoubleLinkedList<T>::UnlinkItem(T* pItem)
{
	if (pItem != nullptr) {
		if (pItem != &m_Head) {
			if (pItem != &m_Tail) {
				pItem->m_pPrev->m_pNext = pItem->m_pNext;
				pItem->m_pNext->m_pPrev = pItem->m_pPrev;
				m_nNrOfEntries--;
				return(true);
			}
		}
	}
	return(false);
}

// Removes the item pItem from the list and frees its memory
template<class T> void CDoubleLinkedList<T>::FreeItem(T* pItem, LIST_DELETE_CALLBACK FreeFunc)
{
	if (UnlinkItem(pItem)) {
		if (FreeFunc)
			FreeFunc(pItem);
		else
			DeleteObj( pItem );
	}
}

// Description      : returns the item #nItem
template<class T> T* CDoubleLinkedList<T>::GetItem(Int32 nItem)
{
	T* pRet = nullptr;
	Int32 a = 0;

	T* pNext = First();
	while (pNext) {
		if (a == nItem) pRet = pNext;
		pNext = Next(pNext);
		a++;
	}
	return pRet;
}

template<class T> void CDoubleLinkedList<T>::FromArray(T** pArray, Int32 lSize)
{
	Int32 a;
	UnlinkList();
	for (a = 0; a < lSize; a++)
	{
		Append(pArray[a]);
	}
}
#endif // _LIST_H_

