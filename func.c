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
