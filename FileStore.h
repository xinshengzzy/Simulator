#ifndef FILE_STORE_H
#define FILE_STORE_H
#include "Common.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

class ProducerInfo
{
public:
	char* producer;
	int numOfFiles;
	ProducerInfo* next;
	ProducerInfo()
	{
		producer = NULL;
		numOfFiles = 0;
		next = NULL;
	}
};

class FileStore
{
private:
	double alpha;	// the exponent in the zipf distribution
	double c;		// the coefficient in the zipf distribution
	int numOfFiles;	
	int maxNumOfFiles;
	char** files;
	ProducerInfo* producerInfoList;
	static FileStore* fileStore;

	FileStore()
	{
		alpha = 0.8;
		numOfFiles = 0;
		maxNumOfFiles = 0;
		files = NULL;
		producerInfoList = NULL;
	}
	void addFile(char* producer)
	{
		char* tempStr;
		Common* common = Common::getCommon();
		common->generateRandomString(tempStr, 40);
		int len1 = strlen(producer);
		int len2 = strlen(tempStr);
		int len = len1 + len2 + 2;
		char* fileName = (char*)malloc((len + 1)*sizeof(char));
		int i = 0;
		fileName[i++] = '/';
		for(int j = 0; j < len1; ++j)
			fileName[i++] = producer[j];
		fileName[i++] = '/';
		for(int j = 0; j < len2; ++j)
			fileName[i++] = tempStr[j];
		fileName[len] = '\0';
		free(tempStr);
		if(numOfFiles >= maxNumOfFiles)
		{
			int tempMaxNumOfFiles = 2*maxNumOfFiles + 1;
			char** tempFiles = (char**)malloc(tempMaxNumOfFiles*sizeof(char*));
			memset(tempFiles, 0, tempMaxNumOfFiles*sizeof(char));
			for(int i = 0; i < numOfFiles; ++i)
				tempFiles[i] = files[i];
			free(files);
			files = tempFiles;
			maxNumOfFiles = tempMaxNumOfFiles;
		}
		files[numOfFiles++] = fileName;
	}
	void addProducer(char* producer, int numOfFiles)
	{
		for(int i = 0; i < numOfFiles; ++i)
		{
			addFile(producer);
		}
	}
public:
	static FileStore* getFileStore()
	{
		if(NULL == fileStore) fileStore = new FileStore();
		return fileStore;
	}
	void addProducerInfo(char* producer, int numOfFiles)
	{
		ProducerInfo* producerInfo = new ProducerInfo();
		Common* common = Common::getCommon();
		common->copystring(producerInfo->producer, producer);
		producerInfo->numOfFiles = numOfFiles;
		producerInfo->next = producerInfoList;
		producerInfoList = producerInfo;
	}
	void generateFileNames()
	{
		if(NULL == producerInfoList)
		{
			printf("Error: failed to generate file names.\n");
			return;
		}
		ProducerInfo* producerInfo = producerInfoList;
		while(NULL != producerInfo)
		{
			addProducer(producerInfo->producer, producerInfo->numOfFiles);
			producerInfo = producerInfo->next;
		}
		// we shuffle the array for three times
		for(int i = numOfFiles - 1; i >= 0; --i)
		{
			int j = rand()%(i + 1);
			char* temp = files[i];
			files[i] = files[j];
			files[j] = temp;
		}
		for(int i = numOfFiles - 1; i >= 0; --i)
		{
			int j = rand()%(i + 1);
			char* temp = files[i];
			files[i] = files[j];
			files[j] = temp;
		}
		for(int i = numOfFiles - 1; i >= 0; --i)
		{
			int j = rand()%(i + 1);
			char* temp = files[i];
			files[i] = files[j];
			files[j] = temp;
		}
		c = 0;
		for(int i = 0; i < numOfFiles; ++i)
		{
			c += 1.0/pow((double)i + 1.0, alpha);
		}
	}
	char* getFileName()
	{
		if(0 == numOfFiles) return NULL;
		double randomNum = (double)rand()/(double)RAND_MAX;
		int k = 0;
		double sum = 0;
		for(; k < numOfFiles; ++k)
		{
				sum += 1.0/pow((double)k + 1.0, alpha);
				if(sum >= randomNum*c) return files[k];
		}
		return files[numOfFiles - 1];
	}	
	void print()
	{
		for(int i = 0; i < numOfFiles; ++i)
			printf("%d %s\n", i, files[i]);
	}
};
FileStore* FileStore::fileStore = NULL;
#endif