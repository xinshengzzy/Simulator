#ifndef ROUTER_H
#define ROUTER_H
#include "ContentStore.h"
#include "Pit.h"
#include "PitEntry.h"
#include "StaticRoutingTable.h"
#include "InterestPacket.h"
#include "DataPacket.h"
#include "NackPacket.h"
#include "Common.h"
#include "MemoryPool.h"
#include "EventQueue.h"
#include "Config.h"
#include "IncomingFace.h"
#include "DynamicRoutingTable.h"
#include "FileAccessInfoBase.h"
#include "TraceFile.h"

class Router : public Node
{
public:
	ContentStore* contentStore;
	Pit* pit;
	StaticRoutingTable* staticRoutingTable;
	DynamicRoutingTable* dynamicRoutingTable;
	char* idStr;			// the identifier of the router. It is a string.
	double egoBetweenness;	// the ego betweenness of the router.

	Router(int id) : Node(id)
	{
		nodeType = NODE_TYPE_ROUTER;
		contentStore = new ContentStore(1000);
		pit = new Pit();
		staticRoutingTable = new StaticRoutingTable();
		dynamicRoutingTable = new DynamicRoutingTable();
		Common* common = Common::getCommon();
		common->generateRandomString(idStr, 30);
		egoBetweenness = 0.0;
	}

	void processInterestPacketBySado(InterestPacket* interestPacket, int& forwardingFace)
	{
		++interestPacket->currentRouterDist;
		Digest v1 = interestPacket->hashValue;
		double w1 = interestPacket->cachingRouterWeight;
		NameParser* nameParser = NameParser::getNameParser();
		nameParser->parseName(interestPacket->name);
		char* extendedPrefix = nameParser->components[nameParser->numOfComponents - 3];
		int lenExtendedPrefix = strlen(extendedPrefix);
		int lenIdStr = strlen(this->idStr);
		char* temp = (char*)malloc((lenExtendedPrefix + lenIdStr + 1)*sizeof(char));
		int k = 0;
		for(int i = 0; i < lenExtendedPrefix; ++i)
			temp[k++] = extendedPrefix[i];
		for(int i = 0; i < lenIdStr; ++i)
			temp[k++] = this->idStr[i];
		temp[k] = '\0';
		Common* common = Common::getCommon();
		Digest v2 = common->computeDigest(temp);
		free(temp);
		double w2 = (double)contentStore->capacity/this->egoBetweenness;
		double alpha1 = 2*w1/(w1 + w2);
		double alpha2 = 1.0;
		if(w2 >= w1)
		{
			alpha1 = 1.0;
			alpha2 = 2*w2/(w1 + w2);
		}
		if(alpha2*v2 > alpha1*v1) // select the current router as the new caching router
		{
			interestPacket->cachingRouterWeight = w2;
			interestPacket->hashValue = v2;
			interestPacket->cachingRouterDist = interestPacket->currentRouterDist;
		}
		forwardingFace = lookupBySado(interestPacket->name, interestPacket->fromNode);
	}
	

	void processInterestPacketByBetweenness(InterestPacket* interestPacket, int& forwardingFace)
	{
		if(this->egoBetweenness > interestPacket->betweenness)
			interestPacket->betweenness = this->egoBetweenness;
		StaticRoutingEntry* staticRoutingEntry = staticRoutingTable->lookup(interestPacket->name);
		forwardingFace = -1;
		if(NULL != staticRoutingEntry)
			forwardingFace = staticRoutingEntry->forwardingFace;
	}

	void processInterestPacketByWave(InterestPacket* interestPacket, int& forwardingFace)
	{
		processInterestPacketByCcn(interestPacket, forwardingFace);
	}

	void processInterestPacketByCcn(InterestPacket* interestPacket, int& forwardingFace)
	{
		StaticRoutingEntry* staticRoutingEntry = staticRoutingTable->lookup(interestPacket->name);
		forwardingFace = -1;
		if(NULL != staticRoutingEntry)
			forwardingFace = staticRoutingEntry->forwardingFace;
	}


	void processIncomingInterestPacket(InterestPacket* interestPacket)
	{
		//if(17230 == interestPacket->id && 9 == this->nodeId)
			//printf("one\n");
		//printf("enter processIncomingInterestPacket(...)\n");
		Common* common = Common::getCommon();
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		EventQueue* eventQueue = EventQueue::getEventQueue();
		// if the interest packet has travelled for too long distance, discard it.
		++interestPacket->pathLength;
		--interestPacket->lifetime;
		if(interestPacket->lifetime <= 0)
		{
			printf("interestPacket->lifetime <= 0\n");
			NackPacket* nackPacket = memoryPool->getNackPacket();
			nackPacket->fromNode = this->nodeId;
			nackPacket->toNode = interestPacket->fromNode;
			double delay = getDelay(nackPacket->toNode);
			common->copystring(nackPacket->name, interestPacket->name);
			nackPacket->time = common->clock + delay;
			eventQueue->appendEvent(nackPacket);
			memoryPool->addInterestPacket(interestPacket);
			return;
		}

		// check the content store.
		DataPacket* dataPacket = NULL;
		//printf("before contentStore->lookup(...)\n");
		//printf("interestPacket->name = %s\n", interestPacket->name);
		dataPacket = contentStore->lookup(interestPacket->name);
		//printf("after contentStore->lookup(...)\n");
		if(NULL != dataPacket) // the data packet has been cached locally.
		{
			if(NULL != TraceFile::file)
				fprintf(TraceFile::file, "%f h %d\n", common->clock, this->nodeId);
			dataPacket->fromNode = this->nodeId;
			dataPacket->toNode = interestPacket->fromNode;
			dataPacket->time = common->clock + getDelay(interestPacket->fromNode);
			dataPacket->pathLength = interestPacket->pathLength;
			switch(CACHING_METHOD)
			{
			case CACHING_METHOD_SADO:
				++dataPacket->currentRouterDist;
				dataPacket->currentRouterDist = interestPacket->currentRouterDist;
				dataPacket->cachingRouterDist = interestPacket->cachingRouterDist;
				break;
			case CACHING_METHOD_EGOBETW:
				dataPacket->betweenness = interestPacket->betweenness;
				//printf("dataPacket->betweenness = %f\n", dataPacket->betweenness);
				break;
			case CACHING_METHOD_WAVE:
				NameParser* nameParser = NameParser::getNameParser();
				nameParser->parseName(dataPacket->name);
				char* fileName = nameParser->components[nameParser->numOfComponents - 3];
				int packetIndex = nameParser->packetIndex;
				FileAccessInfo* fileAccessInfo = contentStore->fileAccessInfoBase->lookupByFileName(fileName);
				FileAccessInfoPerFace* info = fileAccessInfo->getInfo(dataPacket->toNode);
				if(packetIndex > info->cached && packetIndex <= info->upperBound)
				{
					dataPacket->cachingFlag = 1;
					info->cached = packetIndex;
				}
				else if(packetIndex < info->cached)
				{
					dataPacket->cachingFlag = 1;
					info->cached = packetIndex;
					info->upperBound = 0;
					while(info->upperBound < packetIndex)
					{
						info->upperBound = 2*info->upperBound + 1;
					}
				}
				else dataPacket->cachingFlag = 0;
				if(info->cached == info->upperBound)
					info->upperBound = 2*info->upperBound + 1;
				break;
			}
			eventQueue->appendEvent(dataPacket);
			memoryPool->addInterestPacket(interestPacket);
			//printf("after NULL != dataPacket\n");
			return;
		}

		// check the pit.
		//printf("before pit check\n");
		PitEntry* pitEntry = pit->lookup(interestPacket->name);
		if(NULL != pitEntry)
		{
			if(2 == this->nodeId && 16990 == interestPacket->id)
				printf("hello\n");

			//printf("two\n");
			IncomingFace* incomingFace = memoryPool->getIncomingFace();
			incomingFace->face = interestPacket->fromNode;
			pitEntry->addFace(incomingFace);
			//fprintf(TraceFile::debugFile, "before addInterestPacket(...)\n");
			memoryPool->addInterestPacket(interestPacket);
			//fprintf(TraceFile::debugFile, "after addInterestPacket(...)\n");
			//printf("one\n");
			return;
		}
		// there is no matching cached data packet nor matching pit entry, so forward the interest packet.
		int forwardingFace;
		switch(CACHING_METHOD)
		{
		case CACHING_METHOD_SADO: 
			processInterestPacketBySado(interestPacket, forwardingFace);
			break;
		case CACHING_METHOD_EGOBETW:
			processInterestPacketByBetweenness(interestPacket, forwardingFace);
			break;
		case CACHING_METHOD_WAVE:
			processInterestPacketByWave(interestPacket, forwardingFace);
			break;
		case CACHING_METHOD_CCN:
			processInterestPacketByCcn(interestPacket, forwardingFace);
			break;
		}
		pit->addInfo(interestPacket->name, interestPacket->fromNode);
		double delay = getDelay(forwardingFace);
		// We assume the all the faces returned by the 'lookup' operations are valid,
		// so we don't judge whether this delay is legal here.
		interestPacket->fromNode = this->nodeId;
		interestPacket->toNode = forwardingFace;
		interestPacket->time = common->clock + delay;
		//if(17230 == interestPacket->id && 9 == this->nodeId)
			//printf("forwardingFace = %d\n", forwardingFace);

		eventQueue->appendEvent(interestPacket);
	}

	void processDataPacketBySado(DataPacket* dataPacket, PitEntry* pitEntry)
	{
		--dataPacket->currentRouterDist;
		if(dataPacket->cachingRouterDist == dataPacket->currentRouterDist)
			contentStore->cacheDataPacket(dataPacket);
		else if(dataPacket->currentRouterDist < dataPacket->cachingRouterDist)
			dynamicRoutingTable->addInfo(dataPacket->name, dataPacket->fromNode, \
			dataPacket->cachingRouterDist - dataPacket->currentRouterDist, contentStore->averageResidenceTime);
		else
		{
			IncomingFace* face = pitEntry->faces;
			while(NULL != face)
			{
				dynamicRoutingTable->addInfo(dataPacket->name, face->face,\
					dataPacket->currentRouterDist - dataPacket->cachingRouterDist, contentStore->averageResidenceTime);
				face = face->next;
			}
		}
	}

	void processDataPacketByBetweenness(DataPacket* dataPacket)
	{
		if(this->egoBetweenness == dataPacket->betweenness)
			contentStore->cacheDataPacket(dataPacket);
	}

	void processDataPacketByWave(DataPacket* dataPacket)
	{
		if(1 == dataPacket->cachingFlag)
		{
			contentStore->cacheDataPacket(dataPacket);
			dataPacket->cachingFlag = 0;
		}
	}

	void processDataPacketByCcn(DataPacket* dataPacket)
	{
		contentStore->cacheDataPacket(dataPacket);
	}

	void processIncomingDataPacket(DataPacket* dataPacket)
	{
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		PitEntry* pitEntry = pit->lookup(dataPacket->name);
		if(NULL == pitEntry)
		{
			memoryPool->addDataPacket(dataPacket);
			return;
		}
		switch(CACHING_METHOD)
		{
		case CACHING_METHOD_SADO:
			processDataPacketBySado(dataPacket, pitEntry); 
			break;
		case CACHING_METHOD_EGOBETW:
			processDataPacketByBetweenness(dataPacket);
			break;
		case CACHING_METHOD_WAVE:
			processDataPacketByWave(dataPacket);
			break;
		case CACHING_METHOD_CCN:
			processDataPacketByCcn(dataPacket);
			break;
		}
		Common* common = Common::getCommon();
		EventQueue* eventQueue = EventQueue::getEventQueue();
		IncomingFace* face = pitEntry->faces;
		while(NULL != face)
		{
			//fprintf(TraceFile::debugFile, "Router::processIncomingDataPacket(...) before:\n");
			//common->flag = 1;
			DataPacket* tempDataPacket = memoryPool->getDataPacket();
			//common->flag = 0;
			//fprintf(TraceFile::debugFile, "Router::processIncomingDataPacket(...) after:\n");
			tempDataPacket->copy(dataPacket);
			tempDataPacket->fromNode = this->nodeId;
			tempDataPacket->toNode = face->face;
			tempDataPacket->time = common->clock + getDelay(face->face);
			//tempDataPacket->id = common->generatePacketId();
			eventQueue->appendEvent(tempDataPacket);
			face = face->next;
		}
		pit->removeEntry(pitEntry);
//		fprintf(TraceFile::debugFile, "delete packet: nodeId=%d, dataPacket->id=%d\n", this->nodeId, dataPacket->id);
//		fprintf(TraceFile::debugFile, "before addDataPacket(...)\n");
		memoryPool->addDataPacket(dataPacket);
//		fprintf(TraceFile::debugFile, "after addDataPacket(...)\n");
	}

	// look up the forwarding face. The forwarding face should be different from the incoming face.
	int lookupBySado(char* interestPacketName, int incomingFace)
	{
		// lookup the static routing table at first
		StaticRoutingEntry* staticRoutingEntry = staticRoutingTable->lookup(interestPacketName);
		int staticDistance = staticRoutingEntry->distance;
		int staticFace = staticRoutingEntry->forwardingFace;
		double minDist = 2*staticDistance;
		int minFace = staticFace;

		// then lookup the dynamic routing table
		DynamicRoutingEntry* dynamicRoutingEntry = dynamicRoutingTable->lookup(interestPacketName);
		if(NULL != dynamicRoutingEntry)
		{
			NameParser* nameParser = NameParser::getNameParser();
			nameParser->parseName(interestPacketName);
			int numOfPackets = nameParser->numOfPackets;
			DynamicRoutingTriple* triple = dynamicRoutingEntry->triples;
			while(NULL != triple)
			{
				double dist = 2*triple->distance + \
					2*staticDistance*(double)(numOfPackets - triple->numOfPackets)/(double)numOfPackets;
				if(dist < minDist && triple->face != incomingFace)
				{
					minDist = dist;
					minFace = triple->face;
				}
				triple = triple->next;
			}
		}
		return minFace;
	}

	void processIncomingNackPacket(NackPacket* nackPacket)
	{
		Common* common = Common::getCommon();
		EventQueue* eventQueue = EventQueue::getEventQueue();
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		PitEntry* pitEntry = pit->lookup(nackPacket->name);
		if(NULL != pitEntry)
		{
			IncomingFace* face = pitEntry->faces;
			while(NULL != face)
			{
				NackPacket* tempNackPacket = memoryPool->getNackPacket();
				tempNackPacket->copy(nackPacket);
				tempNackPacket->fromNode = this->nodeId;
				tempNackPacket->toNode = face->face;
				tempNackPacket->time = common->clock + getDelay(face->face);
				tempNackPacket->id = common->generatePacketId();
				eventQueue->appendEvent(tempNackPacket);
				face = face->next;
			}
			pit->removeEntry(pitEntry);
		}
		memoryPool->addNackPacket(nackPacket);
	}
};
#endif