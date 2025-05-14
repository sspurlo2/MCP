#ifndef COMMON_H
#define COMMON_H

#define MAX_PROCESSES 128
#define MAX_LINE_LENGTH 256
#define MAX_ARGS 32

void read_workload(const char *filename, char ***lines, int *count);
void free_workload(char **lines, int count);
void print_proc_info(pid_t pid); //used for part 4

#endif
