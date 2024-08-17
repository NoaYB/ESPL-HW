#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

int debug = 0;

void execute(cmdLine *pCmdLine)
{
    if (strcmp(pCmdLine->arguments[0], "quit") == 0) // from task 0a
    {
        _exit(0);
    }
    else
    {
        int pid = fork(); // duplicate for child and parent
        if (pid > 0)
        {
            if (pCmdLine->blocking == 1)
            {
                if (waitpid(pid, NULL, 0) == -1) // call the father to wait
                {
                    perror("the wait operation fails");
                    _exit(1);
                }
            }
        }    
        else if (pid == 0)
        {
            if (debug)
            {
                fprintf(stderr, "PID : %i\n", getpid());
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) 
                {
                    fprintf(stderr, "PATH : %s\n", cwd);
                }
                else 
                {
                    perror("getcwd() error");
                    _exit(1);
                }
                fprintf(stderr, "Executing command : %s\n", pCmdLine->arguments[0]);
            }

            // Redirection
            if (pCmdLine->inputRedirect != NULL)
            {
                int input = open(pCmdLine->inputRedirect, O_RDONLY);// READ FROM inputRedirect THE CMDLINE IN READ ONLY 
                if (input == -1)
                {
                    perror("the open operation fails");
                    _exit(1);
                }
                if (dup2(input, STDIN_FILENO) == -1) // IF THE FILE OPEN THIS LINE DUPLICATE THE STANDART INPUT TO AVOID WORKING ON ORIGINAL
                {
                    perror("the dup2 operation fails");
                    _exit(1);
                }
                close(input);
            }
            if (pCmdLine->outputRedirect != NULL)
            {
                int output = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT , 0644); // TRUNC - DONT DELETE EVERYTHING KEEP, 0644 - PERMISSION OWNER GROUP OTHERS 
                if (output == -1)
                {
                    perror("the open operation fails");
                    _exit(1);
                }
                if (dup2(output, STDOUT_FILENO) == -1)
                {
                    perror("the dup2 operation fails");
                    _exit(1);
                }
                close(output);
            }

            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
            {
                perror("the execvp operation fails");
                _exit(1);
            }
        }
        else
        {
            perror("fork operation failed");
        }
    }
}

int main(int argc, char **argv) 
{ 
    if ((argc > 1) && (strcmp(argv[1], "-d") == 0)) { debug = 1; }

    while (1)
    {
        if (debug)
        {
            fprintf(stderr, "PID : %i\n", getpid());
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) 
            {
                fprintf(stderr, " %s\n", cwd);
            }
            else 
            {
                perror("getcwd() error");
            }
        }

        char line[2048];
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            break; // fgets failed or EOF, exit the loop
        }

        cmdLine *cmdline = parseCmdLines(line);
        if (cmdline == NULL)
        {
            continue; // if parsing failed, prompt again
        }

        if (debug)
        {
            fprintf(stderr, "Executing command : %s\n", cmdline->arguments[0]);
        }
        if (strcmp(cmdline->arguments[0], "cd") == 0)
        {
            if (chdir(cmdline->arguments[1]) == -1) 
            {
                perror("cd error");
            }
            else
            {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) 
                {
                    printf("%s\n", cwd);
                } 
                else 
                {
                    perror("getcwd() error");
                }
            }
        }
        else if (strcmp(cmdline->arguments[0], "alarm") == 0)
        {
            if (kill(atoi(cmdline->arguments[1]), SIGCONT) == -1) 
            {
                perror("the alarm operation fails");
            }
            else
            {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) 
                {
                    printf("PATH : %s\n", cwd);
                } 
                else 
                {
                    perror("getcwd() error");
                }
            }
        }
        else if (strcmp(cmdline->arguments[0], "blast") == 0)
        {
            if (kill(atoi(cmdline->arguments[1]), SIGKILL) == -1) 
            {
                perror("the blast operation fails");
            }
            else
            {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) 
                {
                    printf("PATH : %s\n", cwd);
                } 
                else 
                {
                    perror("getcwd() error");
                }
            }
        }
        else
        {
            execute(cmdline);
        }
        freeCmdLines(cmdline);
    }  
    return 0;
}
