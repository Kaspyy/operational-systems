#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <stdint.h>

int tab[2][10];
int suma1, suma2;

void *licz1()
{
	for(int i = 0; i < 10; i++)
		suma1 += tab[0][i];
	printf("%ld",pthread_self());
	printf("\nsuma wiersza 1: %d\n", suma1);
	pthread_exit((void *) 10);//exit(0); return 0;
}

void *licz2()
{
	for(int i = 0; i < 10; i++)
		suma2 += tab[1][i];

	printf("suma wiersza 2: %d\n", suma2);
	pthread_exit(0);
}

int main()
{
	int status;
	pthread_t thread1, thread2;
	srand(time(NULL));

	for(int i = 0; i < 2; i++)
	{
		for(int j = 0; j < 10; j++)
		{
			tab[i][j] = rand() % 10;
			/*if(j == 9)
				printf("%d\n", tab[i][j]);
			else
				printf("%d, ", tab[i][j]);*/
		}
	}
	
	switch(pthread_create(&thread1, NULL, licz1, NULL))
	{
		case -1:
			perror("thread1 creation error");
			break;
	}

	switch(pthread_create(&thread2, NULL, licz2, NULL))
	{
		case -1:
			perror("thread2 creation error");
			break;
	}

	switch(pthread_join(thread1,(void **) &status))
	{
		case -1:
			perror("thread1 joining error");
			break;
	}
printf("status = %d", status);
	switch(pthread_join(thread2, NULL))
	{
		case -1:
			perror("thread2 joining error");
			break;
	}

	printf("suma elementow tablicy: %d\n\n", suma1 + suma2);
	
	switch(pthread_detach(thread1))
	{
		case -1:
			perror("thread1 detach error");
			break;
	}
	switch(pthread_detach(thread2))
	{
		case -1:
			perror("thread2 detach error");
			break;
	}
	return 0;
}
