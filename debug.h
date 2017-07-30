#include <stdio.h>

int debug()
{
	printf("FILE:%s, LINE:%s\n",__FILE__, __LINE__);
	return 1;
}
