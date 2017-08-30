#include "dynamicdata.h"

void *DeprecatedSysMemAlloc(Int s);
void DeprecatedSysMemFree(void *mem);

void *DeprecatedMemAllocUseNewMemInstead(Int s)
{
	return DeprecatedSysMemAlloc(s);
}

void DeprecatedMemFreeUseDeleteMemInstead(void **mem)
{
	if (*mem) DeprecatedSysMemFree(*mem);
	*mem=nullptr;
}

Int _Log2(Int num)
{
	Int b=0,d=16;

	while (d>0)
	{
		if (num>>(b+d)) b+=d;
		d>>=1;
	}

	return b;
}
