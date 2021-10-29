#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <errno.h>

#define PRODUCENT 0
#define KONSUMENT 1

int pipeDescriptors[2];
long liczba_producentow, liczba_konsumentow, liczba_znakow;

void spr_argumenty(int argc, char *argv[]);
void create_sb(int ilosc, int kto);
void producent();
void konsument();
char losuj_znak();

int main(int argc, char *argv[])
{
  int cpid, status;
  char cmd[120];

  spr_argumenty(argc, argv);

  pipe(pipeDescriptors); //tworzenie potoku nienazwanego

  create_sb(liczba_producentow, PRODUCENT);
  create_sb(liczba_konsumentow, KONSUMENT);

  close(pipeDescriptors[0]);
  close(pipeDescriptors[1]);

  for (int i = 0; i < liczba_producentow + liczba_konsumentow; i++)
  {
    cpid = wait(&status);
    if (cpid == -1)
    {
      printf("Blad potomka kod powrotu potomka to: %d\n", status / 256);
    }
    else
    {
      printf("Potomek zakonczyl sie poprawnie, id: %d\n", cpid);
    }
  }

  return 0;
}

int spr_limit()
{

  FILE *fp_in, *fp_in2;

  fp_in = popen("ps ux | grep sostudent52 | wc -l", "r");

  char tmp[50];
  char *end;
  int suma1;

  if (fp_in != NULL)
  {
    if (fgets(tmp, sizeof(tmp), fp_in))
    {
      suma1 = strtol(tmp, &end, 10);
    }
    if (errno == ERANGE)
    {
      perror("nieprawidlowy zakres\n");
      exit(EXIT_FAILURE);
    }
    else if (end == tmp)
    {
      printf("bledny format\n");
      exit(EXIT_FAILURE);
    }
  }
  pclose(fp_in);

  fp_in2 = popen("ulimit -u", "r");

  char tmp2[50];
  char *end2;
  int suma2;

  if (fp_in2 != NULL)
  {
    if (fgets(tmp2, sizeof(tmp2), fp_in2))
    {
      suma2 = strtol(tmp2, &end2, 10);
    }
    if (errno == ERANGE)
    {
      perror("nieprawidlowy zakres\n");
      exit(EXIT_FAILURE);
    }
    else if (end == tmp2)
    {
      printf("bledny format\n");
      exit(EXIT_FAILURE);
    }
  }
  pclose(fp_in2);

  int limit = suma2 - suma1;
  printf("Limit = %d\n", limit);

  return limit;
}

void spr_argumenty(int argc, char *argv[])
{

  if (argc != 4)
  {
    printf("Nieprawidłowa liczba argumentow wywolania!\n");
    exit(EXIT_FAILURE);
  }

  liczba_producentow = strtol(argv[1], NULL, 0);
  if (errno == ERANGE)
  {
    perror("Limit producentow przekroczony");
    exit(EXIT_FAILURE);
  }

  liczba_konsumentow = strtol(argv[2], NULL, 0);
  if (errno == ERANGE)
  {
    perror("Limit konsumentow przekroczony");
    exit(EXIT_FAILURE);
  }

  liczba_znakow = strtol(argv[3], NULL, 0);
  if (errno == ERANGE)
  {
    perror("Limit znakow przekroczony");
    exit(EXIT_FAILURE);
  }

  if (liczba_producentow < 1 || liczba_konsumentow < 1)
  {
    printf("Zbyt malo %s.\n", liczba_producentow < 1 ? (liczba_konsumentow < 1 ? "producentow i konsumentow" : "producentow") : "konsumentow");
    exit(EXIT_FAILURE);
  }

  if (liczba_producentow + liczba_konsumentow > spr_limit())
  {
    printf("Proszę podac mniejsza ilosc procesow");
    exit(EXIT_FAILURE);
  }
}

void create_sb(int ilosc, int kto)
{

  for (int i = 0; i < ilosc; i++)
  {
    switch (fork())
    {
    case -1:
      perror("Nieprawidlowe wywolanie fork()\n");
      exit(EXIT_FAILURE);

    case 0: //jezeli proces jest procesem potomnym

      if (kto == PRODUCENT)
      {
        producent();
        printf("PPPP\n");
      }
      else
      {
        konsument();
        printf("KKKKK\n");
      }
      exit(0);
    }
  }
}

void producent()
{
  char plik_wejsciowy[32];
  char znak;

  close(pipeDescriptors[0]); //zamykam czytanie
  sprintf(plik_wejsciowy, "we_%ld.txt", (long int)getpid());

  FILE *fp = fopen(plik_wejsciowy, "w");
  if (!fp)
  {
    printf("Błąd otwarcia pliku z danymi!\n");
    exit(1);
  }

  for (int i = 0; i < liczba_znakow; i++)
  {
    znak = losuj_znak();
    fprintf(fp, "%c", znak);                        //zapis do pliku
    write(pipeDescriptors[1], &znak, sizeof(char)); //zapis do potoku
  }

  fclose(fp);
  close(pipeDescriptors[1]);

  return;
}

void konsument()
{
  char plik_wyjsciowy[32];
  char znak;

  close(pipeDescriptors[1]); //zamykam czytanie
  sprintf(plik_wyjsciowy, "wy_%ld.txt", (long int)getpid());
  FILE *fp = fopen(plik_wyjsciowy, "w");
  if (!fp)
  {
    perror("Błąd otwarcia pliku wynikowego\n");
    exit(EXIT_FAILURE);
  }

  while (read(pipeDescriptors[0], &znak, sizeof(char)))
    putc(znak, fp);
  close(pipeDescriptors[0]);
}

char losuj_znak()
{
  char c;
  srand((int)getpid());

  //znaki ascii drukowalne od 32-126
  c = rand() % (126 - 32) + 32;
  return c;
}
