#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _chunk_node
{
	char * chunk;
	struct _chunk_node * next;
}ChunkNode;


typedef struct _mem_pool
{
	int total;
	int size;
	int cksize;
	struct _chunk_node * tail;
	struct _chunk_node * head;
}MemPool;

typedef struct _mem_slab
{
	struct _chunk_node * head;
	struct _chunk_node * tail;
}MemSlab;

typedef struct _chunk_cursor
{
	char **ptr;
	struct _chunk_node * node;
	struct _mem_slab * slab;
}ChunkCursor;


MemPool * mp;
MemSlab *memslab;
 
int initMemPool(MemPool * mp, int mpsize, int cksize);
int createMemPool(MemPool * mp);
ChunkNode * mpalloc();
int mpfree(ChunkNode * node);
MemSlab * createMemSlab(int size);
int insertMemSlab(MemSlab *memslab, ChunkNode *node);
int freeMemSlab(MemSlab *memslab);
ChunkCursor * createCursor();
int moveCursor(ChunkCursor * cursor, int len);
int freeCursor(ChunkCursor * cursor);
