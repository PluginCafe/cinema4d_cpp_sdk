#include "c4d_memory.h"
#include "c4d_general.h"

void *DeprecatedSysMemAlloc(Int s)
{
	Int size = s + sizeof(Int);
	void *mem = (size > 0) ? NewMem(UChar, size) : nullptr;			// NC is equivalent to HeapAlloc( , 0, )
	if (!mem)
		return nullptr;
	*((Int*)mem)=s;													// ###REMOVE: there's no need to store the size anymore, call GeGetAllocSize()
	return (UChar*)mem+sizeof(Int);
}

void DeprecatedSysMemFree(void *mem)
{
	mem=(UChar*)mem-sizeof(Int);
	DeleteMem(mem);
}

