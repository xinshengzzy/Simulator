#ifndef TRACE_FILE_H
#define TRACE_FILE_H
#include <stdio.h>
// the format of the trace file is as follows.
// When a server responds to an interest packet, the output of the trace file is 
// "time s fromNode toNode", where "time" is the time when the event occurs, "s" is a flag, "fromNode" and "toNode" are the fromNode and toNode fields of the 
// response data packet respectively.
// When a client receives a data packet, the output of the trace file is
// "time c fromNode toNode current-router-dist client-to-server-dist", where "time" is the time this event occurs, "c" is a flag, "fromNode" and "toNode" are
// the "fromNode" and "toNode" fields of the data packet respectively, "current-router-dist" is the distance from current client to the data provider,
// "client-to-server-dist" is the distance from current client to the source server corresponding to the data packet.
// When cache hit occurs, the output of the trace file is 
// "time h nodeId", where "time" is the time when this event occurs, "h" is a flag and "nodeId" is the id of the router where this cache hit event occurs.
// When a data packet is cached, the output of the trace file is
// "time a", where "time" is the time when this event occurs and "a" is a flag.
// When a packet is transmitted from a node to another node, the output of the trace file is 
// "time t fromNode toNode packetType", where "time" is the time when this event occurs, "t" is a flag, "fromNode" and "toNode" are the IDs of the nodes where the
// packet is from and where the packet is transmitted to, respectively. If it is a data packet, the packetType is "d", it is "i" for an interest packet and 
// it is "n" for a nack packet.
class TraceFile
{
public:
	static FILE* file;
	static FILE* debugFile;
	static void setPath(char* path)
	{
		fopen_s(&file, path, "w+");
	}
	static void setDebugPath(char* path)
	{
		fopen_s(&debugFile, path, "w+");
	}
};
FILE* TraceFile::file = NULL;
FILE* TraceFile::debugFile = NULL;
#endif