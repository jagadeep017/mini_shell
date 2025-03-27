#include "main.h"

void scan_input(char *prompt, char *input_string){
    while(1){        
        printf("%s ",prompt);
        scanf(" %[^\n]s",input_string);
        if(!strncmp("PS1",input_string,3)){
            char *temp=input_string+3;
            while(*temp){
                if(*temp==' '){
                    return ;
                }
                temp++;
            }
            if(*(input_string+3)=='='){
                strncpy(prompt,input_string+4,25);
            }
            else{
                input_string[3]='\0';
                return;
            }
            *input_string='\0';
        }
        else{
            return ;
        }
    }
}
