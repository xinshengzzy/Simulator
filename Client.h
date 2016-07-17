#ifndef CLIENT_H
#define CLIENT_H

#include <string.h>
#include <stdlib.h>
#include "Node.h"
#include "DataPacket.h"
#include "MemoryPool.h"
#include "EventQueue.h"
#include "Config.h"
#include "Common.h"
#include "GenerateInterestEvent.h"
#include "FileStore.h"
#include "StaticRoutingTable.h"
#include "StaticRoutingEntry.h"
#include "TraceFile.h"

class Client : public Node
{
private:
	int numOfOutstandingInterestPackets;
	int maxNumOfOutstandingInterestPackets;
	double rtt;
	int hasOutstandingGenerateInterestEvent;
	OutstandingInterestPacket* outstandingInterestPacketsHead;
	OutstandingInterestPacket* outstandingInterestPacketsTail;
	int numOfPackets;	// the number of packets a single contains. To simplify the problem, we 
			// assume that every file have the same number of packets.
	int packetIndex;	// the index of the packets to be generated. 
	char* fileName;		// the file to be requested.
public:
	StaticRoutingTable* staticRoutingTable;	// the static routing table of the client
	Client(int id) : Node(id)
	{
		nodeType = NODE_TYPE_CLIENT;
		numOfOutstandingInterestPackets = 0;
		maxNumOfOutstandingInterestPackets = CLIENT_MAX_NUM_OF_OUTSTANDING_INTEREST_PACKETS;
		rtt = 1.0;
		hasOutstandingGenerateInterestEvent = 0;
		outstandingInterestPacketsHead = outstandingInterestPacketsTail = new OutstandingInterestPacket();
		numOfPackets = NUM_OF_PACKET_PER_FILE;
		packetIndex = 0;
		fileName = NULL;
		staticRoutingTable = new StaticRoutingTable();
	}

	// generate an interest packet, append the packet to the manager, 
	// and then make itself run in an pre-defined interval.
	void generateInterestPacket()
	{
		if(numOfOutstandingInterestPackets >= maxNumOfOutstandingInterestPackets)
		{
			return;
		}
		char* packetName;
		generatePacketName(packetName);
		if(NULL == packetName) return;
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		Common* common = Common::getCommon();
		// generate an interest packet and forward it into the network.
		InterestPacket* interestPacket = memoryPool->getInterestPacket();
		Face* face = getDefaultFace();
		PacketId packetId = common->generatePacketId();
		interestPacket->name = packetName;
		interestPacket->id = packetId;
		interestPacket->fromNode = this->nodeId;
		interestPacket->toNode = face->dest;
		interestPacket->time = getClock() + face->delay;
		interestPacket->cachingRouterDist = 0;
		interestPacket->currentRouterDist = 0;
		interestPacket->cachingRouterWeight = 0.0;
		interestPacket->hashValue = 0;
		interestPacket->pathLength = 0;
		interestPacket->betweenness = 0;
		StaticRoutingEntry* staticRoutingEntry = staticRoutingTable->lookup(packetName);
		interestPacket->lifetime = 2*staticRoutingEntry->distance;
		//interestPacket->lifetime = 2;
		enqueueEvent(interestPacket);
		++numOfOutstandingInterestPackets;

		// generate an outstanding interest packet and keep it locally
		// meanwhile, generate an interest packet timeout event and append it to the global event queue.
		OutstandingInterestPacket* outstandingInterestPacket = memoryPool->getOutstandingInterestPacket();
		InterestPacketTimeoutEvent* interestPacketTimeoutEvent = memoryPool->getInterestPacketTimeoutEvent();
		outstandingInterestPacket->interestPacketTimeoutEvent = interestPacketTimeoutEvent;
		interestPacketTimeoutEvent->outstandingInterestPacket = outstandingInterestPacket;
		common->copystring(outstandingInterestPacket->name, packetName);
		outstandingInterestPacket->time = getClock();
		outstandingInterestPacketsTail->next = outstandingInterestPacket;
		outstandingInterestPacket->previous = outstandingInterestPacketsTail;
		outstandingInterestPacketsTail = outstandingInterestPacket;
		interestPacketTimeoutEvent->time = getClock() + 2*rtt;
		interestPacketTimeoutEvent->nodeId = this->nodeId;
		enqueueEvent(interestPacketTimeoutEvent);
		
		// Make a notification for itself to generate a new Interest packet in the future.
		deliverGenerateInterestEvent(getClock() + TIME_FOR_GENERATING_INTEREST_PACKET);
	}

	void processIncomingInterestPacket(InterestPacket* interestPacket)
	{
		printf("Error: this is a client and it should not process an interest packet.\n");
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		memoryPool->addInterestPacket(interestPacket);
	}

	void processIncomingDataPacket(DataPacket* dataPacket)
	{
		//printf("processIncomingDataPacket(...)\n");
		Common* common = Common::getCommon();
		StaticRoutingEntry* entry = staticRoutingTable->lookup(dataPacket->name);
		if(NULL != TraceFile::file)
			fprintf(TraceFile::file, "%f c %d %d %d %d\n", \
			common->clock, dataPacket->fromNode, dataPacket->toNode, dataPacket->pathLength, entry->distance);
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		OutstandingInterestPacket* outstandingInterestPacket = outstandingInterestPacketsHead->next;
		while(NULL != outstandingInterestPacket)
		{
			if(0 == strcmp(dataPacket->name, outstandingInterestPacket->name)) break;
			outstandingInterestPacket = outstandingInterestPacket->next;
		}
		if(NULL != outstandingInterestPacket)
		{
			//fprintf(TraceFile::debugFile, "NULL != outstandingInterestPacket\n");
			// update the rtt.
			double new_rtt = dataPacket->time - outstandingInterestPacket->time;
			rtt = (1 - ALPHA)*rtt + ALPHA*new_rtt;
			// detach the outstanding interest packet from the outstanding interest packet list.
			detachOutstandingInterestPacket(outstandingInterestPacket);

			InterestPacketTimeoutEvent* interestPacketTimeoutEvent = outstandingInterestPacket->interestPacketTimeoutEvent;
			outstandingInterestPacket->interestPacketTimeoutEvent = NULL;
			interestPacketTimeoutEvent->outstandingInterestPacket = NULL;
			EventQueue* eventQueue = EventQueue::getEventQueue();
			eventQueue->detachEvent(interestPacketTimeoutEvent);

			memoryPool->addOutstandingInterestPacket(outstandingInterestPacket);
			memoryPool->addInterestPacketTimeoutEvent(interestPacketTimeoutEvent);
		}
		//else {
		//	fprintf(TraceFile::debugFile, "NULL == outstandingInterestPacket\n");
		//}
		//fprintf(TraceFile::debugFile, "delete data packet: nodeId=%d, dataPacket->id=%d\n", nodeId, dataPacket->id);
		memoryPool->addDataPacket(dataPacket);
	}
	
	void processIncomingNackPacket(NackPacket* nackPacket)
	{
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		OutstandingInterestPacket* outstandingInterestPacket = outstandingInterestPacketsHead->next;
		while(NULL != outstandingInterestPacket)
		{
			if(0 == strcmp(nackPacket->name, outstandingInterestPacket->name)) break;
		}
		if(NULL != outstandingInterestPacket)
		{
			// detach the outstanding interest packet from the outstanding interest packet list.
			detachOutstandingInterestPacket(outstandingInterestPacket);

			InterestPacketTimeoutEvent* interestPacketTimeoutEvent = outstandingInterestPacket->interestPacketTimeoutEvent;
			outstandingInterestPacket->interestPacketTimeoutEvent = NULL;
			interestPacketTimeoutEvent->outstandingInterestPacket = NULL;
			EventQueue* eventQueue = EventQueue::getEventQueue();
			eventQueue->detachEvent(interestPacketTimeoutEvent);

			memoryPool->addOutstandingInterestPacket(outstandingInterestPacket);
			memoryPool->addInterestPacketTimeoutEvent(interestPacketTimeoutEvent);
		}
		memoryPool->addNackPacket(nackPacket);
	}

	void startAt(double time)
	{
		deliverGenerateInterestEvent(time);
	}

	void deliverGenerateInterestEvent(double time)
	{
		if(0 == hasOutstandingGenerateInterestEvent)
		{
			MemoryPool* memoryPool = MemoryPool::getMemoryPool();
			EventQueue* eventQueue = EventQueue::getEventQueue();
			GenerateInterestEvent* generateInterestEvent = memoryPool->getGenerateInterestEvent();
			generateInterestEvent->nodeId = this->nodeId;
			generateInterestEvent->time = time;
			eventQueue->appendEvent(generateInterestEvent);
			hasOutstandingGenerateInterestEvent = 1;
		}
	}

	void respondGenerateInterestEvent(GenerateInterestEvent* generateInterestEvent)
	{
		if(1 == hasOutstandingGenerateInterestEvent)
		{
			hasOutstandingGenerateInterestEvent = 0;
			MemoryPool* memoryPool = MemoryPool::getMemoryPool();
			memoryPool->addGenerateInterestEvent(generateInterestEvent);
			generateInterestPacket();
		}
		else
		{
			printf("else\n");
			MemoryPool* memoryPool = MemoryPool::getMemoryPool();
			memoryPool->addGenerateInterestEvent(generateInterestEvent);
		}
	}

	void respondInterestPacketTimeoutEvent(InterestPacketTimeoutEvent* interestPacketTimeoutEvent)
	{
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		OutstandingInterestPacket* outstandingInterestPacket = interestPacketTimeoutEvent->outstandingInterestPacket;
		// detach the outstanding interest packet from the outstanding interest packet list.
		detachOutstandingInterestPacket(outstandingInterestPacket);
		// push the outstanding interest packet as well as the interest packet timeout event into the memory pool.
		interestPacketTimeoutEvent->outstandingInterestPacket = NULL;
		outstandingInterestPacket->interestPacketTimeoutEvent = NULL;
		memoryPool->addInterestPacketTimeoutEvent(interestPacketTimeoutEvent); 
		memoryPool->addOutstandingInterestPacket(outstandingInterestPacket);
	}

	void generatePacketName(char* & packetName)
	{
		if(NULL == fileName || packetIndex == numOfPackets)
		{
			FileStore* fileStore = FileStore::getFileStore();
			fileName = fileStore->getFileName();
			packetIndex = 0;
		}
		char index[10];
		_itoa_s(packetIndex++, index, 10);
		char num[10];
		_itoa_s(numOfPackets, num, 10);
		int len1 = strlen(fileName);
		int len2 = strlen(num);
		int len3 = strlen(index);
		int len = len1 + len2 + len3 + 3;
		packetName = (char*)malloc(len*sizeof(char));
		int i = 0;
		for(int j = 0; j < len1; ++j)
			packetName[i++] = fileName[j];
		packetName[i++] = '/';
		for(int j = 0; j < len2; ++j)
			packetName[i++] = num[j];
		packetName[i++] = '/';
		for(int j = 0; j < len3; ++j)
			packetName[i++] = index[j];
		packetName[i] = '\0';
		//printf("packetName = %s\n", packetName);
	}

	void detachOutstandingInterestPacket(OutstandingInterestPacket* outstandingInterestPacket)
	{
		outstandingInterestPacket->previous->next = outstandingInterestPacket->next;
		if(outstandingInterestPacket == outstandingInterestPacketsTail)
		{
			outstandingInterestPacketsTail = outstandingInterestPacket->previous;
			outstandingInterestPacketsTail->next = NULL;
		}
		else outstandingInterestPacket->next->previous = outstandingInterestPacket->previous;
		--numOfOutstandingInterestPackets;

		// reschedule the timer to generate interest packets.
		deliverGenerateInterestEvent(getClock());
	}
};
#endif