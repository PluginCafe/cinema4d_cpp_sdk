#ifndef _DYN_ARRAY_H_
#define _DYN_ARRAY_H_

#include <string.h>
#include "dynamicdata.h"

#ifdef __API_INTERN__
	#include "ge_math.h"
	#include "ge_memory.h"

	#ifndef DebugAssert
		#include "ge_tools.h"
	#else
		#include "c4d_memory.h"
	#endif

	class DeprecatedMemoryPool
	{
	public:
		void MemFreeS(void* mem)
		{
			_GePoolFreeS((Hndl)this, &mem);
		}
		void* MemAllocS(Int size, Bool clear = true)
		{
			return GePoolAllocS((Hndl)this, size, clear);
		}
		void* MemReAllocS(void* old, Int size, Bool clear = true)
		{
			return GePoolReAllocS((Hndl)this, old, size, clear);
		}
		void MemFree(void* mem, Int size)
		{
			_GePoolFree((Hndl)this, &mem, size);
		}
		void* MemAlloc(Int size, Bool clear = true)
		{
			return GePoolAlloc((Hndl)this, size, clear);
		}
	};

	class Filename;
	class BaseFile;
#endif

#include "shellsort.h"

Bool CDynamicArrayWriteHeader(HyperFile* pFile, Int32 lElements, Int32 lLevel);
Bool CDynamicArrayReadHeader(HyperFile* pFile, Int32& lElements, Int32& lLevel);
Bool CDynamicArrayWriteHeader(BaseFile* pFile, Int32 lElements, Int32 lLevel);
Bool CDynamicArrayReadHeader(BaseFile* pFile, Int32& lElements, Int32& lLevel);

template <class ArrayElement> class CDynamicArray
{
public:
	CDynamicArray();
	CDynamicArray(ArrayElement *pDefaultElement);
	CDynamicArray(DeprecatedMemoryPool* pPool);
	CDynamicArray(DeprecatedMemoryPool* pPool, ArrayElement *pDefaultElement);
	virtual ~CDynamicArray();

	Bool Alloc(Int32 lExtraElements, Int32 lNewElementCount = -1, Bool bClear = true);
	Bool SetMinSize(Int32 lElements);
	Bool SetMinSizeNoCopy(Int32 lElements, Bool bClear = true);
	Bool Init(const ArrayElement* pArray, Int32 lCount); // copies lCount elements of pArray into this
	Bool Init(const CDynamicArray<ArrayElement> &ar); // copies ar into this
	Bool InitNoCopy(ArrayElement* pArray, Int32 lCount);
	ArrayElement* GetNext(Int32 lNewElementCount = -1, Bool bClear = true);
	ArrayElement* InsertAtA(Int32 l = 0, Int32 lNewElementCount = -1);
	ArrayElement* InsertMoreAt(Int32 l = 0, Int32 lInsert = 1, Int32 lNewElementCount = -1);
	ArrayElement* InsertMoreAt(Int32 lPos, Int32 lCount, const ArrayElement* pElements);
	ArrayElement* GetElement(Int32 lIndex) const;
	const ArrayElement* GetArray() const { return m_pElements; }
	ArrayElement* GetArray() { return m_pElements; }
	Bool Append(const ArrayElement &e, Int32 lGetNextAdd = 20, Bool bClear = true);
	Bool Append(const ArrayElement *e, Int32 lElements); // appends lElements of ArrayElement to this
	Bool InsertAtB(const ArrayElement& e, Int32 l, Int32 lNewElementCount = -1);
	Int32 GetElementCount() const { return m_lElements; }
	Bool IsEmpty() const { return m_lElements == 0; }
	Int32 GetMaxElementCount() const { return m_lMaxElements; }
	void ResetCounter(Bool bErase = false);
	void SetCounter(Int32 l) { m_lElements = l; }
	Bool RemoveAt(Int32 l, Bool bFillZeros);
	Bool RemoveAt(Int32 lPos, Int32 lCount, Bool bFillZeros);
	void SetData(void* pData) { m_pData = pData; }
	void* GetData() const { return m_pData; }
	virtual void Free();
	virtual void ExchangeContents(CDynamicArray <ArrayElement> *pDest);
	void Release();
	ArrayElement& operator [] (Int32 l) const;
	virtual Bool CopyTo(CDynamicArray<ArrayElement>* pDest) const;
	void Flip();
	Bool IsEqual(const CDynamicArray<ArrayElement>* pCmp) const;
	Bool Read(HyperFile* pFile, Bool bFlush = true);
	Bool Write(HyperFile* pFile, Int32 lLevel) const;
	Bool Read(BaseFile* pFile, Bool bFlush = true);
	Bool Write(BaseFile* pFile, Int32 lLevel) const;
	virtual Bool ReadElements(HyperFile* pFile, ArrayElement* pElements, Int32 lCount, Int32 lLevel);
	virtual Bool WriteElements(HyperFile* pFile, const ArrayElement* pElements, Int32 lCount) const;
	virtual Bool ReadElements(BaseFile* pFile, ArrayElement* pElements, Int32 lCount, Int32 lLevel);
	virtual Bool WriteElements(BaseFile* pFile, const ArrayElement* pElements, Int32 lCount) const;

	ArrayElement *m_pElements;
	Int32 m_lElements, m_lMaxElements;

	ArrayElement* m_pDefaultElement;
	void* m_pData;
	DeprecatedMemoryPool* m_pPool;
};

template<class ArrayElement> CDynamicArray<ArrayElement>::CDynamicArray()
{
	m_pElements = nullptr;
	m_lElements = 0; m_lMaxElements = 0;
	m_pDefaultElement = nullptr;
	m_pData = nullptr;
	m_pPool = nullptr;
}

template<class ArrayElement> CDynamicArray<ArrayElement>::CDynamicArray(ArrayElement *pDefaultElement)
{
	m_pElements = nullptr;
	m_lElements = 0; m_lMaxElements = 0;
	m_pDefaultElement = pDefaultElement;
	m_pPool = nullptr;
	m_pData = nullptr;
}

template<class ArrayElement> CDynamicArray<ArrayElement>::CDynamicArray(DeprecatedMemoryPool* pPool)
{
	m_pElements = nullptr;
	m_lElements = 0; m_lMaxElements = 0;
	m_pDefaultElement = nullptr;
	m_pData = nullptr;
	m_pPool = pPool;
}

template<class ArrayElement> CDynamicArray<ArrayElement>::CDynamicArray(DeprecatedMemoryPool* pPool, ArrayElement *pDefaultElement)
{
	m_pElements = nullptr;
	m_lElements = 0; m_lMaxElements = 0;
	m_pDefaultElement = pDefaultElement;
	m_pPool = pPool;
	m_pData = nullptr;
}

template<class ArrayElement> CDynamicArray<ArrayElement>::~CDynamicArray()
{
	Free();
}

template<class ArrayElement> void CDynamicArray<ArrayElement>::Free()
{
	m_lElements = 0;
	m_lMaxElements = 0;
	m_pDefaultElement = nullptr;
	if (m_pPool)
	{
		m_pPool->MemFreeS(m_pElements);
		m_pElements = nullptr;
	}
	else
		DeleteMem(m_pElements);
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::Alloc(Int32 lExtraElements, Int32 lNewElementCount, Bool bClear)
{
	if (m_lMaxElements >= lExtraElements + m_lElements) return true;

	if (lNewElementCount < lExtraElements) lNewElementCount = lExtraElements;

	ArrayElement* pTmp;
	if (m_pPool)
		pTmp = (ArrayElement*)m_pPool->MemReAllocS(m_pElements, (lNewElementCount + m_lElements) * sizeof(ArrayElement), false);
	else
		pTmp = ReallocMem(m_pElements, lNewElementCount + m_lElements);

	if (!pTmp) return false;

	m_pElements = pTmp;
	m_lMaxElements = lNewElementCount + m_lElements;
	Int32 a;
	if (bClear)
	{
		ClearMemType(&(m_pElements[m_lElements]), (m_lMaxElements - m_lElements));
	}
	else if (m_pDefaultElement)
	{
		for (a = m_lElements; a < m_lMaxElements; a++)
			m_pElements[a] = *m_pDefaultElement;
	}

	return true;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::Init(const ArrayElement* pArray, Int32 lCount)
{
	Free();
	if (!SetMinSizeNoCopy(lCount, false))
		return false;
	CopyMemType(pArray, m_pElements, lCount);
	m_lElements = lCount;
	return true;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::Init(const CDynamicArray<ArrayElement> &ar)
{
	Free();
	if (!SetMinSizeNoCopy(ar.m_lElements, false))
		return false;
	CopyMemType(ar.m_pElements, m_pElements, ar.m_lElements);
	m_lElements = ar.m_lElements;
	return true;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::InitNoCopy(ArrayElement* pArray, Int32 lCount)
{
	Free();
	m_pElements = pArray;
	m_lElements = m_lMaxElements = lCount;
	return true;
}


template<class ArrayElement> Bool CDynamicArray<ArrayElement>::SetMinSize(Int32 lElements)
{
	if (m_lMaxElements >= lElements) return true;
	Int32 lBack = m_lElements;
	m_lElements = m_lMaxElements;
	if (!Alloc(lElements - m_lMaxElements + 1)) return false;
	m_lElements = lBack;
	return true;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::SetMinSizeNoCopy(Int32 lElements, Bool bClear)
{
	DebugAssert(lElements >= 0);
	if (lElements <= 0)
		return true;
	if (m_lMaxElements >= lElements)
	{
		if (bClear)
			ClearMemType(m_pElements, lElements);
		m_lElements = 0;
		return true;
	}
	m_lElements = m_lMaxElements = 0;

	if (m_pPool)
		m_pPool->MemFreeS(m_pElements);
	else
		DeleteMem(m_pElements);
	m_pElements = nullptr;

	ArrayElement* pTmp;
	if (bClear)
	{
		if (m_pPool)
			pTmp = (ArrayElement*)m_pPool->MemAllocS((lElements) * sizeof(ArrayElement));
		else
		{
			pTmp = NewMemClear(ArrayElement, lElements);
		}
	}
	else
	{
		if (m_pPool)
			pTmp = (ArrayElement*)m_pPool->MemAllocS((lElements) * sizeof(ArrayElement), false);
		else
		{
			pTmp = NewMem(ArrayElement, lElements);
		}
	}

	if (!pTmp) return false;
	m_pElements = pTmp;
	m_lMaxElements = lElements;

#ifdef _DEBUG
	if (!bClear)
	{
		Int i, s = m_lMaxElements * sizeof(ArrayElement);
		UChar* c = (UChar*)m_pElements;
		for (i = 0; i < s; i++)
			*c++ = 0xcd;
	}
#endif

	return true;
}

template<class ArrayElement> ArrayElement* CDynamicArray<ArrayElement>::GetElement(Int32 lIndex) const
{
	DebugAssert(!(lIndex < 0 || lIndex >= m_lElements));
	if (lIndex < 0 || lIndex >= m_lElements) return nullptr;
	return &(m_pElements[lIndex]);
}

template<class ArrayElement> void CDynamicArray<ArrayElement>::ResetCounter(Bool bErase)
{
	if (bErase)
		ClearMemType(m_pElements, m_lElements);

	m_lElements = 0;
}


template<class ArrayElement> ArrayElement* CDynamicArray<ArrayElement>::GetNext(Int32 lNewElementCount, Bool bClear)
{
	if (m_lElements >= m_lMaxElements)
	{
		if (lNewElementCount <= 0)
		{
			// negative count means add -lNewElementCount% of the original size
			Int32 lAdd = LMax(20, (Int32)((-(Int64)lNewElementCount * (Int64)m_lElements) / 100));
			if (!Alloc(lAdd, -1, bClear)) { DebugAssert(false); return nullptr; }
		}
		else
		{
			if (!Alloc(lNewElementCount, -1, bClear)) { DebugAssert(false); return nullptr; }
		}
	}

	ArrayElement* e = &(m_pElements[m_lElements]);
	m_lElements++;
	return e;
}

template<class ArrayElement> ArrayElement* CDynamicArray<ArrayElement>::InsertAtA(Int32 l, Int32 lNewElementCount)
{
	return InsertMoreAt(l, 1, lNewElementCount);
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::InsertAtB(const ArrayElement& e, Int32 l, Int32 lNewElementCount)
{
	ArrayElement* pNew = InsertAtA(l, lNewElementCount);
	if (!pNew)
		return false;
	*pNew = e;
	return true;
}


template<class ArrayElement> ArrayElement* CDynamicArray<ArrayElement>::InsertMoreAt(Int32 lPos, Int32 lInsert, Int32 lNewElementCount)
{
	ArrayElement* e;
	if (lPos < 0) // append
		lPos = m_lElements;

	// if we have enough space, just move the elements
	if (m_lElements + lInsert < m_lMaxElements)
		memmove(&(m_pElements[lPos + lInsert]), &(m_pElements[lPos]), (m_lElements - lPos) * sizeof(ArrayElement));
	else
	{
		Int32 lAdd = LMax(lInsert, lNewElementCount);
		lAdd = LMax(lAdd, 20);

		ArrayElement* pNew;
		if (m_pPool)
			pNew = (ArrayElement*)m_pPool->MemReAllocS(m_pElements, sizeof(ArrayElement) * (m_lMaxElements + lAdd), false);
		else
			pNew = ReallocMem(m_pElements, m_lMaxElements + lAdd);

		if (!pNew)
			return nullptr;
		m_pElements = pNew;
		memmove(&(m_pElements[lPos + lInsert]), &(m_pElements[lPos]), (m_lElements - lPos) * sizeof(ArrayElement));
		if (m_pDefaultElement)
		{
			Int32 a;
			for (a = 0; a < lInsert; a++)
				pNew[a + lPos] = *m_pDefaultElement;
		}
		m_lMaxElements += lAdd;
	}
	e = &(m_pElements[lPos]);

	m_lElements += lInsert;
	return e;
}

template<class ArrayElement> ArrayElement* CDynamicArray<ArrayElement>::InsertMoreAt(Int32 lPos, Int32 lCount, const ArrayElement* pElements)
{
	ArrayElement* pNew = InsertMoreAt(lPos, lCount, -1);
	if (!pNew)
		return nullptr;
	CopyMemType(pElements, pNew, lCount);
	return pNew;
}


template<class ArrayElement> Bool CDynamicArray<ArrayElement>::RemoveAt(Int32 l, Bool bFillZeros)
{
	if (l < 0 || l >= m_lElements) return false;
	memmove(&(m_pElements[l]), &(m_pElements[l + 1]), (m_lElements - l - 1) * sizeof(ArrayElement));
	if (bFillZeros)
		ClearMemType(&(m_pElements[m_lElements - 1]), 1);
	m_lElements--;
	return true;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::RemoveAt(Int32 lPos, Int32 lCount, Bool bFillZeros)
{
	if (lPos < 0 || lPos >= m_lElements) return false;
	if (lPos + lCount >= m_lElements)
		lCount = m_lElements - lPos;
	memmove(&(m_pElements[lPos]), &(m_pElements[lPos + lCount]), (m_lElements - lPos - lCount) * sizeof(ArrayElement));
	if (bFillZeros)
		ClearMemType(&(m_pElements[m_lElements - lCount]), lCount);
	m_lElements -= lCount;
	return true;
}


template<class ArrayElement> void CDynamicArray<ArrayElement>::ExchangeContents(CDynamicArray <ArrayElement> *pDest)
{
	Int32 l;
	ArrayElement* e;
	void* v;

	e = m_pElements; m_pElements = pDest->m_pElements; pDest->m_pElements = e;
	e = m_pDefaultElement; m_pDefaultElement = pDest->m_pDefaultElement; pDest->m_pDefaultElement = e;
	l = m_lElements; m_lElements = pDest->m_lElements; pDest->m_lElements = l;
	l = m_lMaxElements; m_lMaxElements = pDest->m_lMaxElements; pDest->m_lMaxElements = l;
	v = m_pData; m_pData = pDest->m_pData; pDest->m_pData = v;
}

template<class ArrayElement> void CDynamicArray<ArrayElement>::Release()
{
	m_pElements = nullptr;
	m_lElements = 0; m_lMaxElements = 0;
}


template<class ArrayElement> Bool CDynamicArray<ArrayElement>::Append(const ArrayElement &e, Int32 lGetNextAdd, Bool bClear)
{
	if (m_lElements < m_lMaxElements)
		m_pElements[m_lElements++] = e;
	else
	{
		ArrayElement* pNew = GetNext(lGetNextAdd, bClear);
		if (!pNew)
			return false;
		*pNew = e;
	}
	return true;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::Append(const ArrayElement *e, Int32 lElements)
{
	if (!Alloc(lElements, -1, false))
		return false;
	CopyMemType(e, &(m_pElements[m_lElements]), lElements);
	m_lElements += lElements;
	return true;
}


template<class ArrayElement> ArrayElement& CDynamicArray<ArrayElement>::operator [] (Int32 l) const
{
	DebugAssert(l >= 0 && l < m_lElements);
	return m_pElements[l];
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::CopyTo(CDynamicArray<ArrayElement>* pDest) const
{
	pDest->Free();
	if (!pDest->SetMinSizeNoCopy(m_lElements))
		return false;
	pDest->SetCounter(m_lElements);
	CopyMemType(m_pElements, pDest->m_pElements, m_lElements);
	return true;
}

template<class ArrayElement> void CDynamicArray<ArrayElement>::Flip()
{
	ArrayElement t;
	Int32 a, max;
	max = m_lElements / 2;
	for (a = 0; a < max; a++)
	{
		t = m_pElements[a];
		m_pElements[a] = m_pElements[m_lElements - 1 - a];
		m_pElements[m_lElements - 1 - a] = t;
	}
}

template<class ArrayElement>Bool CDynamicArray<ArrayElement>::IsEqual(const CDynamicArray<ArrayElement>* pCmp) const
{
	if (!pCmp)
		return false;
	if (m_lElements != pCmp->m_lElements)
		return false;
	Int32 a;
	for (a = 0; a < m_lElements; a++)
	{
		if (m_pElements[a] != pCmp->m_pElements[a])
			return false;
	}
	return true;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::Read(HyperFile* pFile, Bool bFlush)
{
	Int32 lElements, lLevel;
	if (bFlush)
		Free();
	if (!CDynamicArrayReadHeader(pFile, lElements, lLevel)) return false;
	if (bFlush)
	{
		if (!SetMinSizeNoCopy(lElements))
			return false;
		m_lElements = 0;
	}
	else
	{
		if (!SetMinSizeNoCopy(m_lElements + lElements))
			return false;
	}
	if (lElements > 0)
	{
		if (!ReadElements(pFile, &(m_pElements[m_lElements]), lElements, lLevel))
			return false;
	}
	m_lElements += lElements;
	return true;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::Write(HyperFile* pFile, Int32 lLevel) const
{
	Int32 lElements = GetElementCount();
	if (!CDynamicArrayWriteHeader(pFile, lElements, lLevel)) return false;
	if (lElements > 0)
	{
		if (!WriteElements(pFile, m_pElements, lElements))
			return false;
	}
	return true;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::Read(BaseFile* pFile, Bool bFlush)
{
	Int32 lElements, lLevel;
	if (bFlush)
		Free();
	if (!CDynamicArrayReadHeader(pFile, lElements, lLevel)) return false;
	if (bFlush)
	{
		if (!SetMinSizeNoCopy(lElements))
			return false;
		m_lElements = 0;
	}
	else
	{
		if (!SetMinSizeNoCopy(m_lElements + lElements))
			return false;
	}
	if (lElements > 0)
	{
		if (!ReadElements(pFile, &(m_pElements[m_lElements]), lElements, lLevel))
			return false;
	}
	m_lElements += lElements;
	return true;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::Write(BaseFile* pFile, Int32 lLevel) const
{
	Int32 lElements = GetElementCount();
	if (!CDynamicArrayWriteHeader(pFile, lElements, lLevel)) return false;
	if (lElements > 0)
	{
		if (!WriteElements(pFile, m_pElements, lElements))
			return false;
	}
	return true;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::ReadElements(HyperFile* pFile, ArrayElement* pElements, Int32 lCount, Int32 lLevel)
{
	// override
	DebugAssert(false);
	return false;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::WriteElements(HyperFile* pFile, const ArrayElement* pElements, Int32 lCount) const
{
	// override
	DebugAssert(false);
	return false;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::ReadElements(BaseFile* pFile, ArrayElement* pElements, Int32 lCount, Int32 lLevel)
{
	// override
	DebugAssert(false);
	return false;
}

template<class ArrayElement> Bool CDynamicArray<ArrayElement>::WriteElements(BaseFile* pFile, const ArrayElement* pElements, Int32 lCount) const
{
	// override
	DebugAssert(false);
	return false;
}






/*****************************************************************************\
*   CDynamicSortSet                                                           *
\*****************************************************************************/

template <class ArrayElement> class CDynamicSortSet : public CDynamicArray<ArrayElement>
{
public:
	CDynamicSortSet() : CDynamicArray<ArrayElement>() { }
	CDynamicSortSet(ArrayElement *pDefaultElement) : CDynamicArray<ArrayElement>(pDefaultElement) { }
	CDynamicSortSet(DeprecatedMemoryPool* pPool) : CDynamicArray<ArrayElement>(pPool) { }
	CDynamicSortSet(DeprecatedMemoryPool* pPool, ArrayElement *pDefaultElement) : CDynamicArray<ArrayElement>(pPool, pDefaultElement) { }
	virtual ~CDynamicSortSet();

	ArrayElement* ElementExists(const ArrayElement* pElement, Int32* plPos = nullptr) const;
	Bool ElementExists(const ArrayElement& element) const;
	ArrayElement* AddElement(const ArrayElement* pElement, Bool& bExisted, Int32 lSizeInc = -1);
	ArrayElement* AddElement(const ArrayElement& element, Int32 lSizeInc = -1);
	Bool RemoveElement(const ArrayElement& element);
	Int32 Find(const ArrayElement* pElement) const;
	static Int32 SortCompare(const ArrayElement* a, const ArrayElement* b, const void* pData);
	void Sort();
	Bool operator == (const CDynamicArray<ArrayElement>& c);

protected:
	virtual Int32 Compare(const ArrayElement* a, const ArrayElement* b) const = 0;
};

template<class ArrayElement> CDynamicSortSet<ArrayElement>::~CDynamicSortSet()
{
}

template<class ArrayElement> ArrayElement* CDynamicSortSet<ArrayElement>::ElementExists(const ArrayElement* pElement, Int32* plPos) const
{
	if (GCC34T m_lElements == 0) return nullptr;

	Int32 l = 0, r = GCC34T m_lElements - 1,  c;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(&GCC34T m_pElements[c], pElement);

		if (lComp == 0)
		{
			if (plPos)
				*plPos = c;
			return &GCC34T m_pElements[c];
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return nullptr;
}

template<class ArrayElement> Bool CDynamicSortSet<ArrayElement>::ElementExists(const ArrayElement& element) const
{
	if (GCC34T m_lElements == 0) return false;

	Int32 l = 0, r = GCC34T m_lElements - 1,  c;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(&GCC34T m_pElements[c], &element);

		if (lComp == 0)
		{
			return true;
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return false;
}

template<class ArrayElement> ArrayElement* CDynamicSortSet<ArrayElement>::AddElement(const ArrayElement* pElement, Bool& bExisted, Int32 lSizeInc)
{
	if (GCC34T m_lElements == 0)
	{
		ArrayElement* pNew = GCC34T GetNext();
		bExisted = false;
		return pNew;
	}
	Int32 c, l = 0, r = GCC34T m_lElements - 1;
	Int32 lInsert;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(&GCC34T m_pElements[c], pElement);
		if (lComp == 0)
		{
			bExisted = true;
			return &GCC34T m_pElements[c];
		}
		else if (lComp < 0)
		{
			lInsert = c + 1;
			l = c + 1;
		}
		else
		{
			lInsert = c;
			r = c - 1;
		}
	} while (l <= r);

	ArrayElement* pNew = GCC34T InsertAtA(lInsert, lSizeInc);
	if (!pNew)
		return nullptr;
	bExisted = false;

	return pNew;
}

template<class ArrayElement> ArrayElement* CDynamicSortSet<ArrayElement>::AddElement(const ArrayElement& element, Int32 lSizeInc)
{
	if (GCC34T m_lElements == 0)
	{
		ArrayElement* pNew = GCC34T GetNext();
		if (!pNew)
			return nullptr;
		*pNew = element;
		return pNew;
	}
	Int32 c, l = 0, r = GCC34T m_lElements - 1;
	Int32 lInsert;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(&GCC34T m_pElements[c], &element);
		if (lComp == 0)
		{
			return &GCC34T m_pElements[c];
		}
		else if (lComp < 0)
		{
			lInsert = c + 1;
			l = c + 1;
		}
		else
		{
			lInsert = c;
			r = c - 1;
		}
	} while (l <= r);

	ArrayElement* pNew = GCC34T InsertAtA(lInsert, lSizeInc);
	if (!pNew)
		return nullptr;
	*pNew = element;

	return pNew;
}

template<class ArrayElement> Bool CDynamicSortSet<ArrayElement>::RemoveElement(const ArrayElement& element)
{
	if (GCC34T m_lElements == 0)
		return false;

	Int32 c, l = 0, r = GCC34T m_lElements - 1;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(&GCC34T m_pElements[c], &element);
		if (lComp == 0)
		{
			if (!GCC34T RemoveAt(c, false))
				return false;
			return true;
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);

	return false;
}

template<class ArrayElement> Int32 CDynamicSortSet<ArrayElement>::Find(const ArrayElement* pElement) const
{
	if (GCC34T m_lElements == 0) return -1;

	Int32 l = 0, r = GCC34T m_lElements - 1,  c;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(&GCC34T m_pElements[c], pElement);

		if (lComp == 0)
		{
			return c;
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return -1;
}

template<class ArrayElement> void CDynamicSortSet<ArrayElement>::Sort()
{
	CShellSort <ArrayElement> sort;
	sort.Sort(GCC34T m_pElements, SortCompare, GCC34T m_lElements, this);
}

template<class ArrayElement> Bool CDynamicSortSet<ArrayElement>::operator == (const CDynamicArray<ArrayElement>& c)
{
	Int32 lElements = GCC34T GetElementCount();
	if (lElements != c.m_lElements)
		return false;
	ArrayElement* pc = c.m_pElements;
	Int32 a;
	for (a = 0; a < lElements; a++)
	{
		if (Compare(&(GCC34T m_pElements[a]), &(pc[a])) != 0)
			return false;
	}
	return true;
}



/*****************************************************************************\
*   CDynamicSortArray                                                         *
\*****************************************************************************/

template <class ArrayElement> class CDynamicSortArray : public CDynamicArray<ArrayElement>
{
public:
	CDynamicSortArray() : CDynamicArray<ArrayElement>() { }
	CDynamicSortArray(ArrayElement *pDefaultElement) : CDynamicArray<ArrayElement>(pDefaultElement) { }
	CDynamicSortArray(DeprecatedMemoryPool* pPool) : CDynamicArray<ArrayElement>(pPool) { }
	CDynamicSortArray(DeprecatedMemoryPool* pPool, ArrayElement *pDefaultElement) : CDynamicArray<ArrayElement>(pPool, pDefaultElement) { }
	virtual ~CDynamicSortArray();

	ArrayElement* AddElement(const ArrayElement* pElement, Int32 lInsertSpace = 1, Int32 lSizeInc = -1);
	ArrayElement* AddElement(const ArrayElement& e, Int32 lInsertSpace = 1, Int32 lSizeInc = -1);
	Int32 Find(const ArrayElement* pElement) const;
	Bool GetBounds(const ArrayElement* pElement, Int32 &lLower, Int32 &lUpper) const;
	Int32 FindSpecial(const void* pData) const;
	void DeleteDuplicates();
	void Sort();
	static Int32 SortCompare(const ArrayElement* a, const ArrayElement* b, const void* pData);
	virtual Int32 Compare(const ArrayElement* a, const ArrayElement* b) const = 0;
	virtual Int32 CompareSpecial(const ArrayElement* a, const void* pSpecial) const { DebugAssert(false); return 0; }
};

template<class ArrayElement> Int32 CDynamicSortSet<ArrayElement>::SortCompare(const ArrayElement* a, const ArrayElement* b, const void* pData)
{
	typedef CDynamicSortArray<ArrayElement> TTTT;

	CDynamicSortArray<ArrayElement> *pp = (TTTT*)pData;

	return pp->Compare(a, b);
}


template<class ArrayElement> CDynamicSortArray<ArrayElement>::~CDynamicSortArray()
{
}

template<class ArrayElement> ArrayElement* CDynamicSortArray<ArrayElement>::AddElement(const ArrayElement* pElement, Int32 lInsertSpace, Int32 lSizeInc)
{
	if (GCC34T m_lElements == 0)
	{
		ArrayElement* pNew = GCC34T InsertMoreAt(0, lInsertSpace, lSizeInc);
		return pNew;
	}
	Int32 c, l = 0, r = GCC34T m_lElements - 1;
	Int32 lInsert;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(&GCC34T m_pElements[c], pElement);
		if (lComp == 0)
		{
			lInsert = c;
			break;
		}
		else if (lComp < 0)
		{
			lInsert = c + 1;
			l = c + 1;
		}
		else
		{
			lInsert = c;
			r = c - 1;
		}
	} while (l <= r);

	ArrayElement* pNew = GCC34T InsertMoreAt(lInsert, lInsertSpace, lSizeInc);
	if (!pNew)
		return nullptr;

	return pNew;
}


template<class ArrayElement> ArrayElement* CDynamicSortArray<ArrayElement>::AddElement(const ArrayElement& e, Int32 lInsertSpace, Int32 lSizeInc)
{
	ArrayElement* pNew = AddElement((const ArrayElement*)&e, lInsertSpace, lSizeInc);
	if (!pNew)
		return nullptr;
	*pNew = e;
	return pNew;
}

template<class ArrayElement> Int32 CDynamicSortArray<ArrayElement>::Find(const ArrayElement* pElement) const
{
	if (GCC34T m_lElements == 0) return -1;

	Int32 l = 0, r = GCC34T m_lElements - 1,  c;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(&GCC34T m_pElements[c], pElement);

		if (lComp == 0)
		{
			return c;
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return -1;
}

template<class ArrayElement> Bool CDynamicSortArray<ArrayElement>::GetBounds(const ArrayElement* pElement, Int32 &lLower, Int32 &lUpper) const
{
	if (GCC34T m_lElements == 0) return false;

	Int32 l = 0, r = GCC34T m_lElements - 1,  c;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(&GCC34T m_pElements[c], pElement);

		if (lComp == 0)
		{
			lLower = lUpper = c;
			while (lLower > 0)
			{
				if (Compare(&GCC34T m_pElements[lLower - 1], pElement) == 0)
					lLower--;
				else
					break;
			}
			while (lUpper < GCC34T m_lElements - 1)
			{
				if (Compare(&GCC34T m_pElements[lUpper + 1], pElement) == 0)
					lUpper++;
				else
					break;
			}

			return true;
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return false;
}


template<class ArrayElement> Int32 CDynamicSortArray<ArrayElement>::FindSpecial(const void* pData) const
{
	if (GCC34T m_lElements == 0) return -1;

	Int32 l = 0, r = GCC34T m_lElements - 1,  c;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = CompareSpecial(&GCC34T m_pElements[c], pData);

		if (lComp == 0)
		{
			return c;
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return -1;
}

template<class ArrayElement> void CDynamicSortArray<ArrayElement>::Sort()
{
	CShellSort <ArrayElement> sort;
	sort.Sort(GCC34T m_pElements, SortCompare, GCC34T m_lElements, this);
}

template<class ArrayElement> Int32 CDynamicSortArray<ArrayElement>::SortCompare(const ArrayElement* a, const ArrayElement* b, const void* pData)
{
	return ((CDynamicSortArray<ArrayElement>*)pData)->Compare(a, b);
}

template<class ArrayElement> void CDynamicSortArray<ArrayElement>::DeleteDuplicates()
{
	// the array must be sorted
	if (GCC34T m_lElements <= 0)
		return;

	Int32 dest = 0, src = 1;
	while (src < GCC34T m_lElements)
	{
		DebugAssert(Compare(&GCC34T m_pElements[dest], &GCC34T m_pElements[src]) <= 0);
		if (Compare(&GCC34T m_pElements[dest], &GCC34T m_pElements[src]) == 0)
		{
			src++;
			continue;
		}
		dest++;
		if (dest != src)
			GCC34T m_pElements[dest] = GCC34T m_pElements[src];
		src++;
	}
	GCC34T m_lElements = dest + 1;
}

/*****************************************************************************\
*   CFifo                                                                     *
\*****************************************************************************/

template <class ArrayElement> class CFifo : public CDynamicArray<ArrayElement>
{
public:
	CFifo();
	CFifo(ArrayElement *pDefaultElement);
	CFifo(DeprecatedMemoryPool* pPool);
	CFifo(DeprecatedMemoryPool* pPool, ArrayElement *pDefaultElement);
	virtual ~CFifo();

	inline Bool IsEmpty() { return m_lFifoRead == GCC34T m_lElements; }
	Bool Add(const ArrayElement& e);
	Bool Add(const ArrayElement& e, Int32 lAddElements);
	virtual ArrayElement* GetAndRemoveFirst();
	void EmptyFifo();
	void SetMaxRead(Int32 l) { m_lMaxRead = l; }

protected:
	Int32 m_lFifoRead, m_lMaxRead;
};

template<class ArrayElement> CFifo<ArrayElement>::CFifo() : CDynamicArray<ArrayElement>()
{
	m_lFifoRead = 0;
	m_lMaxRead = 100;
}

template<class ArrayElement> CFifo<ArrayElement>::CFifo(ArrayElement *pDefaultElement) : CDynamicArray<ArrayElement>(pDefaultElement)
{
	m_lFifoRead = 0;
	m_lMaxRead = 100;
}

template<class ArrayElement> CFifo<ArrayElement>::CFifo(DeprecatedMemoryPool* pPool) : CDynamicArray<ArrayElement>(pPool)
{
	m_lFifoRead = 0;
	m_lMaxRead = 100;
}

template<class ArrayElement> CFifo<ArrayElement>::CFifo(DeprecatedMemoryPool* pPool, ArrayElement *pDefaultElement) : CDynamicArray<ArrayElement>(pPool, pDefaultElement)
{
	m_lFifoRead = 0;
	m_lMaxRead = 100;
}

template<class ArrayElement> CFifo<ArrayElement>::~CFifo()
{
}


template<class ArrayElement> Bool CFifo<ArrayElement>::Add(const ArrayElement& e)
{
	if (GCC34T m_lElements < GCC34T m_lMaxElements - 1)
		GCC34T m_pElements[GCC34T m_lElements++] = e;
	else
	{
		ArrayElement* pNew = GCC34T GetNext(-1, false);
		if (!pNew)
			return false;
		*pNew = e;
	}
	return true;
}

template<class ArrayElement> Bool CFifo<ArrayElement>::Add(const ArrayElement& e, Int32 lAddElements)
{
	if (GCC34T m_lElements < GCC34T m_lMaxElements - 1)
		GCC34T m_pElements[GCC34T m_lElements++] = e;
	else
	{
		ArrayElement* pNew = GCC34T GetNext(lAddElements, false);
		if (!pNew)
			return false;
		*pNew = e;
	}
	return true;
}

template<class ArrayElement> ArrayElement* CFifo<ArrayElement>::GetAndRemoveFirst()
{
	if (m_lFifoRead >= GCC34T m_lElements)
		return nullptr;
	if (m_lFifoRead >= m_lMaxRead)
	{
		memmove(GCC34T m_pElements, &(GCC34T m_pElements[m_lMaxRead]), sizeof(ArrayElement) * (GCC34T m_lElements - m_lMaxRead));
		m_lFifoRead -= m_lMaxRead;
		GCC34T m_lElements -= m_lMaxRead;
	}
	return &(GCC34T m_pElements[m_lFifoRead++]);
}

template<class ArrayElement> void CFifo<ArrayElement>::EmptyFifo( )
{
	m_lFifoRead = 0;
	GCC34T m_lElements = 0;
}






/*****************************************************************************\
*   CStack                                                                    *
\*****************************************************************************/

template <class ArrayElement> class CStack : public CDynamicArray<ArrayElement>
{
public:
	CStack();
	CStack(ArrayElement *pDefaultElement);
	CStack(DeprecatedMemoryPool* pPool);
	CStack(DeprecatedMemoryPool* pPool, ArrayElement *pDefaultElement);
	virtual ~CStack();

	inline Bool IsEmpty() { return GCC34T m_lElements == 0; }
	Bool Push(const ArrayElement& e);
	ArrayElement* Pop();
	ArrayElement* Top() { return GCC34T m_lElements == 0 ? nullptr : &(GCC34T m_pElements[GCC34T m_lElements - 1]); }
	void Sort();
};

template<class ArrayElement> CStack<ArrayElement>::CStack() : CDynamicArray<ArrayElement>()
{
}

template<class ArrayElement> CStack<ArrayElement>::CStack(ArrayElement *pDefaultElement) : CDynamicArray<ArrayElement>(pDefaultElement)
{
}

template<class ArrayElement> CStack<ArrayElement>::CStack(DeprecatedMemoryPool* pPool) : CDynamicArray<ArrayElement>(pPool)
{
}

template<class ArrayElement> CStack<ArrayElement>::CStack(DeprecatedMemoryPool* pPool, ArrayElement *pDefaultElement) : CDynamicArray<ArrayElement>(pPool, pDefaultElement)
{
}

template<class ArrayElement> CStack<ArrayElement>::~CStack()
{
}

template<class ArrayElement> Bool CStack<ArrayElement>::Push(const ArrayElement& e)
{
	if (GCC34T m_lElements < GCC34T m_lMaxElements - 1)
		GCC34T m_pElements[GCC34T m_lElements++] = e;
	else
	{
		ArrayElement* pNew = GCC34T GetNext();
		if (!pNew)
			return false;
		*pNew = e;
	}
	return true;
}

template<class ArrayElement> ArrayElement* CStack<ArrayElement>::Pop()
{
	if (GCC34T m_lElements == 0)
		return nullptr;
	return &(GCC34T m_pElements[--GCC34T m_lElements]);
}



/*****************************************************************************\
*   CPriorityQueue                                                            *
\*****************************************************************************/

template <class ArrayElement, class PrioType> struct CPriorityQueueItem
{
	PrioType prio;
	ArrayElement e;
};

/*template <class ArrayElement, class PrioType> class CPriorityQueue : public CFifo< struct <ArrayElement, PrioType> >
{
};*/

template <class H, class ArrayElement, class PrioType> class CPriorityQueue : public CFifo<H>
{
public:
	CPriorityQueue();
	virtual ~CPriorityQueue();

	Bool Add(const ArrayElement& e, PrioType prio);
	Bool AddSorted(const ArrayElement& e, PrioType prio);
	ArrayElement* GetAndRemoveTop();
	static Int32 SortCompare(const H* a, const H* b);
	void Sort();
};

template <class H, class ArrayElement, class PrioType> CPriorityQueue<H, ArrayElement, PrioType>::CPriorityQueue() : CFifo<H>()
{
}

template <class H, class ArrayElement, class PrioType> CPriorityQueue<H, ArrayElement, PrioType>::~CPriorityQueue()
{
	GCC34T Free();
}

template <class H, class ArrayElement, class PrioType> Bool CPriorityQueue<H, ArrayElement, PrioType>::Add(const ArrayElement& e, PrioType prio)
{
	H h;
	h.prio = prio;
	h.e = e;
	return CFifo<H>::Add(h);
}

template <class H, class ArrayElement, class PrioType> Bool CPriorityQueue<H, ArrayElement, PrioType>::AddSorted(const ArrayElement& e, PrioType prio)
{
	// just a hack - will be optimized later...
	if (!Add(e, prio))
		return false;
	Sort();
	return true;
}

template <class H, class ArrayElement, class PrioType> ArrayElement* CPriorityQueue<H, ArrayElement, PrioType>::GetAndRemoveTop()
{
	H* h = GCC34T GetAndRemoveFirst();
	if (!h)
		return nullptr;
	return &(h->e);
}

template <class H, class ArrayElement, class PrioType> Int32 CPriorityQueue<H, ArrayElement, PrioType>::SortCompare(const H* a, const H* b)
{
	if (a->prio < b->prio) return -1;
	else if (a->prio > b->prio) return 1;
	return 0;
}

template <class H, class ArrayElement, class PrioType> void CPriorityQueue<H, ArrayElement, PrioType>::Sort()
{
	CShellSort <H> s;
	s.Sort(&(GCC34T m_pElements[GCC34T m_lFifoRead]), SortCompare, GCC34T m_lElements - GCC34T m_lFifoRead);
}


/*****************************************************************************\
*   CDynamicObjectArray                                                       *
\*****************************************************************************/

template <class ArrayElement> class CDynamicObjectArray
{
public:

	typedef ArrayElement* ArrayElementPtr;

	CDynamicObjectArray();
	virtual ~CDynamicObjectArray();

	ArrayElement* GetNextObject();
	ArrayElement* InsertAt(Int32 l);

	ArrayElement* GetNextObjectType(Int32 lID);
	ArrayElement* InsertAtType(Int32 lID, Int32 l);

	Int32 GetObjectIndex(const ArrayElement* pObj) const;
	ArrayElement* GetObjectAt(Int32 l) const;
	Int32 GetElementCount() const;
	Bool RemoveAt(Int32 l);
	Bool RemoveObjectPointer(const ArrayElement* pObj);
	const CDynamicArray<ArrayElementPtr>* GetArray() const { return &m_Objects; }
	CDynamicArray<ArrayElementPtr>* GetArray() { return &m_Objects; }
	ArrayElement* operator [] (Int32 l) const;
	Bool IsEmpty() const { return m_Objects.GetElementCount() == 0; }
	Bool IsEqual(const CDynamicObjectArray <ArrayElement>* pArray) const;

	Bool RemovePointer(Int32 lIndex); // removes the object, doesn't free memory
	Bool InsertObject(ArrayElement* pObj, Int32 lIndex); // inserts an existing object into the array at position lIndex
	Bool Init(const ArrayElement* const* ppObjects, Int32 lCount, AliasTrans* pTrans = nullptr, Bool bAppend = false);
	virtual Bool CopyTo(CDynamicObjectArray <ArrayElement>* pDest, AliasTrans* pTrans, Bool bAppend = false) const;
	Bool Alloc(Int32 lElementCount); // allocates memory to hold lElementCount elements more than currently, increases the counter

	virtual void Free();
	Bool Read(BaseFile* pFile, Bool bFlush = true);
	Bool Write(BaseFile* pFile, Int32 lLevel) const;
	Bool Read(HyperFile* pFile, Bool bFlush = true);
	Bool Write(HyperFile* pFile, Int32 lLevel) const;
	Bool ReadCreateObject(HyperFile* pFile, Bool bFlush = true);
protected:
	virtual ArrayElement* AllocObject() const = 0;
	virtual void FreeObject(ArrayElement *&pObj) const = 0;
	virtual ArrayElement* AllocObjectType(Int32 lType) const;
	virtual ArrayElement* GetClone(const ArrayElement* pObj, AliasTrans* pTrans) const;
	CDynamicArray<ArrayElementPtr> m_Objects;

	virtual Bool ReadElement(BaseFile* pFile, ArrayElement* pElement, Int32 lLevel);
	virtual Bool WriteElement(BaseFile* pFile, const ArrayElement* pElement) const;
	virtual Bool ReadElement(HyperFile* pFile, ArrayElement* pElement, Int32 lLevel);
	virtual Bool WriteElement(HyperFile* pFile, const ArrayElement* pElement) const;
	virtual Bool ReadElementCreate(HyperFile* pFile, ArrayElement* &pElement, Int32 lLevel);
	virtual Bool TestEqual(const ArrayElement* pObjA, const ArrayElement* pObjB) const;
};

template<class ArrayElement> CDynamicObjectArray<ArrayElement>::CDynamicObjectArray()
{
}

template<class ArrayElement> CDynamicObjectArray<ArrayElement>::~CDynamicObjectArray()
{
	DebugAssert(m_Objects.GetElementCount() == 0);
	// don't call Free here, it will cause a pure virtual function call
	// call Free in the destructor of your derived class.
}


template<class ArrayElement> ArrayElement* CDynamicObjectArray<ArrayElement>::GetNextObject()
{
	ArrayElement* pNew = AllocObject();
	if (!pNew)
		return nullptr;

	ArrayElement** ppNew = m_Objects.GetNext();
	if (!ppNew)
	{
		FreeObject(pNew);
		return nullptr;
	}

	//DebugAssert(!*ppNew);
	*ppNew = pNew;
	return *ppNew;
}

template<class ArrayElement> ArrayElement* CDynamicObjectArray<ArrayElement>::GetNextObjectType(Int32 lID)
{
	ArrayElement* pNew = AllocObjectType(lID);
	if (!pNew)
		return nullptr;

	ArrayElement** ppNew = m_Objects.GetNext();
	if (!ppNew)
	{
		FreeObject(pNew);
		return nullptr;
	}

	DebugAssert(!*ppNew);
	*ppNew = pNew;
	return *ppNew;
}

template<class ArrayElement> ArrayElement* CDynamicObjectArray<ArrayElement>::AllocObjectType(Int32 lID) const
{
	// override!
	DebugAssert(false);
	return nullptr;
}

template<class ArrayElement> ArrayElement* CDynamicObjectArray<ArrayElement>::GetClone(const ArrayElement* pObj, AliasTrans* pTrans) const
{
	// override!
	DebugAssert(false);
	return nullptr;
}

template<class ArrayElement> Int32 CDynamicObjectArray<ArrayElement>::GetObjectIndex(const ArrayElement* pObj) const
{
	Int32 lElements = m_Objects.GetElementCount();
	ArrayElement* const* ppElements = m_Objects.GetArray();

	if (lElements == 0 || ppElements == 0)
		return -1;

	Int32 l = 0;
	for (l = 0; l < lElements; l++)
	{
		if (ppElements[l] == pObj)
			return l;
	}
	return -1;
}

template<class ArrayElement> ArrayElement* CDynamicObjectArray<ArrayElement>::GetObjectAt(Int32 l) const
{
	if (l < 0 || l >= m_Objects.GetElementCount())
		return nullptr;

	ArrayElement** ppElement = m_Objects.GetElement(l);
	if (!ppElement)
		return nullptr;

	return *ppElement;
}

template<class ArrayElement> void CDynamicObjectArray<ArrayElement>::Free()
{
	Int32 l, lCount = m_Objects.GetElementCount();
	ArrayElement** ppElements = m_Objects.GetArray();
	for (l = 0; l < lCount; l++)
	{
		FreeObject(ppElements[l]);
	}
	m_Objects.Free();
}

template<class ArrayElement> Int32 CDynamicObjectArray<ArrayElement>::GetElementCount() const
{
	return m_Objects.GetElementCount();
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::RemoveAt(Int32 l)
{
	if (l < 0 || l >= m_Objects.GetElementCount())
		return false;
	ArrayElement** ppElement = m_Objects.GetElement(l);
	if (!ppElement)
		return false;
	if (!*ppElement)
		return false;
	ArrayElement* a = *ppElement;
	FreeObject(a);
	m_Objects.RemoveAt(l, true);
	return true;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::RemoveObjectPointer(const ArrayElement* pObj)
{
	Int32 lIndex = GetObjectIndex(pObj);
	if (lIndex < 0)
		return false;
	return RemoveAt(lIndex);
}

template<class ArrayElement> ArrayElement* CDynamicObjectArray<ArrayElement>::InsertAt(Int32 l)
{
	ArrayElement* pNew = AllocObject();
	if (!pNew)
		return nullptr;

	ArrayElement** ppNew = m_Objects.InsertAtA(l);
	if (!ppNew)
	{
		FreeObject(pNew);
		return nullptr;
	}
	*ppNew = pNew;
	return *ppNew;
}

template<class ArrayElement> ArrayElement* CDynamicObjectArray<ArrayElement>::InsertAtType(Int32 lType, Int32 l)
{
	ArrayElement* pNew = AllocObjectType(lType);
	if (!pNew)
		return nullptr;

	ArrayElement** ppNew = m_Objects.InsertAtA(l);
	if (!ppNew)
	{
		FreeObject(pNew);
		return nullptr;
	}
	*ppNew = pNew;
	return *ppNew;
}

template<class ArrayElement> ArrayElement* CDynamicObjectArray<ArrayElement>::operator [] (Int32 l) const
{
	return m_Objects[l];
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::Read(BaseFile* pFile, Bool bFlush)
{
	Int32 lElements, lLevel;
	if (bFlush)
		m_Objects.Free();
	if (!CDynamicArrayReadHeader(pFile, lElements, lLevel)) return false;
	if (bFlush)
	{
		if (!m_Objects.SetMinSizeNoCopy(lElements))
			return false;
	}

	for (Int32 a = 0; a < lElements; a++)
	{
		ArrayElement* pElement = AllocObject();
		if (!pElement) return false;
		if (!ReadElement(pFile, pElement, lLevel))
		{
			FreeObject(pElement);
			return false;
		}
		if (!m_Objects.Append(pElement))
		{
			FreeObject(pElement);
			return false;
		}
	}
	return true;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::Write(BaseFile* pFile, Int32 lLevel) const
{
	Int32 lElements = m_Objects.GetElementCount();
	if (!CDynamicArrayWriteHeader(pFile, lElements, lLevel)) return false;
	for (Int32 a = 0; a < lElements; a++)
	{
		if (!WriteElement(pFile, m_Objects[a]))
			return false;
	}
	return true;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::Read(HyperFile* pFile, Bool bFlush)
{
	Int32 lElements, lLevel;
	if (bFlush)
		m_Objects.Free();
	if (!CDynamicArrayReadHeader(pFile, lElements, lLevel)) return false;
	if (bFlush)
	{
		if (!m_Objects.SetMinSizeNoCopy(lElements))
			return false;
	}

	for (Int32 a = 0; a < lElements; a++)
	{
		ArrayElement* pElement = AllocObject();
		if (!pElement) return false;
		if (!ReadElement(pFile, pElement, lLevel))
		{
			FreeObject(pElement);
			return false;
		}
		if (!m_Objects.Append(pElement))
			return false;
	}
	return true;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::ReadCreateObject(HyperFile* pFile, Bool bFlush)
{
	Int32 lElements, lLevel;
	if (bFlush)
		m_Objects.Free();
	if (!CDynamicArrayReadHeader(pFile, lElements, lLevel)) return false;
	if (bFlush)
	{
		if (!m_Objects.SetMinSizeNoCopy(lElements))
			return false;
	}

	for (Int32 a = 0; a < lElements; a++)
	{
		ArrayElement* pElement = nullptr;
		if (!ReadElementCreate(pFile, pElement, lLevel))
		{
			FreeObject(pElement);
			return false;
		}
		if (!m_Objects.Append(pElement))
			return false;
	}
	return true;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::Write(HyperFile* pFile, Int32 lLevel) const
{
	Int32 lElements = m_Objects.GetElementCount();
	if (!CDynamicArrayWriteHeader(pFile, lElements, lLevel)) return false;
	for (Int32 a = 0; a < lElements; a++)
	{
		if (!WriteElement(pFile, m_Objects[a]))
			return false;
	}
	return true;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::ReadElement(BaseFile* pFile, ArrayElement* pElement, Int32 lLevel)
{
	// override!
	DebugAssert(false);
	return false;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::WriteElement(BaseFile* pFile, const ArrayElement* pElement) const
{
	// override!
	DebugAssert(false);
	return false;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::ReadElement(HyperFile* pFile, ArrayElement* pElement, Int32 lLevel)
{
	// override!
	DebugAssert(false);
	return false;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::WriteElement(HyperFile* pFile, const ArrayElement* pElement) const
{
	// override!
	DebugAssert(false);
	return false;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::ReadElementCreate(HyperFile* pFile, ArrayElement* &pElement, Int32 lLevel)
{
	// override!
	DebugAssert(false);
	return false;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::TestEqual(const ArrayElement* pObjA, const ArrayElement* pObjB) const
{
	// override!
	DebugAssert(false);
	return false;
}


template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::RemovePointer(Int32 lIndex)
{
	return m_Objects.RemoveAt(lIndex, true);
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::InsertObject(ArrayElement* pObj, Int32 lIndex)
{
	ArrayElement** ppNew = m_Objects.InsertAtA(lIndex);
	if (!ppNew)
		return false;
	*ppNew = pObj;
	return true;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::Init(const ArrayElement* const* ppObjects, Int32 lCount, AliasTrans* pTrans, Bool bAppend)
{
	if (!bAppend)
		Free();
	if (!ppObjects || !lCount)
		return true;
	m_Objects.SetMinSizeNoCopy(lCount);
	Int32 a;
	for (a = 0; a < lCount; a++)
	{
		ArrayElement* pClone = GetClone(ppObjects[a], pTrans);
		if (!pClone)
			return false;
		if (!m_Objects.Append(pClone))
		{
			FreeObject(pClone);
			return false;
		}
	}
	return true;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::CopyTo(CDynamicObjectArray <ArrayElement>* pDest, AliasTrans* pTrans, Bool bAppend) const
{
	if (!bAppend)
		pDest->Free();
	Int32 a, lElements;
	lElements = m_Objects.GetElementCount();
	const ArrayElement* const* ppElements = m_Objects.GetArray();
	pDest->m_Objects.SetMinSizeNoCopy(lElements);
	for (a = 0; a < lElements; a++)
	{
		if (!ppElements[a])
			continue;
		ArrayElement* pClone = GetClone(ppElements[a], pTrans);
		if (!pClone)
			return false;
		if (!pDest->m_Objects.Append(pClone))
		{
			FreeObject(pClone);
			return false;
		}
	}

	return true;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::IsEqual(const CDynamicObjectArray <ArrayElement>* pArray) const
{
	if (pArray->GetElementCount() != GetElementCount())
		return false;

	const ArrayElement * const * ppDest = pArray->GetArray()->GetArray(), *const *ppThis = GetArray()->GetArray();
	Int32 lElements = GetElementCount();
	for (Int32 a = 0; a < lElements; a++)
	{
		if (!ppDest[a] || !ppThis[a])
			return false;
		if (!TestEqual(ppThis[a], ppDest[a]))
			return false;
	}

	return true;
}

template<class ArrayElement> Bool CDynamicObjectArray<ArrayElement>::Alloc(Int32 lElementCount) // allocates memory to hold lElementCount elements more than currently, increases the counter
{
	Int32 lOldCounter = m_Objects.GetElementCount();
	if (!m_Objects.Alloc(lElementCount))
		return false;

	Int32 lNewCounter = lOldCounter + lElementCount;
	Int32 i;
	ArrayElement** ppDest = m_Objects.GetArray();
	for (i = lOldCounter; i < lNewCounter; i++)
	{
		ppDest[i] = AllocObject();
		if (!ppDest[i])
			return false;
	}

	m_Objects.SetCounter(lNewCounter);

	return true;
}


/*****************************************************************************\
*   CDynamicSortObjectSet                                                     *
\*****************************************************************************/

template <class ArrayElement> class CDynamicSortObjectSet : public CDynamicObjectArray<ArrayElement>
{
public:
	ArrayElement* Add(const ArrayElement* pNew, Bool &bExisted); // returns a new object
	ArrayElement* InsertObject(ArrayElement* pElement, Bool &bExisted); // inserts pElement into the array
	ArrayElement* InsertObject(ArrayElement* pElement, Bool &bExisted, Int32* plPos); // inserts pElement into the array
	Int32 GetObjectIndex(const ArrayElement* pObj) const;
	Bool RemoveObject(const ArrayElement* pObj);
	Bool RemoveObjectPointer(ArrayElement*& pObj);
	ArrayElement* SearchObject(const void* pData, Int32 *plPos = nullptr) const;

	typedef ArrayElement* ArrayElementPtr;
	static Int32 SortCompare(const ArrayElementPtr* a, const ArrayElementPtr* b, const void* pData);
	void Sort();
protected:
	virtual Int32 Compare(const ArrayElement* a, const ArrayElement* b) const = 0;
	virtual Int32 CompareSearch(const ArrayElement* a, const void* b) const;
};

template<class ArrayElement> ArrayElement* CDynamicSortObjectSet<ArrayElement>::Add(const ArrayElement* pElement, Bool &bExisted)
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	ArrayElement** ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0)
	{
		ArrayElement* pNew = GCC34T GetNextObject();
		bExisted = false;

		return pNew;
	}
	Int32 c, l = 0, r = lElements - 1;
	Int32 lInsert;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(ppElements[c], pElement);
		if (lComp == 0)
		{
			bExisted = true;
			return ppElements[c];
		}
		else if (lComp < 0)
		{
			lInsert = c + 1;
			l = c + 1;
		}
		else
		{
			lInsert = c;
			r = c - 1;
		}
	} while (l <= r);

	ArrayElement* pNew = GCC34T InsertAt(lInsert);
	if (!pNew)
		return nullptr;
	bExisted = false;

	return pNew;
}

template<class ArrayElement> ArrayElement* CDynamicSortObjectSet<ArrayElement>::InsertObject(ArrayElement* pElement, Bool &bExisted)
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	ArrayElement** ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0)
	{
		ArrayElement** ppNew = GCC34T m_Objects.InsertAtA(0);
		if (!ppNew)
			return nullptr;
		*ppNew = pElement;
		bExisted = false;
		return *ppNew;
	}
	Int32 c, l = 0, r = lElements - 1;
	Int32 lInsert;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(ppElements[c], pElement);
		if (lComp == 0)
		{
			bExisted = true;
			return ppElements[c];
		}
		else if (lComp < 0)
		{
			lInsert = c + 1;
			l = c + 1;
		}
		else
		{
			lInsert = c;
			r = c - 1;
		}
	} while (l <= r);

	ArrayElement** ppNew = GCC34T m_Objects.InsertAtA(lInsert);
	if (!ppNew)
	{
		return nullptr;
	}
	*ppNew = pElement;
	bExisted = false;

	return *ppNew;
}

template<class ArrayElement> ArrayElement* CDynamicSortObjectSet<ArrayElement>::InsertObject(ArrayElement* pElement, Bool &bExisted, Int32* plPos)
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	ArrayElement** ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0)
	{
		ArrayElement** ppNew = GCC34T m_Objects.InsertAtA(0);
		if (!ppNew)
			return nullptr;
		*plPos = 0;
		*ppNew = pElement;
		return *ppNew;
	}
	Int32 c, l = 0, r = lElements - 1;
	Int32 lInsert;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(ppElements[c], pElement);
		if (lComp == 0)
		{
			bExisted = true;
			return ppElements[c];
		}
		else if (lComp < 0)
		{
			lInsert = c + 1;
			l = c + 1;
		}
		else
		{
			lInsert = c;
			r = c - 1;
		}
	} while (l <= r);

	ArrayElement** ppNew = GCC34T m_Objects.InsertAtA(lInsert);
	if (!ppNew)
	{
		return nullptr;
	}
	*ppNew = pElement;
	bExisted = false;
	*plPos = lInsert;

	return *ppNew;
}

template<class ArrayElement> Int32 CDynamicSortObjectSet<ArrayElement>::GetObjectIndex(const ArrayElement* pObj) const
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	const ArrayElement* const* ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0 || ppElements == 0)
		return -1;

	Int32 l = 0, r = lElements - 1,  c;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(ppElements[c], pObj);

		if (lComp == 0)
		{
			return c;
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return -1;
}

template<class ArrayElement> ArrayElement* CDynamicSortObjectSet<ArrayElement>::SearchObject(const void* pData, Int32 *plPos) const
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	ArrayElement* const* ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0 || ppElements == 0)
		return nullptr;

	Int32 l = 0, r = lElements - 1,  c;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = CompareSearch(ppElements[c], pData);

		if (lComp == 0)
		{
			if (plPos)
				*plPos = c;
			return ppElements[c];
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return nullptr;
}

template<class ArrayElement> Bool CDynamicSortObjectSet<ArrayElement>::RemoveObject(const ArrayElement* pObj)
{
	Int32 lIndex = GetObjectIndex(pObj);
	if (lIndex < 0)
		return false;
	return GCC34T RemoveAt(lIndex);
}

template<class ArrayElement> Bool CDynamicSortObjectSet<ArrayElement>::RemoveObjectPointer(ArrayElement*& pObj)
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	for (Int32 l = 0; l < lElements; l++)
	{
		if (GCC34T m_Objects[l] == pObj)
		{
			GCC34T m_Objects.RemoveAt(l, false);
			this->FreeObject(pObj);
			return true;
		}
	}
	return false;
}

template<class ArrayElement> void CDynamicSortObjectSet<ArrayElement>::Sort()
{
	CShellSort <ArrayElement*> sort;
	ArrayElement** ppElements = GCC34T m_Objects.GetArray();
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	sort.Sort(ppElements, SortCompare, lElements, this);
}

template<class ArrayElement> Int32 CDynamicSortObjectSet<ArrayElement>::SortCompare(const ArrayElementPtr* a, const ArrayElementPtr* b, const void* pData)
{
	return ((CDynamicSortObjectSet<ArrayElement>*)pData)->Compare(*a, *b);
}

template<class ArrayElement> Int32 CDynamicSortObjectSet<ArrayElement>::CompareSearch(const ArrayElement* a, const void* b) const
{
	// override!
	DebugAssert(false);
	return Compare(a, (const ArrayElement*)b);
}





/*****************************************************************************\
*   CDynamicSortObjectArray                                                       *
\*****************************************************************************/

template <class ArrayElement> class CDynamicSortObjectArray : public CDynamicObjectArray<ArrayElement>
{
public:

	typedef ArrayElement* ArrayElementPtr;

	ArrayElement* Add(const ArrayElement* pNew); // returns a new object
	ArrayElement* InsertObject(ArrayElement* pElement); // inserts pElement into the array
	ArrayElement* InsertObjectInRange(ArrayElement* pElement, Int32 lLeft = NOTOK, Int32 lRight = NOTOK); // inserts pElement into the array
	Int32 GetObjectIndex(const ArrayElement* pObj) const;
	Int32 GetObjectIndexInRange(const ArrayElement* pObj, Int32 lLeft = NOTOK, Int32 lRight = NOTOK) const;
	Bool RemoveObject(const ArrayElement* pObj);
	ArrayElement* SearchObject(const void* pData, Int32 *plPos = nullptr) const;
	Bool GetBounds(const ArrayElement* pElement, Int32 &lLower, Int32 &lUpper) const;
	static Int32 SortCompare(const ArrayElementPtr *a, const ArrayElementPtr* b, const void* pData);
	void Sort();
protected:
	virtual Int32 Compare(const ArrayElement* a, const ArrayElement* b) const = 0;
	virtual Int32 CompareSearch(const ArrayElement* a, const void* b) const;
};

template<class ArrayElement> ArrayElement* CDynamicSortObjectArray<ArrayElement>::Add(const ArrayElement* pElement)
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	ArrayElement** ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0)
	{
		ArrayElement* pNew = GCC34T GetNextObject();
		return pNew;
	}
	Int32 c, l = 0, r = lElements - 1;
	Int32 lInsert;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(ppElements[c], pElement);
		if (lComp == 0)
		{
			lInsert = c;
			break;
		}
		else if (lComp < 0)
		{
			lInsert = c + 1;
			l = c + 1;
		}
		else
		{
			lInsert = c;
			r = c - 1;
		}
	} while (l <= r);

	ArrayElement* pNew = GCC34T InsertAt(lInsert);
	if (!pNew)
		return nullptr;

	return pNew;
}

template<class ArrayElement> ArrayElement* CDynamicSortObjectArray<ArrayElement>::InsertObject(ArrayElement* pElement)
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	ArrayElement** ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0)
	{
		ArrayElement** ppNew = GCC34T m_Objects.InsertAtA(0);
		if (!ppNew)
			return nullptr;
		*ppNew = pElement;
		return *ppNew;
	}
	Int32 c, l = 0, r = lElements - 1;
	Int32 lInsert;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(ppElements[c], pElement);
		if (lComp == 0)
		{
			lInsert = c;
			break;
		}
		else if (lComp < 0)
		{
			lInsert = c + 1;
			l = c + 1;
		}
		else
		{
			lInsert = c;
			r = c - 1;
		}
	} while (l <= r);


	ArrayElement** ppNew = GCC34T m_Objects.InsertAtA(lInsert);
	if (!ppNew)
	{
		return nullptr;
	}
	*ppNew = pElement;
	return *ppNew;
}

template<class ArrayElement> ArrayElement* CDynamicSortObjectArray<ArrayElement>::InsertObjectInRange(ArrayElement* pElement, Int32 lLeft, Int32 lRight)
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	ArrayElement** ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0)
	{
		ArrayElement** ppNew = GCC34T m_Objects.InsertAtA(0);
		if (!ppNew)
			return nullptr;
		*ppNew = pElement;
		return *ppNew;
	}
	Int32 c, l = (lLeft >= 0 && lLeft < lElements) ? lLeft : 0, r = (lRight >= 0 && lRight < lElements) ? lRight : (lElements - 1);
	Int32 lInsert;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(ppElements[c], pElement);
		if (lComp == 0)
		{
			lInsert = c;
			break;
		}
		else if (lComp < 0)
		{
			lInsert = c + 1;
			l = c + 1;
		}
		else
		{
			lInsert = c;
			r = c - 1;
		}
	} while (l <= r);


	ArrayElement** ppNew = GCC34T m_Objects.InsertAtA(lInsert);
	if (!ppNew)
	{
		return nullptr;
	}
	*ppNew = pElement;
	return *ppNew;
}

template<class ArrayElement> Int32 CDynamicSortObjectArray<ArrayElement>::GetObjectIndex(const ArrayElement* pObj) const
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	const ArrayElement* const* ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0 || ppElements == 0)
		return -1;

	Int32 l = 0, r = lElements - 1,  c;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(ppElements[c], pObj);

		if (lComp == 0)
		{
			return c;
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return -1;
}

template<class ArrayElement> Int32 CDynamicSortObjectArray<ArrayElement>::GetObjectIndexInRange(const ArrayElement* pObj, Int32 lLeft, Int32 lRight) const
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	const ArrayElement* const* ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0 || ppElements == 0)
		return -1;

	Int32 c, l = (lLeft >= 0 && lLeft < lElements) ? lLeft : 0, r = (lRight >= 0 && lRight < lElements) ? lRight : (lElements - 1);

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(ppElements[c], pObj);

		if (lComp == 0)
		{
			return c;
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return -1;
}

template<class ArrayElement> Bool CDynamicSortObjectArray<ArrayElement>::GetBounds(const ArrayElement* pElement, Int32 &lLower, Int32 &lUpper) const
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	ArrayElement** ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0 || ppElements == nullptr)
		return false;

	Int32 l = 0, r = lElements - 1,  c;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = Compare(ppElements[c], GCC34T pObj);

		if (lComp == 0)
		{
			lLower = lUpper = c;
			while (lLower > 0)
			{
				if (Compare(&GCC34T m_pElements[lLower - 1], pElement) == 0)
					lLower--;
				else
					break;
			}
			while (lUpper < GCC34T m_lElements - 1)
			{
				if (Compare(&GCC34T m_pElements[lUpper + 1], pElement) == 0)
					lUpper++;
				else
					break;
			}
			return true;
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return false;
}


template<class ArrayElement> ArrayElement* CDynamicSortObjectArray<ArrayElement>::SearchObject(const void* pData, Int32 *plPos) const
{
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	ArrayElement* const* ppElements = GCC34T m_Objects.GetArray();

	if (lElements == 0 || ppElements == 0)
		return nullptr;

	Int32 l = 0, r = lElements - 1,  c;

	do
	{
		c = (l + r) / 2;
		Int32 lComp = CompareSearch(ppElements[c], pData);

		if (lComp == 0)
		{
			if (plPos)
				*plPos = c;
			return ppElements[c];
		}
		else if (lComp < 0)
		{
			l = c + 1;
		}
		else
		{
			r = c - 1;
		}
	} while (l <= r);
	return nullptr;
}

template<class ArrayElement> Bool CDynamicSortObjectArray<ArrayElement>::RemoveObject(const ArrayElement* pObj)
{
	Int32 lIndex = GetObjectIndex(pObj);
	if (lIndex < 0)
		return false;
	return GCC34T RemoveAt(lIndex);
}

template<class ArrayElement> void CDynamicSortObjectArray<ArrayElement>::Sort()
{
	CShellSort <ArrayElement*> sort;
	ArrayElement ** ppElements = GCC34T m_Objects.GetArray();
	Int32 lElements = GCC34T m_Objects.GetElementCount();
	sort.Sort(ppElements, SortCompare, lElements, this);
}

template<class ArrayElement> Int32 CDynamicSortObjectArray<ArrayElement>::SortCompare(const ArrayElementPtr* a, const ArrayElementPtr* b, const void* pData)
{
	return ((CDynamicSortObjectArray<ArrayElement>*)pData)->Compare(*a, *b);
}

template<class ArrayElement> Int32 CDynamicSortObjectArray<ArrayElement>::CompareSearch(const ArrayElement* a, const void* b) const
{
	// override!
	DebugAssert(false);
	return Compare(a, (ArrayElement*)b);
}






/*****************************************************************************\
*   CObjectStack                                                              *
\*****************************************************************************/

template <class ArrayElement> class CObjectStack : public CDynamicObjectArray <ArrayElement>
{
public:
	CObjectStack();
	virtual ~CObjectStack();

	ArrayElement* Push(); // creates a new object on the stack
	ArrayElement* Pop(); // removes the object from the stack - free the object's memory on your own
	ArrayElement* Top(); // returns the topmost object
	Bool PushObject(const ArrayElement* pObj);
};

template<class ArrayElement> CObjectStack<ArrayElement>::CObjectStack() : CDynamicObjectArray <ArrayElement> ()
{
}

template<class ArrayElement> CObjectStack<ArrayElement>::~CObjectStack()
{
}

template<class ArrayElement> ArrayElement* CObjectStack<ArrayElement>::Push()
{
	return GCC34T GetNextObject();
}

template<class ArrayElement> Bool CObjectStack<ArrayElement>::PushObject(const ArrayElement* pObj)
{
	return GCC34T m_Objects.Append(pObj);
}

template<class ArrayElement> ArrayElement* CObjectStack<ArrayElement>::Pop()
{
	if (GCC34T m_Objects.GetElementCount() <= 0)
		return nullptr;

	Int32 lPos = GCC34T m_Objects.GetElementCount() - 1;
	ArrayElement* pObj = GCC34T GetObjectAt(lPos);
	GCC34T RemovePointer(lPos);
	return pObj;
}

template<class ArrayElement> ArrayElement* CObjectStack<ArrayElement>::Top()
{
	if (GCC34T m_Objects.GetElementCount() <= 0)
		return nullptr;

	Int32 lPos = GCC34T m_Objects.GetElementCount() - 1;
	ArrayElement* pObj = GCC34T GetObjectAt(lPos);
	return pObj;
}

/************************************************************************/
/* CObjectMap                                                           */
/************************************************************************/
class CDynamicObjectMapItem
{
public:
	virtual ~CDynamicObjectMapItem() { }
	String m_strKey;
};

class CDynamicObjectMap : public CDynamicSortObjectSet<CDynamicObjectMapItem>
{
public:
	virtual Int32 Compare(const CDynamicObjectMapItem* a, const CDynamicObjectMapItem* b) const
	{
		// override if necessary
		DebugAssert(false);
		return 0;
	}

	CDynamicObjectMapItem* GetObject(const String &strKey, Bool bCreate = false)
	{
		Int32 lElements = GCC34T m_Objects.GetElementCount();
		CDynamicObjectMapItem** ppElements = GCC34T m_Objects.GetArray();

		DebugAssert(!bCreate || (bCreate && strKey.Content()));

		if (lElements == 0)
		{
			if (bCreate)
			{
				CDynamicObjectMapItem* pNew = InsertAt(0);
				if (!pNew)
					return nullptr;
				pNew->m_strKey = strKey;
				return pNew;
			}
			return nullptr;
		}
		Int32 c, l = 0, r = lElements - 1;
		Int32 lInsert;

		do
		{
			c = (l + r) / 2;
			Int32 lComp = ppElements[c]->m_strKey.Compare(strKey);
			if (lComp == 0)
			{
				return ppElements[c];
			}
			else if (lComp < 0)
			{
				lInsert = c + 1;
				l = c + 1;
			}
			else
			{
				lInsert = c;
				r = c - 1;
			}
		} while (l <= r);

		if (bCreate)
		{
			CDynamicObjectMapItem* pNew = GCC34T InsertAt(lInsert);
			if (!pNew)
				return nullptr;
			pNew->m_strKey = strKey;

			return pNew;
		}
		return nullptr;
	}
};

/************************************************************************/
/* misc.                                                                */
/************************************************************************/
class CDynamicLRealArray : public CDynamicArray<Float64>
{
public:
	Bool Write(HyperFile* pFile) { return CDynamicArray<Float64>::Write(pFile, 0); }
private:
	virtual Bool ReadElements(HyperFile* pFile, Float64* pElements, Int32 lCount, Int32 lLevel);
	virtual Bool WriteElements(HyperFile* pFile, const Float64* pElements, Int32 lCount) const;
};

class CDynamicSRealArray : public CDynamicArray<Float32>
{
public:
	Bool Write(HyperFile* pFile) { return CDynamicArray<Float32>::Write(pFile, 0); }
private:
	virtual Bool ReadElements(HyperFile* pFile, Float32* pElements, Int32 lCount, Int32 lLevel);
	virtual Bool WriteElements(HyperFile* pFile, const Float32* pElements, Int32 lCount) const;
};

typedef CDynamicArray<Int32> CDynamicLongArray;
typedef CDynamicArray<UInt32> CDynamicULongArray;
typedef CDynamicArray<UInt16> CDynamicUWordArray;
typedef CDynamicArray<Vector> CDynamicVectorArray;
#ifdef __FLOAT_32_BIT
typedef CDynamicArray<Float32> CDynamicRealArray;
#else
typedef CDynamicArray<Float64> CDynamicRealArray;
#endif

class CDynamicSortLongSet : public CDynamicSortSet<Int32>
{
protected:
	virtual Bool ReadElements(HyperFile* pFile, Int32* pElements, Int32 lCount, Int32 lLevel);
	virtual Bool WriteElements(HyperFile* pFile, const Int32* pElements, Int32 lCount) const;
	virtual Int32 Compare(const Int32* a, const Int32* b) const
	{
		return *a - *b;
	}
};

class CDynamicSortULongSet : public CDynamicSortSet<UInt32>
{
protected:
	Int32 Compare(const UInt32* a, const UInt32* b) const
	{
		if (*a < *b) return -1;
		else if (*a > *b) return 1;
		return 0;
	}
};

class CDynamicSortLULongSet : public CDynamicSortSet<UInt64>
{
protected:
	Int32 Compare(const UInt64* a, const UInt64* b) const
	{
		if (*a < *b) return -1;
		else if (*a > *b) return 1;
		return 0;
	}
};

class CDynamicSortLongArray : public CDynamicSortArray<Int32>
{
protected:
	Int32 Compare(const Int32* a, const Int32* b) const
	{
		return *a - *b;
	}
	Int32 CompareSpecial(const Int32* a, const void* pSpecial) const
	{
		return *a - *((const Int32*)pSpecial);
	}
};

template <class TTTT> class CDynamicSortPointerSet : public CDynamicSortSet<TTTT*>
{
protected:
	typedef TTTT* TTT ;
	Int32 Compare(const TTT *a, const TTT* b) const
	{
		const void *l1 = *a;
		const void *l2 = *b;
		if (l1 < l2) return -1;
		else if (l1 > l2) return 1;
		else return 0;
	}
};

class CDynamicStringArray : public CDynamicObjectArray<String>
{
public:
	virtual ~CDynamicStringArray();
	Bool AddString(const String &str);
	String* GetClone(const String* pObj, AliasTrans* pTrans) const;
protected:
	virtual Bool ReadElement(BaseFile* pFile, String* pElement, Int32 lLevel);
	virtual Bool WriteElement(BaseFile* pFile, const String* pElement) const;
	virtual Bool ReadElement(HyperFile* pFile, String* pElement, Int32 lLevel);
	virtual Bool WriteElement(HyperFile* pFile, const String* pElement) const;
	String* AllocObject() const;
	void FreeObject(String *&pObj) const;
};

class CDynamicFilenameArray : public CDynamicObjectArray<Filename>
{
public:
	virtual ~CDynamicFilenameArray();
	Bool AddFilename(const Filename &fn);
	Filename* GetClone(const Filename* pObj, AliasTrans* pTrans) const;
	virtual Bool ReadElement(BaseFile* pFile, Filename* pElement, Int32 lLevel);
	virtual Bool WriteElement(BaseFile* pFile, const Filename* pElement) const;
	virtual Bool ReadElement(HyperFile* pFile, Filename* pElement, Int32 lLevel);
	virtual Bool WriteElement(HyperFile* pFile, const Filename* pElement) const;
	Bool TestEqual(const Filename* pObjA, const Filename* pObjB) const;
	Filename* AllocObject() const;
	void FreeObject(Filename *&pObj) const;
};

class CDynamicFilenameSet : public CDynamicSortObjectSet<Filename>
{
public:
	virtual ~CDynamicFilenameSet();
	Bool AddFilename(const Filename &fn);
	Filename* GetClone(const Filename* pObj, AliasTrans* pTrans) const;
	void RemoveFilename(const Filename &fn);
protected:
	virtual Filename* AllocObject() const;
	virtual void FreeObject(Filename *&pObj) const;
	virtual Int32 Compare(const Filename* a, const Filename* b) const;
	virtual Int32 CompareSearch(const Filename* a, const void* b) const;
	virtual Bool ReadElement(BaseFile* pFile, Filename* pElement, Int32 lLevel);
	virtual Bool WriteElement(BaseFile* pFile, const Filename* pElement) const;
	virtual Bool ReadElement(HyperFile* pFile, Filename* pElement, Int32 lLevel);
	virtual Bool WriteElement(HyperFile* pFile, const Filename* pElement) const;
	virtual Bool TestEqual(const Filename* pObjA, const Filename* pObjB) const;
};

class CDynamicStringSet : public CDynamicSortObjectSet<String>
{
public:
	virtual ~CDynamicStringSet();
	Bool AddString(const String &str);
	void RemoveString(const String &str);
	String* GetClone(const String* pObj, AliasTrans* pTrans) const;
	Bool TestEqual(const String* pObjA, const String* pObjB) const;
protected:
	virtual String* AllocObject() const;
	virtual void FreeObject(String *&pObj) const;
	virtual Int32 Compare(const String* a, const String* b) const;
	virtual Int32 CompareSearch(const String* a, const void* b) const;
	virtual Bool ReadElement(BaseFile* pFile, String* pElement, Int32 lLevel);
	virtual Bool WriteElement(BaseFile* pFile, const String* pElement) const;
	virtual Bool ReadElement(HyperFile* pFile, String* pElement, Int32 lLevel);
	virtual Bool WriteElement(HyperFile* pFile, const String* pElement) const;
};

template <class ArrayElement> class CBaseObjectArray : public CDynamicObjectArray<ArrayElement>
{
public:
	virtual ~CBaseObjectArray() { GCC34T Free(); }
protected:
	virtual ArrayElement* AllocObject() const { return ArrayElement::Alloc(); }
	virtual void FreeObject(ArrayElement *&pObj) const { ArrayElement::Free(pObj); }
};

#define AutoArray(aa_name, aa_type, aa_size) aa_type* aa_name = nullptr; CDynamicArray<aa_type> autoarray_##aa_name; autoarray_##aa_name.SetMinSize(aa_size); aa_name = autoarray_##aa_name.GetArray();
#define AutoArrayNC(aa_name, aa_type, aa_size) aa_type* aa_name = nullptr; CDynamicArray<aa_type> autoarray_##aa_name; autoarray_##aa_name.SetMinSizeNoCopy(aa_size, false); aa_name = autoarray_##aa_name.GetArray();

#ifdef __API_INTERN__
#undef DeprecatedMemoryPool
#undef AliasTrans
#endif // __API_INTERN__

#endif // _DYN_ARRAY_H_
