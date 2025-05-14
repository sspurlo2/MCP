#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "common.h" //used for comone functions in all parts

pid_t pid_array[MAX_PROCESSES];       // child PIDs
bool is_alive[MAX_PROCESSES];         // status of each child
int num_processes;                    // total workload lines
int current_index = -1;

void alarm_handler(int sig) {
    //stop current alive process, if there is one
    (void)sig;
    if(current_index != -1 && is_alive[current_index]){
        kill(pid_array[current_index], SIGSTOP);
        printf("scheduler: stopped PID %d\n", pid_array[current_index]);
    }

    //move to the next alive process
    int next_index = -1;
    for(int i = 0; i < num_processes; i++){
        int idx = (current_index + i) % num_processes;
        if(is_alive[idx]) {
            next_index = idx;
            break;
        }
    }
    if(next_index != -1){
        current_index = next_index;
        kill(pid_array[current_index], SIGCONT);
        printf("scheduler: resumed PID %d\n", pid_array[current_index]);
        alarm(1); //resetting for next time slice
    } else{
        printf("scheduler: no alive processes to schedule\n");
    }
}

int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr, "usage: %s <workload_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigmask, NULL);

    //SIGALRM handling
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    char **lines;
    read_workload(argv[1], &lines, &num_processes);

    for(int i = 0; i < num_processes; i++){
        char *args[MAX_ARGS];
        int arg_count = 0;
        char *token = strtok(lines[i], " \t\n");
        while(token && arg_count < MAX_ARGS - 1){
            args[arg_count++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[arg_count] = NULL;

        pid_t pid = fork();
        if(pid < 0){
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if(pid == 0){
            int sig;
            sigwait(&sigmask, &sig); //wait for SIGUSR1
            execvp(args[0], args);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
   pid_array[i] = pid;
        is_alive[i] = true;
    }

    // send SIGUSR1 to start all children
    for(int i = 0; i < num_processes; i++){
        kill(pid_array[i], SIGUSR1);
    }

    // stop all children initially
    for (int i = 0; i < num_processes; i++) {
        kill(pid_array[i], SIGSTOP);
    }

    // start scheduling
    current_index = 0;
    kill(pid_array[current_index], SIGCONT);
    printf("scheduler: started PID %d\n", pid_array[current_index]);
    alarm(1);

    int alive_count = num_processes;
    while(alive_count > 0){
        for(int i = 0; i < num_processes; i++){
            if(is_alive[i]){
                int status;
                pid_t result = waitpid(pid_array[i], &status, WNOHANG);
                if (result == pid_array[i] && WIFEXITED(status)) {
                    is_alive[i] = false;
                    alive_count--;
                    printf("scheduler: PID %d has exited.\n", pid_array[i]);
                }
            }
        }
        pause(); // wait for SIGALRM
    }

    free_workload(lines, num_processes);
    printf("scheduler: all processes complete.\n");
    return 0;
}