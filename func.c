#include "func.h"

char * trim(char * s, int len)
{
    char * i = s+len-1;
    while(isspace(*s) && *s != '\0'){
        s++;
    }
    for(i; isspace(*i) && i>s; i--){
        (*i) = '\0';
    }
    return s;
}

int read_buffer(char *buf, int buflen, char *chunk, const int len)
{
	int cplen;
	cplen = buflen > len ? len : buflen;
	memcpy(chunk, buf, cplen);
	return cplen;
}

int read_cmd(int fd)
{
	char *cmd = command;
	int sz = 16384;
	int buflen = 0;
	int len = read(fd, cmd, sz);
	buflen += len;
	cmd += len;
	
	if( len <= 0 )
    {
		return -1;
	}
	
	while(*(cmd-1) != '\n')
    {
		len = read(fd, cmd, sz);
		cmd += len;
		buflen += len;
    }
	*cmd = '\0';
	return buflen;
}

void write_reply(int fd, const char *buf, int buflen)
{
	int total = 0;
	int wlen;
	
	while(total < buflen)
	{
		wlen = write(fd, buf, buflen-total);
		buf += wlen;
		total += wlen;
	}
	
	
}