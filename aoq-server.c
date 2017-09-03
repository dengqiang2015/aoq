/*
 * AOQ by Deng Qiang
 * -- https://github.com/dengqiang2015/aoq
 * -- https://my.oschina.net/u/554660/blog
 *
 * Copyright (c) Deng qiang.

 * All rights reserved.
*/

#include "aoq-server.h"
#include "hashtable.c"
#include "mempool.c"
#include "aoq.c"
#include "parser.c"
#include "command.c"
#include "aoqlog.c"
#include "lsp.c"

static void show_help(void)
{
    const char *b = "--------------------------------------------------------------------------------------------------\n"
          "Simple Queue Service - AOQ v" AOQ_VERSION " (08, 2017)\n\n"
          "Author: Deng Qiang (https://my.oschina.net/u/554660/blog),E-mail: 962404383@qq.com\n"
          "This is free software, and you are welcome to modify and redistribute it under the New BSD License\n"
          "\n"
          "-p <num>      TCP port number to listen on (default: 8899)\n"
          "-q <num>      Maximum size of each queue (default: 2147483647)\n"
          "-m <num>      Maximum of memory,unit is KB(default: 1048576)\n"
          "-f <path>     The Path of work dir(default: /tmp)\n"
          "-t <num>      Maximum of queues(default: 1024)\n"
          "-s <num>      Size of mempool(default: 100000)\n"
          "-c <num>      Size of each chunk(default: 1024)\n"
          "-l <num>      Maximum of client connection(default: 65535)\n"
		  "-a            Automatically optimize memory pools\n"
		  "-x            Persistent storage\n"
		  "-r            Recover data from aof log file\n"
		  "-v            Show version of aoq\n"
          "-d            Run as a daemon\n"
          "-h            Print this help and exit\n\n"
          "Use command \"killall aoq-server\", \"pkill aoq-server\" and \"kill `cat /tmp/aoq.pid`\" to stop aoq-server.\n"
          "Please note that don't use the command \"pkill -9 aoq-server\" and \"kill -9 PID of aoq-server\"!\n"
          "\n"
          "Please visit \"https://my.oschina.net/u/554660/blog\" for more help information.\n\n"
          "--------------------------------------------------------------------------------------------------\n"
          "\n";
    fprintf(stderr, b, strlen(b));
}


static void init_daemon(char *path)   
{   
    int pid;   
    int i;   
    if(pid=fork())
    {
        exit(0);
    }       
    else if(pid< 0)
    {
		serverlog("Failed to create daemon.");
        exit(1);
    }  
    
    setsid();
    if(pid=fork())
    {
        exit(0);
    }       
    else if(pid< 0)
    {
		serverlog("Failed to create daemon.");
        exit(1);
    }  
     
    for(i=0;i< NOFILE;++i)
    {
        close(i); 
    }
          
    chdir(path); 
    umask(0);
    return;   
} 


int mkpidfile(int pid, char *pid_file)
{
    FILE *fp_pidfile;
    fp_pidfile = fopen(pid_file, "w");
    if(fp_pidfile == NULL)
    {
        return -1;
    }
    int r = fprintf(fp_pidfile, "%d\n", pid);
    if(r < 0)
    {
        fclose(fp_pidfile);
        return -2;
    }
    fclose(fp_pidfile);
    return 1;
}

int checkpidfile(char *pid_file)
{
    if(!access(pid_file, 0))
    {
        return 1;
    }       
    return 0;
}


static void accept_cb(int fd, short events, void* arg)
{
    
    evutil_socket_t sockfd;

    struct sockaddr_in client;
    socklen_t len;
    
    sockfd = accept(fd, (struct sockaddr*)&client, &len );
    evutil_make_socket_nonblocking(sockfd);

    if(Serv->client_connection < Serv->max_client_connection){
        
        Serv->client_connection++;
        
        struct event_base *base = (event_base*)arg;

        struct event *ev = event_new(NULL, -1, 0, NULL, NULL);

        event_assign(ev, base, sockfd, EV_READ | EV_PERSIST, socket_read_cb, (void *)ev);

        event_add(ev, NULL);
    }
    else
    {
        serverlog("Connection error, exceeding maximum number of connections.");
        return; 
    }
    
}

static void socket_read_cb(int fd, short events, void *arg)
{

    ChunkNode *node = mpalloc();
    int len = read(fd, node->chunk, CK_SIZE-1);
    int r = 0;

    if( len <= 0 )
    {
        mpfree(node);
        struct event *ev = (struct event*)arg;

        Serv->client_connection--;
        close(event_get_fd(ev));
        event_free(ev);
        return ;
    }
    
    MemSlab *memslab = createMemSlab(1);
    insertMemSlab(memslab, node);
    int command_num = 0;


    while(*(node->chunk+len-1) != '\n')
    {
        node = mpalloc();
        len = read(fd, node->chunk, CK_SIZE-1);
        insertMemSlab(memslab, node);
    }
    
    Arg *args = createArgs(5);

    r = parse_args(memslab, &command_num, args);


    if(r >= 0)
    {
		if(r > 0 && Serv->persistent == 1)
		{
			writeAoqLog(memslab);
		}
        (*commandFunc[command_num])(fd, args);
    }
    else
    {
        freeArgs(args);
    }

}

typedef struct sockaddr SA;
int tcp_server_init(int port, int listen_num)
{
    int errno_save;
    evutil_socket_t listener;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if( listener == -1 )
	{
		serverlog("Create socket error.");
        return -1;
	}

	const char flag = 1;
    setsockopt(listener, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
	
    evutil_make_listen_socket_reuseable(listener);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);

    if( bind(listener, (SA*)&sin, sizeof(sin)) < 0 )
	{
		serverlog("Bind socket error.");
        goto error;
	}
		

    if( listen(listener, listen_num) < 0)
	{
		serverlog("Listen socket error.");
        goto error;
	}
		
    evutil_make_socket_nonblocking(listener);

    return listener;

    error:
        errno_save = errno;
        evutil_closesocket(listener);
        errno = errno_save;

        return -1;
}

static void kill_signal(int sig) 
{
	if(Serv->persistent == 1)
	{
		int r = saveAoqLog(1);
		fclose(aoqlog->fp_logfile);
	}
	
    if(checkpidfile(Serv->pid_file) == 1 ) {
        remove(Serv->pid_file);
    }
	
	serverlog("Aoq server stop.");
    exit(0);
}

static void event_savelog_cb(evutil_socket_t fd, short event, void *arg)
{

	int r = saveAoqLog(0);
    struct event *savelog;
    savelog = (struct event *)arg;
    struct timeval savelog_tv;
    evutil_timerclear(&savelog_tv);
    savelog_tv.tv_sec = 1;
    savelog_tv.tv_usec = 0;
    evtimer_add(savelog, &savelog_tv);
    return;
}

static void createParamDataFile()
{
	
	char datafile[1024] = {'\0'};
	sprintf(datafile, "%s/%s", Serv->work_dir_path, AOQ_DATA_FILE_NAME);
	FILE *fp_datafile = fopen(datafile, "wb");
	if(fp_datafile != NULL)
	{
		fputc('\0', fp_datafile);
		fclose(fp_datafile);
	}
	return;
}

void saveParamtersData()
{


	char datafile[1024] = {'\0'};
	sprintf(datafile, "%s/%s", Serv->work_dir_path, AOQ_DATA_FILE_NAME);
	int X[21];
	int Y[21];
	int i = 0;

	for(i=0;i<21;i++)
	{
		X[i] = i;
		Y[i] = Serv->chunk_used_num;
	}

	FILE *fp_datafile = fopen(datafile, "rb");
	if (!fp_datafile)
	{
		return;
	}
	
	i=0;
	while(!feof(fp_datafile) && i<20)
	{
		if (2 != fscanf(fp_datafile, (const char*)"%d %d", &X[i], &Y[i]))
		{ 
			break;
		}
		i++;
	}
	
	fclose(fp_datafile);
	fp_datafile = fopen(datafile, "wb");
	
	for(i=1;i<21;i++)
	{
		fprintf(fp_datafile, "%d %d\n", i, Y[i]);
	}
	fclose(fp_datafile);
    return;

}


static void event_calculate_cb(evutil_socket_t fd, short event, void *arg)
{

	double X = 21;
	int num;
	saveParamtersData();
	num = (int)ForecastY(X);
	num = num ? num : Serv->chunk_used_num;
	tuneMempool(num);
    struct event *calculate;
    calculate = (struct event *)arg;
    struct timeval calculate_tv;
    evutil_timerclear(&calculate_tv);
    calculate_tv.tv_sec = 10;
    calculate_tv.tv_usec = 0;
    evtimer_add(calculate, &calculate_tv);

} 


int aoq_start()
{
    int listener = tcp_server_init(Serv->port, 10);
    if( listener == -1 )
    {
        serverlog("Tcp server init error.");
        return -1;
    }
    
    
    struct event_base *base = event_base_new();

    struct event *ev_listen = event_new(base, listener, EV_READ | EV_PERSIST, accept_cb, base);
    event_add(ev_listen, NULL);

	if(Serv->persistent == 1)
	{
		struct event savelog;
		struct timeval savelog_tv;
		
		evtimer_assign(&savelog, base, event_savelog_cb, (void*)&savelog);
		//evtimer_new(base, event_test_cb, (void*)&timeout);
		evutil_timerclear(&savelog_tv);
		savelog_tv.tv_sec = 1;
		savelog_tv.tv_usec = 0;
		evtimer_add(&savelog, &savelog_tv);
	}
	
	if(Serv->aomp == 1)
	{
		struct event calculate;
		struct timeval calculate_tv;
		
		evtimer_assign(&calculate, base, event_calculate_cb, (void*)&calculate);
		//evtimer_new(base, event_test_cb, (void*)&timeout);
		evutil_timerclear(&calculate_tv);
		calculate_tv.tv_sec = 1;
		calculate_tv.tv_usec = 0;
		evtimer_add(&calculate, &calculate_tv);
	}	
	
    event_base_dispatch(base);
    return 1;
}


int main(int argc, char** argv)
{
    int opt = 0;
    int port = 0;
    long aoq_max_size = 0;
    int max_memory = 0;
    int ht_size = 0;
    int mempool_size = 0;
    int chunk_size = 0;
    int daemon = 0;
    int max_client_connection = 0;
    char *work_dir_path = NULL;
    char *pid_file = NULL;
	int persistent = 0;
	int recover = 0;
	int aomp = 0;
	
    while ((opt = getopt(argc, argv, "p:q:m:f:t:s:c:l:axrvdh")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
            break;
            case 'q':
                aoq_max_size = atoi(optarg);
            break;
            case 'm':
                max_memory = atoi(optarg);
            break;
            case 'f':
                work_dir_path = (char *)optarg;
            break;
            case 't':
                ht_size = atoi(optarg);
            break;
            case 's':
                mempool_size = atoi(optarg);
            break;
            case 'c':
                chunk_size = atoi(optarg);
            break;
            case 'l':
                max_client_connection = atoi(optarg);
            break;
			case 'a':
                aomp = 1;
            break;
			case 'x':
                persistent = 1;
            break;
			case 'r':
                recover = 1;
            break;
            case 'v':
                printf("%s\n", AOQ_VERSION);
                return 1;
            break;
            case 'd':
                daemon = 1;
            break;
            case 'h':
            default:
                show_help();
                return 1;
            break;
              
        }
    }

    Serv = (ServerStat *)malloc(sizeof(ServerStat));
        
	if(work_dir_path == NULL)
	{
		int wplen = strlen(WORK_DIR_PATH);
		work_dir_path = (char *)malloc((wplen+1)*sizeof(char));
		memset(work_dir_path, '\0', wplen+1);
		memcpy(work_dir_path, WORK_DIR_PATH, wplen);
	}
	
	if(daemon == 1)
	{
	   
		int wplen = strlen(work_dir_path);
		pid_file = (char *)malloc((wplen+9)*sizeof(char));
		memset(pid_file, '\0', wplen+9);
		sprintf(pid_file, "%s/%s", work_dir_path, AOQ_PID_FILE_NAME);

        if(checkpidfile(pid_file) == 1 ) {
            printf("%s is exist!\n", pid_file);
            return 1;
        }
        
        init_daemon(work_dir_path);
        Serv->pid = getpid();
        int r = mkpidfile(Serv->pid, pid_file);
        if(r < 0)
        {
            free(Serv);
            Serv = NULL;        
            serverlog("Make pid file error.");
            return -1;
        }
        Serv->pid_file = pid_file;
      
    }
    else
    {
        Serv->pid = getpid();
    }
    
    time_t now;
    struct tm *tm_now;
    time(&now);
    tm_now = localtime(&now);
    Serv->start_time = (char *)malloc(20*sizeof(char));
    memset(Serv->start_time, '\0', 20);
    strftime(Serv->start_time, 20, "%Y-%m-%d %H:%M:%S", tm_now);
    
    ht = (HashTable *)malloc(sizeof(HashTable));
    ht_size = ht_size > 0 ? ht_size : HTSIZE;
    hash_init(&ht, ht_size);
    
    mempool_size = mempool_size > 0 ? mempool_size : MP_SIZE;
    chunk_size = chunk_size > 0 ? chunk_size : CK_SIZE;
    mp = (MemPool *)malloc(sizeof(MemPool));
    initMemPool(mp, mempool_size, chunk_size);
    createMemPool(mp);

    Serv->host = DEFAULT_HOST;
    Serv->port = port > 0 ? port : DEFAULT_PORT;
    Serv->ht = ht;
    Serv->mp = mp;
    Serv->max_memory = max_memory > 0 ? max_memory : MAX_MEMORY;
    Serv->max_client_connection = max_client_connection > 0 ? max_client_connection : MAX_CLIENT_CONNECTION;
    Serv->client_connection = 0;
    Serv->aoq_max_size = aoq_max_size > 0 ? aoq_max_size : AOQ_MAX_SIZE;
	Serv->work_dir_path = work_dir_path;
	Serv->persistent = persistent;
	Serv->chunk_used_num = 0;
	Serv->aomp = aomp;
    
	if(recover == 1)
	{
		recovery_from_aoflog();
	}
	
	if(persistent == 1)
	{
		initAoqLog();
	}
	
	if(aomp == 1)
	{
		createParamDataFile();
	}
	
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, kill_signal);
    signal(SIGKILL, kill_signal);
    signal(SIGQUIT, kill_signal);
    signal(SIGTERM, kill_signal);
    signal(SIGHUP, kill_signal);

	serverlog("Aoq server start.");
    aoq_start();

    return 0;
}

