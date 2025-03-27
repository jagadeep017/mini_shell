#include "main.h"
#include <fcntl.h>

extern int status,pid;

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
        "exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", NULL};
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
    }
}


void execute_internal_commands(char *input_string){
    if(!strcmp(input_string,"exit")){
        exit(0);
    }
    else if(!strcmp(input_string,"pwd")){
        char cwd[100];
        getcwd(cwd,sizeof(cwd));
        printf("%s\n",cwd);
    }
    else if(!strncmp(input_string,"cd ",3)||!strcmp(input_string,"cd")){
        if(strcmp(input_string+3,"\0")==0||!strcmp(input_string,"cd")){
            chdir(getenv("HOME"));
        }
        else if(chdir(input_string+3)<0){
            printf("cd: %s: No such file or directory\n",input_string+3);
        }
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
    }
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
        char *argv[count+2];
        int index=0;
        argv[index++]=cmd;
        for(int i=0;cmd[i];i++){
            if(cmd[i]==' '){
                cmd[i]='\0';
                if(cmd[i+1]!=' '&&cmd[i+1]!='\0'){
                    argv[index++]=cmd+i+1;
                }
            }
        }
        argv[index]=NULL;
        int pid=fork();
        if(pid==0){
            execvp(argv[0],argv);
        }
        else if(pid>0){
            wait(NULL);
        }
    }
    else{
        exec_n_cmd(cmd,count);
    }
    free(cmd);
}

void exec_n_cmd(char *input,int count){

}
