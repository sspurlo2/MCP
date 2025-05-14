#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define MAX_PROCESSES 128

void read_workload(char *filename, char ***lines_out, int *count_out) {
    static char *lines[MAX_PROCESSES];
    char buffer[MAX_LINE_LENGTH];
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        exit(1);
    }

    int count = 0;
    while (fgets(buffer, sizeof(buffer), fp) && count < MAX_PROCESSES) {
        lines[count] = strdup(buffer);
        count++;
    }
    fclose(fp);

    *lines_out = lines;
    *count_out = count;
}

    void free_workload(char **lines, int count) {
    for (int i = 0; i < count; i++) {
        free(lines[i]);
    }
}

