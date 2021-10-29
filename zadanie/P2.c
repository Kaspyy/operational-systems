#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#define PROCES 2

int licznik = 1;

int semafor;
key_t key;
FILE *plik;
static void utworz_nowy_semafor(void);
static void semafor_p(int);
static void semafor_v(int);

int main()
{

    if ((plik = fopen("wynik.txt", "a")) == NULL)
    {
        perror("fopen error");
        exit(1);
    }

    utworz_nowy_semafor();

    //1
    printf("Sekcja t%d%d procesu o PID=%d\n", PROCES, licznik, getpid());
    fprintf(plik, "Sekcja t%d%d procesu o PID=%d\n", PROCES, licznik++, getpid());
    fflush(plik);
    sleep(1);

    //2
    semafor_v(0);

    //3
    semafor_p(1);

    //4
    printf("Sekcja t%d%d procesu o PID=%d\n", PROCES, licznik, getpid());
    fprintf(plik, "Sekcja t%d%d procesu o PID=%d\n", PROCES, licznik++, getpid());
    fflush(plik);
    sleep(1);

    //5
    semafor_v(2);

    //6
    semafor_p(3);

    //7
    printf("Sekcja t%d%d procesu o PID=%d\n", PROCES, licznik, getpid());
    fprintf(plik, "Sekcja t%d%d procesu o PID=%d\n", PROCES, licznik++, getpid());
    fflush(plik);
    sleep(1);

    //8
    semafor_v(4);

    fclose(plik);
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
}

static void semafor_p(int numer)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num = numer;
    bufor_sem.sem_op = -1;
    bufor_sem.sem_flg = SEM_UNDO;
    zmien_sem = semop(semafor, &bufor_sem, 1);
    if (zmien_sem == -1)
    {
        printf("Nie moglem zamknac semafora.\n");
        exit(EXIT_FAILURE);
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
}
