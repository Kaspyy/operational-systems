#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semafor;

static void poczatek(void);
static void utworz_nowy_semafor(void);
static void ustaw_semafor(void);
static void semafor_p(void);
static void semafor_v(void);
static void usun_semafor(void);

int main()
{
  poczatek();
  utworz_nowy_semafor();
  ustaw_semafor();
  semafor_p();
  semafor_v();
  usun_semafor();
}

static void poczatek(void)
{
  printf("Semafory.\n");
}

static void utworz_nowy_semafor(void)
{
  semafor = semget(11, 1, 0777 | IPC_CREAT);
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

static void ustaw_semafor(void)
{
  int ustaw_sem;
  ustaw_sem = semctl(semafor, 0, SETVAL, 0);
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

static void semafor_p(void)
{
  int zmien_sem;
  struct sembuf bufor_sem;
  bufor_sem.sem_num = 0;
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

static void semafor_v(void)
{
  int zmien_sem;
  struct sembuf bufor_sem;
  bufor_sem.sem_num = 0;
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
