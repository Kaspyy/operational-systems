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

char losuj_znak()
{
        char c;
        srand((int)getpid());

        //znaki ascii drukowalne od 32-126
        c = rand() % (126 - 32) + 32;
        return c;
}

int main(int argc, char *argv[])
{
        fprintf(stdout, "Producent.\n");
        int liczba_znakow;
        char *end;

        liczba_znakow = strtol(argv[1], &end, 10);
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

        //printf("%d\n", liczba_znakow);

        char plik_wejsciowy[32];
        char znak;
        int potok;

        sprintf(plik_wejsciowy, "we_%ld.txt", (long int)getpid());
        //otwieramy plik wejsciowy
        FILE *fp = fopen(plik_wejsciowy, "w");
        if (!fp)
        {
                printf("Błąd otwarcia pliku z danymi!\n");
                exit(1);
        }
        //otwieramy potok
        potok = open(NAZWA_FIFO, O_WRONLY);
        if (potok == -1)
        {
                printf("Nie mozna otworzyc potoku\n");
                exit(EXIT_FAILURE);
        }
        //zapis
        for (int i = 0; i < liczba_znakow; i++)
        {
                znak = losuj_znak();
                fprintf(fp, "%c", znak); //zapis do pliku

                if (write(potok, &znak, sizeof(char)) == -1)
                {
                        printf("blad zapisu do potoku\n");
                        exit(EXIT_FAILURE);
                }
        }

        close(potok);
        fclose(fp);

        return 0;
}
