#include <errno.h>
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
	int sz = 17520;
	int buflen = 0;
	int len = read(fd, cmd, sz);
	//disconnect
	if( len == 0 )
        {
		return -1;
	}

       if(len < 0)
       {
		if(errno == EAGAIN ||  errno == EWOULDBLOCK || errno == EINTR )
		{
			return read_cmd(fd);
		}
		return -1;		
       }

    buflen += len;
    cmd += len;
    while(*(cmd-1) != '\n')
    {
		len = read(fd, cmd, sz);
		
		if(len == 0)
		{
			return -1;
		}		

		if(len < 0)
                {
		    if(errno == EAGAIN ||  errno == EWOULDBLOCK || errno == EINTR )
			{
                     		continue;
			}
			return -1;
                }
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
