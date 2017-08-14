#include<stdio.h>

typedef struct _aoq_log
{
	char * log;
	int len;
	int size;
	FILE *fp_logfile;
}AOQLOG;

AOQLOG *aoqlog;


void initAoqLog()
{
	
	aoqlog = (AOQLOG *)malloc(sizeof(AOQLOG));
	aoqlog->size = 1024*1024*50;
	aoqlog->log = (char *)malloc((aoqlog->size)*sizeof(char));
	memset(aoqlog->log, '\0', aoqlog->size);
	aoqlog->len =0;
	char logfile[4096] = {'\0'};
	sprintf(logfile, "%s/aoq.log", Serv->work_dir_path);
	aoqlog->fp_logfile = fopen(logfile, "a");
}


void writeAoqLog(MemSlab *memslab)
{
	int len;
	ChunkNode *cnode = memslab->head->next;
	while(cnode != NULL && cnode->chunk != NULL)
	{

		len = strlen(cnode->chunk);
		if(aoqlog->len+len > aoqlog->size)
		{
			aoqlog->size = aoqlog->size*2;
			aoqlog->log = (char *)realloc(aoqlog->log, (aoqlog->size)*sizeof(char));
		}
		memcpy((char *)(aoqlog->log+aoqlog->len), cnode->chunk, len);
		aoqlog->len += len;
		cnode = cnode->next;
	}
	
}

int saveAoqLog(const int force)
{
	if(aoqlog->len == 0 || (force == 0 && aoqlog->len < 100000))
	{
		return 0;
	}
	else
	{
		if(aoqlog->fp_logfile == NULL)
		{
			char logfile[4096] = {'\0'};
			sprintf(logfile, "%s/aoq.log", Serv->work_dir_path);
			aoqlog->fp_logfile = fopen(logfile, "a");
		}
		
		int r = fprintf(aoqlog->fp_logfile, "%s", aoqlog->log);
		
		if(r < 0)
		{
			fclose(aoqlog->fp_logfile);
			char logfile[4096] = {'\0'};
			sprintf(logfile, "%s/aoq.log", Serv->work_dir_path);
			aoqlog->fp_logfile = fopen(logfile, "a");
			return -2;
		}
		
		memset(aoqlog->log, '\0', aoqlog->size);
		aoqlog->len = 0;
	}

    return 1;
}

void  handleAoqLog()
{
	
}


