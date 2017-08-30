//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// PLEASE NOTE
// -----------
//
// ******* INTERNAL ONLY NOT FOR PUBLIC OR THIRD-PARTIES *******
//
// *** PLEASE DO NOT USE ANY CLASS FROM THIS HEADER ***
// ****** SOME OF THESE ARE POTENTIALLY LETHAL ******
//
// ANY CHANGES SHOULD BE VERIFIED WITH David O'Reilly
// CHANGES HERE AFFECT MANY AREAS OF THE CORE AND MODULES
//
// THANKS
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef __DYNAMIC_DATA_H__
#define __DYNAMIC_DATA_H__

#include "ge_math.h"

#ifndef __API_INTERN__
	#include "c4d_memory.h"
	#include "c4d_tools.h"
	#include "c4d_general.h"
	#include "c4d_baseselect.h"
#else
	#include "ge_memory.h"
	#include "baseselect.h"
	#include "ge_string.h"
	#include "ge_tools.h"
	#ifdef _DEBUG
	void GePrint(const String &str);
	#endif
#endif

#include <string.h>

#ifdef _DEBUG
	#include "listnode.h"
#endif

//////////////////////////////////////////////////////////////////////////

Int32 qwrap(Int32 x, Int32 l, Int32 u);

template <class TYPE> void FillMem(TYPE *pData, Int32 lCount, TYPE *pFill)
{
	if ( lCount > 0 )
	{
#ifdef _DEBUG
		FillMemType( TYPE, pData, lCount, 0xFF );
#endif
		CopyMemType(pFill, pData, 1 );

		lCount--;
		pFill=pData;
		pData+=1;

		Int32 lSize=1;

		while ( lCount > lSize )
		{
			CopyMemType(pFill, pData, lSize );
			lCount-=lSize;
			pData+=lSize;
			lSize+=lSize;
		}

		if ( lCount > 0 )
			CopyMemType(pFill, pData, lCount );
	}
}

template <class TYPE> void FillMem(TYPE *pData, Int32 lCount, const TYPE &pFill)
{
	FillMem(pData,lCount,(TYPE*)&pFill);
}

//////////////////////////////////////////////////////////////////////////

void *DeprecatedMemAllocUseNewMemInstead(Int s);
void DeprecatedMemFreeUseDeleteMemInstead(void **mem);
Int _Log2(Int num);

#define MemAlloc(x) ((GCC34T m_pMemStack)?GCC34T m_pMemStack->AllocS(x):NewMemClear(UChar, Int64(x)))
#define MemFree(x) ((GCC34T m_pMemStack)?GCC34T m_pMemStack->FreeS(x):DeleteMem(x))

//////////////////////////////////////////////////////////////////////////

//#define __DUMMY_MEMSTACK__

#ifndef __DUMMY_MEMSTACK__

#define MEMSTACK_FLAG_ENABLEGARBAGE		1
#define MEMSTACK_FLAG_AUTOCOLLECT		2
#define MEMSTACK_FLAG_DBLSIZE			4
#define MEMSTACK_FLAG_HLVSIZE			8

#define MEMSTACK_BINS 14
#define MEMSTACK_OFFSET 3

#define MEMSTACK_DEADSPACE_LEN	(16)
#define MEMSTACK_DEADSPACE_SIZE	(MEMSTACK_DEADSPACE_LEN*sizeof(Int32))

#ifdef _DEBUG
//#define _MEMSTACK_STATS_
//#define _MEMSTACK_CHECK_
//#define _MEMSTACK_NONZERO_
//#define _MEMSTACK_BOUNDARY_
#endif

#ifndef __API_INTERN__
#define _PrintStat(a) GePrint(String(#a)+String(" : ")+String::IntToString(a));
#define _PrintStatMem(a) GePrint(String(#a)+String(" : ")+String::MemoryToString(a));
#else
#define _PrintStat(a) GePrint(String(#a)+String(" : ")+String::IntToString(a)+String("\n"));
#define _PrintStatMem(a) GePrint(String(#a)+String(" : ")+String::MemoryToString(a)+String("\n"));
#endif

#ifdef _MEMSTACK_STATS_
#define _SetStat(a) a
#else
#define _SetStat(a)
#endif

class MemoryStack
{
private:

	struct MemoryGarbage
	{
		MemoryGarbage()
		{
			m_pStart=nullptr;
			m_pEnd=nullptr;
		}
		MemoryGarbage(void *s, Int32 l)
		{
			m_pStart=s;
			m_pEnd=(UChar*)s+l;
		}

		Bool operator<(const MemoryGarbage &rhs) const { return m_pStart<rhs.m_pStart; }
		Bool operator>(const MemoryGarbage &rhs) const { return m_pStart>rhs.m_pStart; }
		Bool operator<=(const MemoryGarbage &rhs) const { return m_pStart<=rhs.m_pStart; }
		Bool operator>=(const MemoryGarbage &rhs) const { return m_pStart>=rhs.m_pStart; }
		Bool operator==(const MemoryGarbage &rhs) const { return m_pStart==rhs.m_pStart; }

		MemoryGarbage &operator=(const MemoryGarbage &src) { m_pStart=src.m_pStart; m_pEnd=src.m_pEnd; return *this; }

		void *m_pStart;
		void *m_pEnd;
	};

	struct MemoryChunkHeader
	{
		MemoryChunkHeader() { m_pNext=nullptr; }
		MemoryChunkHeader *m_pNext;
	};

	struct MemoryWasteHeader
	{
		MemoryWasteHeader *m_pNext;
		Int m_Size;
	};

	struct MemoryBlockHeader
	{
#ifdef _MEMSTACK_STATS_
		Int32 m_Overhead;
#endif
#ifdef _MEMSTACK_CHECK_
		Int32 m_Sanity;
#endif
		MemoryBlockHeader *m_pNext;
		MemoryBlockHeader *m_pPrev;
		//////////////////////////////////////////////////////////////////////////
		Int m_Size;
	};

	struct MemoryRecycleHeader
	{
#ifdef _MEMSTACK_CHECK_
		Int32 m_Sanity;
		Int32 m_Size;
#endif
		MemoryRecycleHeader *m_pNext;
	};

	struct MemoryBin
	{
		MemoryBin()
		{
			Reset();
		}
		void Reset()
		{
			m_pNext=nullptr;
			m_pTail=nullptr;
		}

		MemoryBlockHeader *m_pNext;
		MemoryBlockHeader *m_pTail;
	};

public:

	MemoryStack()
	{
		m_pLastChunk=&m_Chunks;
		m_pLast=nullptr;
		m_Size=65536;
		m_Free=0;
		m_pRecycleBlocks=nullptr;
		m_pDeadChunks=nullptr;
		m_pLargeBlocks=nullptr;
		m_Flags=0;

#ifdef _MEMSTACK_STATS_
		m_LargeAllocated=0;
		m_MaxLargeAllocated=0;
		m_MaxAllocated=0;
		m_MaxWasteAllocated=0;
		m_MaxOverheadAllocated=0;
		m_MaxRequested=0;
		m_AllocateCalls=0;
		m_FreeCalls=0;
		m_ChunksAllocated=0;
		m_ChunkAllocCalls=0;
		m_OverheadAllocated=0;
		m_Allocated=0;
		m_UnsedCount=0;
		m_MaxUnused=0;
#endif
	}
	MemoryStack(Int32 size, Int32 flags=0)
	{
		m_pLastChunk=&m_Chunks;
		m_pLast=nullptr;
		m_Size=size;
		m_Free=0;
		m_pRecycleBlocks=nullptr;
		m_pDeadChunks=nullptr;
		m_pLargeBlocks=nullptr;
		m_Flags=flags;

#ifdef _MEMSTACK_STATS_
		m_LargeAllocated=0;
		m_MaxLargeAllocated=0;
		m_MaxAllocated=0;
		m_MaxWasteAllocated=0;
		m_MaxOverheadAllocated=0;
		m_MaxRequested=0;
		m_AllocateCalls=0;
		m_FreeCalls=0;
		m_ChunksAllocated=0;
		m_ChunkAllocCalls=0;
		m_OverheadAllocated=0;
		m_Allocated=0;
		m_UnsedCount=0;
		m_MaxUnused=0;
#endif
	}

	~MemoryStack()
	{
		Free();
	}

	void SetFlags(Int32 flags) { m_Flags=flags; }
	void SetSize(Int32 size) { m_Size=size; }

	void *Alloc(Int size)
	{
		size=(size+3)&(-4);

#ifdef _MEMSTACK_BOUNDARY_
		ValidateBoundaries();
#endif

		_SetStat(m_ChunkAllocCalls++);

#ifdef _MEMSTACK_STATS_

		Int32 wastecnt=0;
		MemoryWasteHeader *wn=m_pDeadChunks;
		while (wn)
		{
			wastecnt+=wn->m_Size;
			wn=wn->m_pNext;
		}

		if (wastecnt>m_MaxWasteAllocated) m_MaxWasteAllocated=wastecnt;
#endif

		if (size>m_Free)
		{
			if (m_pDeadChunks && m_Free<m_pDeadChunks->m_Size && m_pDeadChunks->m_Size>=size)
			{
				if (m_Free>((Int32)sizeof(MemoryWasteHeader)+32))
				{
					MemoryWasteHeader *waste=(MemoryWasteHeader*)m_pLast;
					waste->m_Size=m_Free;
					m_pLast=(UChar*)m_pDeadChunks;
					m_Free=m_pDeadChunks->m_Size;
					waste->m_pNext=m_pDeadChunks->m_pNext;
					m_pDeadChunks=waste;
				}
				else
				{
					m_pLast=(UChar*)m_pDeadChunks;
					m_Free=m_pDeadChunks->m_Size;
					m_pDeadChunks=m_pDeadChunks->m_pNext;
				}
			}
			else
			{
				if (m_Free>((Int32)sizeof(MemoryWasteHeader)+32))
				{
					MemoryWasteHeader *waste=(MemoryWasteHeader*)m_pLast;

					if (m_pDeadChunks && m_Free>m_pDeadChunks->m_Size)
					{
						waste->m_pNext=m_pDeadChunks->m_pNext;
						waste->m_Size=m_Free;
						m_pDeadChunks->m_pNext=waste;
					}
					else
					{
						waste->m_pNext=m_pDeadChunks;
						waste->m_Size=m_Free;
						m_pDeadChunks=waste;
					}
				}

				Int asize=m_Size;
				if (size>asize)
				{
					if (!(m_Flags & (MEMSTACK_FLAG_DBLSIZE|MEMSTACK_FLAG_HLVSIZE)))
						asize=size;
					else if (m_Flags & MEMSTACK_FLAG_DBLSIZE)
						m_Size=asize=asize*2;
					else if (m_Flags & MEMSTACK_FLAG_HLVSIZE)
						m_Size=asize=asize+asize/2;
					else
						asize=size;
				}

				_SetStat(m_ChunksAllocated+=asize);

				m_pLastChunk->m_pNext=(MemoryChunkHeader*)DeprecatedMemAllocUseNewMemInstead(asize+sizeof(MemoryChunkHeader));
				if (!m_pLastChunk->m_pNext) return nullptr;

				m_pLast=((UChar*)m_pLastChunk->m_pNext)+sizeof(MemoryChunkHeader);
				m_pLastChunk=m_pLastChunk->m_pNext;
				m_pLastChunk->m_pNext=nullptr;
				m_Free=asize;
			}
		}

		DebugAssert(m_Free>0);

		void *data=m_pLast;
		m_pLast+=size;
		m_Free-=size;

		DebugAssert(m_Free>=0);
#ifdef _MEMSTACK_NONZERO_
		ClearMem(data,size,0xFF);
#endif

		return data;
	}

	void Free()
	{
#ifdef _MEMSTACK_BOUNDARY_
		ValidateBoundaries();
#endif

		if (m_Chunks.m_pNext)
		{
			MemoryChunkHeader *next=m_Chunks.m_pNext,*chunk;
			while (next)
			{
				chunk=next;
				next=chunk->m_pNext;
				DeprecatedMemFreeUseDeleteMemInstead((void**)&chunk);
			}
		}

		if (m_pLargeBlocks)
		{
			MemoryBlockHeader *next=m_pLargeBlocks,*block;
			while (next)
			{
				block=next;
				next=block->m_pNext;
				DeprecatedMemFreeUseDeleteMemInstead((void**)&block);
			}
		}

		m_pLastChunk=&m_Chunks;
		m_pLast=nullptr;
		m_Free=0;
		m_pRecycleBlocks=nullptr;
		m_pLargeBlocks=nullptr;
		m_Chunks.m_pNext=nullptr;
		m_pDeadChunks=nullptr;

		for (Int32 i=0;i<MEMSTACK_BINS;i++) { m_Bin[i].Reset(); }

#ifdef _MEMSTACK_STATS_
		m_LargeAllocated=0;
		m_MaxLargeAllocated=0;
		m_MaxAllocated=0;
		m_MaxWasteAllocated=0;
		m_MaxOverheadAllocated=0;
		m_MaxRequested=0;
		m_AllocateCalls=0;
		m_FreeCalls=0;
		m_ChunksAllocated=0;
		m_ChunkAllocCalls=0;
		m_OverheadAllocated=0;
		m_Allocated=0;
		m_UnsedCount=0;
		m_MaxUnused=0;
#endif

#ifdef _MEMSTACK_CHECK_
		m_PMemPtrs.FlushAll();
		m_SMemPtrs.FlushAll();
		m_LMemPtrs.FlushAll();
#endif
	}

	void DumpStats()
	{
#ifdef _MEMSTACK_STATS_
		_PrintStatMem(m_ChunksAllocated);
		_PrintStat(m_ChunkAllocCalls);
		_PrintStat(m_AllocateCalls);
		_PrintStat(m_FreeCalls);
		_PrintStatMem(m_MaxRequested);
		_PrintStatMem(m_MaxAllocated);
		_PrintStatMem(m_MaxLargeAllocated);
		_PrintStatMem(m_MaxUnused);
		_PrintStatMem(m_MaxWasteAllocated);
		_PrintStatMem(m_MaxOverheadAllocated);
#endif
	}

	Bool IsStats()
	{
#ifdef _MEMSTACK_STATS_
		return IsAllocated();
#else
		return false;
#endif
	}

	Bool IsAllocated()
	{
		return m_Chunks.m_pNext!=nullptr;
	}

	void *AllocP(Int32 size)
	{
		MemoryRecycleHeader *block;

		size=(size+3)&(-4);

#ifdef _MEMSTACK_BOUNDARY_
		ValidateBoundaries();
		size+=2*MEMSTACK_DEADSPACE_SIZE;
#endif

		if (m_pRecycleBlocks)
		{
#ifdef _MEMSTACK_CHECK_
			DebugAssert(m_pRecycleBlocks->m_Size==size);
#endif
			block=m_pRecycleBlocks;
			m_pRecycleBlocks=m_pRecycleBlocks->m_pNext;
		}
		else
		{
			block=(MemoryRecycleHeader*)Alloc(size+sizeof(MemoryRecycleHeader));
			if (!block) return nullptr;
#ifdef _MEMSTACK_CHECK_
			block->m_Size=size;
#endif
		}

#ifdef _MEMSTACK_CHECK_
		block->m_Sanity=0xFEEDBACA;
		m_PMemPtrs.Add((UChar*)block+sizeof(MemoryRecycleHeader));
#endif

#ifdef _MEMSTACK_NONZERO_
		ClearMem((UChar*)block+sizeof(MemoryRecycleHeader),size,0xFF);
#endif

#ifdef _MEMSTACK_BOUNDARY_
		Int32 fill=0xDEADBEEF;
		FillMem((Int32*)((UChar*)block+sizeof(MemoryRecycleHeader)),MEMSTACK_DEADSPACE_LEN,&fill);
		FillMem((Int32*)((UChar*)block+sizeof(MemoryRecycleHeader)+size-MEMSTACK_DEADSPACE_SIZE),MEMSTACK_DEADSPACE_LEN,&fill);
		block=(MemoryRecycleHeader*)((UChar*)block+MEMSTACK_DEADSPACE_SIZE);
#endif

		return (UChar*)block+sizeof(MemoryRecycleHeader);
	}

	void FreeP(void *mem)
	{
		if (!mem) return;

#ifdef _MEMSTACK_BOUNDARY_
		mem=(UChar*)mem-MEMSTACK_DEADSPACE_SIZE;
		ValidateBoudary(mem);
		ValidateBoundaries();
#endif

#ifdef _MEMSTACK_CHECK_
		DebugAssert(m_PMemPtrs.Find(mem)!=nullptr);
		m_PMemPtrs.Find(mem)->Remove();
#endif

		mem=(UChar*)mem-sizeof(MemoryRecycleHeader);

#ifdef _MEMSTACK_BOUNDARY_
		ValidateBoudary((UChar*)mem+((MemoryRecycleHeader*)mem)->m_Size+sizeof(MemoryRecycleHeader)-MEMSTACK_DEADSPACE_SIZE);
#endif
#ifdef _MEMSTACK_CHECK_
		DebugAssert(((MemoryRecycleHeader*)mem)->m_Sanity==0xFEEDBACA);
#endif

		if (!m_pRecycleBlocks)
		{
			m_pRecycleBlocks=(MemoryRecycleHeader*)mem;
			m_pRecycleBlocks->m_pNext=nullptr;
		}
		else
		{
			((MemoryRecycleHeader*)mem)->m_pNext=m_pRecycleBlocks;
			m_pRecycleBlocks=(MemoryRecycleHeader*)mem;
		}
	}

	Int GetSizeBin(Int size)
	{
		Int b=_Log2(size)-MEMSTACK_OFFSET;
		if (b<0) b=0;
		else if (b>=MEMSTACK_BINS) b=MEMSTACK_BINS-1;
		return b;
	}

	void *AllocS(Int size)
	{
		MemoryBlockHeader *block=nullptr;
		MemoryBin *bin;
		Int b;

		_SetStat(m_AllocateCalls++);
		_SetStat(m_MaxRequested=LMax(m_MaxRequested,size));

		size=(size+3)&(-4);

#ifdef _MEMSTACK_BOUNDARY_
		ValidateBoundaries();
		size+=2*MEMSTACK_DEADSPACE_SIZE;
#endif

		b=GetSizeBin(size);
		bin=&m_Bin[b];

		if (bin->m_pNext)
		{
			if (size<=bin->m_pNext->m_Size)
			{
				block=bin->m_pNext;
				bin->m_pNext=bin->m_pNext->m_pNext;
				if (bin->m_pNext)
					bin->m_pNext->m_pPrev=nullptr;
				else
					bin->m_pTail=nullptr;
			}
			else if (size<=bin->m_pTail->m_Size)
			{
				block=bin->m_pTail;
				bin->m_pTail->m_pPrev->m_pNext=nullptr;
				bin->m_pTail=bin->m_pTail->m_pPrev;
			}
			else if (b<(MEMSTACK_BINS-1) && m_Bin[b+1].m_pNext)
			{
				bin=&m_Bin[b+1];

				block=bin->m_pNext;
				bin->m_pNext=bin->m_pNext->m_pNext;
				if (bin->m_pNext)
					bin->m_pNext->m_pPrev=nullptr;
				else
					bin->m_pTail=nullptr;
			}
		}
		else if (b<(MEMSTACK_BINS-1) && m_Bin[b+1].m_pNext)
		{
			bin=&m_Bin[b+1];

			block=bin->m_pNext;
			bin->m_pNext=bin->m_pNext->m_pNext;
			if (bin->m_pNext)
				bin->m_pNext->m_pPrev=nullptr;
			else
				bin->m_pTail=nullptr;
		}

#ifdef _DEBUG
		if (bin->m_pNext || bin->m_pTail)
		{
			DebugAssert(bin->m_pNext!=nullptr);
			DebugAssert(bin->m_pTail!=nullptr);

			if (bin->m_pNext!=bin->m_pTail)
			{
				DebugAssert(bin->m_pNext->m_pPrev==nullptr);
				DebugAssert(bin->m_pTail->m_pNext==nullptr);
				DebugAssert(bin->m_pNext->m_pNext!=nullptr);
				DebugAssert(bin->m_pTail->m_pPrev!=nullptr);
			}
			else
			{
				DebugAssert(bin->m_pNext->m_pNext==nullptr);
				DebugAssert(bin->m_pNext->m_pPrev==nullptr);
			}
		}
#endif

		if (!block)
		{
			block=(MemoryBlockHeader*)Alloc(size+sizeof(MemoryBlockHeader));
			if (!block) return nullptr;
			block->m_Size=size;

#ifdef _MEMSTACK_CHECK_
			block->m_Sanity=0xFEEDBACB;
#endif
		}
#ifdef _MEMSTACK_STATS_
		else
		{
			m_UnsedCount-=block->m_Size;
		}

		block->m_Overhead=block->m_Size-size+sizeof(MemoryBlockHeader);
		m_OverheadAllocated+=block->m_Overhead;
		m_Allocated+=block->m_Size;

		if (m_OverheadAllocated>m_MaxOverheadAllocated) m_MaxOverheadAllocated=m_OverheadAllocated;
		if (m_Allocated>m_MaxAllocated) m_MaxAllocated=m_Allocated;
#endif

#ifdef _MEMSTACK_CHECK_
		m_SMemPtrs.Add((UChar*)block+sizeof(MemoryBlockHeader));
#endif
#ifdef _MEMSTACK_NONZERO_
		ClearMem((UChar*)block+sizeof(MemoryBlockHeader),size,0xFF);
#endif

#ifdef _MEMSTACK_BOUNDARY_
		Int32 fill=0xDEADBEEF;
		FillMem((Int32*)((UChar*)block+sizeof(MemoryBlockHeader)),MEMSTACK_DEADSPACE_LEN,&fill);
		FillMem((Int32*)((UChar*)block+sizeof(MemoryBlockHeader)+size-MEMSTACK_DEADSPACE_SIZE),MEMSTACK_DEADSPACE_LEN,&fill);
		block=(MemoryBlockHeader*)((UChar*)block+MEMSTACK_DEADSPACE_SIZE);
#endif
		return (UChar*)block+sizeof(MemoryBlockHeader);
	}

	void FreeS(void *mem)
	{
		if (!mem) return;

#ifdef _MEMSTACK_BOUNDARY_
		mem=(UChar*)mem-MEMSTACK_DEADSPACE_SIZE;
		ValidateBoudary(mem);
		ValidateBoundaries();
#endif

#ifdef _MEMSTACK_CHECK_
		DebugAssert(m_SMemPtrs.Find(mem)!=nullptr);
		m_SMemPtrs.Find(mem)->Remove();
#endif

		_SetStat(m_FreeCalls++);

		mem=(UChar*)mem-sizeof(MemoryBlockHeader);

#ifdef _MEMSTACK_BOUNDARY_
		ValidateBoudary((UChar*)mem+((MemoryBlockHeader*)mem)->m_Size+sizeof(MemoryBlockHeader)-MEMSTACK_DEADSPACE_SIZE);
#endif
#ifdef _MEMSTACK_CHECK_
		DebugAssert(((MemoryBlockHeader*)mem)->m_Sanity==0xFEEDBACB);
#endif

		MemoryBlockHeader *block=(MemoryBlockHeader*)mem;
		MemoryBin *bin;

#ifdef _MEMSTACK_STATS_
		m_OverheadAllocated-=block->m_Overhead;
		m_Allocated-=block->m_Size;
		m_UnsedCount+=block->m_Size;

		if (m_UnsedCount>m_MaxUnused) m_MaxUnused=m_UnsedCount;
#endif

		/*
		if (m_Flags & MEMSTACK_FLAG_ENABLEGARBAGE)
		{
			m_Collector.Add(MemoryGarbage(block,block->m_Size+sizeof(MemoryBlockHeader)));
		}
		*/

		bin=&m_Bin[GetSizeBin(block->m_Size)];

		if (!bin->m_pNext)
		{
			bin->m_pTail=bin->m_pNext=block;
			block->m_pPrev=nullptr;
			block->m_pNext=nullptr;
		}
		else if (block->m_Size<=bin->m_pNext->m_Size)
		{
			bin->m_pNext->m_pPrev=block;
			block->m_pPrev=nullptr;
			block->m_pNext=bin->m_pNext;
			bin->m_pNext=block;
		}
		else if (block->m_Size>=bin->m_pTail->m_Size)
		{
			bin->m_pTail->m_pNext=block;
			block->m_pPrev=bin->m_pTail;
			block->m_pNext=nullptr;
			bin->m_pTail=block;
		}
		else if (block->m_Size<=((bin->m_pNext->m_Size+bin->m_pTail->m_Size)>>1))
		{
			block->m_pPrev=bin->m_pNext;
			block->m_pNext=bin->m_pNext->m_pNext;
			bin->m_pNext->m_pNext->m_pPrev=block;
			bin->m_pNext->m_pNext=block;
		}
		else
		{
			block->m_pNext=bin->m_pTail;
			block->m_pPrev=bin->m_pTail->m_pPrev;
			bin->m_pTail->m_pPrev->m_pNext=block;
			bin->m_pTail->m_pPrev=block;
		}

#ifdef _DEBUG
		if (bin->m_pNext || bin->m_pTail)
		{
			DebugAssert(bin->m_pNext!=nullptr);
			DebugAssert(bin->m_pTail!=nullptr);

			if (bin->m_pNext!=bin->m_pTail)
			{
				DebugAssert(bin->m_pNext->m_pPrev==nullptr);
				DebugAssert(bin->m_pTail->m_pNext==nullptr);
				DebugAssert(bin->m_pNext->m_pNext!=nullptr);
				DebugAssert(bin->m_pTail->m_pPrev!=nullptr);
			}
			else
			{
				DebugAssert(bin->m_pNext->m_pNext==nullptr);
				DebugAssert(bin->m_pNext->m_pPrev==nullptr);
			}
		}
#endif

		if (m_Flags & MEMSTACK_FLAG_AUTOCOLLECT) CollectGarbage();
	}

#ifdef _MEMSTACK_BOUNDARY_
	void ValidateBoudary(void *mem)
	{
		Int32 *data=(Int32*)mem;
		for (Int32 i=0;i<MEMSTACK_DEADSPACE_LEN;i++)
		{
			DebugAssert(data[i]==0xDEADBEEF);
		}
	}

	void ValidateBoundaries()
	{
		Int32 b=0;
		for (b=0;b<MEMSTACK_BINS;b++)
		{
			MemoryBlockHeader *block=m_Bin[b].m_pNext;
			while (block)
			{
				DebugAssert(block->m_Sanity==0xFEEDBACB);
				ValidateBoudary((UChar*)block+sizeof(MemoryBlockHeader));
				ValidateBoudary((UChar*)block+sizeof(MemoryBlockHeader)+block->m_Size-MEMSTACK_DEADSPACE_SIZE);
				block=block->m_pNext;
			}
		}

		MemoryRecycleHeader *rnext=m_pRecycleBlocks;
		while (rnext)
		{
			DebugAssert(rnext->m_Sanity==0xFEEDBACA);
			ValidateBoudary((UChar*)rnext+sizeof(MemoryRecycleHeader));
			ValidateBoudary(rnext+sizeof(MemoryRecycleHeader)+rnext->m_Size-MEMSTACK_DEADSPACE_SIZE);
			rnext=rnext->m_pNext;
		}

		MemoryBlockHeader *block=m_pLargeBlocks;
		while (block)
		{
			DebugAssert(block->m_Sanity==0xFEEDBACC);
			ValidateBoudary((UChar*)block+sizeof(MemoryBlockHeader));
			ValidateBoudary((UChar*)block+sizeof(MemoryBlockHeader)+block->m_Size-MEMSTACK_DEADSPACE_SIZE);
			block=block->m_pNext;
		}

		sDataNode<void*> *pn;

		pn=m_PMemPtrs.GetFirst();
		while (pn)
		{
			rnext=(MemoryRecycleHeader*)((UChar*)pn->m_Data-sizeof(MemoryRecycleHeader));
			DebugAssert(rnext->m_Sanity==0xFEEDBACA);
			ValidateBoudary((UChar*)rnext+sizeof(MemoryRecycleHeader));
			ValidateBoudary(rnext+sizeof(MemoryRecycleHeader)+rnext->m_Size-MEMSTACK_DEADSPACE_SIZE);
			pn=pn->GetNext();
		}
		pn=m_SMemPtrs.GetFirst();
		while (pn)
		{
			block=(MemoryBlockHeader*)((UChar*)pn->m_Data-sizeof(MemoryBlockHeader));
			DebugAssert(block->m_Sanity==0xFEEDBACB);
			ValidateBoudary((UChar*)block+sizeof(MemoryBlockHeader));
			ValidateBoudary((UChar*)block+sizeof(MemoryBlockHeader)+block->m_Size-MEMSTACK_DEADSPACE_SIZE);
			pn=pn->GetNext();
		}
		pn=m_LMemPtrs.GetFirst();
		while (pn)
		{
			block=(MemoryBlockHeader*)((UChar*)pn->m_Data-sizeof(MemoryBlockHeader));
			DebugAssert(block->m_Sanity==0xFEEDBACC);
			ValidateBoudary((UChar*)block+sizeof(MemoryBlockHeader));
			ValidateBoudary((UChar*)block+sizeof(MemoryBlockHeader)+block->m_Size-MEMSTACK_DEADSPACE_SIZE);
			pn=pn->GetNext();
		}
	}

#endif
	void CollectGarbage()
	{
		return;
/*
		if (m_Collector.GetCount()==0) return;

		m_Collector.Sort();

		Int32 i,s=0,l,t=0,m=0;
		void *ptr=m_Collector[0].m_pEnd,*sptr;
		MemoryBlockHeader *block;
		MemoryBin *bin;
		MemoryWasteHeader *waste;

		for (i=1;i<m_Collector.GetCount();i++)
		{
			if (m_Collector[i].m_pStart!=ptr || i==(m_Collector.GetCount()-1))
			{
				if (s<i)
				{
					sptr=m_Collector[s].m_pStart;
					t=(UChar*)m_Collector[i].m_pEnd-(UChar*)sptr;
				}
				if (s<i && t>=(sizeof(MemoryWasteHeader)+32))
				{
					for (l=s;l<=i;l++)
					{
						block=(MemoryBlockHeader*)m_Collector[l].m_pStart;
						bin=nullptr;

						if (block->m_pPrev)
							block->m_pPrev->m_pNext=block->m_pNext;
						else
						{
							bin=&m_Bin[GetSizeBin(block->m_Size)];
							bin->m_pNext=block->m_pNext;
						}

						if (block->m_pNext)
							block->m_pNext->m_pPrev=block->m_pPrev;
						else
						{
							if (!bin) bin=&m_Bin[GetSizeBin(block->m_Size)];
							bin->m_pTail=block->m_pPrev;
						}

						m_Collector[l].m_pStart=nullptr;
					}

					if (sptr)
					{
						waste=(MemoryWasteHeader*)sptr;

						if (m_pDeadChunks && t>m_pDeadChunks->m_Size)
						{
							waste->m_pNext=m_pDeadChunks->m_pNext;
							waste->m_Size=t;
							m_pDeadChunks->m_pNext=waste;
						}
						else
						{
							waste->m_pNext=m_pDeadChunks;
							waste->m_Size=t;
							m_pDeadChunks=waste;
						}
					}

					m++;
				}
				s=i+1;
				ptr=nullptr;
			}
			else
				ptr=m_Collector[i].m_pEnd;
		}

		if (m>0)
		{
			for (i=0;i<m_Collector.GetCount();)
			{
				if (m_Collector[i].m_pStart==nullptr)
					m_Collector.Remove(i);
				else
					i++;
			}
		}*/
	}

private:

	MemoryChunkHeader m_Chunks;
	MemoryChunkHeader *m_pLastChunk;
	MemoryWasteHeader *m_pDeadChunks;
	UChar *m_pLast;

	Int m_Size;
	Int m_Free;

	Int32 m_Flags;

	//////////////////////////////////////////////////////////////////////////

	MemoryBlockHeader *m_pLargeBlocks;

	//////////////////////////////////////////////////////////////////////////

	MemoryRecycleHeader *m_pRecycleBlocks;

	//////////////////////////////////////////////////////////////////////////

	MemoryBin	m_Bin[MEMSTACK_BINS];

	//////////////////////////////////////////////////////////////////////////

//	DynamicArray<MemoryGarbage,8> m_Collector;

	//////////////////////////////////////////////////////////////////////////

#ifdef _MEMSTACK_STATS_

	Int32 m_MaxAllocated;
	Int32 m_MaxLargeAllocated;
	Int32 m_MaxWasteAllocated;
	Int32 m_MaxOverheadAllocated;
	Int32 m_MaxUnused;

	Int32 m_OverheadAllocated;
	Int32 m_Allocated;
	Int32 m_LargeAllocated;
	Int32 m_UnsedCount;

	Int32 m_MaxRequested;

	Int32 m_ChunkAllocCalls;
	Int32 m_AllocateCalls;
	Int32 m_FreeCalls;

	Int32 m_ChunksAllocated;

#endif

#ifdef _MEMSTACK_CHECK_
	sDataNode<void*> m_PMemPtrs;
	sDataNode<void*> m_SMemPtrs;
	sDataNode<void*> m_LMemPtrs;
#endif
};

#undef MEMSTACK_BINS
#undef MEMSTACK_OFFSET

#else

class MemoryStack
{
public:

	MemoryStack()
	{
	}
	MemoryStack(Int32 size)
	{
	}
	~MemoryStack()
	{
	}

	void *Alloc(Int32 size)
	{
		return m_Pool->MemAllocS(size);
	}

	void Free()
	{
	}

	void *AllocP(Int32 size)
	{
		return m_Pool->MemAllocS(size);
	}

	void FreeP(void *mem)
	{
		m_Pool->MemFreeS(mem);
	}

	void *AllocS(Int32 size)
	{
		return m_Pool->MemAllocS(size);
	}

	void FreeS(void *mem)
	{
		m_Pool->MemFreeS(mem);
	}

	Bool IsStats()
	{
		return false;
	}

	void DumpStats()
	{
	}

	AutoAlloc<DeprecatedMemoryPool> m_Pool;
};

#endif

//////////////////////////////////////////////////////////////////////////

template <class TYPE> Int32 BinaryUpperSearch(const TYPE &v, TYPE *pData, Int32 lCount)
{
	if (lCount==0 || v<=pData[0]) return 0;
	if (v>pData[lCount-1]) return lCount;
	if (v==pData[lCount-1]) return lCount-1;

	Int32 l,u,m;

	l=1;
	u=lCount-2;

	while (l<u)
	{
		m=(l+u)>>1;
		if (v>=pData[m])
			l=m+1;
		else
			u=m-1;
	}

	if (l>0 && pData[l-1]==v)
		l--;
	else if (pData[l]<v)
		l++;

	return l;
}

template <class TYPE> Int32 BinarySearch(const TYPE &v, TYPE *pData, Int32 lCount, Bool first=false)
{
	if (lCount==0 || v<pData[0]) return NOTOK;
	if (v>pData[lCount-1]) return NOTOK;

	Int32 l,u,m;

	l=0;
	u=lCount-1;

	while (l<u)
	{
		m=(l+u)>>1;
		if (v>=pData[m])
			l=m+1;
		else
			u=m-1;
	}

	if (pData[l]==v) {}
	else if (l>0 && pData[l-1]==v)
		l--;
	else
		return NOTOK;

	if (first)
	{
		while (l>0 && pData[l-1]==v) { l--; }
	}

	return l;
}

template <class TYPE> void ShellSort(TYPE* pArray, Int32 lNumItems)
{
	Int32 i, j, h;
	TYPE v;

	for (h = 1; h <= lNumItems / 9; h = 3 * h + 1) { }
	for (; h > 0; h /= 3)
	{
		for (i = h; i < lNumItems; i++)
		{
			v = pArray[i];
			j = i;
			while (j > h - 1 && pArray[j-h]>v)
			{
				pArray[j] = pArray[j - h]; j -= h;
			}
			pArray[j] = v;
		}
	}
}

template <class TYPE, class LTYPE> void ShellSortLinked(TYPE* pArray, LTYPE *lArray, Int32 lNumItems)
{
	Int32 i, j, h;
	TYPE v;
	LTYPE w;

	for (h = 1; h <= lNumItems / 9; h = 3 * h + 1) { }
	for (; h > 0; h /= 3)
	{
		for (i = h; i < lNumItems; i++)
		{
			v = pArray[i];
			w = lArray[i];

			j = i;
			while (j > h - 1 && pArray[j - h]>v)
			{
				pArray[j] = pArray[j - h];
				lArray[j] = lArray[j - h];

				j -= h;
			}
			pArray[j] = v;
			lArray[j] = w;
		}
	}
}

template <class TYPE, Int32 BASE> void BucketSort(TYPE *pArray, Int32 lNumItems, MemoryStack *mem=nullptr, TYPE **rArray=nullptr)
{
	Int32 i,k,r,no_of_passes=0,divisor=1,pass_no;
	Int32 buck_count[BASE];

	TYPE *bucket[BASE];
	TYPE largest,smallest;

	TYPE *pNewArray;

	if (rArray) *rArray=nullptr;

	if (mem)
		pNewArray=(TYPE*)mem->AllocS(sizeof(TYPE)*lNumItems);
	else
		pNewArray=NewMemClear(TYPE,lNumItems);

	if (!pNewArray)
	{
		ShellSort<TYPE>(pArray,lNumItems);
		return;
	}

	smallest=largest=pArray[0];
	for(i=1;i<lNumItems;i++)  //Find the largest Number
	{
		if (pArray[i]<smallest)
			smallest=pArray[i];
		else if (pArray[i]>largest)
			largest=pArray[i];
	}

	Int32 passv=largest/1L,offv=smallest/1L;

	if (offv>0) offv=0;

	passv-=offv;
	while (passv>0)  //Find number of digits in largest number
	{
		no_of_passes++;
		passv /= BASE;
	}

	TYPE *sArray=pArray, *dArray=pNewArray;

	for (pass_no=0; pass_no < no_of_passes; pass_no++)
	{
		ClearMemType(buck_count, BASE );

		for (i=0;i<lNumItems;i++)
		{
			r=((sArray[i]-offv)/divisor)%BASE;
			buck_count[r]++;
		}

		bucket[0]=dArray;
		for (k=1; k<BASE; k++) bucket[k]=bucket[k-1]+buck_count[k-1];

		ClearMemType(buck_count, BASE );

		for (i=0;i<lNumItems;i++)
		{
			r=((sArray[i]-offv)/divisor)%BASE;
			bucket[r][buck_count[r]++]=sArray[i];
		}

		divisor *= BASE;
		pArray=dArray; dArray=sArray; sArray=pArray;
	}

	if (sArray==pNewArray)
	{
		if (rArray)
		{
			*rArray=pNewArray;
			pNewArray=nullptr;
		}
		else
			CopyMem(pNewArray,dArray,sizeof(TYPE)*lNumItems);
	}

	if (mem)
		mem->FreeS(pNewArray);
	else
		DeleteMem(pNewArray);
}

/*/////////////////////////////////////////////////////////////////////////

	class DynamicArrayBase
	----------------------

	* DataTYPE used MUST be safe for memory copying and moving, including the default constructed data!

/////////////////////////////////////////////////////////////////////////*/

template <class TYPE, Int32 BLOCK> class DynamicArrayBase
{
protected:

	TYPE	*m_pData;
	Int32	m_Cnt;			// total data size
	Int32	m_Allocated;	// actual allocated elements
	MemoryStack *m_pMemStack;

	DynamicArrayBase() { m_pMemStack=nullptr; Reset(); }
	void Reset() { m_pData=nullptr; m_Cnt=0; m_Allocated=0; }

	Bool Add()
	{
		if (m_Allocated>=m_Cnt)
		{
			Int32 ncnt;
			TYPE *data=nullptr;

			if (m_Cnt==0)
				ncnt = BLOCK;
			else
				ncnt = m_Cnt*2;

			data = (TYPE*)MemAlloc(sizeof(TYPE)*ncnt);
			if (!data) return false;

			if (m_pData) CopyMem(m_pData,data,m_Allocated*sizeof(TYPE));
			MemFree(m_pData);

			m_pData=data;
			m_Cnt=ncnt;
		}

		TYPE def;
		CopyMem(&def,&m_pData[m_Allocated],sizeof(TYPE));

		m_Allocated++;

		return true;
	}

	void InitElements(Int32 cnt)
	{
		TYPE def;
		FillMem(&m_pData[m_Allocated],cnt-m_Allocated,&def);
		m_Allocated=cnt;
	}

	void FreeElements()
	{
		for (Int32 i=0;i<m_Allocated;i++)
		{
			m_pData[i].~TYPE();
		}
		m_Allocated=0;
	}

public:
	~DynamicArrayBase() { Free(); }

	void Attach(MemoryStack *pMem) { Free(); m_pMemStack=pMem; }
	Int GetMemoryUse() { return sizeof(TYPE)*m_Cnt; }
	Int32 GetMaxCount() { return m_Cnt; }

	TYPE &operator[](int i) { DebugAssert(i>=0 && i<m_Allocated); return m_pData[i]; }
	TYPE operator[](int i) const { DebugAssert(i>=0 && i<m_Allocated); return m_pData[i]; }

	const DynamicArrayBase<TYPE,BLOCK>& operator = (const DynamicArrayBase<TYPE,BLOCK> &src)
	{
		((DynamicArrayBase*)&src)->CopyTo(this);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////

	Bool Insert(Int32 i)
	{
		DebugAssert(i>=0 && i<=m_Cnt);

		if (m_Allocated>=m_Cnt)
		{
			Int32 ncnt;
			TYPE *data=nullptr;

			if (m_Cnt==0)
				ncnt = BLOCK;
			else
				ncnt = m_Cnt*2;

			data = (TYPE*)MemAlloc(sizeof(TYPE)*ncnt);
			if (!data) return false;

			if (m_pData)
			{
				if (i>0) CopyMem(m_pData,data,i*sizeof(TYPE));
				if (i<m_Allocated) CopyMem((Char*)m_pData+i*sizeof(TYPE),(Char*)data+(i+1)*sizeof(TYPE),(m_Allocated-i)*sizeof(TYPE));
			}
			MemFree(m_pData);

			m_pData=data;
			m_Cnt=ncnt;
		}
		else
		{
			if (i<m_Allocated) memmove((Char*)m_pData+(i+1)*sizeof(TYPE),(Char*)m_pData+i*sizeof(TYPE),(m_Allocated-i)*sizeof(TYPE));
		}

		TYPE def;
		CopyMem(&def,&m_pData[i],sizeof(TYPE));

		m_Allocated++;

		return true;
	}

	Bool RemoveLast() { if (m_Allocated>0) return Remove(m_Allocated-1); else return true; }

	Bool Remove(Int32 i)
	{
		DebugAssert(i>=0 && i<=m_Cnt);

		m_pData[i].~TYPE();

		if (m_Allocated<=(m_Cnt>>1) && m_Cnt>BLOCK)
		{
			Int32 ncnt=m_Cnt>>1;
			TYPE *data=nullptr;

			if (ncnt == 0)
				return false;
			data = (TYPE*)MemAlloc(sizeof(TYPE)*ncnt);
			if (!data)
				return false;

			if (m_pData)
			{
				if (i>0) CopyMem(m_pData,data,i*sizeof(TYPE));
				if (i<(m_Allocated-1)) CopyMem((Char*)m_pData+(i+1)*sizeof(TYPE),(Char*)data+i*sizeof(TYPE),(m_Allocated-i-1)*sizeof(TYPE));
			}
			MemFree(m_pData);

			m_pData=data;
			m_Cnt=ncnt;
		}
		else
		{
			if (i<(m_Allocated-1)) memmove((Char*)m_pData+i*sizeof(TYPE),(Char*)m_pData+(i+1)*sizeof(TYPE),(m_Allocated-i-1)*sizeof(TYPE));
		}

		m_Allocated--;

		return true;
	}

	TYPE *GetLast()
	{
		if (m_Allocated==0) return nullptr;
		return &m_pData[m_Allocated-1];
	}
	TYPE *GetAt(Int32 i) { DebugAssert(i>=0 && i<m_Allocated); return &m_pData[i]; }
	void Set(Int32 i, const TYPE &data) { DebugAssert(i>=0 && i<m_Allocated); m_pData[i]=data; }
	TYPE &Get(Int32 i) { DebugAssert(i>=0 && i<m_Allocated); return m_pData[i]; }

	Int32 Find(const TYPE &data, Int32 l, Int32 u)
	{
//		DebugAssert(l>=0 && l<m_Allocated);
//		DebugAssert(u>=0 && u<m_Allocated);

		if (m_Allocated==0 || l>u) return NOTOK;	// FIX[54420]

		l=LCut(l,0,m_Allocated-1);	// FIX[51767]
		u=LCut(u,0,m_Allocated-1);

		Int32 i;
		for (i=l;i<=u;i++) { if (m_pData[i]==data) return i; }

		return NOTOK;
	}

	Int32 Find(const TYPE &data)
	{
		for (Int32 i=0;i<m_Allocated;i++) { if (m_pData[i]==data) return i; }
		return NOTOK;
	}

	Int32 FindSorted(const TYPE &data)
	{
		return BinarySearch(data,m_pData,m_Allocated,true);
	}

	Bool SetCount(Int32 cnt)
	{
		if (cnt>m_Cnt)
		{
			if (!Expand(cnt)) return false;
		}

		if (cnt<m_Allocated)
		{
			for (Int32 i=cnt;i<m_Allocated;i++) { m_pData[i].~TYPE(); }
			m_Allocated=cnt;
		}
		else if (cnt>m_Allocated)
		{
			InitElements(cnt);
		}

		return true;
	}
	Int32 GetCount() const { return m_Allocated; }
	Bool Content() const { return m_Allocated>0; }
	Bool IsEmpty() const { return GetCount()==0; }
	Bool Expand(Int32 maxcnt)
	{
		if (maxcnt>m_Cnt)
		{
			Int32 ncnt;
			TYPE *data=nullptr;

			ncnt=maxcnt;
			if (ncnt == 0)
				return false;
			data=(TYPE*)MemAlloc(sizeof(TYPE)*ncnt);
			if (!data)
				return false;

			if (m_pData) CopyMem(m_pData,data,m_Allocated*sizeof(TYPE));
			MemFree(m_pData);

			m_pData=data;
			m_Cnt=ncnt;
		}

		return true;
	}

	Bool CopyTo(DynamicArrayBase<TYPE,BLOCK> *dst) const
	{
		if (dst==this) return true;

		if (!dst->SetCount(m_Allocated)) return false;
		for (Int32 i=0;i<m_Allocated;i++) { dst->m_pData[i]=m_pData[i]; }

		return true;
	}

	Bool CopyFrom(TYPE *data, Int32 cnt)
	{
		if (!SetCount(cnt)) return false;
		for (Int32 i=0;i<m_Allocated;i++) { m_pData[i]=data[i]; }

		return true;
	}
	Bool CopyFrom(const DynamicArrayBase<TYPE,BLOCK> *src) { return src->CopyTo(this); }

	Bool Init(Int32 cnt=NOTOK)
	{
		Free();
		if (cnt==NOTOK) cnt=BLOCK;
		return Expand(cnt);
	}
	void Free() { FreeElements(); MemFree(m_pData); Reset(); }

	void TransferTo(DynamicArrayBase<TYPE,BLOCK> &dst)
	{
		dst.Free();
		dst.m_pMemStack=m_pMemStack;
		dst.m_pData=m_pData;
		dst.m_Cnt=m_Cnt;
		dst.m_Allocated=m_Allocated;
		Reset();
		m_pMemStack=nullptr;
	}

	TYPE *GetArray() { return m_pData; }
	const TYPE *GetArray() const { return m_pData; }
	TYPE *Release()
	{
		DebugAssert(!m_pMemStack);
		if (m_pMemStack) return nullptr;
		TYPE *tmpdata=m_pData;
		Reset();
		return tmpdata;
	}

	void Sort()
	{
		Int32 i, j, h;
		TYPE v;

		for (h = 1; h <= m_Allocated / 9; h = 3 * h + 1) { }
		for (; h > 0; h /= 3)
		{
			for (i = h; i < m_Allocated; i++)
			{
				v = m_pData[i];
				j = i;
				while (j > h - 1 && (m_pData[j - h]>v))
				{
					m_pData[j] = m_pData[j - h]; j -= h;
				}
				m_pData[j] = v;
			}
		}
	}

	TYPE *ToArray()
	{
		if (m_Allocated==0) return nullptr;

		TYPE *data=(TYPE*)MemAlloc(sizeof(TYPE)*m_Allocated);
		if (!data) return nullptr;

		CopyMem(m_pData,data,sizeof(TYPE)*m_Allocated);

		return data;
	}
};

//////////////////////////////////////////////////////////////////////////

// elements can be moved in memory so pointers are not valid and TYPE must be safe with this and a nullptr init

template <class TYPE, Int32 BLOCK> class DynamicArray : public DynamicArrayBase<TYPE,BLOCK>
{
public:
	DynamicArray()
	{
	}

	const DynamicArray<TYPE,BLOCK>& operator = (const DynamicArray<TYPE,BLOCK> &src)
	{
		((DynamicArray*)&src)->CopyTo(this);
		return *this;
	}

	Bool Add() { return DynamicArrayBase<TYPE,BLOCK>::Add(); }
	Bool Add(const TYPE &v)
	{
		if (!DynamicArrayBase<TYPE,BLOCK>::Add()) return false;
		this->Set(GCC34T m_Allocated-1,v);
		return true;
	}
	TYPE *AddLast()
	{
		if (!DynamicArrayBase<TYPE,BLOCK>::Add()) return nullptr;
		return DynamicArrayBase<TYPE,BLOCK>::GetLast();
	}
};

template <class TYPE, Int32 BLOCK> class StackArray : public DynamicArray<TYPE,BLOCK>
{
public:
	Bool Push(const TYPE &v) { return this->Add(v); }
	Bool Pop(TYPE &v)
	{
		if (GCC34T m_Allocated==0) return false;
		v=GCC34T Get(GCC34T m_Allocated-1);
		return GCC34T RemoveLast();
	}
	Bool Pull(TYPE &v)
	{
		if (GCC34T m_Allocated==0) return false;
		v=GCC34T Get(0);
		return GCC34T Remove(0);
	}
};

/*/////////////////////////////////////////////////////////////////////////
NOTES: for the Search, 'v' must always be on the left for the SelectArray operators
/////////////////////////////////////////////////////////////////////////*/

class SelectArray;
template <class TYPE> class SelectArrayExt;

template <class TYPE, Int32 BLOCK> class SortedArrayEx : public DynamicArrayBase<TYPE,BLOCK>
{
	friend class SelectArray;
	friend class SelectArrayExt<TYPE>;

protected:


	Int32 SearchIndex(const TYPE &v)
	{
		if (GCC34T m_Allocated==0 || v<=GCC34T m_pData[0]) return 0;
		if (v>=GCC34T m_pData[GCC34T m_Allocated-1]) return GCC34T m_Allocated;

		Int32 l,u,m;

		l=1; u=GCC34T m_Allocated-2;

		while (l<u)
		{
			m=(l+u)>>1;
			if (v>=GCC34T m_pData[m]) l=m+1;
			else u=m-1;
		}

		if (l>0 && v==GCC34T m_pData[l-1]) l--;
		else if (v>GCC34T m_pData[l]) l++;

		return l;
	}

	Int32 SearchEqualIndex(const TYPE &v)
	{
		if (GCC34T m_Allocated==0 || v<=GCC34T m_pData[0]) return 0;
		if (v>GCC34T m_pData[GCC34T m_Allocated-1]) return GCC34T m_Allocated;
		if (v==GCC34T m_pData[GCC34T m_Allocated-1]) return GCC34T m_Allocated-1;

		Int32 l,u,m;

		l=1; u=GCC34T m_Allocated-2;

		while (l<u)
		{
			m=(l+u)>>1;
			if (v>=GCC34T m_pData[m]) l=m+1;
			else u=m-1;
		}

		if (l>0 && v==GCC34T m_pData[l-1]) l--;
		else if (v>GCC34T m_pData[l]) l++;

		return l;
	}

public:

	SortedArrayEx()
	{
	}

	const SortedArrayEx<TYPE,BLOCK>& operator = (const SortedArrayEx<TYPE,BLOCK> &src)
	{
		((SortedArrayEx*)&src)->CopyTo(this);
		return *this;
	}

	Bool AddLast()
	{
		return DynamicArrayBase<TYPE,BLOCK>::Add();
	}

	Bool AddLast(const TYPE &v)
	{
		if (!DynamicArrayBase<TYPE,BLOCK>::Add()) return false;
		this->Set(GCC34T m_Allocated-1,v);
		return true;
	}

	Bool Add(const TYPE &data)
	{
		Int32 i=SearchIndex(data);
		if (!DynamicArrayBase<TYPE,BLOCK>::Insert(i)) return false;
		this->Set(i,data);
		return true;
	}

	Bool AddUnique(const TYPE &v)
	{
		if (GCC34T m_Allocated>0)
		{
			Int32 i=SearchIndex(v);
			if (i==GCC34T m_Allocated && v==GCC34T m_pData[i-1]) return true;
			if (i<GCC34T m_Allocated && v==GCC34T m_pData[i]) return true;
			if (!DynamicArrayBase<TYPE,BLOCK>::Insert(i)) return false;
			this->Set(i,v);
			return true;
		}
		else
			return Add(v);
	}

	TYPE* GetUnique(const TYPE &v, Bool &bExisted)
	{
		if (GCC34T m_Allocated>0)
		{
			Int32 i=SearchIndex(v);
			if (i==GCC34T m_Allocated && v==GCC34T m_pData[i-1]) { bExisted = true; return &(GCC34T m_pData[i - 1]); }
			if (i<GCC34T m_Allocated && v==GCC34T m_pData[i]) { bExisted = true; return &(GCC34T m_pData[i]); }
			if (!DynamicArrayBase<TYPE,BLOCK>::Insert(i)) return nullptr;
			this->Set(i,v);
			bExisted = false;
			return &(GCC34T m_pData[i]);
		}

		bExisted = false;
		if (!Add(v)) return nullptr;
		return &(GCC34T m_pData[0]);
	}

	Bool AddUnique(const TYPE &v, Bool &set)
	{
		if (GCC34T m_Allocated>0)
		{
			Int32 i=SearchIndex(v);

			set=false;
			if (i==GCC34T m_Allocated && v==GCC34T m_pData[i-1]) return true;
			if (i<GCC34T m_Allocated && v==GCC34T m_pData[i]) return true;
			if (!DynamicArrayBase<TYPE,BLOCK>::Insert(i)) return false;
			this->Set(i,v);
			set=true;
			return true;
		}
		else
		{
			set=true;
			return Add(v);
		}
	}

	Bool RemoveValue(const TYPE &v)
	{
		Int32 i=Find(v);
		if (i==NOTOK) return true;
		return GCC34T Remove(i);
	}

	Bool RemoveValue(const TYPE &v, Bool &set)
	{
		Int32 i=Find(v);
		if (i==NOTOK)
		{
			set=false;
			return true;
		}
		set=true;
		return GCC34T Remove(i);
	}

	Int32 Find(const TYPE &v)
	{
		if (!GCC34T m_pData || GCC34T m_Allocated==0) return NOTOK;

		if (v<GCC34T m_pData[0] || v>GCC34T m_pData[GCC34T m_Allocated-1]) return NOTOK;
		if (v==GCC34T m_pData[0]) return 0;
		if (v==GCC34T m_pData[GCC34T m_Allocated-1]) return GCC34T m_Allocated-1;

		Int32 i=SearchIndex(v);

		if (v==GCC34T m_pData[i]) return i;
		return NOTOK;
	}

	Int32 FindValueIndex(const TYPE &v)
	{
		Int32 i=SearchIndex(v);
		if (GCC34T m_Allocated==0) return 0;
		if (i==GCC34T m_Allocated && v==GCC34T m_pData[i-1]) i--;
		return i;
	}
};

struct ArrayLoopData
{
	Int32 lower;
	Int32 upper;
	void *data;
};

#define ARRAYLOOP_FLAGS_EXIT (1<<0)

template <class TYPE> class ArrayLoop
{
protected:

	Int32 m_Lower,m_Upper;	// array bounds
	Int32 m_Index;	// current index

	Int32 m_Start,m_End;	// loop start/end index (m_End can be less than m_Start, wrapping or -1 increment)

	TYPE *m_pData;	// data ptr, must be kept refreshed if dynamic
	Int32 m_Inc;	// index increment (-1 or 1 only!)

	UInt32 m_Flags;

public:
	ArrayLoop() { m_Start=m_End=m_Index=m_Upper=m_Lower=NOTOK; m_pData=nullptr; m_Inc=1; m_Flags=0; }
	~ArrayLoop() {}

	void Init(const ArrayLoopData &data)
	{
		m_Inc=1;
		m_Start=m_Index=m_Lower=data.lower;
		m_End=m_Upper=data.upper;
		m_pData=(TYPE*)data.data;
		m_Flags=0;
	}

	void Init(const ArrayLoopData &data, Int32 s, Int32 e)
	{
		m_Inc=1;
		m_End=e;
		m_Start=m_Index=s;
		m_Lower=data.lower;
		m_Upper=data.upper;
		m_pData=(TYPE*)data.data;
		m_Flags=0;
	}

	void Init(Int32 s, Int32 l, TYPE *data)
	{
		m_Inc=1;
		m_Start=m_Index=m_Lower=s;
		m_End=m_Upper=s+l-1;
		m_pData=data;
		m_Flags=0;
	}

	Bool Next()
	{
		Bool bCont=!(m_Flags & ARRAYLOOP_FLAGS_EXIT);
		m_Index+=m_Inc;
		if (m_Index==m_End) m_Flags|=ARRAYLOOP_FLAGS_EXIT;

		return bCont;
	}

	void Prev()	// use SetDn(false) for backwards looping
	{
		m_Index-=m_Inc;
	}

	Bool NextWrap()
	{
		Bool bCont=!(m_Flags & ARRAYLOOP_FLAGS_EXIT);
		m_Index=qwrap(m_Index+m_Inc,m_Lower,m_Upper);
		if (m_Index==m_End) m_Flags|=ARRAYLOOP_FLAGS_EXIT;

		return bCont;
	}

	void PrevWrap()
	{
		m_Index=qwrap(m_Index-m_Inc,m_Lower,m_Upper);
	}

	Int32 GetNextWrapIndex()
	{
		return qwrap(m_Index+m_Inc,m_Lower,m_Upper);
	}

	Int32 GetPrevWrapIndex()
	{
		return qwrap(m_Index-m_Inc,m_Lower,m_Upper);
	}

	TYPE Get()
	{
		DebugAssert(m_Upper>=m_Lower && m_Index>=m_Lower && m_Index<=m_Upper);
		return m_pData[m_Index];
	}

	TYPE GetNextWrap()
	{
		DebugAssert(m_Upper>=m_Lower && m_Index>=m_Lower && m_Index<=m_Upper);
		return m_pData[qwrap(m_Index+m_Inc,m_Lower,m_Upper)];
	}
	TYPE GetPrevWrap()
	{
		DebugAssert(m_Upper>=m_Lower && m_Index>=m_Lower && m_Index<=m_Upper);
		return m_pData[qwrap(m_Index-m_Inc,m_Lower,m_Upper)];
	}

	TYPE *GetAt()
	{
		DebugAssert(m_Upper>=m_Lower && m_Index>=m_Lower && m_Index<=m_Upper);
		return &m_pData[m_Index];
	}

	Int32 GetIndex()
	{
		return m_Index;
	}

	void Reset()
	{
		m_Index=m_Start;
		m_Flags&=~ARRAYLOOP_FLAGS_EXIT;
	}

	void SetLoop(Int32 s, Int32 e)
	{
		m_Start=m_Index=s;
		m_End=e;
		m_Flags&=~ARRAYLOOP_FLAGS_EXIT;
	}

	void SetLoop(Int32 s, Int32 e, Bool dn)
	{
		SetLoop(s,e);
		m_Inc=(dn)?1:-1;
	}

	void SetIndex(Int32 i)
	{
		m_Index=i;
		m_Flags&=~ARRAYLOOP_FLAGS_EXIT;
		if (m_Index==m_End) m_Flags|=ARRAYLOOP_FLAGS_EXIT;
	}

	void SetStartIndex(Int32 i)
	{
		m_Start=i;
		m_Flags&=~ARRAYLOOP_FLAGS_EXIT;
		DebugAssert(m_Start>=m_Lower && m_Start<=m_Upper);
	}

	void SetEndIndex(Int32 i)
	{
		m_End=i;
		m_Flags&=~ARRAYLOOP_FLAGS_EXIT;
		DebugAssert(m_End>=m_Lower && m_End<=m_Upper);
	}

	Bool IsExit()
	{
		return (m_Flags & ARRAYLOOP_FLAGS_EXIT);
	}

	Bool IsStart() { return m_Index==m_Start; }
	Bool IsEnd() { return m_Index==m_End; }

	void SetDn(Bool dn) { m_Inc=(dn)?1:-1; }

	Int32 GetLower() { return m_Lower; }
	Int32 GetUpper() { return m_Upper; }
	Int32 GetBoundsLength() { return m_Upper-m_Lower+1; }
	Int32 GetLoopLength()
	{
		if (m_End>=m_Start) return m_End-m_Start+1;
		return m_Upper-m_Start+m_End-m_Lower+2;
	}

	Int32 GetStartIndex() { return m_Start; }
	Int32 GetEndIndex() { return m_End; }

	void SetArray(TYPE *data) { m_pData=data; }

	void ShiftDown(TYPE *data)
	{
		m_Lower--;
		m_Upper--;
		m_Index--;
		m_Start--;
		m_End--;
		m_pData=data;
	}
	void ShiftUp(TYPE *data)
	{
		m_Lower++;
		m_Upper++;
		m_Index++;
		m_Start++;
		m_End++;
		m_pData=data;
	}

	Int32 GetWrapCount(Bool dn)
	{
		if (dn)
		{
			if (m_End>=m_Start)
				return m_End-m_Start+1;
			else
				return m_Upper-m_Start+m_End-m_Lower+2;
		}
		else
		{
			if (m_End<=m_Start)
				return m_Start-m_End+1;
			else
				return m_Start-m_Lower+m_Upper-m_End+2;
		}
	}

	void RemoveIndex(Int32 oi, TYPE *narray, Bool wrap)
	{
		if (oi==NOTOK) oi=m_Index;

		if (m_Start==m_End && oi==m_Start)
		{
			DebugAssert(false);	// check caller handles this condition safely
			m_Flags|=ARRAYLOOP_FLAGS_EXIT;	// loop exit
		}

		DebugAssert(oi>=m_Lower && oi<=m_Upper);	// only valid in this range

		// sanity checks

		DebugAssert(m_Lower<=m_Start);
		DebugAssert(m_Lower<=m_Upper);
		DebugAssert(m_Lower<=m_End);
		DebugAssert(m_Upper>=m_End);

		if (wrap)
		{
			if (m_Index==oi) m_Index=qwrap(m_Index+m_Inc,m_Lower,m_Upper);	// move to next valid index
			if (m_Start==oi) m_Start=qwrap(m_Start+m_Inc,m_Lower,m_Upper);
			if (m_End==oi) m_End=qwrap(m_End-m_Inc,m_Lower,m_Upper);
		}
		else
		{
			if (m_Index==oi) m_Index+=m_Inc;
			if (m_Start==oi) m_Start+=m_Inc;
			if (m_End==oi) m_End-=m_Inc;
		}

		if (m_Index>oi) m_Index--;	// shift down
		if (m_Start>oi) m_Start--;	// shift down
		if (m_End>oi) m_End--;	// shift end down

		if (m_Upper>=oi) m_Upper--;	// reduce array bounds upper

		m_pData=narray;
	}
};

//////////////////////////////////////////////////////////////////////////

// used when elements must not be moved in memory (e.g. pointers to elements used)

template <class TYPE, Int32 BLOCK> class DynamicFixedArray
{
protected:

	struct DynamicDataBlock
	{
	public:
		DynamicDataBlock() { data=nullptr; allocated=0; }
		~DynamicDataBlock() { }

		TYPE *data;
		Int32 allocated;
	};

	Int32 m_Allocated;
	DynamicArray<DynamicDataBlock,128> m_Blocks;
	MemoryStack *m_pMemStack;

	//////////////////////////////////////////////////////////////////////////

	DynamicDataBlock *AllocBlock()
	{
		DynamicDataBlock *data;

		if (!m_Blocks.Add()) return nullptr;
		data=m_Blocks.GetAt(m_Blocks.GetCount()-1);

		if (BLOCK == 0)
			return nullptr;
		data->data=(TYPE*)MemAlloc(sizeof(TYPE)*BLOCK);
		if (!data->data)
			return nullptr;

		return data;
	}

	TYPE *AddData()
	{
		DynamicDataBlock *data=m_Blocks.GetLast();
		if (!data)
		{
			if (!(data=AllocBlock())) return nullptr;
		}

		if (data->allocated==BLOCK)
		{
			if (!(data=AllocBlock())) return nullptr;
		}

		data->allocated++;
		m_Allocated++;

		return data->data+data->allocated-1;
	}

public:

	DynamicFixedArray() { m_pMemStack=nullptr; Reset(); }
	~DynamicFixedArray() { Free(); }

	TYPE &operator[](int i) { DebugAssert(i>=0 && i<m_Allocated); return Get(i); }
	TYPE operator[](int i) const { DebugAssert(i>=0 && i<m_Allocated); return Get(i); }

	void Attach(MemoryStack *pMem) { Free(); m_pMemStack=pMem; m_Blocks.Attach(pMem); }
	Int32 GetMemoryUse() { return m_Blocks.GetMemoryUse()+m_Blocks.GetCount()*BLOCK*sizeof(TYPE); }
	Int32 GetMaxCount() { return m_Blocks.GetCount()*BLOCK; }

	void Free()
	{
		for (Int32 i=0;i<m_Blocks.GetCount();i++)
		{
			for (Int32 l=0;l<m_Blocks[i].allocated;l++)
			{
				m_Blocks[i].data[l].~TYPE();
			}
			MemFree(m_Blocks[i].data);
		}

		m_Blocks.Free();
		Reset();
	}

	void Reset()
	{
		m_Allocated=0;
	}

	Int32 GetCount() { return m_Allocated; }

	TYPE *GetAt(Int32 i)
	{
		Int32 b=i/BLOCK;
		return m_Blocks.GetAt(b)->data+(i-(b*BLOCK));
	}

	TYPE &Get(Int32 i)
	{
		Int32 b=i/BLOCK;
		return *(m_Blocks.GetAt(b)->data+(i-(b*BLOCK)));
	}

	TYPE *GetLast()
	{
		if (m_Allocated==0) return nullptr;
		return GetAt(m_Allocated-1);
	}

	void Set(Int32 i, const TYPE &v)
	{
		Int32 b=i/BLOCK;
		*(m_Blocks.GetAt(b)->data+(i-(b*BLOCK)))=v;
	}

	Bool Add()
	{
		TYPE *data=AddData(),def;
		if (!data) return false;

		CopyMem(&def,data,sizeof(TYPE));

		return true;
	}

	TYPE *AddLast()
	{
		if (!Add()) return nullptr;
		return GetLast();
	}

	Bool Add(const TYPE &v)
	{
		TYPE *data=AddData(),def;
		if (!data) return false;

		CopyMem(&def,data,sizeof(TYPE));
		*data=v;

		return true;
	}
};

//////////////////////////////////////////////////////////////////////////

class SelectArray
{
	class SelectArrayData
	{
	public:

		SelectArrayData() { m_Lower=0; m_Upper=0; }
		SelectArrayData(Int32 index) { m_Upper=m_Lower=index; }
		SelectArrayData(Int32 lower, Int32 upper) { m_Upper=upper; m_Lower=lower; }

		Bool operator<(const SelectArrayData &dst) const { return m_Lower<dst.m_Lower; }
		Bool operator>(const SelectArrayData &dst) const { return m_Lower>dst.m_Upper; }
		Bool operator<=(const SelectArrayData &dst) const { return m_Lower<=dst.m_Upper; }
		Bool operator>=(const SelectArrayData &dst) const { return m_Lower>=dst.m_Lower; }
		Bool operator==(const SelectArrayData &dst) const { return m_Lower>=dst.m_Lower && m_Lower<=dst.m_Upper; }

		SelectArrayData &operator=(const SelectArrayData &src) { m_Lower=src.m_Lower; m_Upper=src.m_Upper; return *this; }

		Int32 m_Lower;
		Int32 m_Upper;
	};

private:

	SortedArrayEx<SelectArrayData,4> m_Data;
	Int32 m_SelectCount;
	Int32 m_LastSegment;

	MemoryStack *m_pMemStack;

	Int32 FindNum(Int32 num)
	{
		Int32 oseg=m_LastSegment;

		if (oseg<m_Data.GetCount() && num>=m_Data[oseg].m_Lower && num<=m_Data[oseg].m_Upper) return oseg;

		oseg=m_Data.SearchEqualIndex(SelectArrayData(num));

		if (oseg==m_Data.GetCount())
			m_LastSegment=0;
		else
			m_LastSegment=oseg;

		return oseg;
	}

public:

	SelectArray() { m_LastSegment=0; m_SelectCount=0; m_pMemStack=nullptr; }
	~SelectArray() { }

	void Attach(MemoryStack *pMem) { DeselectAll(); m_pMemStack=pMem; m_Data.Attach(pMem); }

	Int32 GetCount() const { return m_SelectCount; }
	Int32 GetSegments() const { return m_Data.GetCount(); }

	Bool Select(Int32 num)
	{
		if (m_Data.GetCount()==0)
		{
			m_SelectCount++;
			return m_Data.AddLast(SelectArrayData(num));
		}

		Int32 i=FindNum(num);

		if (i==m_Data.GetCount())
		{
			if (num>(m_Data[i-1].m_Upper+1))
			{
				if (!m_Data.Insert(i)) return false;

				m_Data[i].m_Lower=num;
				m_Data[i].m_Upper=num;

				m_SelectCount++;

				return true;
			}
			else
				i--;
		}

		if (num>=m_Data[i].m_Lower && num<=m_Data[i].m_Upper) return true;

		m_SelectCount++;

		if (i>0 && (m_Data[i-1].m_Upper+1)==num) i--;

		if ((m_Data[i].m_Upper+1)==num)
		{
			m_Data[i].m_Upper++;
			if (i==(m_Data.GetCount()-1) || (m_Data[i+1].m_Lower-1)!=num) return true;

			m_Data[i].m_Upper=m_Data[i+1].m_Upper;

			return m_Data.Remove(i+1);
		}

		if ((m_Data[i].m_Lower-1)==num)
		{
			m_Data[i].m_Lower--;
			if (i==0 || (m_Data[i-1].m_Upper+1)!=num) return true;

			m_Data[i-1].m_Upper=m_Data[i].m_Upper;

			return m_Data.Remove(i);
		}

		if (!m_Data.Insert(i)) return false;

		m_Data[i].m_Lower=num;
		m_Data[i].m_Upper=num;

		return true;
	}

	Bool SelectAll(Int32 min, Int32 max)
	{
		DeselectAll();
		if (max<min) max=min;
		m_SelectCount=max-min+1;
		return m_Data.AddLast(SelectArrayData(min,max));
	}

	Bool Deselect(Int32 num)
	{
		Int32 i=FindNum(num);
		if (i==m_Data.GetCount()) return true;

		if (num>=m_Data[i].m_Lower && num<=m_Data[i].m_Upper)
		{
			m_SelectCount--;

			if (m_Data[i].m_Lower==m_Data[i].m_Upper) return m_Data.Remove(i);

			if (m_Data[i].m_Lower==num)
			{
				m_Data[i].m_Lower++;
				return true;
			}
			else if (m_Data[i].m_Upper==num)
			{
				m_Data[i].m_Upper--;
				return true;
			}
			else
			{
				if (!m_Data.Insert(i)) return false;

				m_Data[i].m_Lower=m_Data[i+1].m_Lower;
				m_Data[i].m_Upper=num-1;
				m_Data[i+1].m_Lower=num+1;
			}
		}

		return true;
	}

	void DeselectAll()
	{
		m_Data.Free();
		m_SelectCount=0;
	}

	Bool Toggle(Int32 num)
	{
		if (IsSelected(num))
			return Deselect(num);
		else
			return Select(num);
	}

	Bool ToggleAll(Int32 min, Int32 max)
	{
		Int32 i;

		for (i=min; i<=max; i++)
		{
			if (!Toggle(i)) return false;
		}

		return true;
	}

	Bool GetRange(Int32 seg, Int32 *a, Int32 *b)
	{
		if (seg>=m_Data.GetCount()) return false;

		*a=m_Data[seg].m_Lower;
		*b=m_Data[seg].m_Upper;

		return true;
	}

	Bool IsSelected(Int32 num)
	{
		Int32 seg;
		return FindSegment(num,&seg);
	}

	Bool FindSegment(Int32 num, Int32 *segment)
	{
		*segment=NOTOK;

		if (m_Data.GetCount()==0) return false;

		Int32 i=FindNum(num);
		if (i==m_Data.GetCount()) return false;

		if (num>=m_Data[i].m_Lower && num<=m_Data[i].m_Upper)
		{
			*segment=i;
			return true;
		}

		return false;
	}

	Bool CopyTo(SelectArray *dst)
	{
		if (dst==this) return true;

		if (!m_Data.CopyTo(&dst->m_Data)) return false;
		dst->m_SelectCount=m_SelectCount;
		return true;
	}

	Bool CopyTo(BaseSelect *bs)
	{
		if (!m_Data.GetCount()) return bs->DeselectAll();

		if (!bs->CopyFrom(nullptr,m_Data.GetCount())) return false;

		CopyMem(m_Data.GetArray(),bs->GetData(),sizeof(Int32)*2*m_Data.GetCount());

		DebugAssert(sizeof(SelectArrayData)==sizeof(Int32)*2);

		return true;
	}

	Bool CopyFrom(BaseSelect *bs)
	{
		if (!bs->GetSegments())
		{
			DeselectAll();
			return true;
		}

		if (!m_Data.SetCount(bs->GetSegments())) return false;

		CopyMem(bs->GetData(),m_Data.GetArray(),sizeof(Int32)*2*m_Data.GetCount());

		DebugAssert(sizeof(SelectArrayData)==sizeof(Int32)*2);

		m_SelectCount=bs->GetCount();

		return true;
	}

	Int GetMemoryUse() { return m_Data.GetMemoryUse(); }
};

//////////////////////////////////////////////////////////////////////////

template <class TYPE, Int32 MINBLOCK, Int32 MAXBLOCK> class DynamicBlockArray
{
protected:

	struct DynamicDataBlock
	{
	public:
		DynamicDataBlock() { data=nullptr; allocated=0; block=MINBLOCK; adjustment=0; }
		~DynamicDataBlock() { }

		TYPE *data;
		Int32 allocated;
		Int32 block;
		Int32 adjustment;
	};

	Int32 m_Allocated;
	DynamicArray<DynamicDataBlock,128> m_Blocks;

	MemoryStack *m_pMemStack;

	//////////////////////////////////////////////////////////////////////////

	DynamicDataBlock *AllocBlock()
	{
		DynamicDataBlock *data;

		if (!m_Blocks.Add()) return nullptr;
		data=m_Blocks.GetAt(m_Blocks.GetCount()-1);

		if (data->block == 0)
			return nullptr;
		data->data=(TYPE*)MemAlloc(sizeof(TYPE) * data->block);
		if (!data->data)
			return nullptr;

		return data;
	}

	TYPE *AddData()
	{
		DynamicDataBlock *data=m_Blocks.GetLast();
		if (!data)
		{
			if (!(data=AllocBlock())) return nullptr;
		}

		if (data->allocated==data->block)
		{
			if (data->block==MAXBLOCK)
			{
				if (!(data=AllocBlock())) return nullptr;
			}
			else
			{
				if (data->block == 0)
					return nullptr;
				TYPE *ndata=(TYPE*)MemAlloc(sizeof(TYPE)*data->block*2);
				if (!ndata)
					return nullptr;

				CopyMem(data->data,ndata,sizeof(TYPE)*data->block);

				MemFree(data->data);
				data->data=ndata;
				data->block*=2;
			}
		}

		data->allocated++;
		m_Allocated++;

		CorrectAdjustments(m_Blocks.GetCount()-1);

		return data->data+data->allocated-1;
	}

	//////////////////////////////////////////////////////////////////////////

public:

	void Attach(MemoryStack *pMem) { Free(); m_pMemStack=pMem; m_Blocks.Attach(pMem); }
	Int GetMemoryUse()
	{
		Int used=0;
		Int32 i;

		for (i=0;i<m_Blocks.GetCount();i++)
		{
			used+=m_Blocks[i].block*sizeof(TYPE);
		}

		return m_Blocks.GetMemoryUse()+used;
	}
	Int GetMaxCount()
	{
		Int32 cnt=0,i;
		for (i=0;i<m_Blocks.GetCount();i++)
		{
			cnt+=m_Blocks[i].block;
		}
		return cnt;
	}

	DynamicDataBlock *InsertBlock(Int32 b)
	{
		DynamicDataBlock *data;

		if (!m_Blocks.Insert(b)) return nullptr;
		data=m_Blocks.GetAt(b);

		if (data->block == 0)
			return nullptr;
		data->data=(TYPE*)MemAlloc(sizeof(TYPE)*data->block);
		if (!data->data)
			return nullptr;

		return data;
	}

	void CorrectAdjustments(Int32 b)
	{
		Int32 a;

		if (b>0)
			a=m_Blocks[b-1].adjustment+(MAXBLOCK-m_Blocks[b-1].allocated);
		else
			a=0;

		for (Int32 i=b;i<m_Blocks.GetCount();i++)
		{
			DebugAssert(m_Blocks[i].allocated>0);

			m_Blocks[i].adjustment=a;
			a+=(MAXBLOCK-m_Blocks[i].allocated);
		}
	}

	TYPE *InsertData(Int32 &b, Int32 &i)
	{
		if (b==m_Blocks.GetCount()) return AddData();

		DynamicDataBlock *data=m_Blocks.GetAt(b);

		DebugAssert(data->allocated>0);

		if (i>=MAXBLOCK)
		{
			DebugAssert(i==MAXBLOCK);

			b++;
			i=0;
			if (!(data=InsertBlock(b))) return nullptr;

			data->allocated++;
			m_Allocated++;

			CorrectAdjustments(b);

			return data->data;
		}

		if (data->allocated==data->block)
		{
			if (data->block==MAXBLOCK)
			{
				DynamicDataBlock *ndata;

				if (b==(m_Blocks.GetCount()-1) || m_Blocks[b+1].allocated==MAXBLOCK)
				{
					if (b>0 && m_Blocks[b-1].allocated<MAXBLOCK)
					{
						ndata=m_Blocks.GetAt(b-1);
						if (ndata->allocated==ndata->block)
						{
							if (ndata->block == 0)
								return nullptr;
							TYPE *nblock=(TYPE*)MemAlloc(sizeof(TYPE)*ndata->block*2);
							if (!nblock)
								return nullptr;

							CopyMem(ndata->data,nblock,sizeof(TYPE)*ndata->block);

							MemFree(ndata->data);
							ndata->data=nblock;
							ndata->block*=2;
						}

						if (i>0)
						{
							Int32 cblock=(ndata->block-ndata->allocated);
							if (cblock>i) cblock=i;

							CopyMem(data->data,ndata->data+ndata->allocated,cblock*sizeof(TYPE));
							ndata->allocated+=cblock;

							if (i>cblock) memmove(data->data,data->data+cblock,(i-cblock)*sizeof(TYPE));
							if (i<data->allocated) memmove(data->data+i-cblock+1,data->data+i,sizeof(TYPE)*(data->allocated-i));

							i-=cblock;
							data->allocated-=(cblock-1);
							m_Allocated++;

							DebugAssert(data->allocated>0);

							CorrectAdjustments(b-1);

							return data->data+i;
						}
						else
						{
							i=ndata->allocated;
							ndata->allocated++;
							m_Allocated++;
							b--;

							CorrectAdjustments(b);

							return ndata->data+i;
						}
					}

					if (!(ndata=InsertBlock(b+1))) return nullptr;
					data=m_Blocks.GetAt(b);
				}
				else
				{
					ndata=m_Blocks.GetAt(b+1);
				}

				if (ndata->allocated==ndata->block)
				{
					if (ndata->block == 0)
						return nullptr;
					TYPE *nblock=(TYPE*)MemAlloc(sizeof(TYPE)*ndata->block*2);
					if (!nblock)
						return nullptr;

					CopyMem(ndata->data,nblock,sizeof(TYPE)*ndata->block);

					MemFree(ndata->data);
					ndata->data=nblock;
					ndata->block*=2;
				}

				Int32 cblock=(ndata->block-ndata->allocated);
				if (cblock<1) cblock=1; else if (cblock>(MAXBLOCK-i)) cblock=MAXBLOCK-i;

				if (ndata->allocated>0) memmove(ndata->data+cblock,ndata->data,sizeof(TYPE)*ndata->allocated);
				CopyMem(data->data+MAXBLOCK-cblock,ndata->data,sizeof(TYPE)*cblock);

				data->allocated-=cblock;
				ndata->allocated+=cblock;

				DebugAssert(data->allocated>0);

				if (i<data->allocated) memmove(data->data+i+1,data->data+i,sizeof(TYPE)*(data->allocated-i));
			}
			else
			{
				if (data->block == 0)
					return nullptr;
				TYPE *ndata=(TYPE*)MemAlloc(sizeof(TYPE)*data->block*2);
				if (!ndata)
					return nullptr;

				if (i>0) CopyMem(data->data,ndata,sizeof(TYPE)*i);
				if (i<data->allocated) CopyMem(data->data+i,ndata+i+1,sizeof(TYPE)*(data->allocated-i));

				MemFree(data->data);
				data->data=ndata;
				data->block*=2;
			}
		}
		else
		{
			if (i<data->allocated) memmove(data->data+i+1,data->data+i,sizeof(TYPE)*(data->allocated-i));
		}

		data->allocated++;
		m_Allocated++;

		CorrectAdjustments(b);

		return data->data+i;
	}

	Int32 GetIndex(Int32 &i)
	{
		Int32 b=i/MAXBLOCK,a=0;
		if (b>=m_Blocks.GetCount()) b--;

		DynamicDataBlock *data=m_Blocks.GetAt(b);

		i=i-(b*MAXBLOCK);
		do
		{
			DebugAssert(data->allocated>0);

			while (i>=data->allocated)
			{
				i-=data->allocated;
				b++;
				a+=(MAXBLOCK-data->allocated);
				data=m_Blocks.GetAt(b);

				DebugAssert(data->allocated>0);
			}

			if (data->adjustment)
			{
				i=i-a+data->adjustment;
				a=data->adjustment;
			}
			else
				break;

		} while (i>=data->allocated);

		return b;
	}

	Int32 GetBlockIndex(Int32 b, Int32 i) const
	{
		if (b>=m_Blocks.GetCount()) return m_Allocated;

		DebugAssert((b*MAXBLOCK-m_Blocks[b].adjustment+i)<m_Allocated);

		return b*MAXBLOCK-m_Blocks[b].adjustment+i;
	}

	Bool BlockInsert(Int32 &b, Int32 &i)
	{
		TYPE *data=InsertData(b,i),def;
		if (!data) return false;

		CopyMem(&def,data,sizeof(TYPE));

		return true;
	}

	void BlockSet(Int32 b, Int32 i, const TYPE &v)
	{
		DebugAssert(b<m_Blocks.GetCount()); DebugAssert(i<m_Blocks[b].allocated);
		m_Blocks[b].data[i]=v;
	}

	TYPE *BlockGetAt(Int32 b, Int32 i)
	{
		DebugAssert(b<m_Blocks.GetCount()); DebugAssert(i<m_Blocks[b].allocated);
		return &m_Blocks[b].data[i];
	}

	TYPE &BlockGet(Int32 b, Int32 i)
	{
		DebugAssert(b<m_Blocks.GetCount()); DebugAssert(i<m_Blocks[b].allocated);
		return m_Blocks[b].data[i];
	}

	TYPE *BlockPrev(Int32 b, Int32 i)
	{
		DebugAssert(b<m_Blocks.GetCount()); DebugAssert(i<m_Blocks[b].allocated);

		if (i>0)
			return &m_Blocks[b].data[i-1];
		else
		{
			DebugAssert(b>0);
			return &m_Blocks[b-1].data[m_Blocks[b-1].allocated-1];
		}
	}

	TYPE *BlockNext(Int32 b, Int32 i)
	{
		DebugAssert(b<m_Blocks.GetCount()); DebugAssert(i<m_Blocks[b].allocated);

		if (i<(m_Blocks[b].allocated-1))
			return &m_Blocks[b].data[i+1];
		else
			return &m_Blocks[b+1].data[0];
	}

	TYPE *BlockGetPrev(Int32 &b, Int32 &i)
	{
		if (i>0)
			i--;
		else
		{
			b--;
			if (b<0) return nullptr;
			i=m_Blocks[b].allocated-1;
		}

		return &m_Blocks[b].data[i];
	}

	TYPE *BlockGetNext(Int32 &b, Int32 &i)
	{
		if (i<(m_Blocks[b].allocated-1))
			i++;
		else
		{
			b++; i=0;
		}

		if (b>=m_Blocks.GetCount() || i>=m_Blocks[b].allocated) return nullptr;

		return &m_Blocks[b].data[i];
	}

	void BlockNextIndex(Int32 &b, Int32 &i)
	{
		if (i<(m_Blocks[b].allocated-1))
			i++;
		else
		{
			b++; i=0;
		}
	}

	TYPE *GetAtInc(Int32 &b, Int32 &i)
	{
		DebugAssert(b<m_Blocks.GetCount()); DebugAssert(i<m_Blocks[b].allocated);

		TYPE *data=&m_Blocks[b].data[i];

		if (i<(m_Blocks[b].allocated-1))
			i++;
		else
		{
			b++; i=0;
		}

		return data;
	}

	Bool Remove(Int32 &b, Int32 &i)
	{
		DebugAssert(b<m_Blocks.GetCount()); DebugAssert(i<m_Blocks[b].allocated);

		m_Blocks[b].data[i].~TYPE();

		if (m_Blocks[b].allocated<=(m_Blocks[b].block>>1) && m_Blocks[b].block>MINBLOCK)
		{
			Int32 ncnt=m_Blocks[b].block>>1;
			TYPE *data=nullptr;

			if (ncnt == 0)
				return false;
			data = (TYPE*)MemAlloc(sizeof(TYPE)*ncnt);
			if (!data)
				return false;

			if (i>0) CopyMem(m_Blocks[b].data,data,i*sizeof(TYPE));
			if (i<(m_Blocks[b].allocated-1)) CopyMem((Char*)m_Blocks[b].data+(i+1)*sizeof(TYPE),(Char*)data+i*sizeof(TYPE),(m_Blocks[b].allocated-i-1)*sizeof(TYPE));

			MemFree(m_Blocks[b].data);

			m_Blocks[b].data=data;
			m_Blocks[b].block=ncnt;
		}
		else
		{
			if (i<(m_Blocks[b].allocated-1)) memmove((Char*)m_Blocks[b].data+i*sizeof(TYPE),(Char*)m_Blocks[b].data+(i+1)*sizeof(TYPE),(m_Blocks[b].allocated-i-1)*sizeof(TYPE));
		}

		m_Blocks[b].allocated--;
		m_Allocated--;

		if (m_Blocks[b].allocated==0)
		{
			MemFree(m_Blocks[b].data);
			if (!m_Blocks.Remove(b)) return false;
			if (b<m_Blocks.GetCount()) CorrectAdjustments(b);
			i=0;
		}
		else
		{
			CorrectAdjustments(b);
			if (i>=m_Blocks[b].allocated) { b++; i=0; }
		}

		return true;
	}

	Bool BlockRemove(Int32 &b, Int32 &i) { return Remove(b,i); }

	//////////////////////////////////////////////////////////////////////////

	DynamicBlockArray() { m_pMemStack=nullptr; Reset(); }
	~DynamicBlockArray() { Free(); }

	TYPE &operator[](int i) { DebugAssert(i>=0 && i<m_Allocated); return Get(i); }
	TYPE operator[](int i) const { DebugAssert(i>=0 && i<m_Allocated); return Get(i); }

	void Free()
	{
		for (Int32 i=0;i<m_Blocks.GetCount();i++)
		{
			for (Int32 l=0;l<m_Blocks[i].allocated;l++)
			{
				m_Blocks[i].data[l].~TYPE();
			}
			MemFree(m_Blocks[i].data);
		}

		m_Blocks.Free();
		Reset();
	}

	void Reset()
	{
		m_Allocated=0;
	}

	Int32 GetCount() const { return m_Allocated; }

	TYPE *GetAt(Int32 i)
	{
		Int32 b=GetIndex(i);
		return m_Blocks.GetAt(b)->data+i;
	}

	TYPE *GetAt(Int32 b, Int32 i)
	{
		DebugAssert(b<m_Blocks.GetCount()); DebugAssert(i<m_Blocks[b].allocated);
		return &m_Blocks[b].data[i];
	}

	TYPE &Get(Int32 i)
	{
		Int32 b=GetIndex(i);
		return *(m_Blocks.GetAt(b)->data+i);
	}

	TYPE &GetLast()
	{
		Int32 b=m_Blocks.GetCount()-1,i;
		i=m_Blocks[b].allocated-1;
		return *(m_Blocks.GetAt(b)->data+i);
	}

	TYPE *GetAtLast()
	{
		Int32 b=m_Blocks.GetCount()-1,i;
		i=m_Blocks[b].allocated-1;
		return m_Blocks.GetAt(b)->data+i;
	}

	TYPE *GetAtLast(Int32 &b, Int32 &i)
	{
		b=m_Blocks.GetCount()-1;
		i=m_Blocks[b].allocated-1;
		return m_Blocks.GetAt(b)->data+i;
	}

	void Set(Int32 i, const TYPE &v)
	{
		Int32 b=GetIndex(i);
		*(m_Blocks.GetAt(b)->data+i)=v;
	}

	Bool Insert(Int32 i)
	{
		if (i==m_Allocated) return Add();

		Int32 b=GetIndex(i);
		TYPE *data=InsertData(b,i),def;
		if (!data) return false;

		CopyMem(&def,data,sizeof(TYPE));

		return true;
	}

	Bool Insert(Int32 i, const TYPE &v)
	{
		if (i==m_Allocated) return Add(v);

		Int32 b=GetIndex(i);
		TYPE *data=InsertData(b,i),def;
		if (!data) return false;

		CopyMem(&def,data,sizeof(TYPE));
		*data=v;

		return true;
	}

	Bool Add()
	{
		TYPE *data=AddData(),def;
		if (!data) return false;

		CopyMem(&def,data,sizeof(TYPE));

		return true;
	}

	Bool Add(const TYPE &v)
	{
		TYPE *data=AddData(),def;
		if (!data) return false;

		CopyMem(&def,data,sizeof(TYPE));
		*data=v;

		return true;
	}

	Bool Remove(Int32 i)
	{
		Int32 b=GetIndex(i);
		return Remove(b,i);
	}

	Bool CopyTo(DynamicBlockArray *dst)
	{
		if (dst==this) return true;

		dst->Free();
		if (!dst->m_Blocks.SetCount(m_Blocks.GetCount())) return false;

		Int32 b,i;

		for (b=0;b<m_Blocks.GetCount();b++)
		{
			if (dst->m_pMemStack)
				dst->m_Blocks[b].data=(TYPE*)dst->m_pMemStack->AllocS(m_Blocks[b].block*sizeof(TYPE));
			else
				dst->m_Blocks[b].data=NewMemClear(TYPE, m_Blocks[b].block);
			if (!dst->m_Blocks[b].data) goto Error2;

			TYPE def;

			FillMem(dst->m_Blocks[b].data,m_Blocks[b].allocated,&def);

			for (i=0;i<m_Blocks[b].allocated;i++) { dst->m_Blocks[b].data[i]=m_Blocks[b].data[i]; }

			dst->m_Blocks[b].adjustment=m_Blocks[b].adjustment;
			dst->m_Blocks[b].allocated=m_Blocks[b].allocated;
			dst->m_Blocks[b].block=m_Blocks[b].block;
		}

		dst->m_Allocated=m_Allocated;

		return true;
	Error2:
		dst->Free();
		return false;
	}

	TYPE *ToTable(MemoryStack *mem=nullptr, Bool directcopy=true)
	{
		if (!m_Allocated) return nullptr;

		TYPE *table;

		if (directcopy)
		{
			if (!mem)
				table=NewMemClear(TYPE,m_Allocated);
			else
				table=(TYPE*)mem->AllocS(sizeof(TYPE)*m_Allocated);

			if (!table) return nullptr;

			Int32 b,o=0;

			for (b=0;b<m_Blocks.GetCount();b++)
			{
				CopyMem(m_Blocks[b].data,table+o,sizeof(TYPE)*m_Blocks[b].allocated);
				o+=m_Blocks[b].allocated;
			}
		}
		else
		{
			Int32 o=0,b,i;

			table=bNewDeprecatedUseArraysInstead TYPE[m_Allocated];
			if (!table) return nullptr;

			for (b=0;b<m_Blocks.GetCount();b++)
			{
				for (i=0;i<m_Blocks[b].allocated;i++)
				{
					table[o++]=m_Blocks[b].data[i];
				}
			}
		}

		return table;
	}

	void FromTable(TYPE *table, Bool directcopy=true)
	{
		if (!m_Allocated || !table) return;

		if (directcopy)
		{
			Int32 b,o=0;

			for (b=0;b<m_Blocks.GetCount();b++)
			{
				CopyMem(table+o,m_Blocks[b].data,sizeof(TYPE)*m_Blocks[b].allocated);
				o+=m_Blocks[b].allocated;
			}
		}
		else
		{
			Int32 o=0,b,i;

			for (b=0;b<m_Blocks.GetCount();b++)
			{
				for (i=0;i<m_Blocks[b].allocated;i++)
				{
					m_Blocks[b].data[i]=table[o++];
				}
			}
		}
	}

	void CopyTo(TYPE *table)
	{
		if (!m_Allocated || !table) return;

		Int32 b,o=0;

		for (b=0;b<m_Blocks.GetCount();b++)
		{
			CopyMem(m_Blocks[b].data,table+o,sizeof(TYPE)*m_Blocks[b].allocated);
			o+=m_Blocks[b].allocated;
		}
	}

	void CopyFrom(TYPE *table)
	{
		if (!m_Allocated || !table) return;

		Int32 b,o=0;

		for (b=0;b<m_Blocks.GetCount();b++)
		{
			CopyMem(table+o,m_Blocks[b].data,sizeof(TYPE)*m_Blocks[b].allocated);
			o+=m_Blocks[b].allocated;
		}
	}

	void Sort(MemoryStack *mem=nullptr)
	{
		TYPE *table=ToTable(mem);
		if (!table)
		{
			Int32 i, j, h;
			TYPE v,t;

			for (h = 1; h <= m_Allocated / 9; h = 3 * h + 1) { }
			for (; h > 0; h /= 3)
			{
				for (i = h; i < m_Allocated; i++)
				{
					v = Get(i);
					j = i;
					while (j > h - 1 && (t=Get(j - h))>v)
					{
						Set(j,t); j -= h;
					}
					Set(j,v);
				}
			}
		}
		else
		{
			ShellSort<TYPE>(table,m_Allocated);
			FromTable(table);

			if (mem)
				mem->FreeS(table);
			else
				DeleteMem(table);
		}
	}

	Bool SetCount(Int32 ncnt)
	{
		Int32 cnt=ncnt,i,l;

		for (i=0;i<m_Blocks.GetCount();i++)
		{
			if (cnt<=MAXBLOCK)
			{
				if (cnt<=m_Blocks[i].allocated)
				{
					for (l=cnt;l<m_Blocks[i].allocated;l++) { m_Blocks[i].data[l].~TYPE(); }
				}
				else if (cnt<=m_Blocks[i].block)
				{
					TYPE def;
					FillMem(&m_Blocks[i].data[m_Blocks[i].allocated],cnt-m_Blocks[i].allocated,&def);
				}
				else
				{
					Int32 bsize=m_Blocks[i].block;
					while (bsize<cnt) { bsize*=2; }

					if (bsize == 0)
						return false;
					TYPE *ndata=(TYPE*)MemAlloc(bsize*sizeof(TYPE));
					if (!ndata)
						return false;

					CopyMem(m_Blocks[i].data,ndata,sizeof(TYPE)*m_Blocks[i].allocated);
					MemFree(m_Blocks[i].data);

					m_Blocks[i].data=ndata;
					m_Blocks[i].block=bsize;

					TYPE def;
					FillMem(&m_Blocks[i].data[m_Blocks[i].allocated],cnt-m_Blocks[i].allocated,&def);
				}

				m_Blocks[i].allocated=cnt;
				if (!m_Blocks.SetCount(i+1)) return false;
				cnt=0;

				break;
			}
			else if (m_Blocks[i].allocated<MAXBLOCK)
			{
				if (m_Blocks[i].block<MAXBLOCK)
				{
					TYPE *ndata=(TYPE*)MemAlloc(MAXBLOCK*sizeof(TYPE));
					if (!ndata) return false;

					CopyMem(m_Blocks[i].data,ndata,sizeof(TYPE)*m_Blocks[i].allocated);
					MemFree(m_Blocks[i].data);

					m_Blocks[i].data=ndata;
					m_Blocks[i].block=MAXBLOCK;
				}

				TYPE def;
				FillMem(&m_Blocks[i].data[m_Blocks[i].allocated],MAXBLOCK-m_Blocks[i].allocated,&def);

				m_Blocks[i].allocated=MAXBLOCK;
			}

			cnt-=MAXBLOCK;
		}

		DynamicDataBlock *data;

		while (cnt>0)
		{
			if (!m_Blocks.Add()) return false;
			data=m_Blocks.GetAt(m_Blocks.GetCount()-1);

			Int32 bsize=MINBLOCK;
			while (bsize<cnt && bsize<MAXBLOCK) { bsize*=2; }

			if (bsize == 0)
				return false;
			data->data=(TYPE*)MemAlloc(sizeof(TYPE)*bsize);
			if (!data->data)
				return false;

			if (cnt>MAXBLOCK)
				data->allocated=MAXBLOCK;
			else
				data->allocated=cnt;

			data->block=bsize;

			TYPE def;
			FillMem(m_Blocks[i].data,m_Blocks[i].allocated,&def);

			cnt-=data->allocated;
		}

		CorrectAdjustments(0);

		m_Allocated=ncnt;

		return true;
	}
};

template <class TYPE, Int32 MINBLOCK, Int32 MAXBLOCK> class SortedBlockArray : public DynamicBlockArray<TYPE,MINBLOCK,MAXBLOCK>
{
	friend class SelectBlockArray;

protected:

	Int32 SearchIndex(const TYPE &v)
	{
		if (GCC34T m_Allocated==0 || v<=GCC34T Get(0)) return 0;
		if (v>=GCC34T Get(GCC34T m_Allocated-1)) return GCC34T m_Allocated;

		Int32 l,u,m;

		l=1; u=GCC34T m_Allocated-2;

		while (l<u)
		{
			m=(l+u)>>1;

			if (v>=GCC34T Get(m)) l=m+1;
			else u=m-1;
		}

		if (l>0 && v==GCC34T Get(l-1)) l--;
		else if (v>GCC34T Get(l)) l++;

		return l;
	}

	Int32 SearchEqualIndex(const TYPE &v)
	{
		TYPE *d;

		if (GCC34T m_Allocated==0 || v<=GCC34T Get(0)) return 0;

		d=GetAt(GCC34T m_Allocated-1);
		if (v>(*d)) return GCC34T m_Allocated;
		if (v==(*d)) return GCC34T m_Allocated-1;

		Int32 l,u,m;

		l=1; u=GCC34T m_Allocated-2;

		while (l<u)
		{
			m=(l+u)>>1;

			if (v>=GCC34T Get(m)) l=m+1;
			else u=m-1;
		}

		if (l>0 && v==GCC34T Get(l-1)) l--;
		else if (v>GCC34T Get(l)) l++;

		return l;
	}

	Int32 SearchBlockIndex(const TYPE &v, Int32 &b, Int32 &i)
	{
		if (GCC34T m_Allocated==0 || v<=GCC34T m_Blocks[0].data[0])
		{
			b=i=0;
			return 0;
		}

		b=GCC34T m_Blocks.GetCount()-1;
		i=GCC34T m_Blocks[b].allocated-1;
		if (v>=GCC34T m_Blocks[b].data[i])
		{
			i++;
			return GCC34T m_Allocated;
		}

		Int32 l,u,m,bl,bu,bm;

		bl=0; bu=b;

		while (bl<bu)
		{
			bm=(bl+bu)>>1;
			if (v>=GCC34T m_Blocks[bm].data[0]) bl=bm+1;
			else bu=bm-1;
		}

		if (bl>0 && v>=GCC34T m_Blocks[bl-1].data[0] && v<=GCC34T m_Blocks[bl-1].data[GCC34T m_Blocks[bl-1].allocated-1]) bl--;
		else if (v>GCC34T m_Blocks[bl].data[GCC34T m_Blocks[bl].allocated-1]) bl++;

		if (bl>=GCC34T m_Blocks.GetCount()) { i=0; b=bl; return GCC34T m_Allocated; }

		TYPE *data=GCC34T m_Blocks[bl].data;

		l=1;
		u=GCC34T m_Blocks[bl].allocated-2;

		if (v<=data[0])
		{
			b=bl;
			i=0;
			return GCC34T GetBlockIndex(b,i);
		}
		else if (v>=data[u+1])
		{
			b=bl;
			i=u+1;
			return GCC34T GetBlockIndex(b,i);
		}

		while (l<u)
		{
			m=(l+u)>>1;
			if (v>=data[m]) l=m+1;
			else u=m-1;
		}

		if (l>0 && v==data[l-1]) l--;
		else if (bl>0 && v==GCC34T m_Blocks[bl-1].data[GCC34T m_Blocks[bl-1].allocated-1])
		{
			bl--;
			l=GCC34T m_Blocks[bl].allocated-1;
		}
		else if (v>data[l])
		{
			l++;
			if (l>GCC34T m_Blocks[bl].allocated)
			{
				l=0;
				bl++;
			}
		}

		b=bl;
		i=l;

		return GCC34T GetBlockIndex(b,i);
	}

	Int32 SearchBlockEqualIndex(const TYPE &v, Int32 &b, Int32 &i)
	{
		if (GCC34T m_Allocated==0 || v<=GCC34T m_Blocks[0].data[0])
		{
			b=i=0;
			return 0;
		}

		b=GCC34T m_Blocks.GetCount()-1;
		i=GCC34T m_Blocks[b].allocated-1;
		if (v>GCC34T m_Blocks[b].data[i]) { i++; return GCC34T m_Allocated; }
		if (v==GCC34T m_Blocks[b].data[i]) return GCC34T m_Allocated-1;

		Int32 l,u,m,bl,bu,bm;

		bl=0; bu=b;

		while (bl<bu)
		{
			bm=(bl+bu)>>1;
			if (v>=(*GCC34T m_Blocks[bm].data)) bl=bm+1;
			else bu=bm-1;
		}

		if (bl>0 && v>=GCC34T m_Blocks[bl-1].data[0] && v<=GCC34T m_Blocks[bl-1].data[GCC34T m_Blocks[bl-1].allocated-1]) bl--;
		else if (v>GCC34T m_Blocks[bl].data[GCC34T m_Blocks[bl].allocated-1]) bl++;

		if (bl>=GCC34T m_Blocks.GetCount()) { i=0; b=bl; return GCC34T m_Allocated; }

		TYPE *data=GCC34T m_Blocks[bl].data;

		l=1;
		u=GCC34T m_Blocks[bl].allocated-2;

		if (v<=data[0])
		{
			b=bl;
			i=0;
			return GCC34T GetBlockIndex(b,i);
		}
		else if (v>=data[u+1])
		{
			b=bl;
			i=u+1;
			return GCC34T GetBlockIndex(b,i);
		}

		while (l<u)
		{
			m=(l+u)>>1;
			if (v>=data[m]) l=m+1;
			else u=m-1;
		}

		if (l>0 && v==data[l-1]) l--;
		else if (bl>0 && v==GCC34T m_Blocks[bl-1].data[GCC34T m_Blocks[bl-1].allocated-1])
		{
			bl--;
			l=GCC34T m_Blocks[bl].allocated-1;
		}
		else if (v>data[l])
		{
			l++;
			if (l>GCC34T m_Blocks[bl].allocated)
			{
				l=0;
				bl++;
			}
		}

		b=bl;
		i=l;

		return GCC34T GetBlockIndex(b,i);
	}

	Bool SearchBlockIndexMatch(const TYPE &v, Int32 &b, Int32 &i) const
	{
		if (GCC34T m_Allocated==0) return false;

		Int32 l,u,m,bl,bu,bm;

		bl=0; bu=GCC34T m_Blocks.GetCount()-1;

		while (bl<bu)
		{
			bm=(bl+bu)>>1;
			if (v>=(*GCC34T m_Blocks[bm].data)) bl=bm+1;
			else bu=bm-1;
		}

		if (bl>0 && v>=GCC34T m_Blocks[bl-1].data[0] && v<=GCC34T m_Blocks[bl-1].data[GCC34T m_Blocks[bl-1].allocated-1]) bl--;
		else if (v>GCC34T m_Blocks[bl].data[GCC34T m_Blocks[bl].allocated-1]) return false;

		TYPE *data=GCC34T m_Blocks[bl].data;

		l=0;
		u=GCC34T m_Blocks[bl].allocated-1;

		if (v<data[0]) return false;
		else if (v>data[u]) return false;

		while (l<u)
		{
			m=(l+u)>>1;
			if (v>=data[m]) l=m+1;
			else u=m-1;
		}

		if (l>0 && v==data[l-1])
		{
			b=bl;
			i=l-1;
			return true;
		}
		else if (bl>0 && v==GCC34T m_Blocks[bl-1].data[GCC34T m_Blocks[bl-1].allocated-1])
		{
			b=bl-1;
			i=GCC34T m_Blocks[bl].allocated-1;
			return true;
		}
		else if (v==data[l])
		{
			b=bl;
			i=l;
			return true;
		}

		return false;
	}

	Bool SearchBlockIndexMatchFull(const TYPE &v, Int32 &b, Int32 &i) const
	{
		if (GCC34T m_Allocated==0) return false;

		Int32 l,u,m,bl,bu,bm;

		bl=0; bu=GCC34T m_Blocks.GetCount()-1;
		l=0; u=GCC34T m_Blocks[bu].allocated-1;

		if (bl<bu)
		{
			while ((bl+1)<bu)
			{
				bm=(bl+bu)>>1;
				if (v>=(*GCC34T m_Blocks[bm].data))
					bl=bm;
				else
					bu=bm;
			}

			if (v>=GCC34T m_Blocks[bu].data[0]) bl=bu;

			l=0;
			u=GCC34T m_Blocks[bl].allocated-1;
		}

		TYPE *data=GCC34T m_Blocks[bl].data;

		while (l<u)
		{
			m=(l+u)>>1;
			if (v>=data[m]) l=m+1;
			else u=m-1;
		}

		if (v==data[l])
		{
			b=bl;
			i=l;
			return true;
		}
		else if (l>0 && v==data[l-1])
		{
			b=bl;
			i=l-1;
			return true;
		}
		else if (bl>0 && v==GCC34T m_Blocks[bl-1].data[GCC34T m_Blocks[bl-1].allocated-1])
		{
			b=bl-1;
			i=GCC34T m_Blocks[bl].allocated-1;
			return true;
		}

		return false;
	}

public:

	SortedBlockArray()
	{
	}

	Bool Add(const TYPE &data)
	{
		Int32 b,i;

		SearchBlockIndex(data,b,i);

		if (!DynamicBlockArray<TYPE,MINBLOCK,MAXBLOCK>::BlockInsert(b,i)) return false;
		this->BlockSet(b,i,data);

		return true;
	}

	const SortedBlockArray<TYPE,MINBLOCK,MAXBLOCK>& operator = (const SortedBlockArray<TYPE,MINBLOCK,MAXBLOCK> &src)
	{
		((SortedBlockArray*)&src)->CopyTo(this);
		return *this;
	}

	Bool AddLast()
	{
		return DynamicBlockArray<TYPE,MINBLOCK,MAXBLOCK>::Add();
	}

	Bool AddLast(const TYPE &v)
	{
		if (!DynamicBlockArray<TYPE,MINBLOCK,MAXBLOCK>::Add()) return false;
		this->Set(GCC34T m_Allocated-1,v);
		return true;
	}

	Bool AddUnique(const TYPE &v)
	{
		if (GCC34T m_Allocated>0)
		{
			Int32 b,i;

			SearchBlockIndex(v,b,i);
			if (i==GCC34T m_Allocated && GCC34T GetLast()==v) return true;
			if (i<GCC34T m_Allocated && GCC34T m_Blocks[b].data[i]==v) return true;

			if (!DynamicBlockArray<TYPE,MINBLOCK,MAXBLOCK>::BlockInsert(b,i)) return false;
			this->BlockSet(b,i,v);

			return true;
		}
		else
			return Add(v);
	}

	Bool RemoveValue(const TYPE &v)
	{
		Int32 i=Find(v);
		if (i==NOTOK) return true;
		return GCC34T Remove(i);
	}

	Int32 Find(const TYPE &v)
	{
		Int32 b,i;
		if (!SearchBlockIndexMatchFull(v,b,i)) return NOTOK;
		return GCC34T GetBlockIndex(b,i);
	}

	Bool Find(const TYPE &v, Int32 &b, Int32 &i)
	{
		return SearchBlockIndexMatchFull(v,b,i);
	}

	Int32 FindValueIndex(const TYPE &v)
	{
		if (GCC34T m_Allocated==0) return 0;

		Int32 b,i;
		return SearchBlockEqualIndex(v,b,i);
	}

	Int32 FindValueIndex(const TYPE &v, Int32 &b, Int32 &i)
	{
		if (GCC34T m_Allocated==0) return 0;
		return SearchBlockEqualIndex(v,b,i);
	}
};

//////////////////////////////////////////////////////////////////////////

#define SELECTBLOCKARRAY_MIN 32
#define SELECTBLOCKARRAY_MAX 512

class SelectBlockArray
{
	class SelectArrayData
	{
	public:

		SelectArrayData() { m_Lower=0; m_Upper=0; }
		SelectArrayData(Int32 index) { m_Upper=m_Lower=index; }
		SelectArrayData(Int32 lower, Int32 upper) { m_Upper=upper; m_Lower=lower; }

		Bool operator<(const SelectArrayData &dst) const { return m_Lower<dst.m_Lower; }
		Bool operator>(const SelectArrayData &dst) const { return m_Lower>dst.m_Upper; }
		Bool operator<=(const SelectArrayData &dst) const { return m_Lower<=dst.m_Upper; }
		Bool operator>=(const SelectArrayData &dst) const { return m_Lower>=dst.m_Lower; }
		Bool operator==(const SelectArrayData &dst) const { return m_Lower>=dst.m_Lower && m_Lower<=dst.m_Upper; }

		SelectArrayData &operator=(const SelectArrayData &src) { m_Lower=src.m_Lower; m_Upper=src.m_Upper; return *this; }

		Int32 m_Lower;
		Int32 m_Upper;
	};

private:

	SortedBlockArray<SelectArrayData,SELECTBLOCKARRAY_MIN,SELECTBLOCKARRAY_MAX> m_Data;
	Int32 m_SelectCount;

	Int32 m_LastBlockSegment;
	MemoryStack *m_pMemStack;

	Int32 FindNum(Int32 num)
	{
		Int32 oseg=m_LastBlockSegment,b,i,l;

		b=oseg/SELECTBLOCKARRAY_MAX;
		i=oseg-b*SELECTBLOCKARRAY_MAX;

		if (b<m_Data.m_Blocks.GetCount() && i<m_Data.m_Blocks[b].allocated && num>=m_Data.m_Blocks[b].data[i].m_Lower && num<=m_Data.m_Blocks[b].data[i].m_Upper) return m_Data.GetBlockIndex(b,i);

		l=m_Data.SearchBlockEqualIndex(SelectArrayData(num),b,i);
		if (l==m_Data.GetCount())
			m_LastBlockSegment=0;
		else
			m_LastBlockSegment=b*SELECTBLOCKARRAY_MAX+i;

		return l;
	}

	Int32 FindBlockNum(Int32 num, Int32 &b, Int32 &i)
	{
		Int32 oseg=m_LastBlockSegment,l;

		b=oseg/SELECTBLOCKARRAY_MAX;
		i=oseg-b*SELECTBLOCKARRAY_MAX;

		if (b<m_Data.m_Blocks.GetCount() && i<m_Data.m_Blocks[b].allocated && num>=m_Data.m_Blocks[b].data[i].m_Lower && num<=m_Data.m_Blocks[b].data[i].m_Upper) return m_Data.GetBlockIndex(b,i);

		l=m_Data.SearchBlockEqualIndex(SelectArrayData(num),b,i);
		if (l==m_Data.GetCount())
			m_LastBlockSegment=0;
		else
			m_LastBlockSegment=b*SELECTBLOCKARRAY_MAX+i;

		return l;
	}

public:

	SelectBlockArray()
	{
		m_SelectCount=0;
		m_LastBlockSegment=0;
		m_pMemStack=nullptr;
	}
	~SelectBlockArray() { }

	void Attach(MemoryStack *pMem) { DeselectAll(); m_pMemStack=pMem; m_Data.Attach(pMem); }
	Int GetMemoryUse() { return m_Data.GetMemoryUse(); }
	Int GetMaxCount() { return m_Data.GetMaxCount(); }

	Int32 GetCount() const { return m_SelectCount; }
	Int32 GetSegments() const { return m_Data.GetCount(); }

	Bool Select(Int32 num)
	{
		if (m_Data.GetCount()==0)
		{
			m_SelectCount++;
			return m_Data.AddLast(SelectArrayData(num));
		}

		Int32 l,b,i;
		SelectArrayData *data,*ndata;

		l=FindBlockNum(num,b,i);

		if (l==m_Data.GetCount())
		{
			data=m_Data.GetAtLast(b,i);
			if (num>(data->m_Upper+1))
			{
				if (!m_Data.AddLast()) return false;

				data=m_Data.GetAtLast();

				data->m_Lower=num;
				data->m_Upper=num;

				m_SelectCount++;

				return true;
			}
			else
				l--;
		}
		else
			data=m_Data.BlockGetAt(b,i);

		if (num>=data->m_Lower && num<=data->m_Upper) return true;

		m_SelectCount++;

		if (l>0)
		{
			ndata=m_Data.BlockPrev(b,i);
			if ((ndata->m_Upper+1)==num)
			{
				data=m_Data.BlockGetPrev(b,i);
				l--;
			}
		}

		if ((data->m_Upper+1)==num)
		{
			data->m_Upper++;
			if (l==(m_Data.GetCount()-1)) return true;

			ndata=m_Data.BlockGetNext(b,i);
			if ((ndata->m_Lower-1)!=num) return true;

			data->m_Upper=ndata->m_Upper;

			return m_Data.BlockRemove(b,i);
		}

		if ((data->m_Lower-1)==num)
		{
			data->m_Lower--;
			if (l==0) return true;

			ndata=m_Data.BlockGetPrev(b,i);
			if ((ndata->m_Upper+1)!=num) return true;

			ndata->m_Upper=data->m_Upper;

			return m_Data.BlockRemove(b,i);
		}

		if (!m_Data.BlockInsert(b,i)) return false;

		data=m_Data.BlockGetAt(b,i);
		data->m_Lower=num;
		data->m_Upper=num;

		return true;
	}

	Bool SelectAll(Int32 min, Int32 max)
	{
		DeselectAll();
		if (max<min) max=min;
		m_SelectCount=max-min+1;
		return m_Data.AddLast(SelectArrayData(min,max));
	}

	Bool Deselect(Int32 num)
	{
		Int32 l,b,i;
		l=FindBlockNum(num,b,i);
		if (l==m_Data.GetCount()) return true;

		SelectArrayData *data=m_Data.BlockGetAt(b,i);

		if (num>=data->m_Lower && num<=data->m_Upper)
		{
			m_SelectCount--;

			if (data->m_Lower==data->m_Upper) return m_Data.BlockRemove(b,i);

			if (data->m_Lower==num)
			{
				data->m_Lower++;
				return true;
			}
			else if (data->m_Upper==num)
			{
				data->m_Upper--;
				return true;
			}
			else
			{
				Int32 lo=data->m_Lower;
				data->m_Lower=num+1;

				if (!m_Data.BlockInsert(b,i)) return false;

				data=&m_Data.m_Blocks[b].data[i];

				data->m_Lower=lo;
				data->m_Upper=num-1;

			}
		}

		return true;
	}

	void DeselectAll()
	{
		m_Data.Free();
		m_SelectCount=0;
	}

	Bool Toggle(Int32 num)
	{
		if (IsSelected(num))
			return Deselect(num);
		else
			return Select(num);
	}

	Bool ToggleAll(Int32 min, Int32 max)
	{
		Int32 i;

		for (i=min; i<=max; i++)
		{
			if (!Toggle(i)) return false;
		}

		return true;
	}

	Bool GetFirstSelected(Int32 *a)
	{
		if (m_Data.m_Blocks.GetCount()==0) return false;
		*a=m_Data.m_Blocks[0].data[0].m_Lower;
		return true;
	}

	Bool GetBlockRange(Int32 &seg, Int32 maxElements, Int32 *va, Int32 *vb)
	{
		Int32 b,i;

		b=seg/SELECTBLOCKARRAY_MAX;
		i=seg-b*SELECTBLOCKARRAY_MAX;

		if (b>=m_Data.m_Blocks.GetCount() || i>=m_Data.m_Blocks[b].allocated) return false;

		SelectArrayData *data=m_Data.BlockGetAt(b,i);
		*va=data->m_Lower;
		*vb=data->m_Upper;

		if (maxElements!=LIMIT<Int32>::MAX)
		{
			if (data->m_Lower>=maxElements)
				return false;
			if (data->m_Upper>=maxElements)
				*vb=maxElements-1;
		}

		i++;
		if (i>=m_Data.m_Blocks[b].allocated)
		{
			b++;
			i=0;
		}

		seg=b*SELECTBLOCKARRAY_MAX+i;

		return true;
	}

	Bool GetRange(Int32 seg, Int32 maxElements, Int32 *a, Int32 *b)
	{
		if (seg>=m_Data.GetCount())
			return false;

		SelectArrayData *data=m_Data.GetAt(seg);
		*a=data->m_Lower;
		*b=data->m_Upper;

		if (maxElements!=LIMIT<Int32>::MAX)
		{
			if (data->m_Lower>=maxElements)
				return false;
			if (data->m_Upper>=maxElements)
				*b=maxElements-1;
		}

		return true;
	}

	Bool IsSelected(Int32 num)
	{
		return FindSegment(num);
	}

	Int32 GetSegment(Int32 b, Int32 i, Int32 *l, Int32 *u) const
	{
		*l=m_Data.m_Blocks[b].data[i].m_Lower;
		*u=m_Data.m_Blocks[b].data[i].m_Upper;
		return m_Data.GetBlockIndex(b,i);
	}

	Bool FindSegment(Int32 num)
	{
		Int32 oseg=m_LastBlockSegment,b,i;

		b=oseg/SELECTBLOCKARRAY_MAX;
		i=oseg-b*SELECTBLOCKARRAY_MAX;

		if (b<m_Data.m_Blocks.GetCount() && i<m_Data.m_Blocks[b].allocated && num>=m_Data.m_Blocks[b].data[i].m_Lower && num<=m_Data.m_Blocks[b].data[i].m_Upper) return true;

#ifdef _DEBUG
		Int32 b1=0,i1=0,b2=0,i2=0;

		Bool res1=m_Data.SearchBlockIndexMatchFull(SelectArrayData(num),b1,i1);
		Bool res2=m_Data.SearchBlockIndexMatch(SelectArrayData(num),b2,i2);

		DebugAssert(res1==res2);
		DebugAssert(b1==b2);
		DebugAssert(i1==i2);
#endif

		if (m_Data.SearchBlockIndexMatchFull(SelectArrayData(num),b,i))
		{
			m_LastBlockSegment=b*SELECTBLOCKARRAY_MAX+i;
			return true;
		}

		return false;
	}

	Bool FindBlockSegment(Int32 num, Int32 &b, Int32 &i) const
	{
		Int32 oseg=m_LastBlockSegment;

		b=oseg/SELECTBLOCKARRAY_MAX;
		i=oseg-b*SELECTBLOCKARRAY_MAX;

		if (b<m_Data.m_Blocks.GetCount() && i<m_Data.m_Blocks[b].allocated && num>=m_Data.m_Blocks[b].data[i].m_Lower && num<=m_Data.m_Blocks[b].data[i].m_Upper) return true;

#ifdef _DEBUG
		Int32 b1=0,i1=0,b2=0,i2=0;

		Bool res1=m_Data.SearchBlockIndexMatchFull(SelectArrayData(num),b1,i1);
		Bool res2=m_Data.SearchBlockIndexMatch(SelectArrayData(num),b2,i2);

		DebugAssert(res1==res2);
		DebugAssert(b1==b2);
		DebugAssert(i1==i2);
#endif

		if (m_Data.SearchBlockIndexMatchFull(SelectArrayData(num),b,i))
		{
			*((Int32*)&m_LastBlockSegment)=b*SELECTBLOCKARRAY_MAX+i; // Hack :-o
			return true;
		}

		return false;
	}

	Bool FindSegment(Int32 num, Int32 *segment)
	{
		Int32 oseg=m_LastBlockSegment,b,i;

		b=oseg/SELECTBLOCKARRAY_MAX;
		i=oseg-b*SELECTBLOCKARRAY_MAX;

		if (b<m_Data.m_Blocks.GetCount() && i<m_Data.m_Blocks[b].allocated && num>=m_Data.m_Blocks[b].data[i].m_Lower && num<=m_Data.m_Blocks[b].data[i].m_Upper)
		{
			*segment=m_Data.GetBlockIndex(b,i);
			return true;
		}

#ifdef _DEBUG
		Int32 b1=0,i1=0,b2=0,i2=0;

		Bool res1=m_Data.SearchBlockIndexMatchFull(SelectArrayData(num),b1,i1);
		Bool res2=m_Data.SearchBlockIndexMatch(SelectArrayData(num),b2,i2);

		DebugAssert(res1==res2);
		DebugAssert(b1==b2);
		DebugAssert(i1==i2);
#endif

		if (m_Data.SearchBlockIndexMatchFull(SelectArrayData(num),b,i))
		{
			m_LastBlockSegment=b*SELECTBLOCKARRAY_MAX+i;
			*segment=m_Data.GetBlockIndex(b,i);
			return true;
		}

		return false;
	}

	Bool CopyTo(SelectBlockArray *dst)
	{
		if (dst==this) return true;

		if (!m_Data.CopyTo(&dst->m_Data)) return false;
		dst->m_SelectCount=m_SelectCount;
		return true;
	}

	Bool CopyTo(BaseSelect *bs)
	{
		if (!m_Data.GetCount()) return bs->DeselectAll();

		if (!bs->CopyFrom(nullptr,m_Data.GetCount())) return false;

		m_Data.CopyTo((SelectArrayData*)bs->GetData());

		DebugAssert(sizeof(SelectArrayData)==sizeof(Int32)*2);

		return true;
	}

	Bool CopyFrom(BaseSelect *bs)
	{
		if (!bs->GetSegments())
		{
			DeselectAll();
			return true;
		}

		if (!m_Data.SetCount(bs->GetSegments())) return false;

		m_Data.CopyFrom((SelectArrayData*)bs->GetData());

		DebugAssert(sizeof(SelectArrayData)==sizeof(Int32)*2);

		m_SelectCount=bs->GetCount();

		return true;
	}
};

//////////////////////////////////////////////////////////////////////////

#undef MemAlloc
#undef MemFree

#endif // __DYNAMIC_DATA_H__
