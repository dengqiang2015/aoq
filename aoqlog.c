#include<stdio.h>

typedef struct _aoq_log
{
	char * log;
	int len;
	int size;
}AOQLOG;

AOQLOG *aoqlog;

void initAoqLog()
{
	
	aoqlog = (AOQLOG *)malloc(sizeof(AOQLOG));
	aoqlog->size = 1024*1024*100;
	aoqlog->log = (char *)malloc((aoqlog->size)*sizeof(char));
	memset(aoqlog->log, '\0', aoqlog->size);
	aoqlog->len =0;
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

int saveAoqLog()
{
	if(aoqlog->len == 0)
	{
		return 0;
	}
	const char *logfile = "/tmp/aoq.log";
	FILE *fp_logfile;
    fp_logfile = fopen(logfile, "a");
    if(fp_logfile == NULL)
    {
        return -1;
    }
    int r = fprintf(fp_logfile, "%s", aoqlog->log);
    if(r < 0)
    {
        fclose(fp_logfile);
        return -2;
    }
    fclose(fp_logfile);
	memset(aoqlog->log, '\0', aoqlog->size);
	aoqlog->len = 0;
    return 1;
}

void  handleAoqLog()
{
	
}


