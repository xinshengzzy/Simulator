#ifndef GENERATE_INTEREST_EVENT_H
#define GENERATE_INTEREST_EVENT_H
#include "Config.h"
#include "Event.h"
// Client responds this type of event by generate a new interest packet.
class GenerateInterestEvent : public Event
{
public:
	int nodeId;
	static int numOfGenerateInterestEvent;
	GenerateInterestEvent()
	{
		++numOfGenerateInterestEvent;
		this->type = EVENT_TYPE_CLIENT_GENERATE_INTEREST_PACKET;
	}

	void reset()
	{
		Event::reset();
		nodeId = -1;
	}
	
};

int GenerateInterestEvent::numOfGenerateInterestEvent = 0;
#endif