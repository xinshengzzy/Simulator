#ifndef PIT_ENTRY_H
#define PIT_ENTRY_H
#include "IncomingFace.h"
#include "Common.h"
class PitEntry
{
public:
	char* interestPacketName;
	IncomingFace* faces;
	unsigned int digest;
	int height;
	double lifetime;
	PitEntry* parent;
	PitEntry* lchild;
	PitEntry* rchild;
	static int numOfPitEntries;

	PitEntry()
	{
		++numOfPitEntries;
		//printf("numOfPitEntries = %d\n", numOfPitEntries);
		this->interestPacketName = NULL;
		faces = NULL;
		Common* common = Common::getCommon();
		digest = 0;
		height = 1;
		parent = NULL;
		lchild = NULL;
		rchild = NULL;
		lifetime = -1.0;
	}

	void setInterestPacketName(char* interestPacketName)
	{
		Common* common = Common::getCommon();
		common->copystring(this->interestPacketName, interestPacketName);
		this->digest = common->computeDigest(this->interestPacketName);
	}

	void reset()
	{
		free(interestPacketName);
		interestPacketName = NULL;
		faces = NULL;
		digest = 0;
		height = 1;
		lifetime = -1.0;
		lchild = rchild = parent = NULL;
	}

	void addFace(IncomingFace* incomingFace)
	{
		incomingFace->next = faces;
		faces = incomingFace;
	}

	void swap(PitEntry* entry)
	{
		char* name = this->interestPacketName;
		this->interestPacketName = entry->interestPacketName;
		entry->interestPacketName = name;

		IncomingFace* tempFace = this->faces;
		this->faces = entry->faces;
		entry->faces = tempFace;

		Digest digest = this->digest;
		this->digest = entry->digest;
		entry->digest = digest;

		int height = this->height;
		this->height = entry->height;
		entry->height = height;

		double lifetime = this->lifetime;
		this->lifetime = entry->lifetime;
		entry->lifetime = this->lifetime;
	}
};
int PitEntry::numOfPitEntries = 0;
#endif