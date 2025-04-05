#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
 

#define BUILTIN		1
#define EXTERNAL	2
#define NO_COMMAND  3

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//scan input command
void scan_input(char *prompt, char *input_string);

//extract first word of the command
char *get_command(char *input_string);

//function to check if command is builtin or external
int check_command_type(char *command, char **ext_cmd);

//execute builtin commands
void execute_internal_commands(char *input_string);

//signal handler for SIGINT, SIGTSTP and SIGCHLD
void signal_handler(int sig_num);

//extract external commands from the txt file
void extract_external_commands(char **external_commands);

//execute external commands
void execute_external_commands(char *input_string);

//execute one command
void exec_one_cmd(char *cmd);

//execute multiple commands
void exec_n_cmd(char *input, int count);

//parse command string by spaces
char **parse_command(char *cmd, int *argc);

#endif