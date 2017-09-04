#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char * trim(char * s, int len);
int read_buffer(char *buf, int buflen, char *chunk, const int len);
void read_cmd(int fd, const char *buf, int buflen);
void write_reply(int fd, const char *buf, int buflen);