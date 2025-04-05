#include "jobs.h"
#include "main.h"
#include <stdio.h>

extern int status;
extern int pid;
extern struct job *job_head;
extern char input[100];

char *get_command(char *input_string){
    int i = 0;
    for(i = 0; input_string[i] != '\0'; i++){   //find size of first word
        if(input_string[i] == ' '){
            break;
        }
    }
    char *command = (char *)malloc(i+1);    //allocate memory
    strncpy(command, input_string, i);      //copy first word
    command[i] = '\0';                      //null terminate
    return command;                         //return first word
}

int check_command_type(char *command, char **ext_cmd){
    //2d array of builtin commands
    char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
        "set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
        "exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", 
        "jobs", "bg", "fg", NULL};
    
    //check if command is builtin
    for(int i = 0; builtins[i] != NULL; i++){
        if(strcmp(command, builtins[i]) == 0){  //if command is builtin
            return BUILTIN;                             
        }
    }

    //check if command is external
    for(int i = 0; ext_cmd[i] != NULL; i++){
        if(strcmp(command, ext_cmd[i]) == 0){   //if command is external
            return EXTERNAL;
        }
    }

    //if command is not builtin or external
    return NO_COMMAND;
}

void extract_external_commands(char **external_commands){
    //open file containing external commands
    int fd = open("ext_cmd.txt", O_RDONLY);
    char buff_arr[30], buff;    //buffers
    int i = 0, index = 0;
    while(read(fd, &buff, sizeof(buff))>0){
        if(buff == '\n'){       //if newline
            buff_arr[i] = '\0';
            external_commands[index] = (char *)malloc(i + 1);   //allocate memory
            strcpy(external_commands[index], buff_arr);     //copy command
            index++;
            i = 0;
        }
        else{
            buff_arr[i] = buff; //store command till newline
            i++;
        }
    }
    close(fd);                  //close file
}


void execute_internal_commands(char *input_string){
    if(!strcmp(input_string, "exit")){          //if command is exit
        exit(EXIT_SUCCESS);
    }
    else if(!strcmp(input_string, "pwd")){      //if command is pwd
        char cwd[100];              //buffer
        getcwd(cwd, sizeof(cwd));           //get current working directory into buffer
        printf("%s\n", cwd);                    //print current working directory
        status = 0;
    }
    else if(!strncmp(input_string, "cd ", 3) || !strcmp(input_string, "cd")){    //if command is cd
        if(strcmp(input_string+3,"\0")==0||!strcmp(input_string,"cd")){     //if no directory is given, goto home
            chdir(getenv("HOME"));
        }
        else if(chdir(input_string + 3) < 0){                                       //change directory did not work
            printf("cd: %s: No such file or directory\n", input_string + 3);        //print error message
            status = 1;
        }
        else{
            status = 0;
        }
    }
    else if(!strncmp(input_string, "echo ", 5)){            //if command is echo
        if(strcmp(input_string + 5, "$SHELL") == 0){            //if $SHELL is given print shell path
            printf("%s\n", getenv("SHELL"));
        }
        else if(strcmp(input_string + 5, "$$") == 0){            //if $$ is given print pid of shell
            printf("%d\n", getpid());
        }
        else if(strcmp(input_string + 5, "$?") == 0){            //if $? is given print exit status of last command
            printf("%d\n", status);
        }
        status = 0;
    }
    else if(!strcmp(input_string, "jobs")){                     //if command is jobs
        print_jobs();                                                   //print all jobs
        status = 0;
    }
    else if (!strncmp(input_string, "fg", 2)){                  //if command is fg
        if (!input_string[2]){
            fg(-1);             // fg -1 means fg the last job
        }
        else{
            int index = atoi(input_string + 3);     //get index
            fg(index);                  //when index is given
        }
    }
    else if (!strncmp(input_string, "bg", 2)){                  //if command is bg
        if (!input_string[2]){
            bg(-1);             // bg -1 means bg the last job
        }
        else{
            int index = atoi(input_string + 3);     //get index
            bg(index);                  //when index is given
        }
    }
}

void exec_one_cmd(char *cmd) {
    int bg = 0;
    int argc = 0;
    char **argv = parse_command(cmd, &argc);
    if (argc > 0 && strcmp(argv[argc - 1], "&") == 0){      //check if the last argument is &
        bg = 1;                                                      //set bg flag to 1
        argv[--argc] = NULL;
    }
    argv[argc] = NULL;

    pid = fork();                                           //create a child process
    if (pid == 0) {                                         //in child
        execvp(argv[0], argv);                          //execute the command
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0){                                      //in parent
        if (bg){                                            //if bg flag is set
            insert(pid, input, RUNNING);            //insert the pid into the jobs list
            printf("[%d] %s\n", pid, input);                 //print the pid
        }
        else{
            while(pid != -1){                       //pid is set to -1 in signal handler when child is stopped or exited
                pause();
            }                                      //wait for the child to terminate
        }
    }
    free(argv);                                 //free the allocated memory in parse_command function
}


void execute_external_commands(char *input_string){
    char *cmd = malloc(strlen(input_string) + 1);
    strcpy(cmd, input_string);
    int count = 0;                //pipe count
    for(int i = 0; cmd[i] != '\0'; i++){            //counting the number of pipes
        if(cmd[i] == '|'){
            count++;
        }
    }
    if(count == 0){               //no pipe
        exec_one_cmd(cmd);
    }
    else if(count < 100){           //pipes
        exec_n_cmd(cmd, count);
    }
    else{                           //excess no of pipes
        printf("excess no of pipes,limit:- 100\n");
        status = 1;
    }
    free(cmd);                                          //free the allocated memory
}

void exec_n_cmd(char *input, int count) {
    int stdout_fd = dup(STDOUT_FILENO);                    //dup the stdout
    int stdin_fd = dup(STDIN_FILENO);                      //dup the stdin
    char *pipes[count + 2];                                  //array to store address of the commands
    int index = 1;

    pipes[0] = input;                                       //store the first command
    for(int i = 0; input[i]; i++){                          //store the rest of the commands
        if(input[i] == '|'){
            if(input[i+1] == ' '){
                pipes[index++] = input + i + 2;
            }
            else{
                pipes[index++] = input + i + 1;
            }
            input[i] = '\0';
        }
    }
    pipes[index] = NULL;                                    //ending with NULL

    int pfd[count][2];                                      //count pipe file descriptors for count + 1 commnads
    pid_t pids[count + 1];                                  //array to store the pids of count commands

    for(int i = 0; i < count; i++){
        if(pipe(pfd[i]) == -1){                     //create count pipes
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < count + 1; i++){                     //loop for count + 1 commands
        pids[i] = fork();                                   //create a child process
        if(pids[i] == -1){
            perror("fork");
            exit(EXIT_FAILURE);                     //exit if fork fails
        }

        if(pids[i] == 0){                                   //in child process
            if(i > 0){                                       //if not the first command
                dup2(pfd[i-1][0], STDIN_FILENO);    //dup pipefd[i-1][0] to stdin
            }

            if(i < count){                                  //if not the last command
                dup2(pfd[i][1], STDOUT_FILENO);     //dup pipefd[i][1] to stdout
            }

            for(int j = 0; j < count; j++){
                close(pfd[j][0]);                       //close unused pipefd in child
                close(pfd[j][1]);
            }

            char **argv = parse_command(pipes[i], NULL);    //parse the command
            execvp(argv[0], argv);                              //execute the command
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < count; i++){
        close(pfd[i][0]);               //close all the pipes in parent
        close(pfd[i][1]);
    }

    for(int i = 0; i < count + 1; i++){    //wait for all the commands to terminate
        waitpid(pids[i], &status, 0);
    }

    dup2(stdout_fd, STDOUT_FILENO); //restore the original stdout
    dup2(stdin_fd, STDIN_FILENO);   //restore the original stdin
    close(stdout_fd);
    close(stdin_fd);
}

char **parse_command(char *cmd, int *argc){
    char **argv = malloc(64 * sizeof(char *));      //array to store the commands of size 64
    char *token;
    int i = 0;
    token = strtok(cmd, " ");                   //tokenize the command string by spaces
    while(token != NULL){
        argv[i++] = token;
        token = strtok(NULL, " ");
    }
    argv[i] = NULL;
    if(argc){                                           //if argc is not null
        *argc = i;                                      //store the number of arguments
    }
    return argv;                                        //return the parsed command
}
