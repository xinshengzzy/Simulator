#ifndef PIT_H
#define PIT_H
#include <string.h>
#include "PitEntry.h"
#include "MemoryPool.h"
#include "IncomingFace.h"

// we assume that pit entries with different names will have different digest values.
// we can add an entry to pit, delete an entry from the pit or add some faces to an existing entry, 
// but we cannot delete some faces from an existing entry.
class Pit
{
private:
	int getHeight(PitEntry* entry)
	{
		int height = 0;
		if(NULL != entry) height = entry->height;
		return height;
	}
	void updateHeight(PitEntry* entry)
	{
		int lchildHeight = 0;
		int rchildHeight = 0;
		if(NULL != entry->lchild)
			lchildHeight = entry->lchild->height;
		if(NULL != entry->rchild)
			rchildHeight = entry->rchild->height;
		entry->height = (lchildHeight > rchildHeight) ? lchildHeight + 1 : rchildHeight + 1;
	}

	PitEntry* connect34(PitEntry* a, PitEntry* b, PitEntry* c,\
		PitEntry* T0, PitEntry* T1, PitEntry* T2, PitEntry* T3)
	{
		a->lchild = T0; if(NULL != T0) T0->parent = a;
		a->rchild = T1; if(NULL != T1) T1->parent = a; updateHeight(a);
		c->lchild = T2; if(NULL != T2) T2->parent = c;
		c->rchild = T3; if(NULL != T3) T3->parent = c; updateHeight(c);
		b->lchild = a; a->parent = b;
		b->rchild = c; c->parent = b; updateHeight(b);

		return b;
	}

	PitEntry* rotateAt(PitEntry* g)
	{
		PitEntry* p = g->lchild;
		if(getHeight(g->rchild) > getHeight(g->lchild)) p = g->rchild;
		PitEntry* v = p->lchild;
		if(getHeight(p->rchild) > getHeight(p->lchild)) v = p->rchild;

		if(p == g->lchild)
		{
			if(v == p->lchild)
			{
				return connect34(v, p, g, v->lchild, v->rchild, p->rchild, g->rchild);
			}
			else 
			{
				return connect34(p, v, g, p->lchild, v->lchild, v->rchild, g->rchild);
			}
		}
		else
		{
			if(v == p->lchild)
			{
				return connect34(g, v, p, g->lchild, v->lchild, v->rchild, p->rchild);
			}
			else
			{
				return connect34(g, p, v, g->lchild, p->lchild, v->lchild, v->rchild);
			}
		}
	}


	// deattach all the incoming faces from the entry.
	void decomposeEntry(PitEntry* entry)
	{
		IncomingFace* face;
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		while(NULL != entry->faces)
		{
			face = entry->faces;
			entry->faces = face->next;
			memoryPool->addIncomingFace(face);
		}
	}

	// if "prefix" is the prefix of "interestPacketName", return 1; return 0 otherwise.
	// We assume 
	int match(char* prefix, char* interestPacketName)
	{
		if(NULL == prefix || NULL == interestPacketName)
		{
			printf("Error: prefix or interest packet name is null in Pit::match(...)\n");
			return -1;
		}
		int prefixLen = strlen(prefix);
		int nameLen = strlen(prefix);
		if(prefixLen > nameLen) return 0;
		for(int i = 0; i < prefixLen; ++i)
			if(prefix[i] != interestPacketName[i]) return 0;
		return 1;
	}

	void addEntry(PitEntry* entry)
	{
		PitEntry* parent = NULL;
		PitEntry* p = entries;
		while(NULL != p)
		{
			parent = p;
			if(entry->digest < p->digest) p = p->lchild;
			else p = p->rchild;
		}



		if(NULL == parent) entries = entry;
		else if(entry->digest < parent->digest)
			parent->lchild = entry;
		else 
		{
			parent->rchild = entry;
		}
		entry->parent = parent;

		p = entry;
		while(NULL != p)
		{
			int lchildHeight = 0;
			int rchildHeight = 0;
			if(NULL != p->lchild) lchildHeight = p->lchild->height;
			if(NULL != p->rchild) rchildHeight = p->rchild->height;
			if(lchildHeight - rchildHeight < -1 || lchildHeight - rchildHeight > 1) break;
			updateHeight(p);
			p = p->parent;
		}

		if(NULL != p)
		{
			parent = p->parent;
			if(NULL != parent)
			{
				if(p == parent->lchild) p = parent->lchild = rotateAt(p);
				else p = parent->rchild = rotateAt(p);
			}
			else
			{
				entries = p = rotateAt(p);
			}
			p->parent = parent;
		}
	}

public:
	PitEntry* entries;
	Pit()
	{
		entries = NULL;
	}

	void removeEntry(PitEntry* entry)
	{
		PitEntry* p = entry;
		PitEntry* parent;
		if(NULL == p->lchild)
		{
			parent = p->parent;
			if(NULL != parent)
			{
				if(p == parent->lchild) parent->lchild = p->rchild;
				else parent->rchild = p->rchild;
			}
			else entries = p->rchild;
			if(NULL != p->rchild) p->rchild->parent = parent;
		}
		else if(NULL == p->rchild)
		{
			parent = p->parent;
			if(NULL != parent)
			{
				if(p == parent->lchild) parent->lchild = p->lchild;
				else parent->rchild = p->lchild;
			}
			else entries = p->lchild;
			p->lchild->parent = parent;
		}
		else
		{
			parent = p;
			p = p->rchild;
			while(NULL != p->lchild)
			{
				parent = p;
				p = p->lchild;
			}
			p->swap(entry);
			if(p == parent->lchild) parent->lchild = p->rchild;
			else parent->rchild = p->rchild;
			if(NULL != p->rchild) p->rchild->parent = parent;
		}
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		decomposeEntry(p);			// remove all the incoming faces from the entry.
		memoryPool->addPitEntry(p);	// put the entry back to the memory pool.
		
		// Rebalance the AVL tree.
		p = parent;
		while(NULL != p)
		{
			int lchildHeight = 0;
			int rchildHeight = 0;
			if(NULL != p->lchild) lchildHeight = p->lchild->height;
			if(NULL != p->rchild) rchildHeight = p->rchild->height;
			if(lchildHeight - rchildHeight < -1 || lchildHeight - rchildHeight > 1)
			{
				parent = p->parent;
				if(NULL != parent)
				{
					if(p == parent->lchild) p = parent->lchild = rotateAt(p);
					else p = parent->rchild = rotateAt(p);
				}
				else entries = p = rotateAt(p);
				p->parent = parent;
			}
			else updateHeight(p);
			p = p->parent;
		}
	}

	PitEntry* lookup(char* interestPacketName)
	{
		Common* common = Common::getCommon();
		Digest digest = common->computeDigest(interestPacketName);
		PitEntry* p = entries;
		while(NULL != p)
		{
			if(digest < p->digest) { p = p->lchild;}
			else if(digest > p->digest) { 
				p = p->rchild; 
			}
			else if(digest == p->digest && 0 != strcmp(interestPacketName, p->interestPacketName))
			{ p = p->rchild; }
			else break;
		}
		return p;
	}

	// add 'face' to the entry corresponding to 'interestPacektName', or create a new entry 
	// if the corresponding entry doesn't exist.
	void addInfo(char* interestPacketName, int face)
	{
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		PitEntry* entry = lookup(interestPacketName);
		if(NULL == entry)
		{
			entry = memoryPool->getPitEntry();
			entry->setInterestPacketName(interestPacketName);
			addEntry(entry);
		}
		IncomingFace* incomingFace = memoryPool->getIncomingFace();
		incomingFace->face = face;
		entry->addFace(incomingFace);
		IncomingFace* iface = entry->faces;
	}
};

#endif