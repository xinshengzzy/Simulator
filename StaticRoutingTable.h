#ifndef STATIC_ROUTING_TABLE_H
#define STATIC_ROUTING_TABLE_H
#include <string.h>
#include "StaticRoutingEntry.h"
#include "Config.h"
#include "NameParser.h"
#include "Common.h"
// Remove an entry from the static routing table is not allowed.
class StaticRoutingTable{
	StaticRoutingEntry* entries;
	void updateHeight(StaticRoutingEntry* entry)
	{
		int lchildHeight = 0;
		int rchildHeight = 0;
		if(NULL != entry->lchild) lchildHeight = entry->lchild->height;
		if(NULL != entry->rchild) rchildHeight = entry->rchild->height;
		entry->height = (lchildHeight > rchildHeight) ? lchildHeight + 1 : rchildHeight + 1;
	}

	StaticRoutingEntry* connect34(StaticRoutingEntry* a, StaticRoutingEntry* b, StaticRoutingEntry* c,\
		StaticRoutingEntry* T0, StaticRoutingEntry* T1, StaticRoutingEntry* T2, StaticRoutingEntry* T3)
	{
		a->lchild = T0; if(NULL != T0) T0->parent = a;
		a->rchild = T1; if(NULL != T1) T1->parent = a; updateHeight(a);
		c->lchild = T2; if(NULL != T2) T2->parent = c;
		c->rchild = T3; if(NULL != T3) T3->parent = c; updateHeight(b);
		b->lchild = a; a->parent = b;
		b->rchild = c; c->parent = b; updateHeight(c);
		return b;
	}

	StaticRoutingEntry* rotateAt(StaticRoutingEntry* g)
	{
		StaticRoutingEntry* p = g->lchild;
		if(getHeight(g->rchild) > getHeight(g->lchild)) p = g->rchild;
		StaticRoutingEntry* v = p->lchild;
		if(getHeight(p->rchild) > getHeight(p->lchild)) v = p->rchild;
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

	int getHeight(StaticRoutingEntry* entry)
	{
		int height = 0;
		if(NULL != entry) height = entry->height;
		return height;
	}

	void addEntry(StaticRoutingEntry* entry)
	{
		StaticRoutingEntry* parent = NULL;
		StaticRoutingEntry* p = entries;
		while(NULL != p)
		{
			parent = p;
			if(entry->digest < p->digest) p = p->lchild;
			else p = p->rchild;
		}
		if(NULL == parent) entries = entry;
		else if(entry->digest < parent->digest) parent->lchild = entry;
		else parent->rchild = entry;
		entry->parent = parent;

		p = entry;
		while(NULL != p)
		{
			if(getHeight(p->lchild) - getHeight(p->rchild) > 1 || getHeight(p->lchild) - getHeight(p->rchild) < -1) break;
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

	StaticRoutingEntry* lookupPrefix(char* prefix)
	{
		Common* common = Common::getCommon();
		Digest digest = common->computeDigest(prefix);
		StaticRoutingEntry* p = entries;
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
	void addInfo(char* prefix, int face, int distance)
	{
		StaticRoutingEntry* entry = new StaticRoutingEntry(prefix, face, distance);
		addEntry(entry);
	}

	StaticRoutingEntry* lookup(char* name)
	{
		NameParser* nameParser = NameParser::getNameParser();
		nameParser->parseName(name);
		char* prefix = nameParser->components[nameParser->numOfComponents - 4];
		StaticRoutingEntry* entry = lookupPrefix(prefix);
		return entry;
	}
};
#endif