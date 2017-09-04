#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _INCLUDE_MEMPOOL_H 
#define _INCLUDE_MEMPOOL_H 1  
#include "mempool.h"
#endif

#ifndef _INCLUDE_PARSER_H 
#define _INCLUDE_PARSER_H 1  
#include "parser.h"
#endif

#ifndef _INCLUDE_AOQ_H 
#define _INCLUDE_AOQ_H 1  
#include "aoq.h"
#endif

#ifndef _INCLUDE_SERVERSTAT_H 
#define _INCLUDE_SERVERSTAT_H 1  
#include "serverstat.h"
#endif

char response[1000010] = {'\0'};
char command[1000010] = {'\0'};

char * get_queue_name(Arg *args, char *qn);
int nil(int fd, Arg *args);
int status(int fd, Arg *args);
int push(int fd, Arg *args);
int pop(int fd, Arg *args);
int queues(int fd, Arg *args);
int queue(int fd, Arg *args);
int delqueue(int fd, Arg *args);

typedef int(*COMMANDFUNCPTR)(int fd, Arg *args);
COMMANDFUNCPTR commandFunc[] = {nil, status, push, pop, queues, queue, delqueue};