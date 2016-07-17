#ifndef SIMULATOR_H
#define SIMULATOR_H
#include "MemoryPool.h"
#include "EventQueue.h"
#include "Common.h"
#include "Topology.h"
#include "stdlib.h"
#include "ProcessPacketEvent.h"
#include "InterestPacketTimeoutEvent.h"
#include "Packet.h"
#include "StopEvent.h"
#include "FileStore.h"
#include "OutstandingInterestPacket.h"
#include "Router.h"
#include "Server.h"
#include "Client.h"
#include "TraceFile.h"

class Route
{
public:
	int nexthop;
	int cost;
};

class PrefixToServer
{
public:
	char* prefix;
	int nodeId;
	PrefixToServer* next;
	PrefixToServer(char* prefix, int nodeId)
	{
		this->prefix = prefix;
		this->nodeId = nodeId;
		next = NULL;
	}
};

class Simulator
{
private:
	static Simulator* simulator ;
public:
	static Simulator* getSimulator()
	{
		if(NULL == simulator) simulator = new Simulator();
		return simulator;
	}

	// Install routes for all the routers.
	void installStaticRoutes()
	{
		Topology* topology = Topology::getTopology();
		int numOfNodes = topology->numOfNodes;
		Route** routes;
		routes = (Route**)malloc(topology->numOfNodes*sizeof(Route*));
		for(int i = 0; i < topology->numOfNodes; ++i)
			routes[i] = (Route*)malloc(topology->numOfNodes*sizeof(Route));
		for(int i = 0; i < numOfNodes; ++i)
			for(int j = 0; j < numOfNodes; ++j)
			{
				routes[i][j].nexthop = -1;
				routes[i][j].cost = -1;
			}
		Link* link = topology->links;
		while(NULL != link)
		{
			int node1 = link->node1;
			int node2 = link->node2;
			double delay = link->delay;
			routes[node1][node2].nexthop = node2;
			routes[node1][node2].cost = 1;
			routes[node2][node1].nexthop = node1;
			routes[node2][node1].cost = 1;
			link = link->next;
		}
		for(int k = 0; k < numOfNodes; ++k)
			for(int i = 0; i < numOfNodes; ++i)
				for(int j = 0; j < numOfNodes; ++j)
				{
					if(routes[i][k].cost >= 0 && routes[k][j].cost >= 0 && \
						(routes[i][j].cost < 0 || routes[i][k].cost + routes[k][j].cost < routes[i][j].cost))
					{
						routes[i][j].cost = routes[i][k].cost + routes[k][j].cost;
						routes[i][j].nexthop = routes[i][k].nexthop;
					}
				}
		// make the map from prefixes to servers.
		PrefixToServer* prefixToServers = NULL;
		for(int i = 0; i < numOfNodes; ++i)
		{
			if(NODE_TYPE_SERVER == topology->nodes[i]->nodeType)
			{
				Server* server = (Server*)topology->nodes[i];
				for(int j = 0; j < server->numOfPrefixes; ++j)
				{
					PrefixToServer* prefixToServer = new PrefixToServer(server->prefixes[j]->prefix, i);
					prefixToServer->next = prefixToServers;
					prefixToServers = prefixToServer;
				}
			}
		}

		// install the static routing tables for every router.
		for(int i = 0; i < numOfNodes; ++i)
			if(NODE_TYPE_ROUTER == topology->nodes[i]->nodeType || NODE_TYPE_CLIENT == topology->nodes[i]->nodeType)
			{
				StaticRoutingTable* staticRoutingTable = NULL;
				if(NODE_TYPE_ROUTER == topology->nodes[i]->nodeType)
					staticRoutingTable = ((Router*)topology->nodes[i])->staticRoutingTable;
				else staticRoutingTable = ((Client*)topology->nodes[i])->staticRoutingTable;
				PrefixToServer* prefixToServer = prefixToServers;
				while(NULL != prefixToServer)
				{
					int j = prefixToServer->nodeId;
					char* prefix = prefixToServer->prefix;
					staticRoutingTable->addInfo(prefix, routes[i][j].nexthop, routes[i][j].cost);
					prefixToServer = prefixToServer->next;
				}
			}

		// Free the allocated memory.
		for(int i = 0; i < numOfNodes; ++i)
			free(routes[i]);
		free(routes);
		while(NULL != prefixToServers)
		{
			PrefixToServer* prefixToServer = prefixToServers;
			prefixToServers = prefixToServers->next;
			free(prefixToServer);
		}
		printf("Installing static routes is done.\n");
	}

	void computeEgoBetweenness()
	{
		Topology* topology = Topology::getTopology();
		for(int i = 0; i < topology->numOfNodes; ++i)
			if(NODE_TYPE_ROUTER == topology->nodes[i]->nodeType)
			{
				computeEgoBetweennessSingleRouter((Router*)topology->nodes[i]);
			}
	}

	void computeEgoBetweennessSingleRouter(Router* router)
	{
		int nodeId = router->nodeId;
		Topology* topology = Topology::getTopology();
		int numOfNodes = topology->numOfNodes;
		int* nodeIdTable = (int*)malloc(numOfNodes*sizeof(int));
		int nodeIdTableSize = 0;
		nodeIdTable[nodeIdTableSize++] = nodeId;
		Link* link = topology->links;
		while(NULL != link)
		{
			int otherNode = -1;
			if(nodeId == link->node1) otherNode = link->node2;		// if this node is in the link,
			else if(nodeId == link->node2) otherNode = link->node1;	// record the other node
			for(int k = 0; k < nodeIdTableSize; ++k)
				if(nodeIdTable[k] == otherNode)	// if the other node has been in the node id table, don't record it.
				{
					otherNode = -1;
					break;
				}
			if(-1 != otherNode)
			{
				nodeIdTable[nodeIdTableSize++] = otherNode;
			}
			link = link->next;
		}
		int** egoNetwork = (int**)malloc(nodeIdTableSize*sizeof(int*));
		for(int k = 0; k < nodeIdTableSize; ++k)
			egoNetwork[k] = (int*)malloc(nodeIdTableSize*sizeof(int));
		for(int j = 0; j < nodeIdTableSize; ++j)
			for(int k = 0; k < nodeIdTableSize; ++k)
				egoNetwork[j][k] = 0;
		link = topology->links;
		while(NULL != link)
		{
			// map the node id in the network into node index in the ego network
			int node1 = -1;
			int node2 = -1;
			for(int k = 0; k < nodeIdTableSize; ++k)
			{
				if(link->node1 == nodeIdTable[k])
					node1 = k;	
				if(link->node2 == nodeIdTable[k])
					node2 = k;
			}
			if(node1 >= 0 && node2 >= 0)
			{
				egoNetwork[node1][node2] = 1;
				egoNetwork[node2][node1] = 1;
			}
			link = link->next;
		}
		double** egoBetweenness = (double**)malloc(nodeIdTableSize*sizeof(double*));
		for(int k = 0; k < nodeIdTableSize; ++k)
			egoBetweenness[k] = (double*)malloc(nodeIdTableSize*sizeof(double));
		for(int m = 0; m < nodeIdTableSize; ++m)
			for(int n = 0; n < nodeIdTableSize; ++n)
				egoBetweenness[m][n] = 0.0;
		for(int m = 0; m < nodeIdTableSize; ++m)
			for(int n = 0; n < nodeIdTableSize; ++n)
				for(int l = 0; l < nodeIdTableSize; ++l)
				{
					egoBetweenness[m][n] += egoNetwork[m][l]*egoNetwork[l][n];
				}
		for(int m = 0; m < nodeIdTableSize; ++m)
			for(int n = 0; n < nodeIdTableSize; ++n)
				egoBetweenness[m][n] = egoBetweenness[m][n]*(1 - egoNetwork[m][n]);
		for(int m = 0; m < nodeIdTableSize; ++m)
			for(int n = 0; n <= m; ++n)
				egoBetweenness[m][n] = 0.0;
		double egoBetweennessValue = 0.0;
		for(int m = 0; m < nodeIdTableSize; ++m)
			for(int n = 0; n < nodeIdTableSize; ++n)
				if(0.0 != egoBetweenness[m][n])
					egoBetweennessValue += 1.0/(double)egoBetweenness[m][n];
		router->egoBetweenness = egoBetweennessValue;

		// free the memory allocated
		free(nodeIdTable);
		for(int i = 0; i < nodeIdTableSize; ++i)
			free(egoNetwork[i]);
		free(egoNetwork);
		for(int i = 0; i < nodeIdTableSize; ++i)
			free(egoBetweenness[i]);
		free(egoBetweenness);
	}

	void updateClock(Event* evnt)
	{
		Common* common = Common::getCommon();
		common->clock = evnt->time;
	}

	void handlePacket(Packet* packet)
	{
		Topology* topology = Topology::getTopology();
		Common* common = Common::getCommon();
		int fromNode = packet->fromNode;
		int toNode = packet->toNode;
		int nodeId = packet->toNode;
		if(packet->type == EVENT_TYPE_INTEREST_PACKET)
		{
			//fprintf(TraceFile::debugFile, "%f interest packet %d: %d ---> %d\n", common->clock, packet->id, fromNode, toNode);
			/*if(tempNodeId == tempToNode) printf("nodeId == toNode\n");
			else printf("nodeId != toNode\n");
			printf("data packet: packet->id=%d, fromNode=%d, toNode=%d, nodeId=%d\n",
			packet->id, tempFromNode, tempToNode, tempNodeId);*/
			//printf("packet->id = %d, fromNode = %d, toNode = %d, nodeId = %d\n", packet->id, fromNode, toNode, nodeId);
		}
		if(NULL != TraceFile::file)
		{
			fprintf(TraceFile::file, "%f t %d %d ", common->clock, fromNode, toNode);
			if(EVENT_TYPE_DATA_PACKET == packet->type) fprintf(TraceFile::file, "d");
			else if(EVENT_TYPE_INTEREST_PACKET == packet->type) fprintf(TraceFile::file, "i");
			else if(EVENT_TYPE_NACK_PACKET == packet->type) fprintf(TraceFile::file, "n");
			fprintf(TraceFile::file, "\n");
		}
		topology->nodes[nodeId]->appendIncomingPacket(packet);
		topology->nodes[nodeId]->deliverProcessPacketEvent(common->clock);
	}

	void handleProcessPacketEvent(ProcessPacketEvent* processPacketEvent)
	{
		Topology* topology = Topology::getTopology();
		int nodeId = processPacketEvent->nodeId;
		topology->nodes[nodeId]->respondProcessPacketEvent(processPacketEvent);
	}

	void handleGenerateInterestEvent(GenerateInterestEvent* generateInterestEvent)
	{
		int nodeId = generateInterestEvent->nodeId;
		Topology* topology = Topology::getTopology();
		if(NODE_TYPE_CLIENT != topology->nodes[nodeId]->nodeType)
		{
			printf("Error: a non-client node receives a generate interest event.\n");
			MemoryPool* memoryPool = MemoryPool::getMemoryPool();
			memoryPool->addGenerateInterestEvent(generateInterestEvent);
			return;
		}
		Client* client = (Client*)topology->nodes[nodeId];
		client->respondGenerateInterestEvent(generateInterestEvent);
	}
	void handleDynamicRoutingUpdateEvent(DynamicRoutingUpdateEvent* dynamicRoutingUpdateEvent)
	{
		char* name = dynamicRoutingUpdateEvent->name;
		int face = dynamicRoutingUpdateEvent->face;
		int dist = dynamicRoutingUpdateEvent->dist;
		DynamicRoutingTable* dynamicRoutingTable = dynamicRoutingUpdateEvent->dynamicRoutingTable;
		dynamicRoutingTable->removeInfo(name, face, dist);
		MemoryPool* memoryPool = MemoryPool::getMemoryPool();
		memoryPool->addDynamicRoutingUpdateEvent(dynamicRoutingUpdateEvent);
	}

	void handleInterestPacketTimeoutEvent(InterestPacketTimeoutEvent* interestPacketTimeoutEvent)
	{
		Topology* topology = Topology::getTopology();
		int nodeId = interestPacketTimeoutEvent->nodeId;
		Client* client = (Client*)topology->nodes[nodeId];
		client->respondInterestPacketTimeoutEvent(interestPacketTimeoutEvent);
	}

	void stopAt(double time)
	{
		EventQueue* eventQueue = EventQueue::getEventQueue();
		StopEvent* stopEvent = new StopEvent(time);
		eventQueue->appendEvent(stopEvent);
	}

	void handleEvents()
	{
		EventQueue* eventQueue = EventQueue::getEventQueue();
		Event* evnt = eventQueue->deleteEvent();
		int running = 1;
		while(1 == running && NULL != evnt)
		{
			updateClock(evnt);
			Common* common = Common::getCommon();
			if(common->clock > common->timeline)
			{
				printf("time = %f\n", common->timeline);
				common->timeline += 0.1;
			}
			//printf("%f\n", common->clock);
			switch(evnt->type)
			{
			case EVENT_TYPE_INTEREST_PACKET:
			case EVENT_TYPE_DATA_PACKET:
			case EVENT_TYPE_NACK_PACKET:
				//printf("before handlePacket(...)\n");
				handlePacket((Packet*)evnt); 
				//printf("after handlePacket(...)\n");
				break;
			case EVENT_TYPE_PROCESS_PACKET:
				//printf("before handleProcessPacketEvent(...)\n");
				handleProcessPacketEvent((ProcessPacketEvent*)evnt); 
				//printf("after handleProcessPacketEvent(...)\n");
				break;
			case EVENT_TYPE_CLIENT_GENERATE_INTEREST_PACKET:
				//printf("before handleGenerateInterestEvent(...)\n");
				handleGenerateInterestEvent((GenerateInterestEvent*)evnt); 
				//printf("after handleGenerateInterestEvent(...)\n");
				break;
			case EVENT_TYPE_STOP:
				running = 0; break;
			case EVENT_TYPE_DYNAMIC_ROUTING_UPDATE:
				//printf("before handleDynamicRoutingUpdateEvent(...)\n");
				handleDynamicRoutingUpdateEvent((DynamicRoutingUpdateEvent*)evnt); 
				//printf("after handleDynamicRoutingUpdateEvent(...)\n");
				break;
			case EVENT_TYPE_INTEREST_PACKET_TIMEOUT:	// an outstanding interest packet has been timed out
				// and it should be cancelled.
				//printf("before handleInterestPacketTimeoutEvent(...)\n");
				//fprintf(TraceFile::debugFile, "EVENT_TYPE_INTEREST_PACKET_TIMEOUT\n");
				handleInterestPacketTimeoutEvent((InterestPacketTimeoutEvent*)evnt);
				//printf("after handleInterestPacketTimeoutEvent(...)\n");
				break;
			}
			evnt = eventQueue->deleteEvent();
		}
	}

	void start()
	{
		installStaticRoutes();
		FileStore* fileStore = FileStore::getFileStore();
		fileStore->generateFileNames();
		computeEgoBetweenness();
		handleEvents();
	}
	void finish()
	{
		if(NULL != TraceFile::file) fclose(TraceFile::file);
	}
};

Simulator* Simulator::simulator = NULL;
#endif