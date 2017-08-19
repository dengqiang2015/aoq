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
#include<zlib.h>
#include "debug.h"

#ifndef _INCLUDE_SERVERSTAT_H 
#define _INCLUDE_SERVERSTAT_H 1  
#include "serverstat.h"
#endif

#define AOQ_VERSION "1.0.0 beta"
#define DEFAULT_HOST "0.0.0.0"
#define DEFAULT_PORT 5211
#define HTSIZE 1024
#define AOQ_MAX_SIZE 2147483647
#define CK_SIZE 1024
#define MP_SIZE 100000
#define MP_RESERVE_SIZE 10000
#define MAX_CLIENT_CONNECTION 65535
#define MAX_MEMORY 1048576//kb
#define WORK_DIR_PATH "/tmp"
#define AOQ_PID_FILE_NAME "aoq.pid"
#define AOQ_DATA_FILE_NAME "parameter.dat"


static void show_help(void);
static void init_daemon(char *path);
int mkpidfile(int pid);
int checkpidfile();   
static void accept_cb(int fd, short events, void* arg);
static void socket_read_cb(int fd, short events, void *arg);
int tcp_server_init(int port, int listen_num);
static void kill_signal(int sig);
int aoq_start();
static void recovery_from_aoflog();
static void execute_command_line(char *line);
int readLogLine(char *line, char *chunk, int size);   
static void createParamDataFile();
void saveParamtersData();
static void event_calculate_cb(evutil_socket_t fd, short event, void *arg);