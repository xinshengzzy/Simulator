#ifndef COMMON_H
#define COMMON_H
#include <string.h>
#include <stdlib.h>
#include "Event.h"
#include "Config.h"

class Common
{
private:
	PacketId packetId;	// this variable guarantees that every data packet, interest 
		// packet and nack packet will have a unique id.
	static Common* common;
	Common()
	{
//		printf("Common()\n");
		clock = 0.0;
		packetId = 0;
		flag = 0;
		timeline = 0.0;
	}
public:
	double clock;	// We maintain a global clock for all the entities.
	int flag;		// this is used for debugging only.
	double timeline;	// this variable is to watch the progress of the program.
	static Common* getCommon()
	{
		//printf("getCommon()\n");
		if(NULL == common)
			common = new Common();
		return common;
	}

	// generate an ID for an interest packet, a data packet or a nack packet.
	PacketId generatePacketId()
	{
		return packetId++;
	}

	Digest computeDigest(char* str)
	{
		Digest hash = 7;
		int len = 0;
		if(NULL != str)
			len = strlen(str);
		for(int i = 0; i < len; ++i)
			hash = hash*31 + str[i];
		return hash;
	}

	void copystring(char* & dest, char* src)
	{
		int len = strlen(src);
		dest = (char*)malloc((len + 1)*sizeof(char));
		for(int i = 0; i < len; ++i)
			dest[i] = src[i];
		dest[len] = '\0';
	}

	void copystring(char* & dest, char* src, int count)
	{
		dest = (char*)malloc((count + 1)*sizeof(char));
		for(int i = 0; i < count; ++i)
			dest[i] = src[i];
		dest[count] = '\0';
	}

	// Generate a random string with the length of 'len', and pointed by str.
	void generateRandomString(char* & str, int len)
	{
		int base = 53;
		char* charSet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@";
		str = (char*)malloc((len + 1)*sizeof(char));
		for(int i = 0; i < len; ++i)
			str[i] = charSet[rand()%base];
		str[len] = '\0';
	}
};

Common* Common::common = NULL;

#endif