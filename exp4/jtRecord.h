#define RECORDLEN 32
typedef struct jtRecord{
	int key;
	char other[RECORDLEN-sizeof(int)];
}jtRecord;

#ifdef HAVE_CONFIG_H
#include<config.h>///
#endif


