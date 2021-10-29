#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

int key = 10;
int rozmiar = sizeof(char);
int pamiec;
int odlaczenie1;
int odlaczenie2;
char *adres;
int semafor;

static void utworz_nowy_semafor(int count);
static void usun_semafor();
static void ustaw_semafor(int nr);
static void semafor_p(int nr);
static void semafor_v(int nr);

void upd();
void upa();
void odlacz_pamiec();

static void utworz_nowy_semafor(int count)
{
    semafor = semget(ftok("konsument.out", key), count, 0600 | IPC_CREAT);
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

static void ustaw_semafor(int nr)
{
    int setSem;
    setSem = semctl(semafor, nr, SETVAL, 1);
    if (setSem == -1)
    {
        printf("Nie mozna ustawic semafora.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Semafor zostal ustawiony.\n");
    }
}

static void usun_semafor()
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

static void semafor_p(int nr)
{
    struct sembuf bufor_sem;
    bufor_sem.sem_num = nr;
    bufor_sem.sem_op = -1;
    bufor_sem.sem_flg = 0;
    if (semop(semafor, &bufor_sem, 1) == -1)
    {
        if (errno == EINTR)
        {
            printf("Producent - Wznowienie procesu\n");
            semafor_p(nr);
        }
        else
        {
            printf("Producent - Nie moglem zablokowac sekcji krytycznej nr %d\n", nr);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("Producent - Sekcja krytyczna nr %d zablokowana.\n", nr);
    }
}

static void semafor_v(int nr)
{
    struct sembuf bufor_sem;
    bufor_sem.sem_num = nr;
    bufor_sem.sem_op = 1;
    bufor_sem.sem_flg = 0;
    if (semop(semafor, &bufor_sem, 1) == -1)
    {
        if (errno == EINTR)
        {
            printf("Producent - Wznowienie procesu\n");
            semafor_v(nr);
        }

        printf("Producent - Nie moglem zablokowac sekcji krytycznej nr %d\n", nr);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Producent - Sekcja krytyczna nr %d odblokowana.\n\n", nr);
    }
}

void upd()
{
    pamiec = shmget(ftok("konsument.out", key), rozmiar, 0600 | IPC_CREAT);
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

int main()
{
    double wait;
    int i;
    FILE *fip;
    srand(time(NULL));

    fip = fopen("input.txt", "r");
    if (!fip)
    {
        printf("Blad otwarcia pliku \"input.txt\"\n");
        exit(1);
    }

    //ustawiam zbior semaforow (praca wykluczajaca konsument-producent)
    utworz_nowy_semafor(2);

    for (i = 0; i < 2; i++)
    {
        ustaw_semafor(i);
    }

    semafor_p(1); //zamykam dostep 1 - konsumentowi

    //tworzenie pamieci wspoldzielonej
    upd();
    upa();

    //przesylanie danych - produkcja,konsumpcja
    while (1)
    {
        semafor_p(0);
        fscanf(fip, "%c", adres);
        if (feof(fip))
        {
            //koniec towaru, zaczynam zwijac stragan
            break;
        }
        printf("Jestem w trakcie produkcji ...\n");
        wait = ((double)rand()) / RAND_MAX;
        printf("Produkcja potrwa %lf sekund\n", wait);
        sleep(wait);
        printf("Wyprodukowano: %c\n", *adres);
        semafor_v(1); //otwieram dostep 1 - konsumentowi
    }

    //wstawiam NULL na polke
    *adres = 0;
    printf("Koncze produkcje!\n");
    semafor_v(1); //otwieram dostep 1 - konsumentowi

    //czekam az konsument skonczy zapisywac do swojego notesu, zeby zlozyc stragan
    semafor_p(0);

    fclose(fip);
    odlacz_pamiec();
    usun_semafor();
    exit(EXIT_SUCCESS);
}
