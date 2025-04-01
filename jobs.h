#ifndef JOBS_H
#define JOBS_H
#include "main.h"

struct job{
    int pid;
    char *command;
    int status;
    struct job *next;
};

enum status{
    RUNNING,
    STOPPED,
    DONE
};

void insert(int pid,char *input,int status);
void remove_job(int pid);
void print_jobs();
void free_job_list(struct job *job);
void update_job_status(int pid,int status);
void bg(int index);
void fg(int index);
#endif