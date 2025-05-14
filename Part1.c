#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 256
#define MAX_ARGS 32
#define MAX_PROCESSES 128

pid_t pid_array[MAX_PROCESSES]; // using pid_array for children processes

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <workload_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    //read program from specific file
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("error opening file");
        return EXIT_FAILURE;
    }

    char *lines[MAX_PROCESSES];
    int line_count = 0;

    // read each line and store it
    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, sizeof(buffer), fp) && line_count < MAX_PROCESSES) {
        lines[line_count] = strdup(buffer); // save line
        line_count++;
    }
    fclose(fp);

    //launch each process
    for (int i = 0; i < line_count; i++) {
        char *args[MAX_ARGS];
        int arg_count = 0;

        char *token = strtok(lines[i], " \t\n");
        while (token && arg_count < MAX_ARGS - 1) {
            args[arg_count++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[arg_count] = NULL;

        pid_array[i] = fork(); //use fork 
        if (pid_array[i] < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid_array[i] == 0) {
            // use execvp system call for child
            if (execvp(args[0], args) == -1) {
                perror("exec failed");
            }
            exit(EXIT_FAILURE); // exit upon failure
        }
    }

    // Parent waits for all children
    for (int i = 0; i < line_count; i++) {
        waitpid(pid_array[i], NULL, 0);
    }

    // Free the lines
    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }

    return EXIT_SUCCESS;
}
