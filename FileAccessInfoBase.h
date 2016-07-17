#ifndef FILE_ACCESS_INFO_BASE_H
#define FILE_ACCESS_INFO_BASE_H
#include "FileAccessInfo.h"
#include "Common.h"
#include "NameParser.h"
#include "MemoryPool.h"
class FileAccessInfoBase
{
private:
	FileAccessInfo* root;
	void updateHeight(FileAccessInfo* entry)
	{
		int lchildHeight = 0;
		int rchildHeight = 0;
		if(NULL != entry->lchild) lchildHeight = entry->lchild->height;
		if(NULL != entry->rchild) rchildHeight = entry->rchild->height;
		entry->height = (lchildHeight > rchildHeight) ? lchildHeight + 1 : rchildHeight + 1;
	}

	FileAccessInfo* connect34(FileAccessInfo* a, FileAccessInfo* b, FileAccessInfo* c,\
		FileAccessInfo* T0, FileAccessInfo* T1, FileAccessInfo* T2, FileAccessInfo* T3)
	{
		a->lchild = T0; if(NULL != T0) T0->parent = a;
		a->rchild = T1; if(NULL != T1) T1->parent = a; updateHeight(a);
		c->lchild = T2; if(NULL != T2) T2->parent = c;
		c->rchild = T3; if(NULL != T3) T3->parent = c; updateHeight(b);
		b->lchild = a; a->parent = b;
		b->rchild = c; c->parent = b; updateHeight(c);
		return b;
	}

	FileAccessInfo* rotateAt(FileAccessInfo* g)
	{
		FileAccessInfo* p = g->lchild;
		if(getHeight(g->rchild) > getHeight(g->lchild)) p = g->rchild;
		FileAccessInfo* v = p->lchild;
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

	int getHeight(FileAccessInfo* entry)
	{
		int height = 0;
		if(NULL != entry) height = entry->height;
		return height;
	}

public:
	void addEntry(FileAccessInfo* entry)
	{
		FileAccessInfo* parent = NULL;
		FileAccessInfo* p = root;
		while(NULL != p)
		{
			parent = p;
			if(entry->digest < p->digest) p = p->lchild;
			else p = p->rchild;
		}
		if(NULL == parent) root = entry;
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
			else root = p = rotateAt(p);
			p->parent = parent;
		}
	}

	// Remove a file access info entry from the file access info base.
	// We assume that entry is not null.
	void removeEntry(FileAccessInfo* entry)
	{
		FileAccessInfo* parent = NULL;
		if(NULL == entry->lchild)	// If the left child is NULL, replace the node by its right subtree.
		{
			parent = entry->parent;
			if(NULL != parent)
			{
				if(parent->lchild == entry) parent->lchild = entry->rchild;
				else parent->rchild = entry->rchild;
			}
			else root = entry->rchild;
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
			else root = entry->lchild;
			entry->lchild->parent = parent;
		}
		else // If neither its left child nor its right child is NULL, exchange the node with its successor.
		{
			FileAccessInfo* w = entry;
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
		FileAccessInfo* p = parent;
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
					root = p = rotateAt(p);
				}
				p->parent = parent;
			}
			else updateHeight(p);
			p = p->parent;
		}

		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		memoryPool->addFileAccessInfo(entry);
	}

	FileAccessInfoBase()
	{
		root = NULL;
	}
	// a data packet is cached in the content store, so the number of packets corresponding to the source file will
	// be increased by 1.
	void addInfo(char* fileName)
	{
		FileAccessInfo* entry = lookupByFileName(fileName);
		if(NULL == entry)
		{
			MemoryPool* memoryPool = MemoryPool::getMemoryPool();
			entry = memoryPool->getFileAccessInfo();
			entry->setFileName(fileName);
			addEntry(entry);
		}
		++entry->numOfPackets;
	}

	// a data packet is removed from the content store, so the number of packets corresponding to the source file will
	// will be decreased by 1.
	void removeInfo(char* fileName)
	{
		//printf("begin: %s\n", fileName);
		FileAccessInfo* entry = lookupByFileName(fileName);
		//printf("middle: %s\n", fileName);
		if(NULL != entry)
		{
			--entry->numOfPackets;
			if(0 == entry->numOfPackets) removeEntry(entry);
		}
		//printf("end: %s\n", fileName);
	}

	FileAccessInfo* lookupByFileName(char* fileName)
	{
		Common* common = Common::getCommon();
		Digest digest = common->computeDigest(fileName);
		FileAccessInfo* p = root;
		while(NULL != p)
		{
			if(digest < p->digest) p = p->lchild;
			else if(digest > p->digest) p = p->rchild;
			else if(digest == p->digest && 0 != strcmp(fileName, p->fileName)) p = p->rchild;
			else break;
		}
		return p;
	}

	FileAccessInfo* lookupByPacketName(char* packetName)
	{
		NameParser* nameParser = NameParser::getNameParser();
		nameParser->parseName(packetName);
		char* fileName = nameParser->components[nameParser->numOfComponents - 3];
		Common* common = Common::getCommon();
		Digest digest = common->computeDigest(fileName);
		FileAccessInfo* p = root;
		while(NULL != p)
		{
			if(digest < p->digest) p = p->lchild;
			else if(digest > p->digest) p = p->rchild;
			else if(digest == p->digest && 0 != strcmp(fileName, p->fileName)) p = p->rchild;
			else break;
		}
		return p;
	}

};
#endif