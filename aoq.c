#ifndef _INCLUDE_AOQ_H 
#define _INCLUDE_AOQ_H 1  
#include "aoq.h"
#endif

AOQ * createQueue(char *qname)
{
	AOQ *aoq = (AOQ *)malloc(sizeof(AOQ));
	aoq->name = qname;
	aoq->total = 0;
	aoq->head = (Qnode *)malloc(sizeof(Qnode));
	aoq->tail = (Qnode *)malloc(sizeof(Qnode));
	aoq->head->uniqid = 0;
	aoq->tail->uniqid = 0;
	aoq->head->arg = NULL;
	aoq->tail->arg = NULL;
	aoq->head->next = NULL;
	aoq->tail->next = NULL;

	return aoq;
}

int pushQueue(AOQ * aoq, Qnode *qnode)
{
	qnode->next = NULL;
	if(aoq->total == 0)
	{
		qnode->uniqid = 1;
		aoq->head->next = qnode;
		aoq->tail->next = qnode;
	}
	else
	{
		qnode->uniqid = aoq->tail->next->uniqid+1;
		aoq->tail->next->next = qnode;
		aoq->tail->next = qnode;
	}


	aoq->total++;

	return 1;
}

int popQueue(AOQ *aoq, Qnode **qnode)
{

	if(aoq->total == 0 || aoq->head->next == NULL)
	{
		aoq->head->next == NULL;
		aoq->tail->next == NULL;
		(*qnode) = NULL;
		return -1;
	}
	else
	{
		(*qnode) = aoq->head->next;
		if(aoq->head->next->next == NULL)
		{
			aoq->head->next = NULL;
			aoq->tail->next = NULL;
		}
		else
		{
			aoq->head->next = aoq->head->next->next;
		}
		
		(*qnode)->next = NULL;
		aoq->total--;
	}
	return 1;
}

int deleteQueue(AOQ *aoq)
{

	Qnode *current;

	if(aoq == NULL)
	{
		return 0;
	}
	
	if(aoq->head != NULL)
	{
		while(aoq->head->next != NULL)
		{
			current = aoq->head->next;
			freeArgs(current->arg);
			if(current->next != NULL)
			{
				aoq->head->next = current->next;
				free(current);
			}
			else
			{
				free(current);
				break;
			}

				
		}
		current = NULL;
		free(aoq->head);
		free(aoq->tail);
		aoq->head = NULL;
		aoq->tail = NULL;
	}
	
	free(aoq);
	aoq = NULL;
	
	return 1;
}


AOQ * getQueue(char *qname, HashTable *ht)
{
	void **result = (void **)calloc(1, sizeof(void *));
	hash_find(ht, qname, result);
	if(*result == NULL)
	{
		AOQ *aoq = createQueue(qname);
		hash_insert(ht, qname, aoq);
		return aoq;
	}
	return (AOQ *)(*result);
}

Qnode * createQnode(Arg *arg)
{
	Qnode *qnode = (Qnode *)malloc(sizeof(Qnode));
	qnode->uniqid = 0;
	qnode->arg = arg;
	qnode->next = NULL;
	return qnode;
}


