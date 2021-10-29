#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semafor;
key_t key;
FILE *plik;
static void utworz_nowy_semafor(void);
static void ustaw_semafor(int);
static void usun_semafor(void);

int main()
{
  int i, t, status;
  pid_t pid = getpid();
  char cmd1[30], cmd2[30];

  if ((plik = fopen("wynik.txt", "w")) == NULL)
  {
    perror("fopen error");
    exit(1);
  }

  utworz_nowy_semafor();

  for (int j = 0; j < 5; j++)
  {
    ustaw_semafor(j);
  }

  for (i = 0; i < 3; i++)
  {
    switch (fork())
    {
    case -1:
      perror("fork error");
      exit(1);
      break;
    case 0:
      sprintf(cmd1, "./P%d", i + 1);
      sprintf(cmd2, "P%d", i + 1);
      switch (execl(cmd1, cmd2, NULL))
      {
      case -1:
        perror("execl error");
        exit(2);
      }
      break;

    default:
      /*brak zadań dla procesu macierzystego*/
      break;
    }
  }

  for (i = 0; i < 3; i++)
  {
    pid_t status = wait(&t);
    if (status == -1)
    {
      perror("wait error");
      exit(3);
    }
  }

  usun_semafor();
  fclose(plik);
  exit(0);

  return 0;
}
static void utworz_nowy_semafor(void)
{
  key = ftok(".", 'A');
  semafor = semget(key, 5, 0600 | IPC_CREAT);
  if (semafor == -1)
  {
    printf("Nie moglem utworzyc nowego semafora.\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    printf("Semafor zostal utworzony : %d\n", semafor);
  }
}

static void ustaw_semafor(int numer)
{
  int ustaw_sem;
  ustaw_sem = semctl(semafor, numer, SETVAL, 0);
  if (ustaw_sem == -1)
  {
    printf("Nie mozna ustawic semafora.\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    printf("Semafor zostal ustawiony.\n");
  }
}

static void semafor_p(int numer)
{
  int zmien_sem;
  struct sembuf bufor_sem;
  bufor_sem.sem_num = numer;
  bufor_sem.sem_op = -1;
  bufor_sem.sem_flg = IPC_NOWAIT;
  zmien_sem = semop(semafor, &bufor_sem, 1);
  if (zmien_sem == -1)
  {
    printf("Nie moglem zamknac semafora.\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    printf("Semafor zostal zamkniety.\n");
  }
}

static void semafor_v(int numer)
{
  int zmien_sem;
  struct sembuf bufor_sem;
  bufor_sem.sem_num = numer;
  bufor_sem.sem_op = 1;
  bufor_sem.sem_flg = SEM_UNDO;
  zmien_sem = semop(semafor, &bufor_sem, 1);
  if (zmien_sem == -1)
  {
    printf("Nie moglem otworzyc semafora.\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    printf("Semafor zostal otwarty.\n");
  }
}

static void usun_semafor(void)
{
  int sem;
  sem = semctl(semafor, 0, IPC_RMID);
  if (sem == -1)
  {
    printf("Nie mozna usunac semafora.\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    printf("Semafor zostal usuniety : %d\n", sem);
  }
}
