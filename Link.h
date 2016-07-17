#ifndef LINK_H
#define LINK_H
class Link
{
public:
	int node1;
	int node2;
	double delay;
	Link* next;
	Link(int node1, int node2, double delay)
	{
		this->node1 = node1;
		this->node2 = node2;
		this->delay = delay;
		next = NULL;
	}
};
#endif