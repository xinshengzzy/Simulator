#ifndef INTEREST_PACKET_H
#define INTEREST_PACKET_H
#include "Packet.h"
#include "Config.h"
class InterestPacket : public Packet{
public:	
	static int numOfInterestPackets;
	int lifetime;

	// this is for the use of SADO
	int currentRouterDist;
	int cachingRouterDist;
	double cachingRouterWeight;
	Digest hashValue;

	// this is for the use of Betweenness
	double betweenness;

	InterestPacket() : Packet()
	{
		++numOfInterestPackets;
		//printf("numOfInterestPackets = %d\n", numOfInterestPackets);
		type = EVENT_TYPE_INTEREST_PACKET;
		lifetime = 0;
		currentRouterDist = 0;
		cachingRouterDist = 0;
		cachingRouterWeight = 0.0;
		hashValue = 0;
		betweenness = -1.0;
	}
	void reset()
	{
		Packet::reset();
		lifetime = 0;
		currentRouterDist = 0;
		cachingRouterDist = 0;
		cachingRouterWeight = 0.0;
		hashValue = 0;
		betweenness = -1.0;
	}
	void copy(InterestPacket* interestPacket)
	{
		Packet::copy(interestPacket);
		this->lifetime = interestPacket->hashValue;
		this->currentRouterDist = interestPacket->currentRouterDist;
		this->cachingRouterDist = interestPacket->cachingRouterDist;
		this->cachingRouterWeight = interestPacket->cachingRouterWeight;
		this->hashValue = interestPacket->hashValue;
		this->betweenness = interestPacket->betweenness;
	}
};
int InterestPacket::numOfInterestPackets = 0;
#endif