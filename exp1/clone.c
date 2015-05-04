#include <sched.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

int producer(void *args);
int consumer(void *args);

pthread_mutex_t mutex;
sem_t product;
sem_t warehouse;

char buffer[8][10];
int bp=0;

int main(int argc,char **args)
{
	
