#ifndef CONTENT_STORE_ENTRY_TIMESTAMP_H
#define CONTENT_STORE_ENTRY_TIMESTAMP_H
#include <stdlib.h>

class ContentStoreEntry;
class ContentStoreEntryTimestamp
{
public:
	ContentStoreEntry* contentStoreEntry;
	ContentStoreEntryTimestamp* previous;
	ContentStoreEntryTimestamp* next;
	double cacheTime;
	ContentStoreEntryTimestamp(ContentStoreEntry* contentStoreEntry, double cacheTime)
	{
		this->contentStoreEntry = contentStoreEntry;
		this->previous = NULL;
		this->next = NULL;
		this->cacheTime = cacheTime;
	}
};
#endif