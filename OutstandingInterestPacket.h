#ifndef OUTSTANDING_INTEREST_PACKET_H
#define OUTSTANDING_INTEREST_PACKET_H
#include "InterestPacket.h"
class InterestPacketTimeoutEvent;
class OutstandingInterestPacket
{
public:
	char* name;
	double time;
	OutstandingInterestPacket* previous;
	OutstandingInterestPacket* next;
	InterestPacketTimeoutEvent* interestPacketTimeoutEvent;
	OutstandingInterestPacket()
	{
		name = NULL;
		time = 0.0;
		previous = NULL;
		next = NULL;
		interestPacketTimeoutEvent = NULL;
	}
	void reset()
	{
		free(name);
		name = NULL;
		time = 0.0;
		previous = NULL;
		next = NULL;
		interestPacketTimeoutEvent = NULL;
	}
};
#endif