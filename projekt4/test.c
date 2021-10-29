#include <stdio.h>

int main()
{
    int messageSize = 10 * sizeof(char); //+ sizeof(long int);
    printf("%zu", sizeof(messageSize));
}
