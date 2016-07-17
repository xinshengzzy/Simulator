#ifndef DYNAMIC_ROUTING_TRIPLE_H
#define DYNAMIC_ROUTING_TRIPLE_H
#include <stdlib.h>
class DynamicRoutingTriple
{
public:
	int face;			// through which face the caching router could be reached
	double distance;		// the distance from current router to the caching router
	int numOfPackets;	// the number of packets with the prefix that are cached there
	DynamicRoutingTriple* next;
	DynamicRoutingTriple* previous;

	DynamicRoutingTriple()
	{
		face = -1;
		distance = 0.0;
		numOfPackets = 0;
		previous = next = NULL;
	}
	void reset()
	{
		face = -1;
		distance = 0.0;
		numOfPackets = 0;
		previous = next = NULL;
	}
};
#endif