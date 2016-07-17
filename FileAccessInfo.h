#ifndef FILE_ACCESS_INFO_H
#define FILE_ACCESS_INFO_H
#include "Config.h"
#include "stdlib.h"
// this class is used as elements in the class FileAccessInfo. Each object of the class corresponds to a single face.
class FileAccessInfoPerFace
{
public:
	int face;			// the face associated with this info object.
	int upperBound;		// for the meaning of the variable upperBound and cached, please refer to the paper "WAVE: Popularity-based
	int cached;			// and Collaborative In-network Caching for Content-Oriented Networks"
	FileAccessInfoPerFace* next;

	FileAccessInfoPerFace()
	{
		face = -1;
		upperBound = 0;
		cached = -1;
		next = NULL;
	}

	void reset()
	{
		face = -1;
		upperBound = 0;
		cached = -1;
		next = NULL;
	}
};
// this class is used for the caching method of WAVE. Each object of the class
// will be associated with a single file.
class FileAccessInfo
{
public:
	char* fileName;	// the name of the file.
	Digest digest;	// the digest of the file name.
	int numOfPackets; // number of packets of the file cached in this router.
	int height;	// the height of the subtree of which this is the root.
	FileAccessInfoPerFace* fileAccessInfoForFaces;
	FileAccessInfo* parent;
	FileAccessInfo* lchild;
	FileAccessInfo* rchild;
	
	FileAccessInfo()
	{
		fileName = NULL;
		digest = 0;
		numOfPackets = 0;
		height = 0;
		fileAccessInfoForFaces = NULL;
		parent = lchild = rchild = NULL;
	}

	void reset()
	{
		if(NULL != fileName)
		{
			free(fileName);
		}
		fileName = NULL;
		digest = 0;
		numOfPackets = 0;
		height = 0;
		fileAccessInfoForFaces = NULL;
		parent = lchild = rchild = NULL;
	}

	void setFileName(char* fileName)
	{
		Common* common = Common::getCommon();
		common->copystring(this->fileName, fileName);
		digest = common->computeDigest(fileName);
	}

	void swap(FileAccessInfo* fileAccessInfo)
	{
		char* tempFileName = this->fileName;
		this->fileName = fileAccessInfo->fileName;
		fileAccessInfo->fileName = tempFileName;

		Digest tempDigest = this->digest;
		this->digest = fileAccessInfo->digest;
		fileAccessInfo->digest = tempDigest;

		int tempNumOfPackets = this->numOfPackets;
		this->numOfPackets = fileAccessInfo->numOfPackets;
		fileAccessInfo->numOfPackets = tempNumOfPackets;

		int tempHeight = this->height;
		this->height = fileAccessInfo->height;
		fileAccessInfo->height = tempHeight;

		FileAccessInfoPerFace* tempFileAccessInfoForFaces = this->fileAccessInfoForFaces;
		this->fileAccessInfoForFaces = fileAccessInfo->fileAccessInfoForFaces;
		fileAccessInfo->fileAccessInfoForFaces = tempFileAccessInfoForFaces;
	}

	FileAccessInfoPerFace* getInfo(int face)
	{
		FileAccessInfoPerFace* info = fileAccessInfoForFaces;
		while(NULL != info)
		{
			if(face == info->face) break;
			info = info->next;
		}
		if(NULL == info)
		{
			info = new FileAccessInfoPerFace();
			info->face = face;
			info->next = fileAccessInfoForFaces;
			fileAccessInfoForFaces = info;
		}
		return info;
	}
};


#endif