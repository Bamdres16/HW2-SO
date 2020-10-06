#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define FALSE 0
#define TRUE 1

char *uk = "unknown";

const char* callname(long call) {
    char buffer[100];
    int find = FALSE;
    char callnum[10];
    sprintf(callnum, "%ld", call);
    FILE *fp = fopen ("syscalls.txt", "r");
    if (fp == NULL)
    {
        perror("Error leyendo el archivo de configuracion");
        exit(EXIT_FAILURE);
    }

    while (fscanf(fp,"%s",buffer)==1) {
        if (strcmp(buffer, callnum) == 0){
            fscanf(fp,"%s",buffer);
            fscanf(fp,"%s",buffer);
            find = TRUE;
            break;
        }
    }
    fclose(fp);
    char *buffer_ptr = buffer;
    if (find == TRUE) {
        return buffer_ptr;
    }
    else {
        return uk;
    }

    
}