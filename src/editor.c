#define _POSIX_C_SOURCE 200809L
#include "editor.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

const char *editors[] = {
    "vi",
    "nano",
    "vim",
    "nvim",
    "emacs",
    NULL
};

char* find_editor() {
    char *editor = getenv("EDITOR");
    if (editor && strlen(editor) > 0) {
        return strdup(editor);
    }

    editor = getenv("VISUAL");
    if (editor && strlen(editor) > 0) {
        return strdup(editor);
    }

    /**/
    /* Check for known editors in PATH */
    /**/
    for (int i = 0; editors[i] != NULL; i++) {
        char command[256];
        snprintf(command, sizeof(command), "which %s > /dev/null 2>&1", editors[i]);
        if (system(command) == 0) {
            return strdup(editors[i]);
        }
    }

    return NULL;
}

int open_editor(const char *filepath) {
    char *editor = find_editor();
    if (!editor) {
        return -1; /* No editor found */
    }

    pid_t pid = fork();
    if (pid == -1) {
        free(editor);
        return -1; /* Fork failed */
    }

    if (pid == 0) {
        /**/
        /* Child process */
        /**/
        execlp(editor, editor, filepath, NULL);
        /**/
        /* execlp only returns on error */
        /**/
        exit(127);
    } else {
        /**/
        /* Parent process */
        /**/
        int status;
        waitpid(pid, &status, 0);
        free(editor);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return 0; /* Editor exited successfully */
        } else {
            return -1; /* Editor exited with an error */
        }
    }
}
