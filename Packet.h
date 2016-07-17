#ifndef PACKET_H
#define PACKET_H
#include "Event.h"
#include "Common.h"
#include "Config.h"
class Packet : public Event
{
public:
	char* name;
	int fromNode;
	int toNode;
	int size;
	PacketId id;
	int pathLength;	// record the length of the path from the client to the data provider.


	void copy(Packet* packet)
	{
		Event::copy(packet);
		Common* common = Common::getCommon();
		if(NULL != this->name) 
		{
//			printf("here.\n");
			free(this->name);
		}
		common->copystring(this->name, packet->name);
		this->fromNode = packet->fromNode;
		this->toNode = packet->toNode;
		this->size = packet->size;
		this->id = packet->id;
		this->pathLength = packet->pathLength;
	}

	Packet()
	{
		Event::Event();
		name = NULL;
		fromNode = -1;
		toNode = -1;
		size = -1;
		id = -1;
		pathLength = 0;
	}

	virtual void reset()
	{
		Event::reset();
		free(name);
		name = NULL;
		fromNode = -1;
		toNode = -1;
		size = -1;
		id = -1;
		pathLength = 0;
	}

	void initialise(int fromNode, int toNode, int size, double time, PacketId id, char* name)
	{
		this->name = name;
		this->fromNode = fromNode;
		this->toNode = toNode;
		this->size = size;
		this->time = time;
		this->id = id;
	}
};
#endif