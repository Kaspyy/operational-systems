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

int liczba_producentow, liczba_znakow;

void spr_argumentow(int argc, char *argv[]);
void create_sb(int liczba_producentow, char *ciag);

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

        create_sb(liczba_producentow, argv[2]);

        for (int i = 0; i < liczba_producentow; i++)
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

        if (argc != 3)
        {
                printf("Nieprawidłowa liczba argumentow wywolania!\n");
                exit(EXIT_FAILURE);
        }

        char *end;

        liczba_producentow = strtol(argv[1], &end, 10);
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

        liczba_znakow = strtol(argv[2], &end, 10);
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

        if (liczba_producentow < 1 || liczba_znakow < 1)
        {
                printf("Zbyt malo %s.\n", liczba_producentow < 1 ? (liczba_znakow < 1 ? "producentow i znakow" : "producentow") : "znakow");
                exit(EXIT_FAILURE);
        }

        if (liczba_producentow > spr_limit())
        {
                printf("Proszę podac mniejsza ilosc procesow");
                exit(EXIT_FAILURE);
        }
}

void create_sb(int liczba_producentow, char *ciag)
{

        for (int i = 0; i < liczba_producentow; i++)
        {
                switch (fork())
                {
                case -1:
                        perror("Nieprawidlowe wywolanie fork()\n");
                        exit(EXIT_FAILURE);

                case 0: //jezeli proces jest procesem potomnym

                        if (execl("./producent2", "./producent2", ciag, NULL) == -1)
                        {
                                perror("blad execl\n");
                                exit(EXIT_FAILURE);
                        }
                }
        }
}
