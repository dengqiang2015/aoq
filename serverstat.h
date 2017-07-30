#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <assert.h>

#ifndef _INCLUDE_MEMPOOL_H 
#define _INCLUDE_MEMPOOL_H 1  
#include "mempool.h"
#endif

#ifndef _INCLUDE_HASHTABLE_H 
#define _INCLUDE_HASHTABLE_H 1  
#include "hashtable.h"
#endif

#define VMRSS_LINE 21//VMRSS������, ע:���ݲ�ͬ��ϵͳ,λ�ÿ�����������.
#define pid_t  int
#define MAX_CLIENT_CONNECTION 65535

typedef struct _server_stat
{
	int pid;
	char *start_time;
	const char *host;
	int port;
	int client_connection;
	HashTable *ht;
	MemPool *mp;
}ServerStat;

ServerStat *Serv;

int get_phy_mem(const pid_t p)
{
    char file[64] = {0};//�ļ���
    FILE *fd;         //�����ļ�ָ��fd
    char line_buff[256] = {0};  //��ȡ�еĻ�����
    sprintf(file,"/proc/%d/status",p);

    fd = fopen (file, "r"); //��R���ķ�ʽ���ļ��ٸ���ָ��fd

    //��ȡvmrss:ʵ�������ڴ�ռ��
    int i;
    char name[32];//�����Ŀ����
    int vmrss;//����ڴ�
    //��ȡVmRSS��һ�е�����
    for (i=0;i<VMRSS_LINE-1;i++)
    {
        char* ret = fgets (line_buff, sizeof(line_buff), fd);
    }
    char* ret1  = fgets (line_buff, sizeof(line_buff), fd);
    sscanf (line_buff, "%s %d", name,&vmrss);
    fclose(fd);     //�ر��ļ�fd
    return vmrss;
}


int get_rmem(pid_t p)
{
    return get_phy_mem(p);
}


int get_total_mem()
{
    const char* file = "/proc/meminfo";//�ļ���
    FILE *fd;         //�����ļ�ָ��fd
    char line_buff[256] = {0};  //��ȡ�еĻ�����
    fd = fopen (file, "r"); //��R���ķ�ʽ���ļ��ٸ���ָ��fd

    //��ȡmemtotal:���ڴ�ռ�ô�С
    int i;
    char name[32];//�����Ŀ����
    int memtotal;//����ڴ��ֵ��С
    char*ret = fgets (line_buff, sizeof(line_buff), fd);//��ȡmemtotal��һ�е�����,memtotal�ڵ�1��
    sscanf (line_buff, "%s %d", name,&memtotal);
    fclose(fd);     //�ر��ļ�fd
    return memtotal;
}