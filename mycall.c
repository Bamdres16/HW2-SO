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
#include <errno.h>
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
                execvp(argv[2], argv + 2);
                printf("Error %s can't execute\n", argv[2]);
                exit(0);
            
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
                clock_t localstart, localend;
                double taken, elapsed = 0;
                while (waitpid(child, &status, 0) && !WIFEXITED(status))
                {
                    taken = 0;
                    if (!in_call)
                    {
                        localstart = clock();
                        ptrace(PTRACE_GETREGS, child, NULL, &regs);
                        
                        if (REG(regs) == -1) { 
                            printf ("Error during %s execution.\n", argv[2]);
                            exit(0);
                        }
                        calls++;
                        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                        in_call = TRUE;
                    }
                    else
                    {
                        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                        localend = clock();
                        taken = (double) (localend - localstart) / CLOCKS_PER_SEC;
                        elapsed += taken;
                        printf("%d. %s used the system call: %s ", calls, argv[2], 
                        callname(REG(regs)));
                        printf("(time taken: %f, elapsed time: %f)\n", taken,elapsed);
                        in_call = FALSE;
                    }
                }
                printf("%50s\n", "------------------- END ---------------");
                printf("Total calls: %d. Elapsed time: %lf.\n", calls, elapsed);
            }
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
