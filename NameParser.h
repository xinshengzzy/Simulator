#ifndef NAME_PARSER_H
#define NAME_PARSER_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Common.h"
class NameParser
{
private:
	NameParser()
	{
		this->numOfComponents = 0;
		this->numOfPackets = 0;
		this->packetIndex = 0;
		this->components = NULL;
	}
	void reset()
	{
		Common* common = Common::getCommon();
		for(int i = 0; i < numOfComponents; ++i)
			free(this->components[i]);
		free(this->components);
		this->numOfComponents = 0;
		this->numOfPackets = 0;
		this->packetIndex = 0;
	}
	static NameParser* nameParser;
public:
	int numOfComponents;	// the number of components in this name
	int numOfPackets;		// the number of data packets making up the file corresponding to this data packet
	int packetIndex;		// the index of this data packet
	char** components;
	static NameParser* getNameParser()
	{
		if(NULL == nameParser)
			nameParser = new NameParser();
		return nameParser;
	}

	// We assume that every component of the name is started with a '/', and the name cannot be ended with '/'
	void parseName(char* name)
	{
		Common* common = Common::getCommon();
//		if(1 == common->flag)
//		{
//			printf("enter parseName(...)\n");
//			printf("%s\n", name);
//		}
//		if(0 == strcmp(name, "/www.baidu.com/wbAHvtSDgVIvlNgYiCGQsQoUzcCfKtKqhYFuX@HV/1000/327"))
//			printf("enter parseName(...)\n");
		int len = strlen(name);
		if('/' != name[0] || '/' == name[len - 1])
		{
			printf("Error: in NameParser::parseName(...): each component of the name must be started with '/' and the \
				   name cannot be ended with '/'.\n");
			return;
		}
		reset();
		this->numOfComponents = 0;
		for(int i = 0; i < len; ++i)
			if('/' == name[i]) ++this->numOfComponents;
		this->components = (char**)malloc(this->numOfComponents*sizeof(char*));
		int componentIndex = -1;
		for(int i = 0; i < len; ++i)
			if('/' == name[i])
			{
				if(componentIndex >= 0)
					common->copystring(this->components[componentIndex], name, i);
				++componentIndex;
			}
		common->copystring(this->components[componentIndex], name);

		char num[10];
		char index[10];
		memset(num, 0, 10*sizeof(char));
		memset(index, 0, 10*sizeof(char));
		int len1 = strlen(this->components[this->numOfComponents - 3]);
		int len2 = strlen(this->components[this->numOfComponents - 2]);
		int len3 = strlen(this->components[this->numOfComponents - 1]);
		int k = 0;
		for(int i = len1 + 1; i < len2; ++i)
			num[k++] = this->components[this->numOfComponents - 2][i];
		num[k] = '\0';
		this->numOfPackets = atoi(num);
		k = 0;
		for(int i = len2 + 1; i < len3; ++i)
		{
			index[k++] = this->components[this->numOfComponents - 1][i];
		}
		index[k] = '\0';
		this->packetIndex = atoi(index);
//		if(0 == strcmp(name, "/www.baidu.com/wbAHvtSDgVIvlNgYiCGQsQoUzcCfKtKqhYFuX@HV/1000/327"))
//			printf("leave parseName(...)\n");
//		if(1 == common->flag)
//			printf("leave parseName(...)\n");
	}
};
NameParser* NameParser::nameParser = NULL;
#endif