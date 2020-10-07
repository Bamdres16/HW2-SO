#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <time.h>
#include "callname.c"
#define TRUE 1

#if __WORDSIZE == 64
#define REG(reg) reg.orig_rax
/*
#else
#define REG(reg) reg.orig_eax
*/
#endif

char *get_time()
{
    time_t mytime = time(NULL);
    char *time_str = ctime(&mytime);
    time_str[strlen(time_str) - 1] = '\0';
    return time_str;
}

int main(int argc, char *argv[])
{
    if (argc >= 3)
    {
        struct stat sb;
        char exec[1000];
        strcpy(exec, argv[2]);
        strcat(exec, " ");
        // Add executable parameters
        for (int i = 3; i < argc; i++)
        {
            strcat(exec, argv[i]);
            strcat(exec, " ");
        }

        char cmd[1000];

        // ==================== INTERACTIVE MODE ====================
        if (strcmp(argv[1], "-i") == 0)
        {

            pid_t child;
            child = fork();
            int calls = 0;

            if (child == 0)
            {
                ptrace(PTRACE_TRACEME, 0, NULL, NULL);
                if (execvp(argv[2], argv + 2) == 0) {
                    printf("Error %s can't execute", argv[2]);
                    exit(0);
                }
            }
            else if (child == -1)
            {
                perror("fork");
                exit;
            }
            else
            {
                int status;
                struct user_regs_struct regs;
                int in_call = FALSE;
                while (waitpid(child, &status, 0) && !WIFEXITED(status))
                {
                    if (!in_call)
                    {
                        ptrace(PTRACE_GETREGS, child, NULL, &regs);
                        printf("[%s] the program used the system call:  %s\n", get_time(), callname(REG(regs)));
                        calls++;
                        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                        in_call = TRUE;
                    }
                    else
                    {
                        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                        in_call = FALSE;
                    }
                }
            }
            printf("%50s\n", "------------------- END ---------------");
            printf("Total calls: %d\n", calls);
        }
        // ==================== AUTOMATIC MODE ====================
        else if (strcmp(argv[1], "-a") == 0)
        {
            strcpy(cmd, "strace -c ");
            strcat(cmd, exec);
            system(cmd);
        }
        else
        {
            printf("Invalid mode. Please select interactive [-i] or automatic [-a] mode.\n");
        }
    }
    else
    {
        printf("Too few arguments. You must specify the executable and the mode.\n");
    }
    return 0;
}
