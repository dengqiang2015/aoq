#include "aoqlog.h"
 
void initAoqLog()
{
	
	aoqlog = (AOQLOG *)malloc(sizeof(AOQLOG));
	aoqlog->size = 1024*1024*50;
	aoqlog->log = (char *)malloc((aoqlog->size)*sizeof(char));
	memset(aoqlog->log, '\0', aoqlog->size);
	aoqlog->len =0;
	char logfile[1024] = {'\0'};
	sprintf(logfile, "%s/%s", Serv->work_dir_path, AOQ_LOG_FILE_NAME);
	aoqlog->fp_logfile = fopen(logfile, "ab+");
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
			char logfile[1024] = {'\0'};
			sprintf(logfile, "%s/%s", Serv->work_dir_path, AOQ_LOG_FILE_NAME);
			aoqlog->fp_logfile = fopen(logfile, "ab+");
		}
		
		int r = fprintf(aoqlog->fp_logfile, "%s", aoqlog->log);
		
		if(r < 0)
		{
			fclose(aoqlog->fp_logfile);
			char logfile[1024] = {'\0'};
			sprintf(logfile, "%s/%s", Serv->work_dir_path, AOQ_LOG_FILE_NAME);
			aoqlog->fp_logfile = fopen(logfile, "ab+");
			return -2;
		}
		
		memset(aoqlog->log, '\0', aoqlog->size);
		aoqlog->len = 0;
	}

    return 1;
}

void getTime(char *timestr)
{
	time_t timep;
	struct tm *p;
	time(&timep);
	p = gmtime(&timep);
	sprintf(timestr, "%d-%d-%d %d:%d:%d", (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
}

void serverlog(const char *slog)
{
	char logfile[1024] = {'\0'};
	char timestr[128] = {'\0'};
	getTime(timestr);
	sprintf(logfile, "%s/%s", Serv->work_dir_path, SERVER_LOG_FILE_NAME);
	FILE *fp_logfile = fopen(logfile, "ab+");
	fprintf(fp_logfile, "[%s] %s\n", timestr, slog);
	fclose(fp_logfile);
}


int readLogLine(char *line, char *chunk, int size)
{
	int len = strlen(line);
	len = len > size ? size : len;
	memcpy(chunk, line, len);
	return len;
}

static void execute_command_line(char *line)
{
	int fd = 0;
    ChunkNode *node = mpalloc();
    int len = readLogLine(line, node->chunk, CK_SIZE-1);
	if(len<12)
	{
		mpfree(node);
		return;
	}
    int r = 0;

    MemSlab *memslab = createMemSlab(1);
    insertMemSlab(memslab, node);
    int command_num = 0;


    while(len == CK_SIZE-1)
    {
		line += len;
        node = mpalloc();
        len = readLogLine(line, node->chunk, CK_SIZE-1);
        insertMemSlab(memslab, node);
    }
    
	Arg *args = createArgs(5);

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

static void recovery_from_aoflog()
{

	serverlog("Recover data from aof log.");
	char logfile[1024] = {'\0'};
	sprintf(logfile, "%s/%s", Serv->work_dir_path, AOQ_LOG_FILE_NAME);
	const int LEN = 1024*1025+100;
	char line[LEN] = {'\0'};
	FILE *fp_logfile = fopen(logfile, "rb");
	
	if(fp_logfile != NULL)
	{
		while(!feof(fp_logfile)){
			if(fgets(line, LEN, fp_logfile) != NULL)
			{
				execute_command_line(line);
			}
		}
		fclose(fp_logfile);
	}
	serverlog("Recover data complate.");

}

