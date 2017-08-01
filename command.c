#ifndef _INCLUDE_COMMAND_H 
#define _INCLUDE_COMMAND_H 1  
#include "command.h"
#endif

#include "func.c"

int nil(int fd, Arg *args)
{
	freeArgs(args);
	return 1;
} 

int status(int fd, Arg *args)
{
	char stat[512] = {'\0'};
	char head[11] = {'\0'};
	int memtotal = get_rmem(Serv->pid);
	int arg_len =0;

	sprintf(stat,
"{\"PID\":%d,\
\"START_TIME\":\"%s\",\
\"CLIENT_CONNECTION\":%d,\
\"MEMORY_TOTAL\":%d,\
\"HASH_TABLE_SIZE\":%d,\
\"HASH_TABLE_TOTAL\":%d,\
\"MEMPOOL_SIZE\":%d,\
\"MEMPOOL_TOTAL\":%d,\
\"CHUNK_SIZE\":%d}\n",
	Serv->pid,
	Serv->start_time,
	Serv->client_connection,
	memtotal,
	Serv->ht->size,
	Serv->ht->total,
	Serv->mp->size,
	Serv->mp->total,
	Serv->mp->cksize);
	arg_len = strlen(stat);
	sprintf(head, "1001%05d ", arg_len);
	write(fd, head, strlen(head));
	write(fd, stat, arg_len);
	freeArgs(args);
	return 1;
} 

int push(int fd, Arg *args)
{

	if(args->len == 0 || args->len >= 1024)
	{
		write(fd, "100200001 0\n", 12);
		freeArgs(args);
		return 1;
	}
	
	Arg *arg = args;
	char * qname = (char *)malloc((arg->len+1)*sizeof(char));
	memset(qname, '\0', arg->len+1);
	memcpy(qname, *(arg->cursor->ptr), arg->len);
	qname = trim(qname, args->len);
	arg++;
	AOQ *aoq = getQueue(qname, ht);
	free(qname);
	Qnode *qnode = createQnode(args);
	pushQueue(aoq, qnode);
	write(fd, "100200001 1\n", 12);

	return 1;
} 

int pop(int fd, Arg *args)
{
	if(args->len == 0 || args->len >= 1024)
	{
		write(fd, "100300000 \n", 11);
		freeArgs(args);
		return 1;
	}
	char head[11] = {'\0'};
	char *qname = (char *)malloc((args->len+1)*sizeof(char));
	memset(qname, '\0', args->len+1);
	memcpy(qname, *(args->cursor->ptr), args->len);
	qname = trim(qname, args->len);
	
	AOQ *aoq = getQueue(qname, ht);
	free(qname);
		
	Qnode **qnode = (Qnode **)malloc(sizeof(Qnode *));

	int r = popQueue(aoq, qnode);
	if(r >0)
	{
		Arg *arg = (*qnode)->arg+1;
		char **ptr = arg->cursor->ptr;
		sprintf(head, "1003%05d ", arg->len);
		write(fd, head, 10);
		write(fd, *ptr, strlen(*ptr));

		while(arg->cursor->node->next != NULL)
		{
			arg->cursor->node = arg->cursor->node->next;
			*ptr = arg->cursor->node->chunk;
			write(fd, *ptr, strlen(*ptr));

		}

		freeArgs((*qnode)->arg);
		ptr = NULL;
	}
	else
	{
		write(fd, "100300000 \n", 11);
	}
	
	free(*qnode);
	*qnode = NULL;
	freeArgs(args);
	return 1;

} 

int queues(int fd, Arg *args)
{
	char head[11] = {'\0'};
	int arg_len = 0;
	char *qs = (char *)malloc(1024*sizeof(char));
	memset(qs, '\0', 1024);

	hash_keys(ht, qs, &arg_len);
	
	if(arg_len == 0)
	{
		write(fd, "100400000 \n", 11);
	}
	else
	{
		sprintf(head, "1004%05d ", arg_len);
		write(fd, head, 10);
		write(fd, qs, arg_len);
	}

	free(qs);
	qs = NULL;
	freeArgs(args);
	return 1;
	
}

int queue(int fd, Arg *args)
{
	if(args->len == 0 || args->len >= 1024)
	{
		write(fd, "100500000 \n", 11);
		freeArgs(args);
		return 1;
	}
	char q[200] = {'\0'};
	char head[11] = {'\0'};
	int arg_len = 0;
	void **r = (void **)calloc(1, sizeof(void *));
	char * qname = (char *)malloc((args->len+1)*sizeof(char));
	memset(qname, '\0', args->len+1);
	memcpy(qname, *(args->cursor->ptr), args->len);
	qname = trim(qname, args->len);
	hash_find(ht, qname, r);
	free(qname);
	AOQ *aoq = (AOQ *)(*r);

	if(aoq == NULL)
	{
		write(fd, "100500000 \n", 11);
	}
	else
	{
		int head_id = 0;
		int tail_id = 0;
		if(aoq->head->next != NULL)
		{
			head_id = aoq->head->next->uniqid;
		}
		
		if(aoq->tail->next != NULL)
		{
			tail_id = aoq->tail->next->uniqid;
		}
		
		sprintf(q, "{\"TOTAL\":%d,\"HEAD_ID\":%d,\"TAIL_ID\":%d}\n", aoq->total, head_id, tail_id);
		arg_len = strlen(q);
		sprintf(head, "1005%05d ", arg_len);
		write(fd, head, 10);
		write(fd, q, arg_len);
	}
	
	freeArgs(args);
	free(r);
	r = NULL;
	return 1;
}  

int delqueue(int fd, Arg *args)
{
	if(args->len == 0 || args->len >= 1024)
	{
		write(fd, "100600001 0\n", 12);
		freeArgs(args);
		return 1;
	}
	void **r = (void **)malloc(sizeof(void *));
	char * qname = (char *)malloc((args->len+1)*sizeof(char));
	memset(qname, '\0', args->len+1);
	memcpy(qname, *(args->cursor->ptr), args->len);
	qname = trim(qname, args->len);
	hash_find(ht, qname, r);
	freeArgs(args);
	AOQ *aoq = (AOQ *)(*r);

	if(aoq == NULL)
	{
	
		write(fd, "100600001 0\n", 12);
	}
	else
	{
		deleteQueue(aoq);
		hash_delete(ht, qname);
		write(fd, "100600001 1\n", 12);
	}
	
	free(qname);
	free(r);
	r = NULL;
	return 1;

}  

COMMANDFUNCPTR commandFunc[] = {nil, status, push, pop, queues, queue, delqueue};
