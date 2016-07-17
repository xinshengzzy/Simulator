#ifndef NODE_H
#define	NODE_H
#include <stdlib.h>

#include "Face.h"
#include "Packet.h"
#include "InterestPacket.h"
#include "DataPacket.h"
#include "ProcessPacketEvent.h"
#include "InterestPacketTimeoutEvent.h"
#include "NackPacket.h"
#include "MemoryPool.h"
#include "Common.h"
#include "EventQueue.h"
#include "Config.h"

class Node
{
protected:
	Packet* incomingPacketsHead;
	Packet* incomingPacketsTail;
	int hasOutstandingProcessPacketEvent;

	virtual void processIncomingInterestPacket(InterestPacket* interestPacket) {
		printf("virtual void processIncomingInterestPacket(...)\n");
	}
	virtual void processIncomingDataPacket(DataPacket* dataPacket) {
		printf("virtual void processIncomingDataPacket(...)\n");
	}
	virtual void processIncomingNackPacket(NackPacket* nackPacket) {
		printf("virtual void processIncomingNackPacket(...)\n");
	}

	virtual void processIncomingPacket()
	{
		Packet* packet = removeIncomingPacket();
		if(NULL != packet)
		{
			Common* common = Common::getCommon();
			switch(packet->type)
			{
			case EVENT_TYPE_INTEREST_PACKET:
				//printf("before processIncomingInterestPacket(...)\n");
				processIncomingInterestPacket((InterestPacket*)packet); 
				//printf("after processIncomingInterestPacket(...)\n");
				deliverProcessPacketEvent(common->clock + INTEREST_PACKET_PROCESSING_TIME);
				break;
			case EVENT_TYPE_DATA_PACKET:
				//printf("before processIncomingDataPacket(...)\n");
				processIncomingDataPacket((DataPacket*)packet);
				//printf("after processIncomingDataPacket(...)\n");
				deliverProcessPacketEvent(common->clock + DATA_PACKET_PROCESSING_TIME);
				break;
			case EVENT_TYPE_NACK_PACKET:
				//printf("before processIncomingNackPacket(...)\n");
				processIncomingNackPacket((NackPacket*)packet);
				//printf("after processIncomingNackPacket(...)\n");
				deliverProcessPacketEvent(common->clock + NACK_PACKET_PROCESSING_TIME);
				break;
			}
		}
	}

	
public:
	// the members are for the faces
	Face** faces;	// the outgoing faces of the node
	int numOfFace;	// the size of the array faces
	int maxNumOfFace;	// the number of outgoing faces available
	int nodeId;
	int nodeType;

	Node(int id)
	{
		faces = NULL;
		numOfFace = 0;
		maxNumOfFace = 0;
		this->nodeId = id;
		incomingPacketsHead = incomingPacketsTail = new Packet();
		hasOutstandingProcessPacketEvent = 0;
	}

	// make the functions of the node run in a specified interval.
	void deliverProcessPacketEvent(double time)
	{
		if(0 == hasOutstandingProcessPacketEvent)
		{
			MemoryPool* memoryPool = MemoryPool::getMemoryPool();
			Common* common = Common::getCommon();
			EventQueue* eventQueue = EventQueue::getEventQueue();
			ProcessPacketEvent* processPacketEvent = memoryPool->getProcessPacketEvent(); 
			processPacketEvent->nodeId = this->nodeId;
			processPacketEvent->time = time;
			eventQueue->appendEvent(processPacketEvent);
			hasOutstandingProcessPacketEvent = 1;
		}
	}

	// Respond to the packet process notification.
	void respondProcessPacketEvent(ProcessPacketEvent* processPacketEvent)
	{
		if(1 == hasOutstandingProcessPacketEvent)
		{
			hasOutstandingProcessPacketEvent = 0;
			MemoryPool* memoryPool = MemoryPool::getMemoryPool();
			memoryPool->addProcessPacketEvent(processPacketEvent);
			//printf("before proessIncomingPacket()\n");
			processIncomingPacket();
			//printf("after processIncomingPacket()\n");
		}
		else
		{
			printf("else: respondProcessPacketEvent(...)\n");
			MemoryPool* memoryPool = MemoryPool::getMemoryPool();
			memoryPool->addProcessPacketEvent(processPacketEvent);
		}
	}

	void deliverInterestPacketTimeoutEvent(char* interestPacketName, double time)
	{
		printf("deliverInterestPacketTimeoutEvent(...) is empty.\n");
		//MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		//InterestPacketTimeoutEvent* interestPacketTimeoutEvent = memoryPool->getInterestPacketTimeoutEvent();
		
	}

	void appendIncomingPacket(Packet* packet)
	{
		packet->next = NULL;
		incomingPacketsTail->next = packet;
		incomingPacketsTail = packet;
	}
	Packet* removeIncomingPacket()
	{
		Packet* p = NULL;
		if(incomingPacketsHead != incomingPacketsTail)
		{
			p = (Packet*)incomingPacketsHead->next;
			incomingPacketsHead->next = p->next;
			if(p == incomingPacketsTail) incomingPacketsTail = incomingPacketsHead;
		}
		return p;
	}

	// add a new outgoing face to this node. The default delay corresponding to the link between this node and 
	// the destination node is 10ms.
	void addFace(int dest, double delay = LINK_DELAY)
	{
		if(numOfFace >= maxNumOfFace)
		{
			maxNumOfFace += 5;
			faces = (Face**)realloc(faces, maxNumOfFace*sizeof(Face*));
		}
		faces[numOfFace] = new Face(dest, delay);
		++numOfFace;
	}

	Face* getDefaultFace()
	{
		if(NULL != faces) 
			return faces[0];
		else return NULL;
	}

	double getDelay(int face)
	{
		double delay = -1.0;
		for(int i = 0; i < numOfFace; ++i)
			if(face == faces[i]->dest)
			{
				delay = faces[i]->delay;
				break;
			}
		return delay;
	}

	void drop(Event* e)
	{
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		switch(e->type)
		{
		case EVENT_TYPE_INTEREST_PACKET:
			memoryPool->addInterestPacket((InterestPacket*)e); break;
		case EVENT_TYPE_DATA_PACKET:
			memoryPool->addDataPacket((DataPacket*)e); break;
		case EVENT_TYPE_NACK_PACKET:
			memoryPool->addNackPacket((NackPacket*)e); break;
		case EVENT_TYPE_INTEREST_PACKET_TIMEOUT:
			memoryPool->addInterestPacketTimeoutEvent((InterestPacketTimeoutEvent*)e); break;
		case EVENT_TYPE_PROCESS_PACKET:
			memoryPool->addProcessPacketEvent((ProcessPacketEvent*)e); break;
		default: 
			printf("Unrecognized event type.\n");
			free(e); break;
		}
	}

	double getClock()
	{
		Common* common = Common::getCommon();
		return common->clock;
	}

	void enqueueEvent(Event* evnt)
	{
		EventQueue* eventQueue = EventQueue::getEventQueue();
		eventQueue->appendEvent(evnt);
	}
};
#endif