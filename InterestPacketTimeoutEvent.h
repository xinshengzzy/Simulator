#ifndef INTEREST_PACKET_TIMEOUT_EVENT
#define INTEREST_PACKET_TIMEOUT_EVENT
#include "Config.h"
#include "Event.h"

class OutstandingInterestPacket;
class InterestPacketTimeoutEvent : public Event{
public:
	OutstandingInterestPacket* outstandingInterestPacket;
	int nodeId;
	static int numOfInterestPacketTimeoutEvents;

	InterestPacketTimeoutEvent()
	{
		++numOfInterestPacketTimeoutEvents;
		//printf("numOfInterestPacketTimeoutEvents = %d\n", numOfInterestPacketTimeoutEvents);
		type = EVENT_TYPE_INTEREST_PACKET_TIMEOUT;
		outstandingInterestPacket = NULL;
		nodeId = -1;
	}

	virtual void reset()
	{
		Event::reset();
		outstandingInterestPacket = NULL;
		nodeId = -1;
	}
};

int InterestPacketTimeoutEvent::numOfInterestPacketTimeoutEvents = 0;
#endif