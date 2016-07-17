#ifndef CONTENT_STORE_H
#define CONTENT_STORE_H
#include <stdlib.h>
#include <string.h>
#include "ContentStoreEntry.h"
#include "DataPacket.h"
#include "MemoryPool.h"
#include "ContentStoreEntryTimestamp.h"
#include "Common.h"
#include "FileAccessInfoBase.h"
#include "Config.h"

class ContentStore
	// we construct the content store as an AVL tree.
{
private:
	ContentStoreEntryTimestamp* timestampHead; // The head pointer of the content store entry timestamps.
	ContentStoreEntryTimestamp* timestampTail; // The tail pointer of the content store entry timestamps.

	int getHeight(ContentStoreEntry* entry)
	{
		return (NULL == entry) ? 0 : entry->height;
	}

	// To simplify the problem, we assume that different entries will have different digests.
	// We call the method only when there is not an entry having the same digest with the entry to be added.
	void addEntry(ContentStoreEntry* entry)
	{
		ContentStoreEntry* p = entries;
		ContentStoreEntry* parent = NULL;
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
		
		// append the timestamp corresponding to this entry to the tail of the timestamp list.
		Common* common = Common::getCommon();
		entry->timestamp->cacheTime = common->clock;
		entry->timestamp->previous = timestampTail;
		timestampTail->next = entry->timestamp;
		timestampTail = entry->timestamp;
		entry->timestamp->next = NULL;

		// if the current caching method used is WAVE, update the file access info base.
		if(CACHING_METHOD == CACHING_METHOD_WAVE)
		{
			char* packetName = entry->dataPacket->name;
			NameParser* nameParser = NameParser::getNameParser();
			//printf("before parseName(...)\n");
			common->flag = 1;
			nameParser->parseName(packetName);
			//printf("after parseName(...)\n");
			common->flag = 0;
			char* fileName = nameParser->components[nameParser->numOfComponents - 3];
			fileAccessInfoBase->addInfo(fileName);
		}
	}

	// Remove a content store entry from the content store.
	// We assume that entry is not null.
	ContentStoreEntry* removeEntry(ContentStoreEntry* entry)
	{
		ContentStoreEntry* parent = NULL;
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
			ContentStoreEntry* w = entry;
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
		ContentStoreEntry* p = parent;
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

		// remove the timestamp corresponding to this entry from the timestamp list.
		entry->timestamp->previous->next = entry->timestamp->next;
		if(NULL != entry->timestamp->next)
			entry->timestamp->next->previous = entry->timestamp->previous;
		entry->timestamp->next = entry->timestamp->previous = NULL;
		// reset the pointers for content store entries in this entry and return it.
		entry->lchild = entry->rchild = entry->parent = NULL;
		Common* common = Common::getCommon();
		averageResidenceTime = (1 -	ALPHA)*averageResidenceTime + ALPHA*(common->clock - entry->timestamp->cacheTime);

		// if the current caching method used is WAVE, update the file access info base.
		if(CACHING_METHOD == CACHING_METHOD_WAVE)
		{
			char* packetName = entry->dataPacket->name;
			NameParser* nameParser = NameParser::getNameParser();
			nameParser->parseName(packetName);
			char* fileName = nameParser->components[nameParser->numOfComponents - 3];
			//printf("before: %s\n", fileName);
			fileAccessInfoBase->removeInfo(fileName);
			//printf("after: %s\n", fileName);
		}
		return entry;
	}



	// Update the height of this entry based on the height of its left child and right child.
	void updateHeight(ContentStoreEntry* entry)
	{
		int lchildHeight = 0;
		int rchildHeight = 0;
		if(NULL != entry->lchild) lchildHeight = entry->lchild->height;
		if(NULL != entry->rchild) rchildHeight = entry->rchild->height;
		entry->height = lchildHeight > rchildHeight ? lchildHeight + 1 : rchildHeight + 1;
	}

	// this is the universal method of rebalancing for AVL tree, which is adopted from Junhui Deng's book.
	ContentStoreEntry* connect34(ContentStoreEntry* a, ContentStoreEntry* b, ContentStoreEntry* c, \
		ContentStoreEntry* T0, ContentStoreEntry* T1, ContentStoreEntry* T2, ContentStoreEntry* T3)
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
	ContentStoreEntry* rotateAt(ContentStoreEntry* g)
	{
		ContentStoreEntry* p;
		ContentStoreEntry* v;
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

	// As the counterpart in the class ContentStoreEntry, given the name of a class,
	// this function will compute the hash code of this name.
	Digest computeDigest(char* str)
	{
		Digest hash = 7;
		int len = 0;
		if(NULL != str)
			len = strlen(str);
		for(int i = 0; i < len; ++i)
			hash = hash*31 + str[i];
		return hash;
	}

public:
	ContentStoreEntry* entries;
	int size;						// the number of entries in this content store.
	int capacity;					// the number of entries this content store could accommodate.
	double averageResidenceTime;	// the moving average duration that a data packet is kept in this content store.
	FileAccessInfoBase* fileAccessInfoBase;	// the info base is used for the caching method of WAVE only.

	ContentStore(int capacity)
	{
		this->capacity = capacity;
		size = 0;
		averageResidenceTime = 1.0;
		entries = NULL;
		timestampHead = new ContentStoreEntryTimestamp(NULL, -1.0);
		timestampTail = timestampHead;
		fileAccessInfoBase = new FileAccessInfoBase();
	}

	// Cache a data packet in this content store.
	void cacheDataPacket(DataPacket* dataPacket)
	{
		Common* common = Common::getCommon();
		DataPacket* tempDataPacket = lookup(dataPacket->name);
		if(NULL != tempDataPacket) return;
		if(NULL != TraceFile::file)
			fprintf(TraceFile::file, "%f a\n", common->clock);
		if(size < capacity)
		{
			size++;
			ContentStoreEntry* entry = new ContentStoreEntry(dataPacket);
			addEntry(entry);
		}
		else
		{
			ContentStoreEntry* entry = findEviction();
			entry = removeEntry(entry);
			entry->cacheDataPacket(dataPacket);
			addEntry(entry);
		}
	}

	// Since the content store has been full, this function will find the entry from which a data packet will
	// be evicted and in which the new data packet will be cached.
	ContentStoreEntry* findEviction()
	{
		return timestampHead->next->contentStoreEntry;
	}


	DataPacket* lookup(char* packetName)
	{
		//printf("enter lookup(...)\n");
		unsigned int digest = computeDigest(packetName);
		ContentStoreEntry* p = entries;
		DataPacket* dataPacket = NULL;
		while(NULL != p)
		{
			if(digest < p->digest) p = p->lchild;
			else if(digest > p->digest) p = p->rchild;
			else if(digest == p->digest && 0 != strcmp(packetName, p->dataPacket->name)) p = p->rchild;
			else break;
		}

		if(NULL != p)
		{
			//printf("NULL != p\n");
			if(timestampTail != p->timestamp)
			{
				p->timestamp->previous->next = p->timestamp->next;
				p->timestamp->next->previous = p->timestamp->previous;
				timestampTail->next = p->timestamp;
				p->timestamp->previous = timestampTail;
				timestampTail = p->timestamp;
			}
			MemoryPool* memoryPool = MemoryPool::getMemoryPool();
			//fprintf(TraceFile::debugFile, "ContentStore::lookup() before:\n");
			Common* common = Common::getCommon();
			//common->flag = 1;
			dataPacket = memoryPool->getDataPacket();
			//common->flag = 0;
			//fprintf(TraceFile::debugFile, "ContentStore::lookup() after:\n");
			dataPacket->copy(p->dataPacket);
		}
		return dataPacket;
	}
};
#endif