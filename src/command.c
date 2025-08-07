#define _POSIX_C_SOURCE 200809L
#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* execute_command(const char *command) {
    FILE *fp;
    char buffer[1024];
    char *result = NULL;
    size_t result_size = 0;

    fp = popen(command, "r");
    if (fp == NULL) {
        return NULL;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t buffer_len = strlen(buffer);
        char *new_result = realloc(result, result_size + buffer_len + 1);
        if (new_result == NULL) {
            free(result);
            pclose(fp);
            return NULL;
        }
        result = new_result;
        strcpy(result + result_size, buffer);
        result_size += buffer_len;
    }

    pclose(fp);
    return result;
}
