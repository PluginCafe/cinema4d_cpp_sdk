#ifdef __API_INTERN__
#include "ge_file.h"
#include "hyperfile.h"
#else
#include "ge_math.h"
#endif
#include "dynarray.h"

/************************************************************************/
/* CDynamicStringArray                                                  */
/************************************************************************/
CDynamicStringArray::~CDynamicStringArray()
{
	Free();
}

Bool CDynamicStringArray::AddString(const String &str)
{
	String* pStr = GetNextObject();
	if (!pStr)
		return false;
	*pStr = str;
	return true;
}

String* CDynamicStringArray::GetClone(const String* pObj, AliasTrans* pTrans) const
{
	return NewObjClear(String, *pObj);
}

Bool CDynamicStringArray::ReadElement(BaseFile* pFile, String* pElement, Int32 lLevel)
{
#ifdef __API_INTERN__
	Int32  len=0;
	Char	*c = nullptr;

	if (!pFile->ReadInt32(&len)) return false;
	if (!len) { *pElement=String(); return true; }

	c = NewMemClear(Char, len);
	if (!c)
	{
		return false;
	}
	if (!pFile->ReadBytes(c,len))
	{
		DeleteMem(c);
		return false;
	}

	pElement->SetCString(c,len-1);
	DeleteMem(c);

	return true;
#else
	return pFile->ReadString(pElement) != 0;
#endif // __API_INTERN__
}

Bool CDynamicStringArray::WriteElement(BaseFile* pFile, const String* pElement) const
{
#ifdef __API_INTERN__
	Bool ok;
	Int32 len  = pElement->GetCStringLen()+1;
	Char *mem = NewMemClear(Char, len);
	if (!mem)
	{
		return false;
	}
	pElement->GetCString(mem, len);
	ok = pFile->WriteInt32(len) && pFile->WriteBytes(mem,len);
	DeleteMem(mem);

	return ok;
#else
	return pFile->WriteString(*pElement) != 0;
#endif // __API_INTERN__
}

Bool CDynamicStringArray::ReadElement(HyperFile* pFile, String* pElement, Int32 lLevel)
{
	return pFile->ReadString(pElement);
}

Bool CDynamicStringArray::WriteElement(HyperFile* pFile, const String* pElement) const
{
	return pFile->WriteString(*pElement);
}

String* CDynamicStringArray::AllocObject() const
{
	return NewObjClear(String);
}

void CDynamicStringArray::FreeObject(String *&pObj) const
{
	DeleteObj(pObj);
}

/************************************************************************/
/* CDynamicFilenameArray                                                */
/************************************************************************/
CDynamicFilenameArray::~CDynamicFilenameArray()
{
	Free();
}

Bool CDynamicFilenameArray::AddFilename(const Filename &fn)
{
	Filename* pFn = GetNextObject();
	if (!pFn)
		return false;
	*pFn = fn;
	return true;
}

Filename* CDynamicFilenameArray::GetClone(const Filename* pObj, AliasTrans* pTrans) const
{
	return NewObjClear(Filename, *pObj);
}

Bool CDynamicFilenameArray::ReadElement(BaseFile* pFile, Filename* pElement, Int32 lLevel)
{
#ifdef __API_INTERN__
	Int32  len=0;
	Char	*c = nullptr;

	if (!pFile->ReadInt32(&len)) return false;
	if (!len) { *pElement = Filename(); return true; }

	c = NewMemClear(Char, len);
	if (!c)
	{
		return false;
	}
	if (!pFile->ReadBytes(c,len))
	{
		DeleteMem(c);
		return false;
	}

	String str;
	str.SetCString(c, len-1);
	*pElement = Filename(str);
	DeleteMem(c);

	return true;
#else
	return pFile->ReadFilename(pElement) != 0;
#endif // __API_INTERN__
}

Bool CDynamicFilenameArray::WriteElement(BaseFile* pFile, const Filename* pElement) const
{
#ifdef __API_INTERN__
	String str = pElement->GetString();
	Bool ok;
	Int32 len  = str.GetCStringLen()+1;
	Char *mem = NewMemClear(Char, len);
	if (!mem)
	{
		return false;
	}
	str.GetCString(mem, len);
	ok = pFile->WriteInt32(len) && pFile->WriteBytes(mem,len);
	DeleteMem(mem);

	return ok;
#else
	return pFile->WriteFilename(*pElement) != 0;
#endif // __API_INTERN__
}

Bool CDynamicFilenameArray::ReadElement(HyperFile* pFile, Filename* pElement, Int32 lLevel)
{
	return pFile->ReadFilename(pElement);
}

Bool CDynamicFilenameArray::WriteElement(HyperFile* pFile, const Filename* pElement) const
{
	return pFile->WriteFilename(*pElement);
}

Bool CDynamicFilenameArray::TestEqual(const Filename* pObjA, const Filename* pObjB) const
{
	return *pObjA == *pObjB;
}

Filename* CDynamicFilenameArray::AllocObject() const
{
	return NewObjClear(Filename);
}

void CDynamicFilenameArray::FreeObject(Filename *&pObj) const
{
	DeleteObj(pObj);
}

/************************************************************************/
/* CDynamicFilenameSet                                                  */
/************************************************************************/
CDynamicFilenameSet::~CDynamicFilenameSet()
{
	Free();
}

Bool CDynamicFilenameSet::AddFilename(const Filename &fn)
{
	Filename* pfn = NewObjClear(Filename, fn);
	if (!pfn)
		return false;
	Bool bExisted = false;
	if (!InsertObject(pfn, bExisted))
	{
		DeleteObj(pfn);
		return false;
	}
	if (bExisted)
		DeleteObj(pfn);
	return true;
}

Filename* CDynamicFilenameSet::GetClone(const Filename* pObj, AliasTrans* pTrans) const
{
	return NewObjClear(Filename, *pObj);
}

void CDynamicFilenameSet::RemoveFilename(const Filename &fn)
{
	RemoveObject(&fn);
}

Filename* CDynamicFilenameSet::AllocObject() const
{
	return NewObjClear(Filename);
}

void CDynamicFilenameSet::FreeObject(Filename *&pObj) const
{
	DeleteObj(pObj);
}

Int32 CDynamicFilenameSet::Compare(const Filename* a, const Filename* b) const
{
	return a->GetString().LexCompare(b->GetString());
}

Int32 CDynamicFilenameSet::CompareSearch(const Filename* a, const void* b) const
{
	return Compare(a, (const Filename*)b);
}

Bool CDynamicFilenameSet::ReadElement(BaseFile* pFile, Filename* pElement, Int32 lLevel)
{
#ifdef __API_INTERN__
	Int32  len=0;
	Char	*c = nullptr;

	if (!pFile->ReadInt32(&len)) return false;
	if (!len) { *pElement = Filename(); return true; }

	c = NewMemClear(Char, len);
	if (!c)
	{
		return false;
	}
	if (!pFile->ReadBytes(c,len))
	{
		DeleteMem(c);
		return false;
	}

	String str;
	str.SetCString(c, len-1);
	*pElement = Filename(str);
	DeleteMem(c);

	return true;
#else
	return pFile->ReadFilename(pElement) != 0;
#endif // __API_INTERN__
}

Bool CDynamicFilenameSet::WriteElement(BaseFile* pFile, const Filename* pElement) const
{
#ifdef __API_INTERN__
	String str = pElement->GetString();
	Bool ok;
	Int32 len  = str.GetCStringLen()+1;
	Char *mem = NewMemClear(Char, len);
	if (!mem)
	{
		return false;
	}
	str.GetCString(mem, len);
	ok = pFile->WriteInt32(len) && pFile->WriteBytes(mem,len);
	DeleteMem(mem);

	return ok;
#else
	return pFile->WriteFilename(*pElement) != 0;
#endif // __API_INTERN__
}

Bool CDynamicFilenameSet::ReadElement(HyperFile* pFile, Filename* pElement, Int32 lLevel)
{
	return pFile->ReadFilename(pElement);
}

Bool CDynamicFilenameSet::WriteElement(HyperFile* pFile, const Filename* pElement) const
{
	return pFile->WriteFilename(*pElement);
}

Bool CDynamicFilenameSet::TestEqual(const Filename* pObjA, const Filename* pObjB) const
{
	return *pObjA == *pObjB;
}

/************************************************************************/
/* CDynamicStringSet                                                    */
/************************************************************************/
CDynamicStringSet::~CDynamicStringSet()
{
	Free();
}

Bool CDynamicStringSet::AddString(const String &str)
{
	String* pstr = NewObjClear(String, str);
	if (!pstr)
		return false;
	Bool bExisted = false;
	if (!InsertObject(pstr, bExisted))
	{
		DeleteObj(pstr);
		return false;
	}
	if (bExisted)
		DeleteObj(pstr);
	return true;
}

void CDynamicStringSet::RemoveString(const String &str)
{
	RemoveObject(&str);
}

String* CDynamicStringSet::GetClone(const String* pObj, AliasTrans* pTrans) const
{
	return NewObjClear(String, *pObj);
}

Bool CDynamicStringSet::TestEqual(const String* pObjA, const String* pObjB) const
{
	return *pObjA == *pObjB;
}

String* CDynamicStringSet::AllocObject() const
{
	return NewObjClear(String);
}

void CDynamicStringSet::FreeObject(String *&pObj) const
{
	DeleteObj(pObj);
}

Int32 CDynamicStringSet::Compare(const String* a, const String* b) const
{
	return a->LexCompare(*b);
}

Int32 CDynamicStringSet::CompareSearch(const String* a, const void* b) const
{
	return Compare(a, (const String*)b);
}

Bool CDynamicStringSet::ReadElement(BaseFile* pFile, String* pElement, Int32 lLevel)
{
#ifdef __API_INTERN__
	Int32  len=0;
	Char	*c = nullptr;

	if (!pFile->ReadInt32(&len)) return false;
	if (!len) { *pElement=String(); return true; }

	c = NewMemClear(Char, len);
	if (!c)
	{
		return false;
	}
	if (!pFile->ReadBytes(c,len))
	{
		DeleteMem(c);
		return false;
	}

	pElement->SetCString(c,len-1);
	DeleteMem(c);

	return true;
#else
	return pFile->ReadString(pElement) != 0;
#endif // __API_INTERN__
}

Bool CDynamicStringSet::WriteElement(BaseFile* pFile, const String* pElement) const
{
#ifdef __API_INTERN__
	Bool ok;
	Int32 len  = pElement->GetCStringLen()+1;
	Char *mem = NewMemClear(Char, len);
	if (!mem)
	{
		return false;
	}
	pElement->GetCString(mem, len);
	ok = pFile->WriteInt32(len) && pFile->WriteBytes(mem,len);
	DeleteMem(mem);

	return ok;
#else
	return pFile->WriteString(*pElement) != 0;
#endif // __API_INTERN__
}

Bool CDynamicStringSet::ReadElement(HyperFile* pFile, String* pElement, Int32 lLevel)
{
	return pFile->ReadString(pElement);
}

Bool CDynamicStringSet::WriteElement(HyperFile* pFile, const String* pElement) const
{
	return pFile->WriteString(*pElement);
}

/************************************************************************/
/* CDynSRealArray                                                       */
/************************************************************************/
Bool CDynamicSRealArray::ReadElements(HyperFile* pFile, Float32* pElements, Int32 lCount, Int32 lLevel)
{
	void* pData;
	Int lDataSize;
	if (!pFile->ReadMemory(&pData, &lDataSize))
		return false;
	if (lDataSize != lCount * sizeof(Float32))
	{
		DebugAssert(false);
		return false;
	}
	CopyMem(pData, pElements, lDataSize);
	DeleteMem(pData);
	lIntel(pElements, lCount);
	return true;
}

Bool CDynamicSRealArray::WriteElements(HyperFile* pFile, const Float32* pElements, Int32 lCount) const
{
#if (defined __MAC && __BIG_ENDIAN__)
	if (lCount == 0)
		return false;
	void* pData = NewMem(Float32, lCount);
	if (!pData)
		return false;
	CopyMem(pElements, pData, lCount * sizeof(Float32));
	lIntel(pData, lCount);
	if (!pFile->WriteMemory(pData, sizeof(Float32) * lCount))
		return false;
#else
	if (!pFile->WriteMemory(pElements, sizeof(Float32) * lCount))
		return false;
#endif
	return true;
}

Bool CDynamicSortLongSet::ReadElements(HyperFile* pFile, Int32* pElements, Int32 lCount, Int32 lLevel)
{
	void* pData;
	Int lDataSize;
	if (!pFile->ReadMemory(&pData, &lDataSize))
		return false;
	if (lDataSize != lCount * sizeof(Int32))
	{
		DebugAssert(false);
		return false;
	}
	CopyMem(pData, pElements, lDataSize);
	DeleteMem(pData);
	lIntel(pElements, lCount);
	return true;
}

Bool CDynamicSortLongSet::WriteElements(HyperFile* pFile, const Int32* pElements, Int32 lCount) const
{
#if (defined __MAC && __BIG_ENDIAN__)
	if (lCount == 0)
		return false;
	void* pData = NewMem(Int32, lCount);
	if (!pData)
		return false;
	CopyMem(pElements, pData, lCount * sizeof(Int32));
	lIntel(pData, lCount);
	if (!pFile->WriteMemory(pData, sizeof(Int32) * lCount))
		return false;
#else
	if (!pFile->WriteMemory(pElements, sizeof(Int32) * lCount))
		return false;
#endif
	return true;
}

/************************************************************************/
/* helper functions                                                     */
/************************************************************************/
Bool CDynamicArrayWriteHeader(HyperFile* pFile, Int32 lElements, Int32 lLevel)
{
	return pFile->WriteInt32(lElements) &&
				 pFile->WriteInt32(lLevel);
}

Bool CDynamicArrayReadHeader(HyperFile* pFile, Int32& lElements, Int32& lLevel)
{
	return pFile->ReadInt32(&lElements) &&
				 pFile->ReadInt32(&lLevel);
}

Bool CDynamicArrayWriteHeader(BaseFile* pFile, Int32 lElements, Int32 lLevel)
{
	return pFile->WriteInt32(lElements) &&
				 pFile->WriteInt32(lLevel);
}

Bool CDynamicArrayReadHeader(BaseFile* pFile, Int32& lElements, Int32& lLevel)
{
	return pFile->ReadInt32(&lElements) &&
				 pFile->ReadInt32(&lLevel);
}
