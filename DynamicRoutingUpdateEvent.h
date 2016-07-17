#ifndef DYNAMIC_ROUTING_UPDATE_EVENT_H
#define DYNAMIC_ROUTING_UPDATE_EVENT_H
#include "Event.h"
#include "stdlib.h"
class DynamicRoutingTable;
class DynamicRoutingUpdateEvent : public Event
{
public:
	char* name;
	int face;
	int dist;
	DynamicRoutingTable* dynamicRoutingTable;
	DynamicRoutingUpdateEvent()
	{
		Event::Event();
		type = EVENT_TYPE_DYNAMIC_ROUTING_UPDATE;
		name = NULL;
		face = -1;
		dist = -1;
		dynamicRoutingTable = NULL;
	}
	void reset()
	{
		Event::reset();
		if(NULL != name) 
		{
			free(name);
		}
		name = NULL;
		face = -1;
		dist = -1;
		dynamicRoutingTable = NULL;
	}
};
#endif