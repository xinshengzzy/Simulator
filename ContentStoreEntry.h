#ifndef CONTENT_STORE_ENTRY_H
#define CONTENT_STORE_ENTRY_H
#include <string.h>
#include <memory.h>
#include "DataPacket.h"
#include "MemoryPool.h"
#include "Common.h"
#include "ContentStoreEntryTimestamp.h"

class ContentStoreEntry
{
public:
	DataPacket* dataPacket;
	unsigned int digest;	// hash code of the packet's name
	int height;
	ContentStoreEntryTimestamp* timestamp;	// When is the data packet corresponding to the entry is cached.
	// However, the main purpose of the member is to implement LRU eviction time.
	ContentStoreEntry* lchild;
	ContentStoreEntry* rchild;
	ContentStoreEntry* parent;


	ContentStoreEntry(DataPacket* dataPacket)
	{
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		//fprintf(TraceFile::debugFile, "ContentStoreEntry(...) before:\n");
		Common* common = Common::getCommon();
		//common->flag = 1;
		this->dataPacket = memoryPool->getDataPacket();
		//common->flag = 0;
		//fprintf(TraceFile::debugFile, "ContentStoreEntry(...) after:\n");
		this->dataPacket->copy(dataPacket);
		digest = computeDigest(dataPacket->name);
		height = 0;
		lchild = NULL;
		rchild = NULL;
		parent = NULL;
		timestamp = new ContentStoreEntryTimestamp(this, common->clock);
	}

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

	void cacheDataPacket(DataPacket* dataPacket)
	{
		this->dataPacket->copy(dataPacket);
		digest = computeDigest(dataPacket->name);
		height = 1;
		Common* common = Common::getCommon();
		timestamp->cacheTime = common->clock;
	}

	void swap(ContentStoreEntry* entry)
	{
		DataPacket* dataPacket = this->dataPacket;
		this->dataPacket = entry->dataPacket;
		entry->dataPacket = dataPacket;

		ContentStoreEntryTimestamp* timestamp;
		timestamp = this->timestamp;
		this->timestamp = entry->timestamp;
		entry->timestamp = timestamp;
		this->timestamp->contentStoreEntry = this;
		entry->timestamp->contentStoreEntry = entry;

		Digest digest;
		digest = this->digest;
		this->digest = entry->digest;
		entry->digest = digest;

		int height;
		height = this->height;
		this->height = entry->height;
		entry->height = height;
	}
};
#endif