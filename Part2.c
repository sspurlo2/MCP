#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "common.h" //used for comone functions in all parts

int main(int argc, char *argv[]) {
    if (argc != 2){
        fprintf(stderr, "usage: %s <workload_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    //1. Block SIGUSR1 in the parent child using sigprocmask() <- used to block signal
    //2. prepare a sigset_t with SIGUSR1 using sigemptyset() and sigaddset()
    sigset_t sigmask; //
    sigemptyset(&sigmask); //initialized signal set pointed to by set
    sigaddset(&sigmask, SIGUSR1); //add individual signal to the set. 
    if(sigprocmask(SIG_BLOCK, &sigmask, NULL) < 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    char **lines;
    int line_count;
    read_workload(argv[1], &lines, &line_count);

    pid_t pid_array[MAX_PROCESSES];

    //read each line
    for(int i = 0; i < line_count; i++) {
        char *args[MAX_ARGS];
        int arg_count = 0;
        char *saveptr;
        char *token = strtok_r(lines[i], " \t\n", &saveptr);
        while(token && arg_count < MAX_ARGS - 1){
            args[arg_count++] = token;
            token = strtok_r(NULL, " \t\n", &saveptr);
        }
        argv[arg_count] = NULL;

        pid_t pid = fork(); //use fork right after each program is created
        if(pid < 0) {
            perror("fork failed"); 
            exit(EXIT_FAILURE);
        }
        if(pid == 0) {
            //child waits for SIGUSR1, then exec
            int sig;
            printf("child %d: waiting for SIGUSR1 \n", getpid());
            sigwait(&sigmask, &sig); //child waiting for sigusr1
            printf("child %d: received SIGUSR1. executing %s\n", getpid(), args[0]);
            execvp(args[0], args); //child now used exec call.
            perror("execvp failed"); 
            exit(EXIT_FAILURE);
        }
        pid_array[i] = pid;
    }
    //parent sends sigusr1 signal to children for unblocking 
    sleep(1); //pauses execution of program or script for 1 second
    printf("parent: sending SIGUSR1 to children\n");
    for(int i = 0; i < line_count; i++){
        kill(pid_array[i], SIGUSR1); //wake up child from sigwait()
    }

    sleep(1);
    printf("parent: sending SIGSTOP to children\n");
    for(int i = 0; i < line_count; i++){
        kill(pid_array[i], SIGSTOP); //suspend programs
    }

    sleep(1);
    printf("parent: sending SIGCONT to children\n");
    for(int i = 0; i < line_count; i++){
        kill(pid_array[i], SIGCONT); //wake up suspended processes
    }
    //wait for all children to finish
    for (int i = 0; i < line_count; i++){
        waitpid(pid_array[i], NULL, 0);
    }
    free_workload(lines, line_count); //freeing all memory
    printf("parent: all processes completed. \n");
    exit(EXIT_SUCCESS);
}