#ifndef TOPOLOGY_H
#define TOPOLOGY_H
#include <stdlib.h>
#include "Node.h"
#include "Client.h"
#include "Server.h"
#include "Router.h"
#include "Config.h"
#include "Link.h"

class Topology
{
private:
	Topology()
	{
		nodes = NULL;
		links = NULL;
		numOfNodes = 0;
		maxNumOfNodes = 0;
	}
	static Topology* topology;

	void addLink(int node1, int node2, double delay = LINK_DELAY)
	{
		if(node1 >= numOfNodes || node2 >= numOfNodes)
		{
			printf("Node IDs are out of range.\n");
			return;
		}
		Link* link = new Link(node1, node2, delay);
		link->next = links;
		links = link;
		nodes[node1]->addFace(node2, delay);
		nodes[node2]->addFace(node1, delay);
	}

public:
	Node** nodes;
	Link* links;
	int numOfNodes;
	int maxNumOfNodes;

	static Topology* getTopology()
	{
		if(NULL == topology)
			topology = new Topology();
		return topology;
	}
	Client* createClient()
	{
		if(numOfNodes >= maxNumOfNodes)
		{
			maxNumOfNodes = 2*maxNumOfNodes + 1;
			nodes = (Node**)realloc(nodes, maxNumOfNodes*sizeof(Node*));
		}
		Client* c = new Client(numOfNodes);
		nodes[numOfNodes] = c;
		++numOfNodes;
		return c;
	}

	Router* createRouter()
	{
		if(numOfNodes >= maxNumOfNodes)
		{
			maxNumOfNodes = 2*maxNumOfNodes + 1;
			nodes = (Node**)realloc(nodes, maxNumOfNodes*sizeof(Node*));
		}
		Router* r = new Router(numOfNodes);
		nodes[numOfNodes] = r;
		++numOfNodes;
		return r;
	}

	Server* createServer()
	{
		if(numOfNodes >= maxNumOfNodes)
		{
			maxNumOfNodes = 2*maxNumOfNodes + 1;
			nodes = (Node**)realloc(nodes, maxNumOfNodes*sizeof(Node*));
		}
		Server* s = new Server(numOfNodes);
		nodes[numOfNodes] = s;
		++numOfNodes;
		return s;
	}

	void addLink(Node* node1, Node* node2, double delay = LINK_DELAY)
	{
		addLink(node1->nodeId, node2->nodeId, delay);
	}
};

Topology* Topology::topology = NULL;
#endif TOPOLOGY_H