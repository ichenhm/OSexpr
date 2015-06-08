#include<stdio.h>
#include<math.h>
#include<sched.h>
#include<pthread.h>
#include<stdlib.h>
#include<semaphore.h>

typedef struct{
	char task_id;
	int call_num;
	int ci;/// the time needs to run completely one time.
	int ti;/// one seg-time must run one time.
	int ci_left;
	int ti_left;
	int flag; /// 0 for not active, 2 for active
	int arg;
	pthread_t th;

}Task;

void proc(int *args);
void *idle();
int  select_proc();

int task_num=0;
int idle_num=0;
int alg;//algorithm 1 for EDF 2 for RMS
int curr_proc=-1;
int demo_time=100;// show time 
char chrs[100];
Task * tasks;
pthread_mutex_t proc_wait[100];
pthread_mutex_t main_wait,idle_wait;
float sum=0;
pthread_t idle_proc;

int main(int argc,char *argv[])
{
	pthread_mutex_init(&main_wait,NULL);
	pthread_mutex_lock(&main_wait);
	pthread_mutex_init(&idle_wait,NULL);
	pthread_mutex_lock(&idle_wait);
	puts("Please input number of real time tasks:");
	scanf("%d",&task_num);
	tasks=(Task*)malloc(task_num *sizeof(Task));
	int i;
	for(i=0;i<task_num;i++)
	{
		pthread_mutex_init(&proc_wait[i],NULL);
		pthread_mutex_lock(&proc_wait[i]);
	}
	for(i=0;i<task_num;i++)
	{
		puts("Please input task id,followed by Ci and Ti:\n");
		scanf("%s%d%d",chrs,&tasks[i].ci,&tasks[i].ti);
		tasks[i].task_id=chrs[0];
		tasks[i].ci_left=tasks[i].ci;
		tasks[i].ti_left=tasks[i].ti;
		tasks[i].flag=2;
		tasks[i].arg=i;
		tasks[i].call_num=1;
		sum=sum+(float)tasks[i].ci/(float)tasks[i].ti;

	}
	puts("Please choose an algorithm ,1 for EDF ,2 for RMS");
	scanf("%d",&alg);
	puts("Please input demo time");
	scanf("%d",&demo_time);
	double r=1;//EDF 
	if(alg==2)
	{
		//RMS 
		r=((double)task_num)*(exp(log(2)/(double)task_num)-1);
		printf("r is %lf\n",r);
	}
	if(sum>r)
	{//can't scheldule
		printf("(sum=%lf>r=%lf,not scheldulable!\n",sum,r);
		exit(2);
	}
	/// create all threads.
	pthread_create(&idle_proc,NULL,(void*)idle,NULL);
	for(i=0;i<task_num ;i++)
		pthread_create(&tasks[i].th,NULL,(void*)proc,&tasks[i].arg);
	for(i=0;i<demo_time;i++)
	{
		int j;
		if((curr_proc=select_proc(alg))!=-1)
		{
			pthread_mutex_unlock(&proc_wait[curr_proc]);//wake up
			pthread_mutex_lock(&main_wait);//main thread wait
		}
		else
		{//no thread to schedule
			pthread_mutex_unlock(&idle_wait);
			pthread_mutex_lock(&main_wait);
		}
		for(j=0;j<task_num;j++)
		{
			//Ti--
			if(--tasks[j].ti_left==0)
			{
				tasks[j].ti_left=tasks[j].ti;
				tasks[j].ci_left=tasks[j].ci;
				pthread_create(&tasks[j].th,NULL,(void*)proc,&tasks[j].arg);
				tasks[j].flag=2;/// set active
			}
		}
	}
	puts("");
	sleep(1);/// no need to wait so long.
}
void proc(int *args)
{
	while(tasks[*args].ci_left > 0)
	{
		pthread_mutex_lock(&proc_wait[*args]); ///ensure execute just one time circle 
		/// and then sleep here ,to wait for  being chosen by main-proc 
		if(idle_num!=0)
		{
			printf("idle(%d)",idle_num);
			idle_num=0;
		}
		printf("%c%d",tasks[*args].task_id,tasks[*args].call_num);///print a run recored.
		tasks[*args].ci_left--;///dec one time
		if(tasks[*args].ci_left==0)
		{
			printf("(%d)",tasks[*args].ci);/// print all run-times
			tasks[*args].flag=0;/// run out of time ,so set not active.
			tasks[*args].call_num++;/// run-time increase.

		}
		pthread_mutex_unlock(&main_wait);///wake up  main-proc
	}
}

void * idle()
{
	
	while(1)
	{
		pthread_mutex_lock(&idle_wait);///same reason with proc.
		printf("->");
		idle_num++;
		pthread_mutex_unlock(&main_wait);/// like proc_wait above.
	}
}

int select_proc(int alg)
{
	int j;
	int temp1,temp2;
	temp1=100000;
	temp2=-1;
	if((alg==2)&&(curr_proc!=-1)&&(tasks[curr_proc].flag!=0))/// can't choose another ,while one is running.
		return curr_proc;
	for(j=0;j<task_num;j++)
	{
if(tasks[j].flag==2)///choose those are active.
		{
			switch(alg)
			{
				case 1:/// EDF
					if(temp1>tasks[j].ti_left)///find a proc whose left-time is shortest.
					{
						temp1=tasks[j].ti_left;
						temp2=j;
					}
					break;
				case 2:///RMS
					if(temp1>tasks[j].ti)///find a proc who is most frequent now  relatively 
					{
						temp1=tasks[j].ti;
						temp2=j;
					}
					break;

			}
		}
	}
	return temp2;
}



