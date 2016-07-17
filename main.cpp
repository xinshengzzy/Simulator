#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Common.h"
#include "NameParser.h"
#include "Config.h"
#include "Topology.h"
#include "Client.h"
#include "Server.h"
#include "Router.h"
#include "Simulator.h"
#include "Event.h"
#include "Pit.h"
#include "PitEntry.h"
#include "ContentStore.h"
#include "ContentStoreEntry.h"
#include "FileStore.h"
#include "math.h"
#include "TraceFile.h"

void initialise()
{
	srand(0);
}

int main()
{
/*	Client* client1 = topology->createClient();
	Client* client2 = topology->createClient();
	Client* client3 = topology->createClient();
	Client* client4 = topology->createClient();
	Client* client5 = topology->createClient();
	Client* client6 = topology->createClient();

	Router* router1 = topology->createRouter();
	Router* router2 = topology->createRouter();
	Router* router3 = topology->createRouter();
	Router* router4 = topology->createRouter();

	Server* server1 = topology->createServer();
	Server* server2 = topology->createServer();
	Server* server3 = topology->createServer();
	Server* server4 = topology->createServer();
	Server* server5 = topology->createServer();
	Server* server6 = topology->createServer();

	topology->addLink(client1, router1);
	topology->addLink(client2, router1);
	topology->addLink(client3, router1);
	topology->addLink(client4, router3);
	topology->addLink(client5, router3);
	topology->addLink(client6, router3);

	topology->addLink(router1, router2);
	topology->addLink(router1, router3);
	topology->addLink(router3, router4);
	topology->addLink(router2, router4);

	topology->addLink(router2, server1);
	topology->addLink(router2, server2);
	topology->addLink(router2, server3);
	topology->addLink(router4, server4);
	topology->addLink(router4, server5);
	topology->addLink(router4, server6);

	server1->addPrefix("/www.baidu.com");
	server2->addPrefix("/www.tencent.com");
	server3->addPrefix("/www.google.com");
	server4->addPrefix("/www.yahoo.com");
	server5->addPrefix("/www.twitter.com");
	server6->addPrefix("/www.facebook.com");

	FileStore* fileStore = FileStore::getFileStore();
	fileStore->addProducerInfo("www.baidu.com", 20);
	fileStore->addProducerInfo("www.tencent.com", 20);
	fileStore->addProducerInfo("www.google.com", 20);
	fileStore->addProducerInfo("www.yahoo.com", 20);
	fileStore->addProducerInfo("www.twitter.com", 20);
	fileStore->addProducerInfo("www.facebook.com", 20);

	client1->startAt(1.0);
	client2->startAt(1.0);
	client3->startAt(1.0);
	client4->startAt(1.0);
	client5->startAt(1.0);
	client6->startAt(1.0);*/

	initialise();
	CACHING_METHOD = CACHING_METHOD_CCN;
	TraceFile::setPath("./trace/out.tr");
	Topology* topology = Topology::getTopology();


	Node* nodes[255];
	int k = 0;
	nodes[k++] = topology->createServer();
	for(int i = 0; i < 126; ++i)
	{
		nodes[k++] = topology->createRouter();
	}
	for(int i = 0; i < 128; ++i)
		nodes[k++] = topology->createClient();
	int i = 0, j = 1;
	while(j < 255)
	{
		topology->addLink(nodes[i], nodes[j++]);
		topology->addLink(nodes[i], nodes[j++]);
		++i;
	}

	Server* server = (Server*)nodes[0];
	server->addPrefix("/www.baidu.com");
	server->addPrefix("/www.tencent.com");
	server->addPrefix("/www.taobao.com");
	server->addPrefix("/www.netease.com");
	server->addPrefix("/www.sina.com");
	server->addPrefix("/www.google.com");
	server->addPrefix("/www.yahoo.com");
	server->addPrefix("/www.twitter.com");
	server->addPrefix("/www.facebook.com");
	server->addPrefix("/www.amazon.com");

	FileStore* fileStore = FileStore::getFileStore();
	fileStore->addProducerInfo("www.baidu.com", 100);
	fileStore->addProducerInfo("www.tencent.com", 100);
	fileStore->addProducerInfo("www.taobao.com", 100);
	fileStore->addProducerInfo("www.netease.com", 100);
	fileStore->addProducerInfo("www.sina.com", 100);
	fileStore->addProducerInfo("www.google.com", 100);
	fileStore->addProducerInfo("www.yahoo.com", 100);
	fileStore->addProducerInfo("www.twitter.com", 100);
	fileStore->addProducerInfo("www.facebook.com", 100);
	fileStore->addProducerInfo("www.amazon.com", 100);

	for(int i = 127; i < 255; ++i)
	{
		Client* client = (Client*)nodes[i];
		client->startAt(0.0);
	}

	Simulator* simulator = Simulator::getSimulator();
	simulator->stopAt(10.0);
	simulator->start();
	return 0;
}