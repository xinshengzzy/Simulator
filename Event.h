#ifndef EVENT_H
#define EVENT_H

#include <stdio.h>

class Event
{
public:
	double time;
	Event* next;
	Event* previous;
	int type;	// which type event it is.
//	static int numOfEvent;
	
	Event()
	{
//		++numOfEvent;
//		printf("numofEvent = %d\n", numOfEvent);
		next = NULL;
		previous = NULL;
		time = -1.0;
	}

	virtual void reset()
	{
		time = -1;
		next = previous = NULL;
	}

	void copy(Event* evnt)
	{
		this->time = evnt->time;
		this->next = evnt->next;
		this->previous = evnt->previous;
		this->type = evnt->type;
	}
};

//int Event::numOfEvent = 0;
#endif