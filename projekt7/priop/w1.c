#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include "semafory.h"

#define LICZBA_SEM 5

#define w1 0
#define w2 1
#define w3 2
#define sp 3
#define sc 4

void write_unit(int licznik);

int main(int argc, char *argv[])
{

  //Tworze nowy set semaforow
  key_t key_semafor = ftok(".", 'S');
  int semID = utworz_nowy_semafor(key_semafor, LICZBA_SEM);

  //Tworze licznik2pisarz ->pamiec dzielona
  key_t key_writecount = ftok(".", 'W');
  int id_writecount = shmget(key_writecount, sizeof(int), IPC_CREAT | 0600);
  if (id_writecount == -1)
  {
    printf("Problem z utworzeniem pamieci dzielonej.\n");
  }
  else
  {
    //printf("C:Pamiec dzielona zostala utworzona.\n");
  }

  //Dolaczam segment pamieci wspoldzielonej do procesu
  void *writecount = shmat(id_writecount, 0, 0);
  if (*(int *)writecount == -1)
  {
    printf("Problem z przydzieleniem adresu.\n");
  }
  else
  {
    //printf("C:Przestrzen adresowa zostala przyznana.\n");
  }

  //GŁÓWNY KOD PROGRAMU
  int licznik = 0;
  while (1)
  {
    licznik++;

    semafor_p(semID, w2);
    (*(int *)(writecount))++;
    if ((*(int *)(writecount)) == 1)
    {
      semafor_p(semID, sc);
    }
    semafor_v(semID, w2);
    semafor_p(semID, sp);

    usleep(30000);

    write_unit(licznik);

    semafor_v(semID, sp);
    semafor_p(semID, w2);
    (*(int *)(writecount))--;
    if ((*(int *)(writecount)) == 0)
    {
      semafor_v(semID, sc);
    }
    semafor_v(semID, w2);
  }
}

void write_unit(int licznik)
{
  FILE *fp = fopen("czytelnia.txt", "w");
  if (!fp)
  {
    printf("Błąd otwarcia pliku z danymi!\n");
    exit(EXIT_FAILURE);
  }
  printf("[PISARZ]Piszę do \"czytelnia.txt\"...\n");
  fprintf(fp, "[PISARZ](Getpid:%d)->Jestem %d raz.\n", getpid(), licznik);
  fflush(fp);
  fclose(fp);
}
