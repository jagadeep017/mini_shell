#include "jobs.h"
#include <sys/stat.h>


struct job *job_head = NULL,*job_tail = NULL;
extern int status;
extern volatile int pid;

struct job *get_job_by_index(int index) {
    struct job *temp = job_head;
    for (int i = 0; temp != NULL && i < index; i++, temp = temp->next);
    return temp;
}

void insert(int pid, char *command, int status) {
    struct job *new = malloc(sizeof(struct job));
    new->pid = pid;
    new->command = strdup(command);
    new->status = status;
    new->next = NULL;

    if (!job_head) {
        job_head = new;
        job_tail = new;
    } else {
        job_tail->next = new;
        job_tail = new;
    }
}

void update_job_status(int pid, int new_status) {
    struct job *temp = job_head;
    while (temp) {
        if (temp->pid == pid) {
            temp->status = new_status;
            break;
        }
        temp = temp->next;
    }
}

void bg(int index) {
    struct job *job = get_job_by_index(index);
    if (job && job->status == STOPPED) {
        kill(job->pid, SIGCONT);
        update_job_status(job->pid, RUNNING);
    }
}

void fg(int index) {
    struct job *job = get_job_by_index(index);
    if (job) {
        pid = job->pid;
        if (job->status == STOPPED) {
            kill(job->pid, SIGCONT);
            update_job_status(job->pid, RUNNING);
        }
        int status;
        waitpid(pid, &status, WUNTRACED);
        if (WIFSTOPPED(status)) {
            update_job_status(pid, STOPPED);
        } else {
            remove_job(pid);
        }
        pid = -1;
    }
}

void print_jobs() {
    struct job *temp = job_head;
    if(!temp){
        printf("empty\n");
    }
    int index=0;
    while (temp) {
        printf("[%d] %d %s\n", index, temp->pid, temp->command);
        temp = temp->next;
    }
    printf("\n");
}

void remove_job(int pid) {
    struct job *temp = job_head, *prev = NULL;
    while (temp) {
        if (temp->pid == pid) {
            if (prev) {
                prev->next = temp->next;
            } else {
                job_head = temp->next;
            }
            if (temp == job_tail) {
                job_tail = prev;
            }
            free(temp->command);
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}