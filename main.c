#include "main.h"
#include <signal.h>

int status;                 //status of the last executed command
int pid = -1;               //pid of currently running command, if no command is running it is set to -1
char prompt[30] = "\033[0;32mMiniShell $\033[0;37m", input[100], *str;    //prompt and input string

int main(){
    char *ext_cmd[200];                     //array to store external commands
    system("clear");
    extract_external_commands(ext_cmd);     //extract external commands from the file

    signal(SIGINT, signal_handler);         //register signal handlers for SIGINT, SIGTSTP and SIGCHLD
    signal(SIGTSTP, signal_handler);
    signal(SIGCHLD, signal_handler);

    while(1){

        scan_input(prompt, input);          //scan input command

        str = get_command(input);           //get first word of the command

        switch(check_command_type(str, ext_cmd)){   //check if command is builtin or external
            case BUILTIN:
                execute_internal_commands(input);   //execute builtin command
                break;
            case EXTERNAL:
                execute_external_commands(input);   //execute external command
                break;
            case NO_COMMAND:
                printf("command not found: %s\n", str); //print error message
                status = 127;                   //set status to 127(cmd not found)
                break;
        }
        
        input[0] = '\0';                        //clear input
        free(str);                         //free the allocated memory in get_command function
    }
    return 0;
}