#ifndef PROCESS_PACKET_EVENT_H
#define PROCESS_PACKET_EVENT_H
#include "Event.h"
#include "Config.h"
class ProcessPacketEvent : public Event
{
public:
	int nodeId;
	static int numOfProcessPacketEvents;
	void copy(ProcessPacketEvent* evnt)
	{
		++numOfProcessPacketEvents;
		//printf("numOfProcessPacketEvents = %d\n", numOfProcessPacketEvents);
		this->nodeId = evnt->nodeId;
	}

	ProcessPacketEvent()
	{
		this->type = EVENT_TYPE_PROCESS_PACKET;
		this->nodeId = -1;
	}

	virtual void reset()
	{
		Event::reset();
		this->nodeId = -1;
	}

	void initialise(int type, int nodeId, double time)
	{
		this->type = type;
		this->nodeId = nodeId;
		this->time = time;
	}
};
int ProcessPacketEvent::numOfProcessPacketEvents = 0;
#endif