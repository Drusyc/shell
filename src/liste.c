#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork()..
#include <sys/types.h> //wait()..
#include <sys/wait.h> //wait()..
#include <string.h> //string & co

#include "liste.h"

void initList (list_t **list) {
        *list = malloc(sizeof(list_t));
        (*list)->next = NULL;
}//initList()

void addList (list_t **list, pid_t procPid, char * procCmd) {
 
        if (*list == NULL) {
                *list = malloc(sizeof(list_t));
                (*list)->pid = procPid;
                (*list)->cmd = malloc(strlen(procCmd)+1);
                strcpy((*list)->cmd, procCmd);
                (*list)->next = NULL;
                return;
        }//fi

        list_t *li = *list;
        while(li->next != NULL)
                li = li->next;

        list_t *newElem = malloc(sizeof(list_t));
        newElem->pid = procPid;
        newElem->cmd = malloc(strlen(procCmd)+1);
        strcpy(newElem->cmd, procCmd);
        newElem->next = NULL;

        li->next = newElem;
}//addList()

void printList (list_t *list) {
        list_t *li = list;
        while (li != NULL) {
                printf("PID:%d\tcmd:%s -> ",li->pid,li->cmd);
                int status = 0;
                int wpid = 0;

                wpid = waitpid(li->pid, &status, WNOHANG); 
                if (wpid == 0) {
                        // process have not terminated yet 
                        printf("have not terminated yet..\n");
                } else if (wpid == -1) {
                        printf("terminated\n");
                } else {
                        if (WIFEXITED(status)) {
                                /* true if the child terminated normally, that is, by calling exit(3) or _exit(2), or by returning from main() */
                                printf("exited normally, status %d\n", WEXITSTATUS(status));
                        } else if (WIFSIGNALED(status)) {
                                /* true if the child process was terminated by a signal */
                                printf("killed by signal %d\n", WTERMSIG(status));
                        } else if (WIFSTOPPED (status)) {
                                /* true if the child process was stopped by delivery of a signal */
                                printf("stopped by signal %d\n", WIFSTOPPED(status));
                        } else {
                                printf("exited for other reasons.. (see man waitpid)\n");
                        }  
                }
                li = li->next;
        }//while
}//printList()

void freeList (list_t **list) {
        if (*list == NULL)
                return;
        
        list_t *next, *cour;
        next = *list;

        while(next != NULL) {
                cour = next;
                next = cour->next;
                free(cour->cmd);
                free(cour);
        }//while();
        cour = NULL;
}//freeList()
