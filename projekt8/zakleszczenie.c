#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ctype.h>
#include <signal.h>

//////////////////////////////////////////////////////////////////////////////////////////////// -> dolaczanie zbioru semaforow

pid_t pid;
int semid;
key_t return_key()
{
    key_t key;
    key = ftok(".", 'B');
    return key;
}

static void dolacz_zbior_semaforow(key_t key)
{
    semid = semget(key, 5, 0666 | IPC_CREAT);
    if (semid == -1)
    {
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Zbior semaforow %d zostal dolaczony do pid=%d\n", semid, getpid());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////// -> ustawianie semaforow

static void ustaw_semafory()
{
    int ustaw_sem;
    for (int i = 0; i < 5; i++)
    {
        ustaw_sem = semctl(semid, i, SETVAL, 1);
        if (ustaw_sem == -1)
        {
            perror("Nie mozna ustawic semafora.\n");
            exit(EXIT_FAILURE);
        }
        else
            printf("Semafor %d zostal ustawiony.\n", i);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////// -> usun zbior semaforow

static void usun_zbior_semaforow()
{
    int usun_sem;
    usun_sem = semctl(semid, 0, IPC_RMID);
    if (usun_sem == -1 && errno != EINVAL)
    {
        perror("Nie mozna usunac zbioru\n");
        exit(EXIT_FAILURE);
    }
    else if (usun_sem == -1 && errno == EINVAL)
    {
        printf("Zbior został juz usuniety\n");
    }
    else
    {
        printf("Zbior zostal usuniety : %d\n", usun_sem);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////// -> semafor_v

static void semafor_v(int numer)
{
    struct sembuf bufor_sem;
    bufor_sem.sem_num = numer;
    bufor_sem.sem_op = 1;
    int oper_sem = semop(semid, &bufor_sem, 1);
    if (oper_sem == -1 && errno == EINTR)
    {
        printf("Otwarcie semafora po zatrzymaniu procesu %d\n", numer);
        semafor_v(numer);
    }
    else if (oper_sem == -1 && errno == EAGAIN)
    {
        printf("Semafor %d nie zostal otwarty.\n", numer);
        semctl(semid, numer, SETVAL, 1);
        semafor_v(numer);
    }
    else if (oper_sem == -1)
    {
        printf("Semafor %d nie zostal otwarty.\n", numer);
        exit(EXIT_FAILURE);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////// -> semaforow_p

static void semafor_p(int numer)
{
    struct sembuf bufor_sem;
    bufor_sem.sem_num = numer;
    bufor_sem.sem_op = -1;
    int oper_sem = semop(semid, &bufor_sem, 1);
    if (oper_sem == -1 && errno == EINTR)
    {
        printf("Zamkniecie semafora po zatrzymaniu procesu %d\n", numer);
        semafor_p(numer);
    }
    else if (oper_sem == -1 && errno == EAGAIN)
    {
        printf("Semafor %d nie zostal zamkniety.\n", numer);
        semctl(semid, numer, SETVAL, 1);
        semafor_p(numer);
    }
    else if (oper_sem == -1)
    {
        printf("Semafor %d nie zostal zamkniety.\n", numer);
        exit(EXIT_FAILURE);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////// -> handler

void handler(int sig)
{
    printf("Wyslano sygnal SIGINT od %d\n", getpid());

    int w, x;
    if (pid == getpid())
    {
        usun_zbior_semaforow();
        for (int i = 0; i < 4; i++)
        {
            w = wait(&x);
            if (w == -1)
                perror("wait error");
            else
                printf("Proces potomny z PID=%d, zakończył się z kodem %d\n", w, x);
        }
    }
    exit(0);
}

//////////////////////////////////////////////////////////////////////////////////////////////// -> fork

void utworz_filozofow(pid_t pid)
{
    key_t key = return_key();
    dolacz_zbior_semaforow(key);
    ustaw_semafory();
    for (int i = 0; i < 4; i++)
    {
        if (getpid() == pid)
        {
            switch (fork())
            {
            case -1:
                perror("Blad przy tworzeniu procesu potomnego.\n");
                exit(EXIT_FAILURE);
            case 0:
                printf("Utworzono filozofa pid = %d\n", getpid());
                dolacz_zbior_semaforow(key);
                break;
            default:
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////// -> myslenie i jedzenie

void myslenie(int numer)
{
    //sleep(rand() % 2 + 1);
    printf("filozof numer: %d -> myslenie\n", numer);
}

void jedzenie(int numer)
{
    //sleep(rand() % 2 + 1);
    printf("filozof numer: %d -> jedzenie\n", numer);
}

//////////////////////////////////////////////////////////////////////////////////////////////// -> main

int main(int argc, char **argv)
{
    srand(time(NULL));
    pid = getpid();
    signal(SIGINT, handler);
    if (pid == getpid())
        utworz_filozofow(pid);
    int numer_pid = getpid();
    int numer = numer_pid % 5;
    printf("numer %d:\n", numer);
    for (;;)
    {
        myslenie(numer);
        semafor_p(numer);
        //sleep(1); // zakleszczenie
        semafor_p((numer + 1) % 5);
        jedzenie(numer);
        semafor_v(numer);
        semafor_v((numer + 1) % 5);
    }
    exit(0);
}
