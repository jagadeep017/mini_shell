#include "main.h"
#include "jobs.h"
#include <stdio.h>
#include <unistd.h>

extern int pid, status;
extern struct job *job_head;
extern char prompt[30],input[100];

void signal_handler(int sig_num){
    if(sig_num == SIGCHLD){                                                 //if a child is stopped or exited
        int c_status;
        pid_t child_pid;
        if((child_pid = waitpid(-1, &c_status, WNOHANG | WUNTRACED)) > 0){    //get the pid of the stopped or exited child with exit c_status
            struct job *curr = job_head;
            struct job *entry = NULL;
            while(curr){                                    //check if the pid is in the jobs list
                if(curr->pid == child_pid){
                    entry = curr;
                    curr = entry->next;
                    break;
                }
                curr = curr->next;
            }
            if(entry){                                            //if pid is in the jobs list
                if(WIFEXITED(c_status) || WIFSIGNALED(c_status)){      //if exited or terminated by signl
                    char *temp = strstr(entry->command, "&");
                    if(temp){                                       //if the job is in the background
                        *(temp - 1) = '\0';
                        printf("\n[%d] done\t%s\n%s ", child_pid, entry->command, prompt);   //print about the job exiting and the prompt
                        fflush(stdout);                     //flush the stdout
                    }
                    else{                                           //if the job is in the foreground
                        printf("[%d] done\t%s\n", child_pid, entry->command);                //print about the job exiting
                    }
                    status = c_status;
                    remove_job(child_pid);                      //remove the job from the list
                }
                else if(WIFSTOPPED(c_status)){                        //if stopped during fg or bg
                    update_job_status(child_pid, STOPPED);      //update the job status
                    printf("\nup[%d] Stopped\t%s\n", child_pid, entry->command);
                    status = 148;
                }
            }
            else{                                               //if pid is not in the jobs list
                if(WIFSTOPPED(c_status)){
                    insert(child_pid, input, STOPPED);      //insert the stopped child into the jobs list
                    printf("[%d] Stopped\t%s\n", child_pid, input);    
                }
            }
            if(child_pid == pid){
                pid = -1;                                       //set the pid to -1 if the stopped child is the current foreground process
                if(WIFEXITED(c_status) || WIFSIGNALED(c_status)){
                    status = WEXITSTATUS(c_status);
                }
                else if(WIFSTOPPED(c_status)){
                    status = 148;
                }
            }
        }
    }
    else if(sig_num == SIGINT){                                 //if ctrl+c is pressed
        if(pid != -1){                                          //if a command is running
            printf("\n");
        }
        else{                                                   //if no command is running
            printf("\n%s ",prompt);
            fflush(stdout);
        }
    }
    else if(sig_num == SIGTSTP){                                //if ctrl+z is pressed
        if(pid != -1){                                          //if a command is running
            printf("\n");
        }
        else{                                                   //if no command is running
            printf("\n%s ",prompt);
            fflush(stdout);
        }
    }
}