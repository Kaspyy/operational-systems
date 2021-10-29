#include <stdio.h>
#include <signal.h>
#include <unistd.h>
void handler(int sig)
{
        printf("Sygnal SIGINT!\n");
        return;
}
int main()
{
        int i;
        signal(SIGINT, handler);
//        signal(SIGINT,SIG_IGN);
        for (i = 0; i < 10; i++)
                sleep(2);
        return 0;
}

