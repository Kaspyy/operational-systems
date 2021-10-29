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

static void semafor_v(int numer1, int numer2)
{
    struct sembuf bufor_sem[2];
    bufor_sem[0].sem_num = numer1;
    bufor_sem[0].sem_op = 1;
    bufor_sem[1].sem_num = numer2;
    bufor_sem[1].sem_op = 1;
    int oper_sem = semop(semid, bufor_sem, 2);
    if (oper_sem == -1 && errno == EINTR)
    {
        printf("Otwarcie semafora po zatrzymaniu procesu \n");
        semafor_v(numer1, numer2);
    }
    else if (oper_sem == -1 && errno == EAGAIN)
    {
        printf("Semafor nie zostal otwarty.\n");
        semctl(semid, numer1, SETVAL, 1);
        semctl(semid, numer2, SETVAL, 1);
        semafor_v(numer1, numer2);
    }
    else if (oper_sem == -1)
    {
        printf("Semafor nie zostal otwarty.\n");
        exit(EXIT_FAILURE);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////// -> semafor_p

static void semafor_p(int numer1, int numer2)
{
    struct sembuf bufor_sem[2];
    bufor_sem[0].sem_num = numer1;
    bufor_sem[0].sem_op = -1;
    bufor_sem[1].sem_num = numer2;
    bufor_sem[1].sem_op = -1;
    int oper_sem = semop(semid, bufor_sem, 2);
    if (oper_sem == -1 && errno == EINTR)
    {
        printf("Zamkniecie semafora po zatrzymaniu procesu \n");
        semafor_p(numer1, numer2);
    }
    else if (oper_sem == -1 && errno == EAGAIN)
    {
        printf("Semafor nie zostal zamkniety.\n");
        semctl(semid, numer1, SETVAL, 1);
        semctl(semid, numer2, SETVAL, 1);
        semafor_p(numer1, numer2);
    }
    else if (oper_sem == -1)
    {
        printf("Semafor nie zostal zamkniety.\n");
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
                printf("Proces potomny z PID=%d, zakonczyl� sie z kodem %d\n", w, x);
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
    sleep(rand() % 2 + numer);
    printf("filozof numer: %d -> myslenie\n", numer);
}

void jedzenie(int numer)
{
    sleep(rand() % 2 + 1 + numer);
    printf("filozof numer: %d -> jedzenie\n", numer);
}

//////////////////////////////////////////////////////////////////////////////////////////////// -> glodzenie
int czekaj = 0;
void myslenie_g(int numer)
{
    if (numer == 0)
    {
        //sleep(20);
        printf("filozof numer: %d -> myslenie\n", numer);
    }
    else if (numer == 1)
    {
        //   if(czekaj != 0)
        //     sleep(1);
        czekaj++;
        printf("filozof numer: %d -> myslenie\n", numer);
    }
    else if (numer == 2)
    {
        //   sleep(2);
        printf("filozof numer: %d -> myslenie\n", numer);
    }
    else if (numer == 3)
    {
        //   sleep(1);
        printf("filozof numer: %d -> myslenie\n", numer);
    }
    else if (numer == 4)
    {
        //   sleep(20);
        printf("filozof numer: %d -> myslenie\n", numer);
    }
}
int licznik = 0;
void jedzenie_g(int numer)
{
    licznik++;
    if (numer == 0)
    {
        printf("filozof numer: %d -> jedzenie poczatek\n", numer);
        printf("filozof numer: %d -> jedzenie koniec\n", numer);
    }
    else if (numer == 1)
    {

        printf("filozof numer: %d -> jedzenie poczatek\n", numer);
        sleep(rand() % 3);
        printf("filozof numer: %d -> jedzenie koniec\n", numer);
    }
    else if (numer == 2)
    {
        printf("filozof numer: %d -> jedzenie poczatek\n", numer);
        //sleep(rand() % 2 + numer);
        printf("filozof numer: %d -> jedzenie koniec\n", numer);
    }
    else if (numer == 3)
    {
        printf("filozof numer: %d -> jedzenie poczatek\n", numer);
        sleep(rand() % 3);
        printf("filozof numer: %d -> jedzenie koniec\n", numer);
    }
    else if (numer == 4)
    {
        printf("filozof numer: %d -> jedzenie poczatek\n", numer);
        printf("filozof numer: %d -> jedzenie koniec\n", numer);
    }
    printf("licznik dla filozofa %d wynosi %d\n", numer, licznik);
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
        //myslenie(numer);
        myslenie_g(numer);
        semafor_p(numer, (numer + 1) % 5);
        //jedzenie(numer);
        jedzenie_g(numer);
        semafor_v(numer, (numer + 1) % 5);
    }
    exit(0);
}
