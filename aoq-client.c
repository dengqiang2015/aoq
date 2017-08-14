#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <getopt.h>

int tcp_connect_server(const char *server_ip, int port);

void cmd_msg_cb(int fd, short events, void *arg);

void server_msg_cb(bufferevent *bev, void *arg);

void event_cb(bufferevent *bev, short event, void *arg);


static void
show_help(void)
{
    const char *b = "--------------------------------------------------------------------------------------------------\n"
          "Simple Queue Client - (08, 2017)\n\n"
          "Author: Deng Qiang (https://my.oschina.net/u/554660/blog), E-mail: 962404383@qq.com\n"
          "This is free software, and you are welcome to modify and redistribute it under the New BSD License\n"
          "\n"
           "-h <host>(host addr)\n"
           "-p <port>(listen on host's port)\n"
           "\n"
           "Please visit \"https://my.oschina.net/u/554660/blog\" for more help information.\n\n"
           "--------------------------------------------------------------------------------------------------\n"
           "\n";
    fprintf(stderr, b, strlen(b));
}


static void
show_command_help(void)
{
    const char *b = "--------------------------------------------------------------------------------------------------\n"
          "Simple Queue Client - (08, 2017)\n\n"
          "Author: Deng Qiang (https://my.oschina.net/u/554660/blog), E-mail: 962404383@qq.com\n"
          "This is free software, and you are welcome to modify and redistribute it under the New BSD License\n"
          "\n"
           "command list:\n"
           "status(show status of server)\n"
           "push <queue_name> <data>(push data into queue)\n"
           "pop <qnene_name>(pop data from queue)\n"
           "queues(show name of all queues)\n"
           "queue <queue_name>(show status of queue_name)\n"
           "\n"
           "Please visit \"https://my.oschina.net/u/554660/blog\" for more help information.\n\n"
           "--------------------------------------------------------------------------------------------------\n"
           "\n";
    fprintf(stderr, b, strlen(b));
}

long command_name2num(char *name)
{
    long num = 0;
    while(*name != '\0')
    {
        num = num*10+(int)(*name++);
    }
    return num;
}

char * str2lower(char *command_str)
{
    char *p = command_str;
    while(*p != '\0')
    {
        *p = tolower(*p);
        p++;
    }
    return command_str;
}

int protocol_convert(char **send_data, char *msg, int *send_data_len)
{
    int i,j=0;
    int len = strlen(msg);
    char *s;
    char *m = msg;

    while(i<len && (*m) != ' ' && (*m) != '\n') 
    {   
        m++;
        i++;
    }

    char *command_str = (char *)malloc((i+1)*sizeof(char));
    memset(command_str, '\0', i+1);
    memcpy(command_str, msg, i);

    command_str = str2lower(command_str);

    long command_num = command_name2num(command_str);
    
    m++;
    i++;

    switch (command_num) 
    {
        
        case 115292://help
            show_command_help();
        break;
        
        case 12769885://status
            s = (char *)calloc(6, sizeof(char));
            *send_data = s;
            *send_data_len = 6;
            sprintf(s, "%s", "0501 \n");
            
        break;
        
        
        case 124954://push
            
            *send_data_len = len+18;
            s = (char *)calloc((len+18), sizeof(char));
            *send_data = s;
    
            while(i<len && (*m) != ' ')
            {
                m++;
                i++;
                j++;
            }
            
            if((len-i-2) <=0)
            {
                printf("%s parameter error\n", command_str);
                return -1;
            }
        
            sprintf(s, "1702%06d%06d ", j, len-i-2);
            s += 17;
            memcpy(s, m-j, j);
            s += j;
            m++;
            memcpy(s, m, len-i-1);
			*(s+len-i-1) = '\n';
        break;
        
        case 12422://pop
            *send_data_len = len+12;
            s = (char *)calloc((len+12), sizeof(char));
            *send_data = s;
    
            while(i<len && (*m) != '\0')
            {
                m++;
                i++;
                j++;
            }
            
            if((j-1) <=0)
            {
                printf("%s parameter error\n", command_str);
                return -1;
            }
        
            sprintf(s, "1103%06d ", j-1);
            s += 11;
            memcpy(s, m-j, j-1);
            *(s+j-1) = '\n';
        break;
        
        case 12583825://queues
            s = (char *)calloc(6, sizeof(char));
            *send_data = s;
            *send_data_len = 6;
            sprintf(s, "%s", "0504 ");
        break;
        
        case 1258371://queue
            *send_data_len = len+12;
            s = (char *)calloc((len+12), sizeof(char));
            *send_data = s;
    
            while(i<len && (*m) != '\0')
            {
                m++;
                i++;
                j++;
            }
            
            if((j-1) <=0)
            {
                printf("%s parameter error\n", command_str);
                return -1;
            }
        
            sprintf(s, "1105%06d ", j-1);
            s += 11;
            memcpy(s, m-j, j-1);
            *(s+j-1) = '\n';
        break;
        
        case 1113058371://delqueue
            *send_data_len = len+12;
            s = (char *)calloc((len+12), sizeof(char));
            *send_data = s;
    
            while(i<len && (*m) != '\0')
            {
                m++;
                i++;
                j++;
            }
            
            if((j-1) <=0)
            {
                printf("%s parameter error\n", command_str);
                return -1;
            }
        
            sprintf(s, "1006%06d ", j-1);
            s += 11;
            memcpy(s, m-j, j-1);
            *(s+j-1) = '\n';
        break;
        
        case 125866://quit
            exit(0);
        break;
        
        default:
            printf("unknow command %s\n", command_str); 
            return -1;
        break;
    }
    return 1;
}


int main(int argc, char **argv) {
    
    char opt;
    char *host;
    int port = 8899;
    while ((opt = getopt(argc, argv, "p:q:m:f:t:s:c:l:dh")) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
            break;
            case 'p':
                port = atoi(optarg);
            break;
            default:
                show_help();
                return 1;
            break;
              
        }
    }
    
    if (host == NULL) {
      host = (char *)"0.0.0.0";
    }

    event_base *base = event_base_new();

    bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    event *ev_cmd = event_new(base, STDIN_FILENO,
                  EV_READ|EV_PERSIST,
                  cmd_msg_cb, (void *)bev);

    event_add(ev_cmd, NULL);

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_aton(host, &server_addr.sin_addr);

    bufferevent_socket_connect(bev, (sockaddr *)&server_addr, sizeof(server_addr));

    bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, (void *)ev_cmd);
    bufferevent_enable(bev, EV_READ|EV_PERSIST);

    event_base_dispatch(base);

    printf("server disconnected\n");
    return 0;

}

void cmd_msg_cb(int fd, short events, void *arg) {
    
    char msg[1000000]= {'\0'};
    char **send_data = (char **)malloc(sizeof(char *));
    int send_data_len = 0;
    int ret = read(fd, msg, 1000000-2);
    if (ret < 0) {
        perror("read fail.\n");
        exit(1);
    }
    int c = protocol_convert(send_data, msg, &send_data_len);
    if(c > 0)
    {
        bufferevent *bev = (bufferevent *)arg;
        bufferevent_write(bev, *send_data, send_data_len);
    }

}

int parse_head_len(char *msg)
{
    char *head_len_str = (char *)malloc(2*sizeof(char));
    int head_len = 0;
    memcpy(head_len_str, msg, 2);
    head_len = atoi(head_len_str);
    free(head_len_str);
    return head_len;
}

void server_msg_cb(bufferevent *bev, void *arg) {
    char msg[1024] = {'\0'};
    int head_len = 0;
    size_t len = bufferevent_read(bev, msg, 1024);
    head_len = parse_head_len(msg);
    
    printf("%s", &msg[head_len]);
    while(len == 1024)
    {
        len = bufferevent_read(bev, msg, 1024);
        printf("%s", msg);
    }

}

void event_cb(bufferevent *bev, short event, void *arg) {
    if (event & BEV_EVENT_EOF) {
        printf("Connection closed.\n");
    }
    else if (event & BEV_EVENT_ERROR) {
        printf("Some other error:%s\n",  evutil_socket_error_to_string(111));
    }
    else if (event & BEV_EVENT_CONNECTED) {
        //printf("Client has successfully cliented.\n");
        return;
    }

    bufferevent_free(bev);

    // free event_cmd
    // need struct as event is defined as parameter
    struct event *ev = (struct event *)arg;
    event_free(ev);
}
