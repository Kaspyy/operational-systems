#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
int main()
{
    printf(" PID = %i\n PPID = %i\n UID = %i\n GID = %i\n\n", getpid(), getppid(), getuid(), getgid());
}