#ifndef __LISTNODE_HEADER__
#define __LISTNODE_HEADER__

#include "ge_math.h"
#include "c4d_general.h"

#ifdef __API_INTERN__
#include "ge_memory.h"
#else
#include "c4d_memory.h"
#endif

class sListNode
{
public:
	sListNode();
	virtual ~sListNode();

	virtual sListNode *Duplicate(sListNode *src);	// override if you will need copying

	void operator=(const sListNode &src)
	{
		sListNode *nd=src.head->next;
		while (nd)
		{
			Add(Duplicate(nd));
			nd=nd->next;
		}
	}

	void Construct()	// warning: to force construct for dynamic allocations
	{
		next=nullptr;
		prev=this;
		head=this;
	}

	sListNode(sListNode &src);

	sListNode *GetAt(Int32 index);

	Int32 GetCount();
	Int32 GetIndex(sListNode *fn=nullptr);

	void FlushAll();

//

	sListNode *RemoveNext();
	sListNode *RemovePrev();
	void Remove();
	void UnLink();
	void UnLinkBefore();

	sListNode *Add(sListNode *add);	// should be used from the head node
	sListNode *AddAfter(sListNode *src, sListNode *add);	// should be used from the head node
	sListNode *AddBefore(sListNode *src, sListNode *add);	// should be used from the head node

	void LinkAfter(sListNode *pnd);
	void LinkBefore(sListNode *nnd);

	virtual Bool Compare(sListNode *na, sListNode *nb, Bool fwd, Int32 mode, void *data);
	void Sort(Bool fwd=true, Int32 mode=0, void *data=nullptr);

	//

	virtual Int32 dsGetMaxSort();
	virtual Bool dsIsEnd(Int32 m, sListNode *na, sListNode *nb);
	virtual Bool dsCompare(Int32 m, sListNode *na, sListNode *nb, Bool fwd);

	sListNode *dSort(Int32 m=0, Bool fwd=true);
	void DoubleSort(Int32 mn=-1, Int32 mx=-1, Bool fwd=true);

	//

	Bool IsEmpty()
	{
		return next==nullptr;
	}

protected:

	sListNode	*next;
	sListNode	*prev;
	sListNode	*head;
};

template <class TYPE> class sDataNode : public sListNode
{
public:
	sDataNode()
	{
		m_Data=TYPE();
	}
	virtual ~sDataNode()
	{
	}

	sDataNode *RemoveNext() { return (sDataNode*)sListNode::RemoveNext(); }
	sDataNode *RemovePrev() { return (sDataNode*)sListNode::RemovePrev(); }

	inline sDataNode *GetPrev()
	{
		if (prev==head) return nullptr;
		return (sDataNode*)prev;
	}

	inline sDataNode *GetNext()
	{
		return (sDataNode*)next;
	}

	inline sDataNode *GetPrevWrap()
	{
		if (!prev || prev==head) return GetTail();
		return (sDataNode*)prev;
	}

	inline sDataNode *GetNextWrap()
	{
		if (!next) return GetFirst();
		return (sDataNode*)next;
	}

	inline sDataNode *GetHead()
	{
		return (sDataNode*)head;
	}

	inline sDataNode *GetFirst()
	{
		return (sDataNode*)(((sDataNode*)head)->next);
	}

	inline sDataNode *GetLast()
	{
		return GetTail();
	}

	inline sDataNode *GetTail()
	{
		return (sDataNode*)(((sDataNode*)head)->prev);
	}

	inline sDataNode *GetAt(Int32 index)
	{
		return (sDataNode*)sListNode::GetAt(index);
	}

	inline TYPE Get(Int32 index, TYPE def=TYPE())
	{
		sDataNode *nd=GetAt(index);
		if (!nd) return def;
		return nd->m_Data;
	}

	sDataNode *Add(TYPE data=TYPE())
	{
		sDataNode *add=(sDataNode*)sListNode::Add(NewObjClear(sDataNode));
		if (!add) return (nullptr);

		add->m_Data=data;

		return (add);
	}

	sDataNode *Add(sDataNode *node)
	{
		return (sDataNode*)sListNode::Add(node);
	}

	sDataNode *AddAfter(sDataNode *src, TYPE data)
	{
		sDataNode *add=(sDataNode*)sListNode::AddAfter(src, NewObjClear(sDataNode));
		if (!add) return (nullptr);

		add->m_Data=data;

		return (add);
	}

	sDataNode *AddBefore(sDataNode *src, TYPE data)
	{
		if (src==head) return nullptr;

		sDataNode *add=(sDataNode*)sListNode::AddBefore(src, NewObjClear(sDataNode));
		if (!add) return (nullptr);

		add->m_Data=data;

		return (add);
	}

	Bool CopyFrom(sDataNode *src)
	{
		FlushAll();

		sDataNode *node=src->GetHead();
		while ((node=node->GetNext())!=nullptr)
		{
			if (!Add(node->m_Data)) return false;
		}
		return true;
	}

	sDataNode *Find(sDataNode *fn)
	{
		sDataNode *node=GetHead();
		while ((node=node->GetNext())!=nullptr)
		{
			if (node==fn) return node;
		}
		return nullptr;
	}

	sDataNode *Find(TYPE data)
	{
		sDataNode *node=GetHead();
		while ((node=node->GetNext())!=nullptr)
		{
			if (node->m_Data==data) return node;
		}
		return nullptr;
	}

	sDataNode *FindNext(TYPE data, sDataNode *node)
	{
		if (!node) node=GetHead();
		return node->FindNext(data);
	}

	sDataNode *FindNext(TYPE data)
	{
		sDataNode *node=this;
		while ((node=node->GetNext())!=nullptr)
		{
			if (node->m_Data==data) return node;
		}
		return nullptr;
	}

	virtual Bool Compare(sListNode *na, sListNode *nb, Bool fwd, Int32 mode, void *data)
	{
		sDataNode *da=(sDataNode*)na,*db=(sDataNode*)nb;

		if (fwd)
			return da->m_Data<db->m_Data;
		else
			return da->m_Data>db->m_Data;
	}

	TYPE m_Data;
};

#endif //__LISTNODE_HEADER__
