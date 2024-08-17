#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pid_t cpid__1, cpid__2;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    cpid__1 = fork();
    if (cpid__1 == -1) {
        perror("fork");
        exit(1);
    }

    if (cpid__1 == 0) {
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");
        close(pipefd[0]);  
        dup2(pipefd[1], 1); 
        close(pipefd[1]); 
        fprintf(stderr, "(child1>going to execute cmd: ls -l)\n");
        execvp("ls", (char *[]){"ls", "-l", NULL});
        perror("execvp");  
        exit(1);
    } else {
        fprintf(stderr, "(parent_process>created process with id: %d)\n", cpid__1);
        close(pipefd[1]);  
    }

    cpid__2 = fork();
    if (cpid__2 == -1) {
        perror("error-  second child");
        exit(1);
    }

    if (cpid__2 == 0) {
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");
        dup2(pipefd[0], 0);
        close(pipefd[0]);
        fprintf(stderr, "(child2>going to execute cmd: tail -n 2)\n");
        execvp("tail", (char *[]){"tail", "-n", "2", NULL});
        perror("execvp");  
        exit(1);
    } else {
        fprintf(stderr, "(parent_process>created process with id: %d)\n", cpid__2);
        close(pipefd[0]);  
    }

    fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");
    waitpid(cpid__1, NULL, 0);
    waitpid(cpid__2, NULL, 0);

    fprintf(stderr, "(parent_process>exiting...)\n");
    return 0;
}
