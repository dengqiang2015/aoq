#ifndef _INCLUDE_MEMPOOL_H 
#define _INCLUDE_MEMPOOL_H 1  
#include "mempool.h"
#endif

int initMemPool(MemPool * mp, int mpsize, int cksize)
{
	mp->head = (ChunkNode *)calloc(1, sizeof(ChunkNode));
	mp->tail = (ChunkNode *)calloc(1, sizeof(ChunkNode));
	mp->head->next = NULL;
	mp->tail->next = NULL;
	mp->total = 0;
	mp->size = mpsize;
	mp->cksize = cksize;
	return 1;
}


int createMemPool(MemPool * mp)
{
	int i = 0;
	ChunkNode * node;
	for(i=0;i<mp->size;i++)
	{
		node = (ChunkNode *)malloc(sizeof(ChunkNode));
		node->chunk = (char *)malloc(mp->cksize*sizeof(char));
		memset(node->chunk, '\0', mp->cksize);
		node->next = NULL;
		mp->tail->next = node;
		mp->tail = node;
		mp->total++;
		if(i == 0)
		{
			mp->head->next = node;
		}
	}
	return 1;
}


ChunkNode * mpalloc()
{
	ChunkNode * node = NULL;
	if(mp->head->next == NULL)
	{
		node = (ChunkNode *)malloc(sizeof(ChunkNode));
	}
	else
	{
		node = mp->head->next;
		mp->head->next = mp->head->next->next;
		mp->total--;
	}
	
	node->next = NULL;
	return node;
}


int mpfree(ChunkNode * node)
{

	if(mp->total >= mp->size)
	{
		free(node);
		node = NULL;
	}
	else
	{
		memset(node->chunk, '\0', mp->cksize);
		node->next = NULL;
		mp->tail->next = node;
		mp->tail = node;
		
		if(mp->total == 0)
		{
			mp->head->next = node;
		}
		
		mp->total++;

	}
	return 1;
}


MemSlab * createMemSlab(int size)
{
	MemSlab * slab = (MemSlab *)malloc(size*sizeof(MemSlab));
	slab->head = (ChunkNode *)malloc(sizeof(ChunkNode)); 
	slab->tail = (ChunkNode *)malloc(sizeof(ChunkNode)); 
	slab->head->next = NULL;
	slab->tail->next = NULL;
	return slab;
}


int insertMemSlab(MemSlab *memslab, ChunkNode *node)
{
	if(memslab->head->next == NULL)
	{
		memslab->head->next = node;
	}
	
	if(memslab->tail->next == NULL)
	{
		memslab->head->next = node;
	}
	else
	{
		memslab->tail->next->next = node;
		memslab->tail->next = node;
	}

	return 1;
}

int freeMemSlab(MemSlab *memslab)
{

	if(memslab == NULL)
	{
	
		return 0;
	}

	ChunkNode *node;

	
	if(memslab->head != NULL)
	{
		while(memslab->head->next != NULL)
		{
			node = memslab->head->next;
			memslab->head->next = node->next;
			mpfree(node);
		}
	}
	free(memslab->head);
	free(memslab->tail);
	memslab->head = NULL;
	memslab->tail = NULL;
	free(memslab);
	memslab = NULL;
	return 1;
}

ChunkCursor * createCursor()
{
	ChunkCursor * cursor = (ChunkCursor *)malloc(sizeof(ChunkCursor));
	cursor->ptr = (char **)malloc(sizeof(char *));
	cursor->node = NULL;
	cursor->slab = NULL;
	return cursor;
}

int moveCursor(ChunkCursor * cursor, int len)
{
	int offset = 0;
	char *ptr = *(cursor->ptr);
	if(len >0)
	{
		while(offset < len)
		{
			if(*ptr == '\0')
			{
				
				if(cursor->node->next == NULL)
				{
					return offset;
				}
				ptr = cursor->node->next->chunk;
				cursor->node = cursor->node->next;
			}
			else
			{
				offset++;
				ptr++;
			}
		}
		*(cursor->ptr) = ptr;
		return offset;
	}
	else
	{
		return 0;
	}
}


int freeCursor(ChunkCursor * cursor)
{
	if(cursor == NULL)
	{
		return 0;
	}
	free(cursor->ptr);
	cursor->ptr = NULL;
	free(cursor);
	cursor = NULL;
	return 1;
}