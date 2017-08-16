#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

  
#ifndef _INCLUDE_MEMPOOL_H 
#define _INCLUDE_MEMPOOL_H 1  
#include "mempool.h"
#endif


typedef struct _arg
{
	int len;
	struct _chunk_cursor *cursor;
}Arg;


int parse_comman_num(ChunkCursor *cursor);
int parse_head_len(ChunkCursor *cursor);
int parse_argn(ChunkCursor *cursor, int * argn);
int parse_argv(int len, ChunkCursor *cursor, Arg *arg);
int parse_args(MemSlab *memslab, int * command_num, Arg *args);
Arg *createArgs(int num);
int freeArgs(Arg *arg);
