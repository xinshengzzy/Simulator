#ifndef DYNAMIC_ROUTING_TABLE_H
#define DYNAMIC_ROUTING_TABLE_H
#include "DynamicRoutingEntry.h"
#include "NameParser.h"
class DynamicRoutingTable
{
	int getHeight(DynamicRoutingEntry* entry)
	{
		return (NULL == entry) ? 0 : entry->height;
	}

	// To simplify the problem, we assume that different entries will have different digests.
	// We call the method only when there is not an entry having the same digest with the entry to be added.
	void addEntry(DynamicRoutingEntry* entry)
	{
		DynamicRoutingEntry* p = entries;
		DynamicRoutingEntry* parent = NULL;
		while(NULL != p)
		{
			parent = p;
			if(entry->digest > p->digest) p = p->rchild;
			else p = p->lchild;
		}

		if(NULL == parent) entries = entry;
		else if(entry->digest > parent->digest) parent->rchild = entry;
		else parent->lchild = entry;
		entry->parent = parent;

		p = entry;
		while(NULL != p)
		{
			int lchildHeight = 0;
			int rchildHeight = 0;
			if(NULL != p->lchild) lchildHeight = p->lchild->height;
			if(NULL != p->rchild) rchildHeight = p->rchild->height;
			if(lchildHeight - rchildHeight > 1 || lchildHeight - rchildHeight < -1) break;
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
			else entries = p = rotateAt(p);
			p->parent = parent;
		}
	}

	// Remove a content store entry from the content store.
	// We assume that entry is not null.
	void removeEntry(DynamicRoutingEntry* entry)
	{
		DynamicRoutingEntry* parent = NULL;
		if(NULL == entry->lchild)	// If the left child is NULL, replace the node by its right subtree.
		{
			parent = entry->parent;
			if(NULL != parent)
			{
				if(parent->lchild == entry) parent->lchild = entry->rchild;
				else parent->rchild = entry->rchild;
			}
			else entries = entry->rchild;
			if(NULL != entry->rchild) entry->rchild->parent = parent;
		}
		else if(NULL == entry->rchild)	// If the right child is NULL, replace the node by its left subtree.
		{
			parent = entry->parent;
			if(NULL != parent)
			{
				if(entry == parent->lchild) parent->lchild = entry->lchild;
				else parent->rchild = entry->lchild;
			}
			else entries = entry->lchild;
			entry->lchild->parent = parent;
		}
		else // If neither its left child nor its right child is NULL, exchange the node with its successor.
		{
			DynamicRoutingEntry* w = entry;
			entry = w->rchild;
			while(NULL != entry->lchild)
				entry = entry->lchild;
			entry->swap(w);

			parent = entry->parent;
			if(entry == parent->lchild) parent->lchild = entry->rchild;
			else parent->rchild = entry->rchild;
			if(NULL != entry->rchild) entry->rchild->parent = parent;
		}

		// rebalance the AVL tree.
		DynamicRoutingEntry* p = parent;
		while(NULL != p)
		{
			parent = p->parent;
			int lchildHeight = 0;
			int rchildHeight = 0;
			if(NULL != p->lchild) lchildHeight = p->lchild->height;
			if(NULL != p->rchild) rchildHeight = p->rchild->height;
			if(lchildHeight - rchildHeight > 1 || lchildHeight - rchildHeight < -1)
			{
				if(NULL != parent)
				{
					if(p == parent->lchild) {
						p = parent->lchild = rotateAt(p);
					}
					else {
						p = parent->rchild = rotateAt(p);
					}
				}
				else {
					entries = p = rotateAt(p);
				}
				p->parent = parent;
			}
			else updateHeight(p);
			p = p->parent;
		}

		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		memoryPool->addDynamicRoutingEntry(entry);
	}



	// Update the height of this entry based on the height of its left child and right child.
	void updateHeight(DynamicRoutingEntry* entry)
	{
		int lchildHeight = 0;
		int rchildHeight = 0;
		if(NULL != entry->lchild) lchildHeight = entry->lchild->height;
		if(NULL != entry->rchild) rchildHeight = entry->rchild->height;
		entry->height = lchildHeight > rchildHeight ? lchildHeight + 1 : rchildHeight + 1;
	}

	// this is the universal method of rebalancing for AVL tree, which is adopted from Junhui Deng's book.
	DynamicRoutingEntry* connect34(DynamicRoutingEntry* a, DynamicRoutingEntry* b, DynamicRoutingEntry* c, \
		DynamicRoutingEntry* T0, DynamicRoutingEntry* T1, DynamicRoutingEntry* T2, DynamicRoutingEntry* T3)
	{
		a->lchild = T0; if(NULL != T0) T0->parent = a;
		a->rchild = T1; if(NULL != T1) T1->parent = a; updateHeight(a);
		c->lchild = T2; if(NULL != T2) T2->parent = c;
		c->rchild = T3; if(NULL != T3) T3->parent = c; updateHeight(c);
		b->lchild = a; a->parent = b;
		b->rchild = c; c->parent = b; updateHeight(b);
		return b;
	}

	// node g is the deepest node which is unbalanced. We will rebalance the tree rooted at g.
	DynamicRoutingEntry* rotateAt(DynamicRoutingEntry* g)
	{
		DynamicRoutingEntry* p;
		DynamicRoutingEntry* v;
		if(getHeight(g->lchild) > getHeight(g->rchild)) p = g->lchild;
		else p = g->rchild;
		if(getHeight(p->lchild) > getHeight(p->rchild)) v = p->lchild;
		else v = p->rchild;
		if(p == g->lchild)
		{
			if(v == p->lchild) return connect34(v, p, g, v->lchild, v->rchild, p->rchild, g->rchild);
			else return connect34(p, v, g, p->lchild, v->lchild, v->rchild, g->rchild);
		}
		else
		{
			if(v == p->lchild) return connect34(g, v, p, g->lchild, v->lchild, v->rchild, p->rchild);
			else return connect34(g, p, v, g->lchild, p->lchild, v->lchild, v->rchild);
		}
	}

	DynamicRoutingEntry* lookupPrefix(char* prefix)
	{
		Common* common = Common::getCommon();
		Digest digest = common->computeDigest(prefix);
		DynamicRoutingEntry* p = entries;
		while(NULL != p)
		{
			if(digest < p->digest) p = p->lchild;
			else if(digest > p->digest) p = p->rchild;
			else if(digest == p->digest && 0 != strcmp(prefix, p->prefix)) p = p->rchild;
			else break;
		}
		return p;
	}


public:
	DynamicRoutingEntry* entries;

	void addInfo(char* name, int face, int distance, double lifetime)
	{
		NameParser* nameParser = NameParser::getNameParser();
		Common* common = Common::getCommon();
		EventQueue* eventQueue = EventQueue::getEventQueue();
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		nameParser->parseName(name);
		char* prefix = nameParser->components[nameParser->numOfComponents - 3];
		DynamicRoutingEntry* entry = lookupPrefix(prefix);
		if(NULL == entry)
		{
			MemoryPool* memoryPool = MemoryPool::getMemoryPool();
			entry = memoryPool->getDynamicRoutingEntry();
			entry->setPrefix(prefix);
			addEntry(entry);
		}
		entry->addInfo(face, distance);
		DynamicRoutingUpdateEvent* dynamicRoutingUpdateEvent = memoryPool->getDynamicRoutingUpdateEvent();
		common->copystring(dynamicRoutingUpdateEvent->name, name);
		dynamicRoutingUpdateEvent->face = face;
		dynamicRoutingUpdateEvent->dist = distance;
		dynamicRoutingUpdateEvent->dynamicRoutingTable = this;
		dynamicRoutingUpdateEvent->time = common->clock + lifetime;
		eventQueue->appendEvent(dynamicRoutingUpdateEvent);
	}

	void removeInfo(char* name, int face, int distance)
	{
		NameParser* nameParser = NameParser::getNameParser();
		nameParser->parseName(name);
		char* prefix = nameParser->components[nameParser->numOfComponents - 3];
		DynamicRoutingEntry* entry = lookupPrefix(prefix);
		if(NULL != entry)
		{
			entry->removeInfo(face, distance);
			if(NULL == entry->triples)
				removeEntry(entry);
		}
	}

	DynamicRoutingEntry* lookup(char* name)
	{
		NameParser* nameParser = NameParser::getNameParser();
		nameParser->parseName(name);
		char* prefix =nameParser->components[nameParser->numOfComponents - 3];
		DynamicRoutingEntry* entry = lookupPrefix(prefix);
		return entry;
	}
};
#endif