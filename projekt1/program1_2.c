#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>


int main()
{
    int i;
    for (i = 0; i < 3; i++)
    {
        switch (fork())
        {
        case -1:
            perror("fork error");
            exit(1);
        case 0:
            if (i == 0)
                printf("POTOMEK 1: ");
            else if (i == 1)
                printf("POTOMEK 2: ");
            else if (i == 2)
                printf("POTOMEK 3: ");
        }
        printf(" PID = %i\n PPID = %i\n UID = %i\n GID = %i\n\n", getpid(), getppid(), getuid(), getgid());
    }
}