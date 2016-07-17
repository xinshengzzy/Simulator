#ifndef INCOMING_FACE_H
#define INCOMING_FACE_H
// Basically, this class is used for the PIT
#include "stdlib.h"
class IncomingFace
{
public:
	int face;
	IncomingFace* next;
	IncomingFace()
	{
		face = -1;
		next = NULL;
	}
	void reset()
	{
		face = -1;
		next = NULL;
	}
};
#endif