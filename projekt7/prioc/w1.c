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

#define pisanie 1
#define LICZBA_SEM 2

void write_unit(int licznik);

int main(int argc, char *argv[])
{

  //Tworze nowy set semaforow
  key_t key_semafor = ftok(".", 'S');
  int semID = utworz_nowy_semafor(key_semafor, 2);

  //GŁÓWNY KOD PROGRAMU
  int licznik = 0;
  while (1)
  {
    licznik++; //ktory raz proces wchodzi

    semafor_p(semID, pisanie);

    write_unit(licznik);
    sleep(2);

    semafor_v(semID, pisanie);
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
