#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include "InterestPacket.h"
#include "DataPacket.h"
#include "NackPacket.h"
#include "ProcessPacketEvent.h"
#include "InterestPacketTimeoutEvent.h"
#include "IncomingFace.h"
#include "PitEntry.h"
#include "GenerateInterestEvent.h"
#include "OutstandingInterestPacket.h"
#include "DynamicRoutingTriple.h"
#include "DynamicRoutingEntry.h"
#include "DynamicRoutingUpdateEvent.h"
#include "FileAccessInfo.h"

class MemoryPool
{
private:
	InterestPacket* interestPacketList;
	DataPacket* dataPacketList;
	NackPacket* nackPacketList;
	ProcessPacketEvent* processPacketEventList;
	InterestPacketTimeoutEvent* interestPacketTimeoutEventList;
	IncomingFace* incomingFaceList;
	PitEntry* pitEntryList;
	GenerateInterestEvent* generateInterestEventList;
	OutstandingInterestPacket* outstandingInterestPacketList;
	DynamicRoutingTriple* dynamicRoutingTripleList;
	DynamicRoutingEntry* dynamicRoutingEntryList;
	DynamicRoutingUpdateEvent* dynamicRoutingUpdateEventList;
	FileAccessInfoPerFace* fileAccessInfoPerFaceList;
	FileAccessInfo* fileAccessInfoList;
	
	// these variables are used for debugging only.
	static int count;
	
	static MemoryPool* memoryPool;

	MemoryPool()
	{
		//printf("MemoryPool()\n");
		interestPacketList = NULL;
		dataPacketList = NULL;
		nackPacketList = NULL;
		processPacketEventList = NULL;
		interestPacketTimeoutEventList = NULL;
		incomingFaceList = NULL;
		pitEntryList = NULL;
		generateInterestEventList = NULL;
		outstandingInterestPacketList = NULL;
		dynamicRoutingTripleList = NULL;
		dynamicRoutingEntryList = NULL;
		dynamicRoutingUpdateEventList = NULL;
		fileAccessInfoPerFaceList = NULL;
		fileAccessInfoList = NULL;
	}

public:
	static MemoryPool* getMemoryPool()
	{
		if(NULL == memoryPool)
			memoryPool = new MemoryPool();
		return memoryPool;
	}

	// Get an interest packet from the event pool.
	InterestPacket* getInterestPacket()
	{
//		int len1 = getInterestPacketListLength();
		if(NULL == interestPacketList)
		{
			interestPacketList = new InterestPacket();
//			fprintf(TraceFile::debugFile, "new InterestPacket()\n");
		}
		InterestPacket* p = interestPacketList;
		interestPacketList = (InterestPacket*)p->next;
		p->next = NULL;
//		int len2 = getInterestPacketListLength();
//		fprintf(TraceFile::debugFile, "getInterestPacket(): len1=%d, len2=%d\n", len1, len2);
		return p;
	}

	// Add an interest packet to the event pool.
	void addInterestPacket(InterestPacket* p)
	{
//		if(141856 == p->id)
	//		printf("done\n");
/*		int ids[] = {			};
		int num = 251;
		if(count < num)
		{
			for(int i = 0; i < num; ++i)
				if(p->id == ids[i])
				{
					printf("%d: %d is here\n", count, ids[i]);
					++count;
				}
		}
		else if(num == count) 
		{
			printf("finish\n");
			++count;
		}*/
//		int len1 = getInterestPacketListLength();
		p->reset();
		p->next = interestPacketList;
		interestPacketList = p;
//		int len2 = getInterestPacketListLength();
		//fprintf(TraceFile::debugFile, "addInterestPacket(...): len1=%d, len2=%d\n", len1, len2);
	}

	int getInterestPacketListLength()
	{
		InterestPacket* p = interestPacketList;
		int len = 0;
		while(NULL != p)
		{
			++len;
			p = (InterestPacket*)p->next;
		}
		return len;
	}

	// Get an data packet from the event pool.
	DataPacket* getDataPacket()
	{
		if(NULL == dataPacketList)
			dataPacketList = new DataPacket();
		DataPacket* p = dataPacketList;
		dataPacketList = (DataPacket*)p->next;
		p->next = NULL;
		return p;
	}

	// Add a data packet to the event pool.
	void addDataPacket(DataPacket* p)
	{
		//fprintf(TraceFile::debugFile, "inside addDataPacket(...)\n");
		p->reset();
		p->next = dataPacketList;
		dataPacketList = p;
	}

	// Get an NACK packet from the event pool.
	NackPacket* getNackPacket()
	{
		if(NULL == nackPacketList)
			nackPacketList = new NackPacket();
		NackPacket* p = nackPacketList;
		nackPacketList = (NackPacket*)nackPacketList->next;
		p->next = NULL;
		return p;
	}

	// Add a nack packet to the event pool.
	void addNackPacket(NackPacket* p)
	{
		p->reset();
		p->next = nackPacketList;
		nackPacketList = p;
	}

	// Get a notification event from the event pool
	ProcessPacketEvent* getProcessPacketEvent()
	{
		if(NULL == processPacketEventList)
			processPacketEventList = new ProcessPacketEvent();
		ProcessPacketEvent* e = processPacketEventList;
		processPacketEventList = (ProcessPacketEvent*)e->next;
		e->next = NULL;
		return e;
	}

	// Add a notification event to the event pool
	void addProcessPacketEvent(ProcessPacketEvent* e)
	{
		e->reset();
		e->next = processPacketEventList;
		processPacketEventList = e;
	}

	// Get a interest packet timeout event from the event pool.
	InterestPacketTimeoutEvent* getInterestPacketTimeoutEvent()
	{
		if(NULL == interestPacketTimeoutEventList)
			interestPacketTimeoutEventList = new InterestPacketTimeoutEvent();
		InterestPacketTimeoutEvent* e = interestPacketTimeoutEventList;
		interestPacketTimeoutEventList = (InterestPacketTimeoutEvent*)e->next;
		e->next = NULL;
		return e;
	}

	// Add a interest packet timeout event to the event pool
	void addInterestPacketTimeoutEvent(InterestPacketTimeoutEvent* interestPacketTimeoutEvent)
	{
		if(NULL != interestPacketTimeoutEvent->outstandingInterestPacket)
		{
			OutstandingInterestPacket* outstandingInterestPacket = interestPacketTimeoutEvent->outstandingInterestPacket;
			interestPacketTimeoutEvent->outstandingInterestPacket = NULL;
			outstandingInterestPacket->interestPacketTimeoutEvent = NULL;
			addOutstandingInterestPacket(outstandingInterestPacket);
		}
		interestPacketTimeoutEvent->reset();
		interestPacketTimeoutEvent->next = interestPacketTimeoutEventList;
		interestPacketTimeoutEventList = interestPacketTimeoutEvent;
	}

	// Get an incoming face object from the memory pool.
	IncomingFace* getIncomingFace()
	{
		if(NULL == incomingFaceList)
			incomingFaceList = new IncomingFace();
		IncomingFace* face = incomingFaceList;
		incomingFaceList = face->next;
		face->next = NULL;
		return face;
	}

	// Add an incoming face object to the memory pool.
	void addIncomingFace(IncomingFace* face)
	{
		face->reset();
		face->next = incomingFaceList;
		incomingFaceList = face;
	}

	// Get a pit entry object from the memory pool.
	PitEntry* getPitEntry()
	{
		if(NULL == pitEntryList)
			pitEntryList = new PitEntry();
		PitEntry* pitEntry = pitEntryList;
		pitEntryList = pitEntry->lchild;
		pitEntry->lchild = NULL;
		return pitEntry;
	}

	// Add a pit entry object to the memory pool.
	void addPitEntry(PitEntry* pitEntry)
	{
		if(NULL != pitEntry->faces)
			printf("NULL != pitEntry->faces\n");
		IncomingFace* face = pitEntry->faces;
		while(NULL != face)
		{
			pitEntry->faces = face->next;
			addIncomingFace(face);
			face = pitEntry->faces;
		}
		pitEntry->reset();
		pitEntry->lchild = pitEntryList;
		pitEntryList = pitEntry;
	}

	// Get a generate interest event from the memory pool.
	GenerateInterestEvent* getGenerateInterestEvent()
	{
		if(NULL == generateInterestEventList)
			generateInterestEventList = new GenerateInterestEvent();
		GenerateInterestEvent* generateInterestEvent = generateInterestEventList;
		generateInterestEventList = (GenerateInterestEvent*)generateInterestEvent->next;
		generateInterestEvent->next = NULL;
		return generateInterestEvent;
	}

	// Add a generate interest event to the memory pool.
	void addGenerateInterestEvent(GenerateInterestEvent* generateInterestEvent)
	{
		generateInterestEvent->reset();
		generateInterestEvent->next = generateInterestEventList;
		generateInterestEventList = generateInterestEvent;
	}

	// Get an outstanding interest packet from the memory pool.
	OutstandingInterestPacket* getOutstandingInterestPacket()
	{
		if(NULL == outstandingInterestPacketList)
			outstandingInterestPacketList = new OutstandingInterestPacket();
		OutstandingInterestPacket* outstandingInterestPacket = outstandingInterestPacketList;
		outstandingInterestPacketList = outstandingInterestPacket->next;
		outstandingInterestPacket->next = NULL;
		return outstandingInterestPacket;
	}

	// Add an outstanding interest packet to the memory pool.
	void addOutstandingInterestPacket(OutstandingInterestPacket* outstandingInterestPacket)
	{
		if(NULL != outstandingInterestPacket->interestPacketTimeoutEvent)
		{
			InterestPacketTimeoutEvent* interestPacketTimeoutEvent = outstandingInterestPacket->interestPacketTimeoutEvent;
			outstandingInterestPacket->interestPacketTimeoutEvent = NULL;
			interestPacketTimeoutEvent->outstandingInterestPacket = NULL;
			addInterestPacketTimeoutEvent(interestPacketTimeoutEvent);
		}
		outstandingInterestPacket->reset();
		outstandingInterestPacket->next = outstandingInterestPacketList;
		outstandingInterestPacketList = outstandingInterestPacket;
	}

	DynamicRoutingTriple* getDynamicRoutingTriple()
	{
		if(NULL == dynamicRoutingTripleList) 
			dynamicRoutingTripleList = new DynamicRoutingTriple();
		DynamicRoutingTriple* dynamicRoutingTriple = dynamicRoutingTripleList;
		dynamicRoutingTripleList = dynamicRoutingTriple->next;
		dynamicRoutingTriple->next = NULL;
		return dynamicRoutingTriple;
	}

	void addDynamicRoutingTriple(DynamicRoutingTriple* dynamicRoutingTriple)
	{
		dynamicRoutingTriple->reset();
		dynamicRoutingTriple->next = dynamicRoutingTripleList;
		dynamicRoutingTripleList = dynamicRoutingTriple;
	}

	DynamicRoutingEntry* getDynamicRoutingEntry()
	{
		if(NULL == dynamicRoutingEntryList)
			dynamicRoutingEntryList = new DynamicRoutingEntry();
		DynamicRoutingEntry* dynamicRoutingEntry = dynamicRoutingEntryList;
		dynamicRoutingEntryList = dynamicRoutingEntry->lchild;
		dynamicRoutingEntry->lchild = NULL;
		return dynamicRoutingEntry;
	}

	void addDynamicRoutingEntry(DynamicRoutingEntry* entry)
	{
		while(NULL != entry->triples)
		{
			DynamicRoutingTriple* triple = entry->triples;
			entry->triples = triple->next;
			triple->reset();
			addDynamicRoutingTriple(triple);
		}
		entry->reset();
		entry->lchild = dynamicRoutingEntryList;
		dynamicRoutingEntryList = entry;
	}

	DynamicRoutingUpdateEvent* getDynamicRoutingUpdateEvent()
	{
		if(NULL == dynamicRoutingUpdateEventList)
			dynamicRoutingUpdateEventList = new DynamicRoutingUpdateEvent();
		DynamicRoutingUpdateEvent* dynamicRoutingUpdateEvent = dynamicRoutingUpdateEventList;
		dynamicRoutingUpdateEventList = (DynamicRoutingUpdateEvent*)dynamicRoutingUpdateEvent->next;
		dynamicRoutingUpdateEvent->next = NULL;
		return dynamicRoutingUpdateEvent;
	}

	void addDynamicRoutingUpdateEvent(DynamicRoutingUpdateEvent* dynamicRoutingUpdateEvent)
	{
		dynamicRoutingUpdateEvent->reset();
		dynamicRoutingUpdateEvent->next = dynamicRoutingUpdateEventList;
		dynamicRoutingUpdateEventList = dynamicRoutingUpdateEvent;
	}

	FileAccessInfoPerFace* getFileAccessInfoPerFace()
	{
		if(NULL == fileAccessInfoPerFaceList)
			fileAccessInfoPerFaceList = new FileAccessInfoPerFace();
		FileAccessInfoPerFace* fileAccessInfoPerFace = fileAccessInfoPerFaceList;
		fileAccessInfoPerFaceList = fileAccessInfoPerFace->next;
		fileAccessInfoPerFace->next = NULL;
		return fileAccessInfoPerFace;
	}

	void addFileAccessInfoPerFace(FileAccessInfoPerFace* fileAccessInfoPerFace)
	{
		fileAccessInfoPerFace->reset();
		fileAccessInfoPerFace->next = fileAccessInfoPerFaceList;
		fileAccessInfoPerFaceList = fileAccessInfoPerFace;
	}

	FileAccessInfo* getFileAccessInfo()
	{
		if(NULL == fileAccessInfoList)
			fileAccessInfoList = new FileAccessInfo();
		FileAccessInfo* fileAccessInfo = fileAccessInfoList;
		fileAccessInfoList = fileAccessInfo->lchild;
		fileAccessInfo->lchild = NULL;
		return fileAccessInfo;
	}

	void addFileAccessInfo(FileAccessInfo* fileAccessInfo)
	{
		while(NULL != fileAccessInfo->fileAccessInfoForFaces)
		{
			FileAccessInfoPerFace* info = fileAccessInfo->fileAccessInfoForFaces;
			fileAccessInfo->fileAccessInfoForFaces = info->next;
			addFileAccessInfoPerFace(info);
		}
		fileAccessInfo->reset();
		fileAccessInfo->lchild = fileAccessInfoList;
		fileAccessInfoList = fileAccessInfo;
	}
};

MemoryPool* MemoryPool::memoryPool = NULL;
int MemoryPool::count = 0;
#endif