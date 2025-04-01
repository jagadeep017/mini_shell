#include "main.h"
#include "jobs.h"
#include <stdio.h>
#include <unistd.h>
extern volatile int pid;
extern struct job *job_head;
extern char prompt[30];

void signal_handler(int sig_num) {
    if (sig_num == SIGCHLD) {
        int status;
        pid_t child_pid;
        if ((child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
            printf("SIGCHLD %d,%d\n", child_pid,pid);
            struct job *curr = job_head;
            struct job *entry = NULL;
            while (curr) {
                if (curr->pid == child_pid) {
                    entry = curr;
                    curr = entry->next;
                    break;
                }
                curr = curr->next;
            }

            if (entry) {                                            //if pid is in the jobs
                if (WIFEXITED(status) || WIFSIGNALED(status)) {     //if exited or terminated by signl
                    remove_job(child_pid);
                    free(entry->command);
                    free(entry);
                } else if (WIFSTOPPED(status)) {                    //if stopped
                    insert(child_pid, entry->command, STOPPED);
                    free(entry->command);
                    free(entry);
                    printf("\n123[%d] Stopped %s\n", child_pid, entry->command);
                }
            }

            if (child_pid == pid) {
                pid = -1;
                printf("test\n");
                // if (WIFSTOPPED(status)) {
                //     printf("\n");
                // }
            }
        }
    } else if (sig_num == SIGINT) {
        if (pid != -1) {
            kill(pid, SIGINT);
        }
        else {
            printf("\n%s ",prompt);
            fflush(stdout);
        }
    } else if (sig_num == SIGTSTP) {
        if (pid != -1) {
            kill(pid, SIGTSTP);
        }
        else {
            printf("\n%s ",prompt);
            fflush(stdout);
        }
    }
}