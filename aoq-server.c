#include "aoq-server.h"
#include "hashtable.c"
#include "mempool.c"
#include "aoq.c"
#include "parser.c"
#include "command.c"

 
void init_daemon(void)   
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
		  
	chdir("/tmp"); 
	umask(0);
	return;   
} 


int mkpidfile(int pid)
{
	FILE *fp_pidfile;
	fp_pidfile = fopen("/tmp/aoq.pid", "w");
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

int checkpidfile()
{
	if(!access("/tmp/aoq.pid",0))
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


	Serv->client_connection++;

	struct event_base *base = (event_base*)arg;

	struct event *ev = event_new(NULL, -1, 0, NULL, NULL);

	event_assign(ev, base, sockfd, EV_READ | EV_PERSIST,
				 socket_read_cb, (void *)ev);

	event_add(ev, NULL);
			
}

void socket_read_cb(int fd, short events, void *arg)
{


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
	
	if(len <=0 )
	{
		freeMemSlab(memslab);
		struct event *ev = (struct event*)arg;
		Serv->client_connection--;
		close(event_get_fd(ev));
		event_free(ev);
		return ;
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
	if(checkpidfile() == 1 ) {
		remove("/tmp/aoq.pid");
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

	if( checkpidfile() == 1 ) {
		printf("/tmp/aoq.pid is exist!\n");
		exit(0);
	}

	init_daemon();

	time_t now;
	struct tm *tm_now;
	time(&now);
    tm_now = localtime(&now);
    ht = (HashTable *)malloc(sizeof(HashTable));
    hash_init(&ht, HTSIZE);
	mp = (MemPool *)malloc(sizeof(MemPool));
	initMemPool(mp, MPSIZE, CKSIZE);
	createMemPool(mp);
	
	Serv = (ServerStat *)malloc(sizeof(ServerStat));
	Serv->pid = getpid();
	Serv->start_time = (char *)malloc(20*sizeof(char));
	memset(Serv->start_time, '\0', 20);
    strftime(Serv->start_time, 20, "%Y-%m-%d %H:%M:%S", tm_now);
	Serv->host = "0.0.0.0";
	Serv->port = 8899;
	Serv->ht = ht;
	Serv->mp = mp;
	Serv->client_connection = 0;
	
	int r = mkpidfile(Serv->pid);
	if(r < 0)
	{
		perror("make pid file error");
		return -1;
	}
	
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, kill_signal);
	signal(SIGKILL, kill_signal);
	signal(SIGQUIT, kill_signal);
	signal(SIGTERM, kill_signal);
	signal(SIGHUP, kill_signal);
	
	aoq_start();

	return 0;
}

