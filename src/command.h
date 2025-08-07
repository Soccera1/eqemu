#ifndef COMMAND_H
#define COMMAND_H

/**/
/* Executes a shell command and returns its standard output as a string. */
/* The caller is responsible for freeing the returned string. */
/**/
char* execute_command(const char *command);

#endif /**/ /* COMMAND_H */
