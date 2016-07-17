#ifndef DATA_PACKET_H
#define DATA_PACKET_H
#include "Packet.h"
#include "Config.h"
#include "TraceFile.h"
class DataPacket : public Packet{
public:
	// this is for the caching method of SADO
	int currentRouterDist;
	int cachingRouterDist;
	// this is for the caching method of Betweenness
	double betweenness;
	// this is for the caching method of WAVE
	int cachingFlag;
	static int numOfDataPackets;
	DataPacket() : Packet()
	{
		++numOfDataPackets;
		//printf("numOfDataPackets = %d\n", numOfDataPackets);
		//fprintf(TraceFile::debugFile, "numOfDataPackets = %d\n", numOfDataPackets);
		Common* common = Common::getCommon();
		//if(0 == common->flag)
			//printf("numOfDataPackets = %d\n", numOfDataPackets);
		type = EVENT_TYPE_DATA_PACKET;
		currentRouterDist = 0;
		cachingRouterDist = 0;
		betweenness = -1.0;
		cachingFlag = 0;
	}

	void reset()
	{
		Packet::reset();
		this->currentRouterDist = 0;
		this->cachingRouterDist = 0;
		this->betweenness = -1.0;
		this->cachingFlag = 0;
	}

	void copy(DataPacket* dataPacket)
	{
		Packet::copy(dataPacket);
		this->currentRouterDist = dataPacket->currentRouterDist;
		this->cachingRouterDist = dataPacket->cachingRouterDist;
		this->betweenness = dataPacket->betweenness;
		this->cachingFlag = dataPacket->cachingFlag;
	}
};
int DataPacket::numOfDataPackets = 0;
#endif