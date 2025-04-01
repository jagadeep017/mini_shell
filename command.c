#include "jobs.h"
#include "main.h"
#include <stdio.h>

extern int status;
extern volatile int pid;
extern struct job *job_head;

char *get_command(char *input_string){
    int i=0;
    for(i=0;input_string[i]!='\0';i++){
        if(input_string[i]==' '){
            break;
        }
    }
    char *command=(char *)malloc(i+1);
    strncpy(command,input_string,i);
    command[i]='\0';
    return command;
}

int check_command_type(char *command,char **ext_cmd){
    char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
        "set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
        "exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", 
        "jobs", "bg", "fg", NULL};
    for(int i=0; builtins[i]!=NULL; i++){
        if(strcmp(command, builtins[i])==0){
            return BUILTIN;
        }
    }
    for(int i=0; ext_cmd[i]!=NULL; i++){
        if(strcmp(command, ext_cmd[i])==0){
            return EXTERNAL;
        }
    }

    return NO_COMMAND;
}

void extract_external_commands(char **external_commands){
    int fd=open("ext_cmd.txt",O_RDONLY);
    char buff_arr[30],buff;
    int i=0,index=0;
    while(read(fd,&buff,sizeof(buff))>0){
        if(buff=='\n'){
            buff_arr[i]='\0';
            external_commands[index]=(char *)malloc(i+1);
            strcpy(external_commands[index],buff_arr);
            index++;
            i=0;
        }
        else{
            buff_arr[i]=buff;
            i++;
        }
    }close(fd);
}


void execute_internal_commands(char *input_string){
    if(!strcmp(input_string,"exit")){
        exit(0);
    }
    else if(!strcmp(input_string,"pwd")){
        char cwd[100];
        getcwd(cwd,sizeof(cwd));
        printf("%s\n",cwd);
        status=0;
    }
    else if(!strncmp(input_string,"cd ",3)||!strcmp(input_string,"cd")){
        if(strcmp(input_string+3,"\0")==0||!strcmp(input_string,"cd")){
            chdir(getenv("HOME"));
        }
        else if(chdir(input_string+3)<0){
            printf("cd: %s: No such file or directory\n",input_string+3);
        }
        status=0;
    }
    else if(!strncmp(input_string,"echo ",5)){
        if(strcmp(input_string+5,"$SHELL")==0){
            printf("%s\n",getenv("SHELL"));
        }
        else if(strcmp(input_string+5,"$$")==0){
            printf("%d\n",getpid());
        }
        else if(strcmp(input_string+5,"$?")==0){
            printf("%d\n",status);
        }
        status=0;
    }
    else if(!strcmp(input_string,"jobs")){
        print_jobs();
        status=0;
    }
    else if (!strncmp(input_string, "fg", 2)) {
        if (!input_string[2]) {
            fg(-1);             // fg -1 means fg the last job
        } else {
            int index = atoi(input_string + 3);
            fg(index);                  //when index is given
        }
    } else if (!strncmp(input_string, "bg", 2)) {
        if (!input_string[2]) {
            bg(-1);             // bg -1 means bg the last job
        } else {
            int index = atoi(input_string + 3);
            bg(index);                  //when index is given
        }
    }
}

pid_t exec_one_cmd(char *cmd) {
    int bg = 0;
    int argc = 0;
    char **argv=parse_command(cmd, &argc);
    if (argc > 0 && strcmp(argv[argc-1], "&") == 0) {
        bg = 1;
        argv[--argc] = NULL;
    }
    argv[argc] = NULL;

    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        exit(1);
    } else if (pid > 0) {
        if (bg) {
            insert(pid, cmd, RUNNING);
            return pid;
        } else {
            int child_status;
            do {
                waitpid(pid, &child_status, WUNTRACED);
                if (WIFSTOPPED(child_status)) {
                    insert(pid, cmd, STOPPED);                  //inserting into jobs list
                    printf("\n[%d] Stopped %s\n", pid, cmd);
                    pid = -1; // Reset PID
                    break;
                }
            } while (!WIFEXITED(child_status) && !WIFSIGNALED(child_status)&&pid!=-1);

            status = WEXITSTATUS(child_status);
            return -1;
        }
    }
    return -1;
}


void execute_external_commands(char *input_string){
    char *cmd=malloc(strlen(input_string)+1);
    strcpy(cmd,input_string);
    int count=0;                //pipe count
    for(int i=0;cmd[i]!='\0';i++){
        if(cmd[i]=='|'){
            count++;
        }
    }
    if(count==0){               //no pipe
        exec_one_cmd(cmd);
    }
    else if(count<100){
        exec_n_cmd(cmd,count);
    }
    else{
        printf("excess no of pipes,limit:-300\n");
        status=1;
    }
    free(cmd);
}

void exec_n_cmd(char *input, int count) {
    int stdout_fd = dup(STDOUT_FILENO);
    int stdin_fd = dup(STDIN_FILENO);
    char *pipes[count + 2];
    int index = 1;

    pipes[0] = input;
    for (int i = 0; input[i]; i++) {
        if (input[i] == '|') {
            if (input[i+1] == ' ') {
                pipes[index++] = input + i + 2;
            } else {
                pipes[index++] = input + i + 1;
            }
            input[i] = '\0';
        }
    }
    pipes[index] = NULL;

    int pfd[count][2];
    pid_t pids[count + 1];
    int background = 0;

    char *last_cmd = pipes[count];
    if (last_cmd) {
        char *amp = strrchr(last_cmd, '&');
        if (amp && *(amp + 1) == '\0') {
            *amp = '\0';
            background = 1;
        }
    }

    for (int i = 0; i < count; i++) {
        if (pipe(pfd[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < count + 1; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pids[i] == 0) {
            if (i > 0) {
                dup2(pfd[i-1][0], STDIN_FILENO);
            }

            if (i < count) {
                dup2(pfd[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < count; j++) {
                close(pfd[j][0]);
                close(pfd[j][1]);
            }

            char **argv = parse_command(pipes[i], NULL);
            execvp(argv[0], argv);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < count; i++) {
        close(pfd[i][0]);
        close(pfd[i][1]);
    }

    if (!background) {
        for (int i = 0; i < count + 1; i++) {
            waitpid(pids[i], &status, 0);
        }
    } else {
        for (int i = 0; i < count + 1; i++) {
            insert(pids[i], pipes[i], RUNNING);
        }
    }

    dup2(stdout_fd, STDOUT_FILENO);
    dup2(stdin_fd, STDIN_FILENO);
    close(stdout_fd);
    close(stdin_fd);
}

char **parse_command(char *cmd, int *argc) {
    char **argv = malloc(64 * sizeof(char *));
    char *token;
    int i = 0;
    token = strtok(cmd, " ");
    while (token != NULL) {
        argv[i++] = token;
        token = strtok(NULL, " ");
    }
    argv[i] = NULL;
    if(argc){
        *argc = i;
    }
    return argv;
}
