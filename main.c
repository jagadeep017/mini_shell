#include "main.h"
#include <signal.h>

int status,pid=-1;
char prompt[30]="\033[0;32mMiniShell $\033[0;37m",input[50],*str;
int main(){
    char *ext_cmd[200];
    system("clear");
    extract_external_commands(ext_cmd);
    signal(SIGINT,signal_handler);
    signal(SIGTSTP,signal_handler);
    while(1){

        scan_input(prompt, input);

        str=get_command(input);

        switch(check_command_type(str,ext_cmd)){
            case BUILTIN:
                execute_internal_commands(input);
                break;
            case EXTERNAL:
                execute_external_commands(input);
                break;
            case NO_COMMAND:
                printf("command not found: %s\n",str);
                break;
        }
        
        input[0]='\0';
        free(str);
    }
    return 0;
}