#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<time.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>

#define Max 5 //Max chimps

sem_t *sem;
sem_t *OnRope;

int total;
int current=0; //On rope currently
int *turnsAB; //Manage order AB
int nextturnAB=1;
int nextslotAB=1;
int *turnsBA; //Manage order BA
int nextturnBA=1;
int nextslotBA=1;
bool direction=0; //AB=0 and BA=1
bool switchdir=0;
int waitAB=0; //Chimps waiting on AB
int waitBA=0; //Chimps waiting on BA
int crossAB=0; //Chimps crossing AB
int crossBA=0; //Chimps crossing BA

void *Chimpanzee(void *n)
{
	int id=*(int *)n;
	free(n);
	bool d;
	sem_wait(sem);
	d=rand()%2;
	if(d==0)
	{
		printf("Chimpanzee %d arrives at AB\n",id+1); 
		turnsAB[id]=nextslotAB;
		nextslotAB++;
		waitAB++;
	}
	else
	{
		printf("Chimpanzee %d arrives at BA\n",id+1);
		turnsBA[id]=nextslotBA;
		nextslotBA++;
		waitBA++;
	}
	sem_post(sem);
	while(1)
	{
		sem_wait(sem);
		if(d==0 && turnsAB[id]==nextturnAB)
		{
			if(d==direction && current<Max && !switchdir)
			{
				current++;
				waitAB--;
				crossAB++;
				nextturnAB++;
				sem_post(sem);
				break;
			}
		}
		if(d==1 && turnsBA[id]==nextturnBA)
		{
			if(d==direction && current<Max && !switchdir)
			{
				current++;
				waitBA--;
				crossBA++;
				nextturnBA++;
				sem_post(sem);
				break;
			}
		}
		sem_post(sem);
		usleep(1000); //No busy wait
	}
	sem_wait(OnRope);
	if(d==0)
	{
		printf("Chimpanzee %d crossing at AB\n",id+1);
		sleep(0.5);
		printf("Chimpanzee %d crossed at AB\n",id+1);
	}
	else
	{
		printf("Chimpanzee %d crossing at BA\n",id+1);
		sleep(0.5);
		printf("Chimpanzee %d crossed at BA\n",id+1);
	}
	sem_post(OnRope);
	sem_wait(sem);
	current--;
	if((direction==0 && waitBA>0 && (crossAB>=10 || waitAB==0)) || (direction==1 && waitAB>0 && (crossBA>=10 || waitBA==0)))
	{
		switchdir=1;
	}
	if(current==0 && switchdir)
	{
		direction=!(direction);
		crossAB=0;
		crossBA=0;
		switchdir=0;
		printf("Direction Changed....\n");
	}
	sem_post(sem);
	return NULL;
}

int main(int argc,char *arr[])
{
	if(argc<2)
	{
		printf("Argument error\n");
	}
	srand(time(NULL));
	sem=sem_open("/sem",O_CREAT,0644,1);
	OnRope=sem_open("/onrope",O_CREAT,0644,5);
	total=atoi(arr[1]);
	pthread_t thread;
	turnsAB=malloc(sizeof(int)*total);
	turnsBA=malloc(sizeof(int)*total);
	for(int i=0;i<total;i++)
	{
		int *id=malloc(sizeof(int));
		*id=i;
		pthread_create(&thread,NULL,Chimpanzee,id);
		pthread_detach(thread); //Remove the thread when done
		sleep(5); //Delay between threads
	}
	sleep(25); //Let remaining complete
	sem_close(sem);
	sem_close(OnRope);
	sem_unlink("/sem");
	sem_unlink("/onrope");
	return 0;
}
