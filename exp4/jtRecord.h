#define RECORDLEN 32
typedef struct{
	int key;
	char other[RECORDLEN-sizeof(int)];
};

#ifdef HAVE_CONFIG_H
#include<config.h>
#endif


