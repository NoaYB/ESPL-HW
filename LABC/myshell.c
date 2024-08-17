
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "LineParser.h"
#include <ctype.h>

#define HISTLEN 20
#define MAX_BUF 200

int debugMode = 0;
char path[PATH_MAX];

typedef struct process
{
    cmdLine *cmd;
    pid_t pid;
    int status;
    struct process *next;
} process;

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

process *process_list = NULL;
char history[HISTLEN][MAX_BUF];
int hist_count = 0;
int hist_oldest = 0;

void print_CurrPath()
{
    if (getcwd(path, sizeof(path)) != NULL)
        printf("%s> ", path);
    else
        perror("getcwd() error");
}

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    process *new_process = (process *)malloc(sizeof(process));
    new_process->pid = pid;
    new_process->cmd = cmd;
    new_process->status = RUNNING;
    new_process->next = *process_list;
    *process_list = new_process;
}

void updateProcessStatus(process *process_list, int pid, int status)
{
    while (process_list != NULL)
    {
        if (process_list->pid == pid)
        {
            process_list->status = status;
            break;
        }
        process_list = process_list->next;
    }
}

void updateProcessList(process **process_list)
{
    process *curr_pro = *process_list;
    while (curr_pro != NULL)
    {
        int status;
        pid_t result = waitpid(curr_pro->pid, &status, WNOHANG);
        if (result != 0)
        {
            if (WIFEXITED(status) || WIFSIGNALED(status))
                curr_pro->status = TERMINATED;
        }
        curr_pro = curr_pro->next;
    }
}

void printProcessList(process **process_list)
{
    updateProcessList(process_list);
    process *curr_pro = *process_list;
    printf("PID\tCommand\t\tSTATUS\n");
    while (curr_pro != NULL)
    {
        printf("%d\t%s\t\t%s\n", curr_pro->pid, curr_pro->cmd->arguments[0],
               curr_pro->status == RUNNING ? "Running" : curr_pro->status == SUSPENDED ? "Suspended"
                                                                                       : "Terminated");
        curr_pro = curr_pro->next;
    }
}

void freeProcessList(process *process_list)
{
    while (process_list != NULL)
    {
        process *temp = process_list;
        process_list = process_list->next;
        freeCmdLines(temp->cmd);
        free(temp);
    }
}

void myAlarm(int proID)
{
    if (kill(proID, SIGCONT) == 0)
        printf("SIGCONT sent, proID %d continue.\n", proID);
    else
        perror("failed to send SIGCONT");
}

void blast(int proID)
{
    if (kill(proID, SIGKILL) == 0)
        printf("SIGKILL sent, proID %d stopped.\n", proID);
    else
        perror("failed to send SIGKILL");
}

void suspend(int proID)
{
    if (kill(proID, SIGTSTP) == 0)
        printf("SIGTSTP sent, proID %d suspended.\n", proID);
    else
        perror("failed to send SIGTSTP");
}

void executeSingleCommand(cmdLine *pCmdLine)
{
    pid_t proID = fork();
    if (proID == -1)
    {
        perror("fork failed, try again");
        exit(1);
    }
    if (proID == 0)
    {
        if (pCmdLine->inputRedirect != NULL)
            freopen(pCmdLine->inputRedirect, "r", stdin);
        if (pCmdLine->outputRedirect != NULL)
            freopen(pCmdLine->outputRedirect, "w", stdout);
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
        {
            perror("execvp failed");
            _exit(1);
        }
    }
    else
    {
        addProcess(&process_list, pCmdLine, proID);
        if (debugMode == 1)
            fprintf(stderr, "proID is: %d and the executing command is: %s \n", proID, pCmdLine->arguments[0]);
        if (pCmdLine->blocking == 1)
            waitpid(proID, NULL, 0);
    }
}

void execute(cmdLine *pCmdLine)
{
    if (strcmp(pCmdLine->arguments[0], "cd") == 0)
    {
        if (chdir(pCmdLine->arguments[1]) == -1)
            perror("dir change failed, try again");
        return;
    }
    else if (strcmp(pCmdLine->arguments[0], "alarm") == 0)
    {
        if (pCmdLine->argCount != 2)
            perror("syntax problem, send 2 param");
        else
        {
            int proIdToAlarm = atoi(pCmdLine->arguments[1]);
            myAlarm(proIdToAlarm);
            updateProcessStatus(process_list, proIdToAlarm, RUNNING);
        }
        return;
        if (strcmp(pCmdLine->arguments[0], "suspend") == 0)
        {
            if (kill(atoi(pCmdLine->arguments[1]), SIGTSTP) == -1)
            {
                perror("the kill operation fails");
                _exit(1);
            }
        }
    }
    else if (strcmp(pCmdLine->arguments[0], "blast") == 0)
    {
        if (pCmdLine->argCount != 2)
            perror("syntax problem, send 2 param");
        else
        {
            int proIdToBlast = atoi(pCmdLine->arguments[1]);
            blast(proIdToBlast);
            updateProcessStatus(process_list, proIdToBlast, TERMINATED);
        }
        return;
    }
    else if (strcmp(pCmdLine->arguments[0], "sleep") == 0)
    {
        if (pCmdLine->argCount != 2)
            perror("syntax problem, send 2 param");
        else
        {
            int proIdToSuspend = atoi(pCmdLine->arguments[1]);
            suspend(proIdToSuspend);
            updateProcessStatus(process_list, proIdToSuspend, SUSPENDED);
        }
        return;
    }
    else if (strcmp(pCmdLine->arguments[0], "procs") == 0)
    {
        printProcessList(&process_list);
        return;
    }

    if (pCmdLine->next != NULL)
    {
        // Handle piping
        int pipefd[2];
        if (pipe(pipefd) == -1)
        {
            perror("pipe is failed");
            exit(1);
        }
        pid_t cpid1 = fork();
        if (cpid1 == -1)
        {
            perror("fork is failed");
            exit(1);
        }
        // THE FIRST CHILD
        if (cpid1 == 0)
        {
            close(pipefd[0]);
            // Handle input redirection for the first command
            if (pCmdLine->inputRedirect != NULL)
            {
                freopen(pCmdLine->inputRedirect, "r", stdin);
            }
            if (dup2(pipefd[1], STDOUT_FILENO) == -1)
            {
                perror("dup2 failed");
                exit(1);
            }
            close(pipefd[1]);
            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
            {
                perror("execvp failed");
                exit(1);
            }
        }
        pid_t cpid2 = fork();
        if (cpid2 == -1)
        {
            perror("fork failed");
            exit(1);
        }
        // THE SECOND CHILD
        if (cpid2 == 0)
        {
            close(pipefd[1]);
            if (pCmdLine->next->outputRedirect != NULL)
            {
                freopen(pCmdLine->next->outputRedirect, "w", stdout);
            }
            if (dup2(pipefd[0], STDIN_FILENO) == -1)
            {
                perror("dup2 failed");
                exit(1);
            }
            close(pipefd[0]);
            if (execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments) == -1)
            {
                perror("execvp failed");
                exit(1);
            }
        }
        close(pipefd[0]);
        close(pipefd[1]);
        if (debugMode)
        {
            fprintf(stderr, "cpid1: %d, executing command: %s\n", cpid1, pCmdLine->arguments[0]);
            fprintf(stderr, "cpid2: %d, executing command: %s\n", cpid2, pCmdLine->next->arguments[0]);
        }
        waitpid(cpid1, NULL, 0);
        waitpid(cpid2, NULL, 0);
    }
    else
    {
        executeSingleCommand(pCmdLine);
    }
}

void printHistory()
{
    int count = hist_count;
    if (hist_count > HISTLEN)
        count = HISTLEN;
    for (int i = 0; i < count; i++)
    {
        int index = (hist_oldest + i) % HISTLEN;
        printf("%d %s", i + 1, history[index]);
    }
}

void execute_History(int index)
{
    int actualIndex = (hist_oldest + index - 1) % HISTLEN;
    cmdLine *pCmdLine = parseCmdLines(history[actualIndex]);
    if (pCmdLine != NULL)
    {
        execute(pCmdLine);
        freeCmdLines(pCmdLine);
    }
    else
    {
        printf("Error executing command from history.\n");
    }
}

void add_to_hist(char *input)
{
    // Check if the command to add to history should be skipped
    if (strcmp(input, "history\n") == 0 || strcmp(input, "!!\n") == 0 ||
        input[0] == '!' || (input[0] >= '0' && input[0] <= '9'))
        return;

    strncpy(history[hist_count % HISTLEN], input, MAX_BUF);
    hist_count++;
    if (hist_count > HISTLEN)
        hist_oldest = (hist_oldest + 1) % HISTLEN;
}

int main(int argc, char **argv)
{
    char input[MAX_BUF];
    cmdLine *pCmdLine;

    for (int i = 0; i < argc; i++)
        if (strcmp(argv[i], "-d") == 0)
            debugMode = 1;

    while (1)
    {
        print_CurrPath();
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            perror("fgets  an error");
            return 1;
        }

        if (input[0] == '\n')
            continue;

        add_to_hist(input);

        pCmdLine = parseCmdLines(input);
        if (pCmdLine == NULL)
        {
            continue;
        }

        if (strcmp(pCmdLine->arguments[0], "quit") == 0)
        {
            freeCmdLines(pCmdLine);
            break;
        }
        else if (strcmp(pCmdLine->arguments[0], "history") == 0)
        {
            printHistory();
        }
        else if (strcmp(pCmdLine->arguments[0], "!!") == 0)
        {
            if (hist_count == 0)
                printf("No commands in history.\n");
            else
                execute_History(hist_count > HISTLEN ? HISTLEN : hist_count - 1);
        }
        else if (pCmdLine->arguments[0][0] == '!' && isdigit(pCmdLine->arguments[0][1]))
        {
            int index = atoi(&pCmdLine->arguments[0][1]);
            if (index < 1 || index > HISTLEN || index > hist_count)
                printf("No such command in history.\n");
            else
                execute_History(index);
        }
        else
            execute(pCmdLine);
    }

    freeProcessList(process_list);
    return 0;
}