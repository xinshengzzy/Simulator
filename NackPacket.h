#ifndef NACK_PACKET_H
#define NACK_PACKET_H
#include "Packet.h"
#include "Config.h"
// In this simulator, the only usage of nack packet is to notify the downstream nodes that
// the destination is unreachable.
class NackPacket : public Packet{
public:
	static int numOfNackPackets;
	NackPacket()
	{
		++numOfNackPackets;
		//printf("numOfNackPackets = %d\n", numOfNackPackets);
		type = EVENT_TYPE_NACK_PACKET;
	}
	void copy(NackPacket* nackPacket)
	{
		Packet::copy(nackPacket);
	}
};

int NackPacket::numOfNackPackets = 0;
#endif