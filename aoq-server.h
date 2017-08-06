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
#include<sys/time.h>
#include<getopt.h>
#include "debug.h"

#ifndef _INCLUDE_SERVERSTAT_H 
#define _INCLUDE_SERVERSTAT_H 1  
#include "serverstat.h"
#endif

#define AOQ_VERSION "1.0.0 beta"
#define HTSIZE 1024
#define AOQ_MAX_SIZE 2147483647
#define CKSIZE 1024
#define MPSIZE 100000
#define MAX_CLIENT_CONNECTION 65535
#define MAX_MEMORY 1048576//kb
#define WORK_DIR_PATH "/tmp"
#define PID_FILE "/tmp/aoq.pid"

void show_help(void);
void init_daemon(char *path);
int mkpidfile(int pid);
int checkpidfile();   
void accept_cb(int fd, short events, void* arg);
void socket_read_cb(int fd, short events, void *arg);
int tcp_server_init(int port, int listen_num);
static void kill_signal(int sig);
int aoq_start();
   
