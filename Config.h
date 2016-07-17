#ifndef CONFIG_H
#define CONFIG_H

typedef unsigned long Digest;
typedef long PacketId;

// bandwidth of clients, servers and rotuers:
#define CLIENT_BANDWIDTH 100000000.0			// the output rate of a client is 100Mbps
#define SERVER_BANDWIDTH 40000000000.0		// the data rate of a server is 40Gbps
#define ROUTER_BANDWIDTH 10000000000.0		// the data rate of a router is 10Gbps

#define LINK_DELAY 0.000020				// the delay of a link is 20 us.

// packet size:
#define DATA_PACKET_SIZE 1500			// the size of a data packet is 1500 bytes
#define INTEREST_PACKET_SIZE 64		// the size of a interest packet is 64 bytes
#define NACK_PACKET_SIZE 64			// the size of a NACK packet is 64 bytes

// processing time of interest packets, data packets and nack packets.
// We assume that the outgoing bandwidth of a router is 10Gbps.
#define INTEREST_PACKET_PROCESSING_TIME 0.0000000512
#define DATA_PACKET_PROCESSING_TIME 0.0000012
#define NACK_PACKET_PROCESSING_TIME 0.0000000512

// the time needed for a client to generate an interest packet
#define TIME_FOR_GENERATING_INTEREST_PACKET 0.00000512
// the time need for a server to generate a data packet.
#define TIME_FOR_GENERATING_DATA_PACKET 0.0000003

// Event types:
#define EVENT_TYPE_INTEREST_PACKET 1	// the event type of interest packets.
#define EVENT_TYPE_DATA_PACKET 2		// the event type of data packets.
#define EVENT_TYPE_NACK_PACKET 3		// the event type of nack packets.
#define EVENT_TYPE_PROCESS_PACKET 4		// notify the client, router or server to process another packet.
#define EVENT_TYPE_INTEREST_PACKET_TIMEOUT 5 // indicate that an outstanding interest packet has timed out.
#define EVENT_TYPE_STOP 6	// this is a stop event.
#define EVENT_TYPE_CLIENT_GENERATE_INTEREST_PACKET 7	// the event type of a generate interest event.
#define EVENT_TYPE_DYNAMIC_ROUTING_UPDATE 8				// the event type of a dynamic routing update event.

#define CLIENT_MAX_NUM_OF_OUTSTANDING_INTEREST_PACKETS 10	// maximum number of ongoing interest packets whose 
		// corresponding data packets have not been returned for a client

// node types of clients, routers and servers.
#define NODE_TYPE_CLIENT 1
#define NODE_TYPE_ROUTER 2
#define NODE_TYPE_SERVER 3

#define ALPHA 0.125

#define NUM_OF_PACKET_PER_FILE 100

// caching methods
#define CACHING_METHOD_SADO 1
#define CACHING_METHOD_PROBCACHE 2
#define CACHING_METHOD_EGOBETW 3
#define CACHING_METHOD_WAVE 4
#define CACHING_METHOD_CCN 5

int CACHING_METHOD = CACHING_METHOD_SADO;

#endif