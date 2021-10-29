#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int pamiec;
int odlaczenie1;
int odlaczenie2;
char *adres;

void poczatek()
{
  printf("Pamiec dzielona 1.00\n");
}

void upd()
{
  pamiec = shmget(10, 256, 0777 | IPC_CREAT);
  if (pamiec == -1)
  {
    printf("Problemy z utworzeniem pamieci dzielonej.\n");
    exit(EXIT_FAILURE);
  }
  else
    printf("Pamiec dzielona zostala utworzona : %d\n", pamiec);
}

void upa()
{
  adres = shmat(pamiec, 0, 0);
  if (*adres == -1)
  {
    printf("Problem z przydzieleniem adresu.\n");
    exit(EXIT_FAILURE);
  }
  else
    printf("Przestrzen adresowa zostala przyznana : %s\n", adres);
}

void odlacz_pamiec()
{
  odlaczenie1 = shmctl(pamiec, IPC_RMID, 0);
  odlaczenie2 = shmdt(adres);
  if (odlaczenie1 == -1 || odlaczenie2 == -1)
  {
    printf("Problemy z odlaczeniem pamieci dzielonej.\n");
    exit(EXIT_FAILURE);
  }
  else
    printf("Pamiec dzielona zostala odlaczona.\n");
}

void wstaw()
{
  printf("Wpisz cos do pamieci :");
  scanf("%s", adres);
}

void wez()
{
  printf("Biore z pamieci : %s\n", adres);
}

int main()
{
  upd();
  upa();
  upa();
  wstaw();
  wez();
  odlacz_pamiec();
  exit(EXIT_SUCCESS);
}
