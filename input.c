#include "main.h"

void scan_input(char *prompt, char *input_string){
    while(1){        
        printf("%s ", prompt);                      //print prompt
        scanf(" %99[^\n]s", input_string);          //scan command
        if(!strncmp("PS1", input_string, 3)){       //if command is PS1
            char *temp = input_string + 3;
            while(*temp){
                if(*temp == ' '){                       //if space is found
                    return;
                }
                temp++;
            }
            if(*(input_string+3) == '='){               //if = is present after PS1
                strncpy(prompt, input_string + 4, 25);  //change prompt
            }
            else{                                   
                return;
            }
            *input_string = '\0';
        }
        else{
            return;
        }
    }
}
