#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void sighandler(int signum, siginfo_t *info, void *ptr)
{
        printf("Sygnal SIGINT!\n");
        return;
}
int main(int argc, char *argv[])
{
	int i;
        struct sigaction sa;
        sa.sa_sigaction = sighandler;
//        sa.sa_flags = SA_SIGINFO;
        sigaction(SIGINT, &sa, NULL);
		for (i = 0; i < 10; i++) {
                sleep(2);
        }
        return 0;
}
