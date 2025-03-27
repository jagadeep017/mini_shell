#include "main.h"
#include <stdio.h>
extern int pid;
extern char prompt[30];
void signal_handler(int sig_num){
    printf("\n");
    if(pid==-1){
        printf("%s ",prompt);
        fflush(stdout);
    }
}