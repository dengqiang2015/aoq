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

void show_help(void)
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
          "-s <num>      size of mempool(default: 100000)\n"
          "-c <num>      size of each chunk(default: 1024)\n"
          "-l <num>      Maximum of client connection(default: 65535)\n"
          "-d            run as a daemon\n"
          "-h            print this help and exit\n\n"
          "Use command \"killall aoq-server\", \"pkill aoq-server\" and \"kill `cat /tmp/aoq.pid`\" to stop aoq-server.\n"
          "Please note that don't use the command \"pkill -9 aoq-server\" and \"kill -9 PID of aoq-server\"!\n"
          "\n"
          "Please visit \"https://my.oschina.net/u/554660/blog\" for more help information.\n\n"
          "--------------------------------------------------------------------------------------------------\n"
          "\n";
    fprintf(stderr, b, strlen(b));
}

void init_daemon(char *path)   
{   
    int pid;   
    int i;   
    if(pid=fork())
    {
        exit(0);
    }       
    else if(pid< 0)
    {
        exit(1);
    }  
    
    setsid();
    if(pid=fork())
    {
        exit(0);
    }       
    else if(pid< 0)
    {
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


void accept_cb(int fd, short events, void* arg)
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
        perror(" Connection error, exceeding maximum number of connections! ");
        return; 
    }
    
}

void socket_read_cb(int fd, short events, void *arg)
{
    ml =0;
    fr = 0;
    ChunkNode *node = mpalloc();
    int len = read(fd, node->chunk, CKSIZE-1);
    int r = 0;

    if( len <=0 )
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


    while(len == CKSIZE-1)
    {
        node = mpalloc();
        len = read(fd, node->chunk, CKSIZE-1);
        insertMemSlab(memslab, node);
    }
    

    Arg *args = (Arg *)malloc(5*sizeof(Arg));
    Arg *a = args;
    for(r=0;r<5;r++)
    {
        a->cursor = NULL;
        a->len = 0;
        a++;
    }

    r = parse_args(memslab, &command_num, args);


    if(r > 0)
    {
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
        return -1;

    evutil_make_listen_socket_reuseable(listener);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);

    if( bind(listener, (SA*)&sin, sizeof(sin)) < 0 )
    
        goto error;

    if( listen(listener, listen_num) < 0)
        goto error;


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
    if(checkpidfile(Serv->pid_file) == 1 ) {
        remove(Serv->pid_file);
    }
    exit(0);
}

int aoq_start()
{
    int listener = tcp_server_init(Serv->port, 10);
    if( listener == -1 )
    {
        perror(" tcp_server_init error ");
        return -1;
    }
    
    struct event_base *base = event_base_new();

    struct event *ev_listen = event_new(base, listener, EV_READ | EV_PERSIST, accept_cb, base);
    event_add(ev_listen, NULL);

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
    char *work_dir_path;
    char *pid_file;

    while ((opt = getopt(argc, argv, "p:q:m:f:t:s:c:l:vdh")) != -1) {
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
                work_dir_path = optarg;
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
        

    if(daemon == 1)
    {
        if(work_dir_path == NULL)
        {
            int wplen = strlen(WORK_DIR_PATH);
            work_dir_path = (char *)malloc((wplen+1)*sizeof(char));
            memset(work_dir_path, '\0', wplen+1);
            memcpy(work_dir_path, WORK_DIR_PATH, wplen);
            
            int pflen = strlen(PID_FILE);
            pid_file = (char *)calloc(pflen, sizeof(char));
            memcpy(pid_file, PID_FILE, pflen);
        }
        else
        {
            int wplen = strlen(work_dir_path);
            pid_file = (char *)malloc((wplen+9)*sizeof(char));
            memset(pid_file, '\0', wplen+9);
            sprintf(pid_file, "%s/aoq.pid", work_dir_path);
        }
        
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
                        
            perror("make pid file error");
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
    
    mempool_size = mempool_size > 0 ? mempool_size : MPSIZE;
    chunk_size = chunk_size > 0 ? chunk_size : CKSIZE;
    mp = (MemPool *)malloc(sizeof(MemPool));
    initMemPool(mp, mempool_size, chunk_size);
    createMemPool(mp);

    Serv->host = "0.0.0.0";
    Serv->port = port > 0 ? port : 8899;
    Serv->ht = ht;
    Serv->mp = mp;
    Serv->max_memory = max_memory > 0 ? max_memory : MAX_MEMORY;
    Serv->max_client_connection = max_client_connection > 0 ? max_client_connection : MAX_CLIENT_CONNECTION;
    Serv->client_connection = 0;
    Serv->aoq_max_size = aoq_max_size > 0 ? aoq_max_size : AOQ_MAX_SIZE;
    
    
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, kill_signal);
    signal(SIGKILL, kill_signal);
    signal(SIGQUIT, kill_signal);
    signal(SIGTERM, kill_signal);
    signal(SIGHUP, kill_signal);

    aoq_start();

    return 0;
}

