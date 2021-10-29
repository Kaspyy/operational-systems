#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MAX 255 //maksymalny rozmiar wiadomosci
#define SERWER 1
#define KLIENT 2

//struktura komunikatu
struct komunikat{
	long mtype;
	char mtext[MAX];
};

int main()
{
	int i;
	int IDkolejki;
	struct komunikat kom;

	if((IDkolejki = msgget(10, IPC_CREAT | 0777)) == -1)
	{
		printf("Blad tworzenia kolejki komunikatów.\n");
		exit(EXIT_FAILURE);
	}
	printf("id=%d\n",IDkolejki);


	printf("Podaj tresc komunikatu: ");
    	scanf("%s",&kom.mtext);



	kom.mtype = SERWER;


		printf("Klient: Wysylanie... %s -> %ld \n",kom.mtext, kom.mtype);

		if(msgsnd(IDkolejki,(struct komunikat*)&kom, strlen(kom.mtext)+1,0) == -1)
		{
			printf("Blad dodania komunikatu do kolejki\n");
			exit(EXIT_FAILURE);
		}

		kom.mtype = KLIENT;

		if(msgrcv(IDkolejki, (struct komunikat *) &kom, MAX, kom.mtype,0) == -1)
		{
			printf("Blad odbioru komunikatu z kolejki\n");
			exit(EXIT_FAILURE);
		}


		printf("Klient: Odebrano: \"%s\" zaadresowane do %ld\n", kom.mtext, kom.mtype);

}


