#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H
#include <string.h>
#include <stdlib.h>
#include "Event.h"
//#include "Topology.h"

class EventQueue
{
private:
	Event* head;	// the head pointer of the event list
	Event* tail;	// the tail pointer of the event list
	static EventQueue* eventQueue;
	EventQueue()
	{
		tail = head = new Event();
	}
public:
	static EventQueue* getEventQueue()
	{
		if(NULL == eventQueue)
			eventQueue = new EventQueue();
		return eventQueue;
	}
	~EventQueue()
	{
		while(head != tail)
		{
			Event* temp = deleteEvent();
			free(temp);
		}
		delete head;
	}

	// append a new event to the event list
	void appendEvent(Event* evnt)
	{
		Event* temp = tail;
		while(evnt->time < temp->time) 
			temp = temp->previous;
		evnt->next = temp->next;
		evnt->previous = temp;
		if(NULL != temp->next)
			temp->next->previous = evnt;
		temp->next = evnt;
		if(temp == tail) 
		{
			tail = evnt;
		}
	}

	// delete the earlist event from the event list.
	Event* deleteEvent()
	{
		Event* ret = NULL;
		if(head != tail)
		{
			ret = head->next;
			head->next = ret->next;
			if(NULL != ret->next)
				ret->next->previous = head;
			else tail = head;
			ret->previous = ret->next = NULL;
		}
		return ret;
	}

	// detach an event from the event list.
	void detachEvent(Event* evnt)
	{
		evnt->previous->next = evnt->next;
		if(tail != evnt)
		{
			evnt->next->previous = evnt->previous;
		}
		else
		{
			tail = evnt->previous;
			tail->next = NULL;
		}
		evnt->previous = evnt->next = NULL;
	}

	void traverse()
	{
		int count1 = 0;
		printf("traverse forward:\n");
		Event* evnt;
		evnt = head;
		while(NULL != evnt)
		{
			++count1;
			if(count1 > 10) break;
			printf("%f\t", evnt->time);
			evnt = evnt->next;
		}
		printf("\n");
		printf("traverse backward:\n");
		evnt = tail;
		count1 = 0;
		while(NULL != evnt)
		{
			++count1;
			if(count1 > 10) break;
			printf("%f\t", evnt->time);
			evnt = evnt->previous;
		}
		printf("\n");
	}
};

EventQueue* EventQueue::eventQueue = NULL;

#endif