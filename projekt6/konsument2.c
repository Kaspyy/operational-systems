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

void main(int argc, char *argv[])
{
  char plik_wyjsciowy[32];
  char znak;
  int potok;

  sprintf(plik_wyjsciowy, "wy_%ld.txt", (long int)getpid());

  //otwieramy plik
  FILE *fp = fopen(plik_wyjsciowy, "w");
  if (!fp)
  {
    perror("Błąd otwarcia pliku wynikowego\n");
    exit(EXIT_FAILURE);
  }

  //otwieramy potok

  potok = open(NAZWA_FIFO, O_RDONLY);
  if (potok == -1)
  {
    printf("Nie mozna otworzyc potoku!\n");
    exit(EXIT_FAILURE);
  }

  else
  {
    while (read(potok, &znak, sizeof(char)) > 0)
      putc(znak, fp);
  }

  close(potok);
  fclose(fp);
}
