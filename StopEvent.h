#ifndef STOP_EVENT_H
#define STOP_EVENT_H
#include "Event.h"
#include "Config.h"
// When an event of this type is encountered, the simulation will finished.
class StopEvent : public Event
{
public:
	StopEvent(double time)
	{
		this->type =  EVENT_TYPE_STOP;
		this->time = time;
	}
};
#endif