#ifndef JOBS_H
#define JOBS_H
#include "main.h"

//struct to store the details of a job
struct job{
    int pid;
    char *command;
    int status;
    struct job *next;
};

//enum to store the status of a job
enum status{
    RUNNING,
    STOPPED,
    DONE
};

//inserts the pid(with extra details) of stopped childs into the jobs list
void insert(int pid, char *input, int status);

//removes the completed job from the list
void remove_job(int pid);

//prints all jobs with their pid and status
void print_jobs();

//updates the status of the job
void update_job_status(int pid, int status);

//continue a stopped job(in the background) based on the index, if index = -1 then continue the last stopped job
void bg(int index);

//continue a stopped job(in the foreground) based on the index, if index = -1 then continue the last stopped job
void fg(int index);

#endif