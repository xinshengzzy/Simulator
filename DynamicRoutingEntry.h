#ifndef DYNAMIC_ROUTING_ENTRY_H
#define DYNAMIC_ROUTING_ENTRY_H

#include "Config.h"
#include "Common.h"
#include "DynamicRoutingTriple.h"

class DynamicRoutingEntry
{
public:
	char* prefix;
	Digest digest;
	int height;
	DynamicRoutingTriple* triples;
	DynamicRoutingEntry* parent;
	DynamicRoutingEntry* lchild;
	DynamicRoutingEntry* rchild;
	void reset()
	{
		prefix = NULL;
		digest = 0;
		height = 1;
		triples = NULL;
		parent = lchild = rchild = NULL;
	}
	DynamicRoutingEntry()
	{
		reset();
	}
	void setPrefix(char* prefix)
	{
		Common* common = Common::getCommon();
		common->copystring(this->prefix, prefix);
		this->digest = common->computeDigest(prefix);
	}
	void addInfo(int face, int distance)
	{
		DynamicRoutingTriple* triple = triples;
		while(NULL != triple)
		{
			if(triple->face == face) break;
			triple = triple->next;
		}
		if(NULL == triple)
		{
			triple = new DynamicRoutingTriple();
			triple->face = face;
			triple->distance = (double)distance;
			triple->numOfPackets = 1;
			triple->next = triples;
			if(NULL != triples) triples->previous = triple;
			triples = triple;
		}
		else
		{
			triple->distance = (triple->distance*triple->numOfPackets + distance)/(triple->numOfPackets + 1);
			++triple->numOfPackets;
		}
	}

	void removeInfo(int face, int distance)
	{
		DynamicRoutingTriple* triple = triples;
		while(NULL != triple)
		{
			if(triple->face == face) break;
			triple = triple->next;
		}
		if(NULL != triple)
		{
			triple->distance = (triple->distance*triple->numOfPackets - distance)/(triple->numOfPackets - 1);
			--triple->numOfPackets;
			if(triple->distance <= 0 || triple->numOfPackets <= 0)
			{
				if(NULL != triple->next) triple->next->previous = triple->previous;
				if(NULL != triple->previous) triple->previous->next = triple->next;
				if(triple == triples) triples = triple->next;
				free(triple);
			}
		}

	}

	void swap(DynamicRoutingEntry* entry)
	{
		char* tempPrefix = this->prefix;
		this->prefix = entry->prefix;
		entry->prefix = tempPrefix;

		Digest tempDigest = this->digest;
		this->digest = entry->digest;
		entry->digest = tempDigest;

		int tempHeight = this->height;
		this->height = entry->height;
		entry->height = tempHeight;

		DynamicRoutingTriple* tempTriple = this->triples;
		this->triples = entry->triples;
		entry->triples = tempTriple;
	}
};
#endif