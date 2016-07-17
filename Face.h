#ifndef FACE_H
#define FACE_H
#include "Config.h"
class Face
{
public:
	int dest;	// the destination node corresponding to this face
	double delay;	// the transmission delay in msec corresponding to the link between this node and the destination node
	Face()
	{
		dest = -1;
		delay = LINK_DELAY;
	}

	Face(int dest, double delay)
	{
		this->dest = dest;
		this->delay = delay;
	}
};
#endif