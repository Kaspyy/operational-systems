#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>

#define MAX 255
#define SERWER 1
#define KLIENT 2



//struktura komunikatu
struct komunikat{
	long mtype;
	char mtext[MAX];
};

int main(){
	int msize;
	int i, pid;
	int IDkolejki;
	struct komunikat kom;


	if((IDkolejki = msgget(10, IPC_CREAT | 0777)) == -1)
	{
		printf("Blad tworzenia kolejki komunikatów.\n");
		exit(EXIT_FAILURE);
	}
	printf("id=%d\n",IDkolejki);


	printf("Serwer: Czekam na komunikat...\n");
	kom.mtype = SERWER;
		if((msgrcv(IDkolejki,(struct msgbuf*)&kom, MAX, kom.mtype, 0)) == -1)
		{
			printf("Blad pobrania komunikatu z kolejki\n");
			exit(EXIT_FAILURE);
		}


		printf("Serwer: Odebrano: \"%s\" zaadresowane do %ld\n", kom.mtext, kom.mtype);


		msize = strlen(kom.mtext);
		for(i=0;i<msize;i++){
			kom.mtext[i] = toupper(kom.mtext[i]);
		}

		//wysylanie

		kom.mtype = KLIENT;

		printf("Serwer: Wysylanie... %s -> %ld \n",kom.mtext, kom.mtype);
		//dodanie komunikatu do kolejki

		if((msgsnd(IDkolejki,(struct msgbuf*)&kom, strlen(kom.mtext)+1,0))==-1)
		{
			printf("Blad dodania komunikatu do kolejki.\n");
			exit(EXIT_FAILURE);
		}


    		if ( msgctl(IDkolejki,IPC_RMID,0)==-1 )
      		{
        	printf("Blad usuniecia kolejki.\n");
        	exit(EXIT_FAILURE);
      		}

}







