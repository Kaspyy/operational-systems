#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#define NAZWA_FIFO "./moje_fifo"

int liczba_konsumentow;

void spr_argumentow(int argc, char *argv[]);
void create_sb(int liczba_konsumentow);

int main(int argc, char *argv[])
{
  int cpid, status;
  spr_argumentow(argc, argv);

  //tworzenie potoku nazwanego

  if (access(NAZWA_FIFO, F_OK) == -1)
  {
    if (mkfifo(NAZWA_FIFO, 0600) == -1)
    {
      fprintf(stderr, "Blad mkfifo %s\n", NAZWA_FIFO);
      exit(EXIT_FAILURE);
    }
  }

  create_sb(liczba_konsumentow);

  for (int i = 0; i < liczba_konsumentow; i++)
  {
    cpid = wait(&status);
    if (cpid == -1)
    {
      printf("Blad potomka.\n");
    }
    else
    {
      printf("Potomek zakonczyl sie poprawnie, id: %d\n", cpid);
    }
  }

  unlink(NAZWA_FIFO);

  return 0;
}

int spr_limit()
{

  FILE *fp_in, *fp_in2;

  fp_in = popen("ps -u ^4640 | wc -l", "r");

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
      perror("nie prawidlowy zakres\n");
      exit(EXIT_FAILURE);
    }
    else if (end == tmp)
    {
      printf("bledny format\n");
      exit(EXIT_FAILURE);
    }
  }

  pclose(fp_in);

  //////////////////////
  fp_in2 = popen("ulimit -u ", "r");

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
      perror("nie prawidlowy zakres\n");
      exit(EXIT_FAILURE);
    }
    else if (end2 == tmp2)
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

void spr_argumentow(int argc, char *argv[])
{

  if (argc != 2)
  {
    printf("Nieprawidłowa liczba argumentow wywolania!\n");
    exit(EXIT_FAILURE);
  }

  char *end;

  liczba_konsumentow = strtol(argv[1], &end, 10);
  if (errno == ERANGE)
  {
    perror("nie prawidlowy zakres\n");
    exit(EXIT_FAILURE);
  }
  else if (end == argv[1])
  {
    printf("bledny format\n");
    exit(EXIT_FAILURE);
  }

  if (liczba_konsumentow < 1)
  {
    printf("Zbyt malo konsumentow.\n");
    exit(EXIT_FAILURE);
  }

  if (liczba_konsumentow > spr_limit())
  {
    printf("Proszę podac mniejsza ilosc procesow");
    exit(EXIT_FAILURE);
  }
}

void create_sb(int liczba_konsumentow)
{

  for (int i = 0; i < liczba_konsumentow; i++)
  {
    switch (fork())
    {
    case -1:
      perror("Nieprawidlowe wywolanie fork()\n");
      exit(EXIT_FAILURE);

    case 0: //jezeli proces jest procesem potomnym

      if (execl("./konsument2", "./konsument2", NULL) == -1)
      {
        perror("blad execl\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}
