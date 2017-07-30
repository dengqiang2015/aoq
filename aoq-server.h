#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<event.h>
#include<time.h>
#include<sys/ioctl.h>
#include<assert.h>
#include<signal.h>
#include<sys/param.h>   
#include<sys/types.h>   
#include<sys/stat.h>
#include "debug.h"

#ifndef _INCLUDE_SERVERSTAT_H 
#define _INCLUDE_SERVERSTAT_H 1  
#include "serverstat.h"
#endif  

void init_daemon(void);
int mkpidfile(int pid);
int checkpidfile();   
void accept_cb(int fd, short events, void* arg);
void socket_read_cb(int fd, short events, void *arg);
int tcp_server_init(int port, int listen_num);
static void kill_signal(int sig);
int aoq_start();
   
