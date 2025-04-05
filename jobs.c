#include "jobs.h"
#include <sys/stat.h>


struct job *job_head = NULL, *job_tail = NULL;
extern int status;
extern int pid;

struct job *get_job_by_index(int index){
    if(index == -1) return job_tail;                    //if index is -1, return the last job
    struct job *temp = job_head;
    for (int i = 0; temp != NULL && i < index; i++, temp = temp->next); //get the job by index
    return temp;        //return the address of the job
}

void insert(int pid, char *command, int status) {
    struct job *new = malloc(sizeof(struct job));       //allocate memory for the new job
    new->pid = pid;                                             //set the pid
    new->command = malloc(strlen(command) + 3);        //allocate memory for the command
    strcpy(new->command, command);                   //copy the command
    new->status = status;                                       //set the status
    new->next = NULL;

    if(!job_head){                                      //if the list is empty
        job_head = new;
        job_tail = new;
    }
    else{                                               //else insert at the end
        struct job *temp = job_head;
        while(temp->next){
            temp = temp->next;
        }
        temp->next = new;
    }
    job_tail = new;                                     //update the tail
}

void update_job_status(int pid, int new_status){
    struct job *temp = job_head;
    while (temp){                               //search for the job with the given pid
        if (temp->pid == pid) {
            temp->status = new_status;          //update the status
            return;
        }
        temp = temp->next;
    }
}

void bg(int index){
    struct job *job1 = get_job_by_index(index);                 //get the job
    if(job1 && job1->status == STOPPED){
        printf("bg: job [%d] , %s resumed\n", job1->pid, job1->command);
        int len = strlen(job1->command);
        job1->command[len] = ' ';
        job1->command[len+1] = '&';             //add & at the end to indicate the job is an background job
        job1->command[len+2] = '\0';
        kill(job1->pid, SIGCONT);                               //resume the stopped job
        update_job_status(job1->pid, RUNNING);           //update the status
        status = 0;                                                      //set the status to 0
    }
    else if(!job1 && job_head){             //if index is out of range
        printf("bg: job not found: %d\n", index);
        status = 127;
    }
}

void fg(int index) {
    struct job *job1 = get_job_by_index(index);                 //get the job
    if(job1 && job1->status == STOPPED){
        pid = job1->pid;
        kill(job1->pid, SIGCONT);                               //resume the stopped job
        update_job_status(job1->pid, RUNNING);          //update the status
        printf("fg: job [%d] , %s resumed\n", job1->pid, job1->command);
        while(pid != -1){                      //wait for the job to terminate, pid is set to -1 in signal handler when child is stopped or exited
            pause();
        }
    }
    else if(!job1 && job_head){             //if index is out of range
        printf("bg: job not found: %d\n", index);
    }
}

void print_jobs() {
    struct job *temp = job_head;
    if(!temp){
        printf("empty\n");
    }
    int index = 0;
    while(temp){                //loop through the list and print the detailes of the jobs
        printf("[%d] %d %s", index, temp->pid, temp->command);
        if(temp->status == STOPPED){
            printf(" stopped\n");
        }
        else if(temp->status == RUNNING){
            printf(" running\n");
        }
        else if(temp->status == DONE){
            printf(" done\n");
        }
        temp = temp->next;
        index++;
    }
}

void remove_job(int pid){
    struct job *temp = job_head, *prev = NULL;
    while(temp){            //loop through the list and find the job
        if(temp->pid == pid){
            if(prev){
                prev->next = temp->next;
            }
            else{
                job_head = temp->next;
            }
            if(temp == job_tail){
                job_tail = prev;
            }
            free(temp->command);                        //free the allocated memory for the command
            free(temp);                                 //free the allocated memory for the job
            return;                                         //return
        }
        prev = temp;
        temp = temp->next;
    }
}