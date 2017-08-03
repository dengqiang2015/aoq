#ifndef _INCLUDE_COMMAND_H 
#define _INCLUDE_COMMAND_H 1  
#include "command.h"
#endif

#include "func.c"

char * get_queue_name(Arg *args)
{
	char * qname = (char *)malloc((args->len+1)*sizeof(char));
	memset(qname, '\0', args->len+1);
	memcpy(qname, *(args->cursor->ptr), args->len);
	qname = trim(qname, args->len);
	return qname;
}


int nil(int fd, Arg *args)
{
	freeArgs(args);
	return 1;
} 

int status(int fd, Arg *args)
{
	char head[12] = {'\0'};
	char res[512] = {'\0'};
	int memtotal = get_rmem(Serv->pid);
	int arg_len = 0;

	sprintf(res,
"0000000000 {\"PID\":%d,\
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
	arg_len = strlen(res);
	sprintf(head, "1101%06d ", arg_len-12);
	memcpy(res, head, 11);
	write(fd, res, arg_len);
	freeArgs(args);
	return 1;
} 

int push(int fd, Arg *args)
{

	if(args->len == 0 || args->len >= 1024)
	{
		write(fd, "1102000001 0\n", 12);
		freeArgs(args);
		return 1;
	}
	

	char *qname = get_queue_name(args);
	if(strlen(qname) <=0 )
	{

		freeArgs(args);
		free(qname);
		write(fd, "1102000001 0\n", 13);
		return 1;
	}
	AOQ *aoq = getQueue(qname, ht);
	free(qname);
	Qnode *qnode = createQnode(args);
	pushQueue(aoq, qnode);
	write(fd, "1102000001 1\n", 13);

	return 1;
} 

int pop(int fd, Arg *args)
{
	if(args->len == 0 || args->len >= 1024)
	{
		write(fd, "1103000000 \n", 12);
		freeArgs(args);
		return 1;
	}

	char *qname = get_queue_name(args);
	if(strlen(qname) <=0 )
	{

		freeArgs(args);
		free(qname);
		write(fd, "1102000000 \n", 12);
		return 1;
	}
	AOQ *aoq = getQueue(qname, ht);
	free(qname);
		
	Qnode **qnode = (Qnode **)malloc(sizeof(Qnode *));

	int r = popQueue(aoq, qnode);
	if(r >0)
	{

		Arg *arg = (*qnode)->arg+1;
		char **ptr = arg->cursor->ptr;
		memset(pop_response, '\0', arg->len+12);
		sprintf(pop_response, "1103%06d ", arg->len);
		strcat(pop_response, *ptr);
		while(arg->cursor->node->next != NULL)
		{
			arg->cursor->node = arg->cursor->node->next;
			*ptr = arg->cursor->node->chunk;
			strcat(pop_response, *ptr);
		}
		write(fd, pop_response, strlen(pop_response));
		freeArgs((*qnode)->arg);
		ptr = NULL;
	}
	else
	{
		write(fd, "1103000000 \n", 12);
	}
	
	free(*qnode);
	*qnode = NULL;
	freeArgs(args);
	return 1;

} 

int queues(int fd, Arg *args)
{

	int arg_len = 11;
	char qs[1048576] = {'\0'};
	sprintf(qs, "1104000000 ", arg_len);

	hash_keys(ht, &qs[11], &arg_len);
	
	if(arg_len == 11)
	{
		write(fd, "1104000000 \n", 12);
	}
	else
	{
		char head[12] = {'\0'};
		sprintf(head, "1104%06d ", arg_len-12);
		memcpy(qs, head, 11);
		write(fd, qs, arg_len);
	}

	freeArgs(args);
	return 1;
	
}

int queue(int fd, Arg *args)
{
	if(args->len == 0 || args->len >= 1024)
	{
		write(fd, "1105000000 \n", 12);
		freeArgs(args);
		return 1;
	}

	char head[12] = {'\0'};
	char res[200] = {'\0'};
	int arg_len = 0;

	char *qname = get_queue_name(args);
	if(strlen(qname) <= 0)
	{

		freeArgs(args);
		free(qname);
		write(fd, "1105000000 \n", 12);
		return 1;
	}
	
	void **r = (void **)calloc(1, sizeof(void *));
	int is_exist = hash_find(ht, qname, r);
	
	AOQ *aoq = (AOQ *)(*r);

	if(is_exist != 1 || aoq == NULL)
	{
		write(fd, "1105000000 \n", 12);
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
		
		sprintf(res, "0000000000 {\"TOTAL\":%d,\"HEAD_ID\":%d,\"TAIL_ID\":%d}\n", aoq->total, head_id, tail_id);
		arg_len = strlen(res);
		sprintf(head, "1106%06d ", arg_len-12);
		memcpy(res, head, 11);
		write(fd, res, arg_len);
	}
	free(qname);
	freeArgs(args);
	free(r);
	r = NULL;
	return 1;
}  

int delqueue(int fd, Arg *args)
{
	if(args->len == 0 || args->len >= 1024)
	{
		write(fd, "1106000001 0\n", 13);
		freeArgs(args);
		return 1;
	}

	char *qname = get_queue_name(args);
	if(strlen(qname) <= 0)
	{

		freeArgs(args);
		free(qname);
		write(fd, "1106000001 0\n", 13);
		return 1;
	}
	void **r = (void **)malloc(sizeof(void *));
	int is_exist = hash_find(ht, qname, r);
	freeArgs(args);
	AOQ *aoq = (AOQ *)(*r);

	if(is_exist != 1 || aoq == NULL)
	{
		write(fd, "1106000001 0\n", 13);
	}
	else
	{
		deleteQueue(aoq);
		hash_delete(ht, qname);
		write(fd, "1106000001 1\n", 13);
	}
	
	free(qname);
	free(r);
	r = NULL;
	return 1;

}  
