#ifndef SERVER_H
#define SERVER_H
#include "Prefix.h"
#include "Node.h"
#include "string.h"
#include "MemoryPool.h"
#include "FileAccessInfoBase.h"
#include "TraceFile.h"

class Server : public Node
{
public:
	Prefix** prefixes;		// a bunch of prefixes will be stored this this pointer.
	int maxNumOfPrefixes;	// the maximum number of prefixes allowed right now; of course it could be enlarged if necessary.
	int numOfPrefixes;		// the number of prefixes
	FileAccessInfoBase* fileAccessInfoBase;	// maintain the access information for the files it provides.
	Server(int id): Node(id)
	{
		nodeType = NODE_TYPE_SERVER;
		prefixes = NULL;
		maxNumOfPrefixes = 0;
		numOfPrefixes = 0;
		if(CACHING_METHOD == CACHING_METHOD_WAVE)
			fileAccessInfoBase = new FileAccessInfoBase();
		else fileAccessInfoBase = NULL;
	}

	void processIncomingDataPacket(DataPacket* dataPacket)
	{
		printf("Error: this is a server and it should not process a data packet.\n");
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		memoryPool->addDataPacket(dataPacket);
	}

	void processIncomingNackPacket(NackPacket* nackPacket)
	{
		printf("Error: this is a server and it should not process a nack packet.\n");
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		memoryPool->addNackPacket(nackPacket);
	}

	void processIncomingInterestPacket(InterestPacket* interestPacket)
	{
		if(141856 == interestPacket->id)
			printf("ok\n");
		//fprintf(TraceFile::debugFile, "interest packet %d: %d ---> %d\n", interestPacket->id, interestPacket->fromNode, interestPacket->toNode);
		//printf("processIncomingInterestPacket()\n");
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		Common* common = Common::getCommon();
		++interestPacket->pathLength;
		int matching = 0;
		for(int i = 0; i < numOfPrefixes; ++i)
			if(isPrefix(prefixes[i]->prefix, interestPacket->name))
			{
				matching = 1;
				break;
			}
		if(0 == matching) 
		{
			printf("name: %s. This server cannot supply the requested data packet.\n", interestPacket->name);
			memoryPool->addInterestPacket(interestPacket);
			return;
		}
		if(141856 == interestPacket->id)
			printf("one.\n");
		//fprintf(TraceFile::debugFile, "Server::processIncomingInterestPacket(...) before:\n");
		//common->flag = 1;
		DataPacket* dataPacket = memoryPool->getDataPacket();
		//common->flag = 0;
		//fprintf(TraceFile::debugFile, "Server::processIncomingInterestPacket(...) after:\n");
		PacketId packetId = common->generatePacketId();
		char* name;
		common->copystring(name, interestPacket->name);
		dataPacket->name = name;
		dataPacket->id = packetId;
		int toNode = interestPacket->fromNode;
		dataPacket->fromNode = this->nodeId;
		dataPacket->toNode = toNode;
		dataPacket->time = getClock() + getDelay(toNode);
		dataPacket->pathLength = interestPacket->pathLength;
		if(141856 == interestPacket->id)
			printf("two\n");
		switch(CACHING_METHOD)
		{
		case CACHING_METHOD_SADO:
			//printf("CACHING_METHOD_SADO\n");
			++interestPacket->currentRouterDist;
			dataPacket->currentRouterDist = interestPacket->currentRouterDist;
			dataPacket->cachingRouterDist = interestPacket->cachingRouterDist;
			break;
		case CACHING_METHOD_EGOBETW:
			//printf("CACHING_METHOD_EGOBETW\n");
			dataPacket->betweenness = interestPacket->betweenness;
			break;
		case CACHING_METHOD_WAVE:
			//printf("CACHING_METHOD_WAVE\n");
			NameParser* nameParser = NameParser::getNameParser();
			nameParser->parseName(dataPacket->name);
			char* fileName = nameParser->components[nameParser->numOfComponents - 3];
			int packetIndex = nameParser->packetIndex;
			FileAccessInfo* fileAccessInfo = fileAccessInfoBase->lookupByFileName(fileName);
			if(NULL == fileAccessInfo)
			{
				fileAccessInfo = memoryPool->getFileAccessInfo();
				fileAccessInfo->setFileName(fileName);
				fileAccessInfoBase->addEntry(fileAccessInfo);
			}
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
		if(141856 == interestPacket->id)
			printf("three\n");
		enqueueEvent(dataPacket);
		if(NULL != TraceFile::file)
			fprintf(TraceFile::file, "%f s %d %d\n", common->clock, dataPacket->fromNode, dataPacket->toNode);
		//printf("before addInterestPacket(...)\n");
		if(141856 == interestPacket->id)
			printf("before\n");
		memoryPool->addInterestPacket(interestPacket);
		//if(141856 == interestPacket->id)
			//printf("after\n");
		//printf("after addInterestPacket(...)\n");
	}

	// Ironically, only adding prefixes is allowed.
	void addPrefix(char* prefix)
	{
		if(numOfPrefixes >= maxNumOfPrefixes)
		{
			maxNumOfPrefixes = 2*maxNumOfPrefixes + 1;
			prefixes = (Prefix**)realloc(prefixes, maxNumOfPrefixes*sizeof(Prefix));
		}
		prefixes[numOfPrefixes] = new Prefix(prefix);
		++numOfPrefixes;
	}

	// Judge if the given prefix is the prefix of the given string. Return 1 if it is, or 0 otherwise.
	int isPrefix(char* prefix, char* str)
	{
		if(strlen(str) < strlen(prefix)) return 0;
		int ret = 1;
		int len = strlen(prefix);
		for(int i = 0; i < len; ++i)
			if(prefix[i] != str[i])
			{
				ret = 0;
				break;
			}
		return ret;
	}
};
#endif