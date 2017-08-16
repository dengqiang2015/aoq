#include<stdio.h>
#include <time.h>
#ifndef _INCLUDE_MEMPOOL_H 
#define _INCLUDE_MEMPOOL_H 1  
#include "mempool.h"
#endif

#define AOQ_LOG_FILE_NAME "aoq.log"
#define SERVER_LOG_FILE_NAME "server.log"

typedef struct _aoq_log
{
	char * log;
	int len;
	int size;
	FILE *fp_logfile;
}AOQLOG;

AOQLOG *aoqlog;
