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

//Red=0 and Green=1
bool light=0; 
int score=0;
sem_t *sem;
bool GameOver=0;

//Light Color
void *LightChange()
{
	while(GameOver==0)
	{
		sleep(2);
		sem_wait(sem);
		light=rand()%2;
		sem_post(sem);
	}
}

//Display Light
void *DisplayLight()
{	
	int current=-1;
	while(GameOver==0)
	{
		sem_wait(sem);
		if(current!=light)
		{
			if(light)
				printf("Green Light\n");
			else
				printf("Red Light\n");
			current=light;
		}
		sem_post(sem);
		sleep(1);
	}
}

int main()
{
	srand(time(0));
	//Create Threads
	pthread_t thread[2];
	sem=sem_open("/sem",O_CREAT,0644,1);
	pthread_create(&thread[0],NULL,LightChange,NULL);
	pthread_create(&thread[1],NULL,DisplayLight,NULL);
	//Loop to keep the game going
	while(GameOver==0)
	{
		char ch=getchar();
		sem_wait(sem);
		bool current=light;
		sem_post(sem);
		if(current && ch=='\n')
		{
			score++;
		}
		else
		{
			GameOver=1;
			printf("GAME OVER \n Score: %d\n",score);
			break;
		}
	}
	pthread_join(thread[0],NULL);
	pthread_join(thread[1],NULL);
	sem_close(sem);
	sem_unlink("/sem");
	return 0;
}
