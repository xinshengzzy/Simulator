#ifndef STATIC_ROUTING_ENTRY_H
#define STATIC_ROUTING_ENTRY_H
#include "Common.h"

// Basically, we assume that the name of a data packet is in the form of "<prefix>/<filename>/<num-of-pkts>/<pkt-seq>". 
// The static routing table maintains routing information for all the "prefix" (including the backslash).
class StaticRoutingEntry
{
public:
	char* prefix;
	int forwardingFace;
	int distance;	// the distance from current router to the distination node (i.e., the source server)
	Digest digest;
	int height;
	StaticRoutingEntry* lchild;
	StaticRoutingEntry* rchild;
	StaticRoutingEntry* parent;
	StaticRoutingEntry(char* prefix, int forwardingFace, int distance)
	{
		this->prefix = prefix;
		this->forwardingFace = forwardingFace;
		this->distance = distance;
		Common* common = Common::getCommon();
		digest = common->computeDigest(prefix);
		height = 1;
		lchild = rchild = parent = NULL;
	}
};
#endif