#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <assert.h>

#ifndef _INCLUDE_MEMPOOL_H 
#define _INCLUDE_MEMPOOL_H 1  
#include "mempool.h"
#endif

#ifndef _INCLUDE_HASHTABLE_H 
#define _INCLUDE_HASHTABLE_H 1  
#include "hashtable.h"
#endif

#define VMPEAK_LINE 12//VMRSS所在行, 注:根据不同的系统,位置可能有所区别.
#define VMRSS_LINE 16//VMRSS所在行, 注:根据不同的系统,位置可能有所区别.
#define pid_t  int
#define MAX_CLIENT_CONNECTION 65535

typedef struct _server_stat
{
	int pid;
	char *start_time;
	const char *host;
	int port;
	int max_memory;
	int max_client_connection;
	int client_connection;
	long aoq_max_size;
	HashTable *ht;
	MemPool *mp;
	char *pid_file;
}ServerStat;

ServerStat *Serv;

void get_process_mem(const pid_t p, int *vmpeak, int *vmrss)
{
    char file[64] = {'\0'};
    FILE *fd;
    char line_buff[256] = {'\0'};
	char vmstr[30] = {'\0'};
    sprintf(file,"/proc/%d/status",p);

    fd = fopen (file, "r");

    int i;
    for (i=0; i<VMRSS_LINE; i++)
    {
		char *ret = fgets (line_buff, 256, fd);
		if(i == (VMPEAK_LINE-1))
		{
			 memcpy(vmstr, ret+7, strlen(line_buff)-10);
			 *vmpeak = atoi(vmstr);
		}
		else if( i == VMRSS_LINE-1)
		{
			memcpy(vmstr, ret+7, strlen(line_buff)-9);
			*vmrss = atoi(vmstr);
		}
		else
		{
			continue;
		}
       
    }
	
    fclose(fd); 
}

