#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("This is a test file\n");
    printf("%s\n", argv[1]);

    return 0;
}