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

static void semafor_p(int nr);
static void semafor_v(int nr);
static void utworz_nowy_semafor(int count);

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
            printf("Konsument - Wznowienie procesu\n");
            semafor_p(nr);
        }
        else
        {
            printf("Konsument - Nie moglem zablokowac sekcji krytycznej nr %d\n", nr);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("Konsument - Sekcja krytyczna nr %d zablokowana.\n", nr);
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
            printf("Konsument - Wznowienie procesu\n");
            semafor_v(nr);
        }

        printf("Konsument - Nie moglem zablokowac sekcji krytycznej nr %d\n", nr);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Konsument - Sekcja krytyczna nr %d odblokowana.\n\n", nr);
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
    FILE *fop;

    srand(time(NULL));

    fop = fopen("output.txt", "w");
    if (!fop)
    {
        printf("Blad otwarcia pliku \"output.txt\"\n");
        exit(1);
    }

    //uzyskanie dostepu do zbioru semaforow (praca wykluczajaca konsument-producent)
    utworz_nowy_semafor(2);

    //uzyskanie dostepu do pamieci wspoldzielonej
    upd();
    upa();

    while (1)
    {
        semafor_p(1);
        if (!(*adres))
        {
            //niech tylko zapisze do notesu i ide
            break;
        }
        printf("Jestem w trakcie konsumpcji ...\n");
        wait = ((double)rand()) / RAND_MAX;
        printf("Konsumpcja potrwa %lf sekund\n", wait);
        sleep(wait);
        fprintf(fop, "%c", *adres);
        printf("Skonsumowalem!\n");
        semafor_v(0); //otwieram dostep 0 - producentowi
    }

    printf("Konsument wychodzi..\n");
    //skonczylem zapisywac do notesu, producent moze skladac stragan
    odlacz_pamiec();
    semafor_v(0);

    return 0;
}