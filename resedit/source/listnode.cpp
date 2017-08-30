#include "listnode.h"

sListNode::sListNode()
{
	next=nullptr;
	prev=this;
	head=this;
}

sListNode::~sListNode()
{
	if (head==this) FlushAll();
}

sListNode *sListNode::Duplicate(sListNode *src)	// override if you will need copying
{
	return (nullptr);
}

sListNode::sListNode(sListNode &src)
{
	sListNode *nd=src.head->next;
	while (nd)
	{
		Add(Duplicate(nd));
		nd=nd->next;
	}
}

sListNode *sListNode::GetAt(Int32 index)
{
	Int32 cnt=0;
	sListNode *nd=(head==this)?next:this;
	while (nd)
	{
		if (cnt==index)
			return (nd);

		cnt++;
		nd=nd->next;
	}

	return (nullptr);
}

Int32 sListNode::GetCount()
{
	Int32 cnt=0;
	sListNode *nd=(head==this)?next:this;
	while (nd)
	{
		cnt++;
		nd=nd->next;
	}
	return (cnt);
}

Int32 sListNode::GetIndex(sListNode *fn)
{
	Int32 cnt=0;
	if (!fn) fn=this;
	sListNode *nd=head->next;
	while (nd)
	{
		if (nd==fn) return (cnt);
		cnt++;
		nd=nd->next;
	}
	return (-1);
}

void sListNode::FlushAll()
{
	if (this!=head)
	{
		head->FlushAll();
		return;
	}

	sListNode *nd=next,*nxt;
	while (nd)
	{
		nxt=nd->next;
		DeleteObj(nd);
		nd=nxt;
	}

	next=nullptr;
	prev=this;
}

sListNode *sListNode::RemoveNext()
{
	sListNode *nxt=next;
	Remove();
	return (nxt);
}

sListNode *sListNode::RemovePrev()
{
	sListNode *nxt=prev;
	Remove();
	return (nxt);
}

void sListNode::Remove()
{
	if (head->prev==this) head->prev=prev;

	if (prev) prev->next=next;
	if (next) next->prev=prev;

	sListNode *ptr=this;
	DeleteObj(ptr);
}

void sListNode::UnLink()
{
	if (head->prev==this) head->prev=prev;

	if (prev) prev->next=next;
	if (next) next->prev=prev;

	prev=nullptr;
	next=nullptr;
}

void sListNode::UnLinkBefore()
{
	if (head->prev==this) head->prev=prev;

	if (prev) prev->next=nullptr;
	prev=nullptr;
}

sListNode *sListNode::Add(sListNode *add)	// should be used from the head node
{
	if (this!=head) return (head->Add(add));

	if (!add) return (nullptr);

	add->head=head;
	prev->next=add;
	add->prev=prev;
	add->next=nullptr;
	prev=add;

	return (add);
}

sListNode *sListNode::AddAfter(sListNode *src, sListNode *add)	// should be used from the head node
{
	if (this!=head) return (head->AddAfter(src, add));
	if (!add) return (nullptr);

	add->head=head;
	add->LinkAfter(src);

	return (add);
}

sListNode *sListNode::AddBefore(sListNode *src, sListNode *add)	// should be used from the head node
{
	if (this!=head) return (head->AddBefore(src, add));
	if (!add) return (nullptr);

	add->head=head;
	add->LinkBefore(src);

	return (add);
}

void sListNode::LinkAfter(sListNode *pnd)
{
	if (!pnd) return;

	if (pnd->next) pnd->next->prev=this;

	head=pnd->head;
	next=pnd->next;
	pnd->next=this;
	prev=pnd;

	if (pnd->head->prev==pnd) pnd->head->prev=this;
}

void sListNode::LinkBefore(sListNode *nnd)
{
	if (!nnd) return;

	if (nnd->prev) nnd->prev->next=this;

	head=nnd->head;
	prev=nnd->prev;
	nnd->prev=this;
	next=nnd;
}

Bool sListNode::Compare(sListNode *na, sListNode *nb, Bool fwd, Int32 mode, void *data)
{
	#ifndef __API_INTERN__
		DebugAssert(false);
	#endif

	return (true);
}

void sListNode::Sort(Bool fwd, Int32 mode, void *data)	// fwd = small to large
{
	sListNode *list=head->next;
	sListNode *p,*q,*e,*tail;
	Int32 insize, nmerges, psize, qsize, i;

	if (!list) return;

	insize = 1;
	while (true)
	{
		p = list;
		list = nullptr;
		tail = nullptr;

		nmerges = 0;  // count number of merges we do in this pass

		while (p)
		{
			nmerges++;  // there exists a merge to be done
			// step `insize' places along from p
			q = p;
			psize = 0;

			for (i = 0; i < insize; i++)
			{
				psize++;
				q = q->next;
				if (!q) break;
			}

			// if q hasn't fallen off end, we have two lists to merge
			qsize = insize;

			// now we have two lists; merge them
			while (psize > 0 || (qsize > 0 && q))
			{
				// decide whether next element of merge comes from p or q
				if (psize == 0)
				{
					// p is empty; e must come from q.
					e = q; q = q->next; qsize--;
				}
				else if (qsize == 0 || !q)
				{
					// q is empty; e must come from p.
					e = p; p = p->next; psize--;
				}
				else if (Compare(p,q,fwd,mode,data))
				{
					//* First element of p is lower (or same), e must come from p.
					e = p; p = p->next; psize--;
				}
				else
				{
					// First element of q is lower; e must come from q.
					e = q; q = q->next; qsize--;
				}

				// add the next element to the merged list
				if (tail)
					tail->next = e;
				else
					list = e;

				// Maintain reverse pointers in a doubly linked list.
				e->prev = tail;
				tail = e;
			}

			// now p has stepped `insize' places along, and q has too
			p = q;
		}

		tail->next = nullptr;

		// If we have done only one merge, we're finished.
		if (nmerges <= 1)   // allow for nmerges==0, the empty list case
		{
			head->next=list;
			list->prev=head;
			head->prev=tail;
			return;
		}

		// Otherwise repeat, merging lists twice the size
		insize *= 2;
	}
}

//

Int32 sListNode::dsGetMaxSort()
{
	return (-1);
}

Bool sListNode::dsIsEnd(Int32 m, sListNode *na, sListNode *nb)
{
	return (true);
}

Bool sListNode::dsCompare(Int32 m, sListNode *na, sListNode *nb, Bool fwd)
{
	return (true);
}

sListNode *sListNode::dSort(Int32 m, Bool fwd)		// NOTE: must not change order of "same" items
{
	if (!next) return (nullptr);

	sListNode *sn=next->next;
	if (!sn) return (nullptr);

	sListNode *bn=next;

	while (sn && !dsIsEnd(m,bn,sn))
	{
		sListNode *pn=sn->prev;
		while (pn!=this)
		{
			if (dsCompare(m,pn,sn,fwd)) break;
			pn=(sListNode*)pn->prev;
		}
		if (pn!=sn->prev)
		{
			sListNode *nn=sn->next;
			sn->UnLink();
			sn->LinkAfter(pn);
			sn=nn;
		}
		else
			sn=sn->next;
	}

	return (sn);
}

void sListNode::DoubleSort(Int32 mn, Int32 mx, Bool fwd)
{
	if (mn==-1) mn=0;
	if (mx==-1) mx=dsGetMaxSort();

	for (Int32 m=mn;m<=mx;m++)
	{
		sListNode *sn=head;
		while (sn)
		{
			sn=sn->dSort(m,fwd);
			if (sn) sn=sn->prev;
		}
	}
}

