/*****************************************************
 * Copyright Grégory Mounié 2008-2013                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork()..
#include <sys/types.h> //wait()..
#include <sys/wait.h> //wait()..
#include <string.h> //string & co

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

void checkStatus (int status) {
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
}//checkStatus()

/*
 * Gestion liste 
 * uilisé pour répertorié les process placés en background 
 * (A place dans un .h lorsqu'on saura comment placer un nouveau .h dans la chaine de compilation..)
 *
 */

struct my_list {
        pid_t pid;
        char * cmd;
        struct my_list *next;
};

typedef struct my_list list_t;

list_t * bgProcList = NULL;

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
                        /* process have not terminated yet */
                        printf("have not terminated yet..\n");
                } else if (wpid == -1) {
                        printf("terminated\n");
                } else {
                        checkStatus(status);
                }
                li = li->next;
        }//while
}//printList()

void freeList (void) {
        if (bgProcList == NULL)
                return;
        
        list_t *next, *cour;
        next = bgProcList;

        while(next != NULL) {
                cour = next;
                next = cour->next;
                free(cour->cmd);
                free(cour);
        }//while();
        cour = NULL;
}//freeList()

/*
 * Fin gestion List
 */

int execCmd (struct cmdline * l, int y) {
        //pid_t parent = getpid();
        //printf("PID parent : %d\n", parent);
        for (int i=0; l->seq[i]!=0; i++) {
                pid_t pidChild =  fork();

                if (pidChild != 0 && strcmp(*l->seq[i], "jobs") == 0) {
                        printList(bgProcList);
                        continue;
                }//fi

                if (pidChild != 0 && l->bg) {
                        addList(&bgProcList, pidChild, *l->seq[i]);
                }//fi
                

                if (pidChild == 0) {
                        /* Child .. */
                        execvp(*(l->seq[i]), *l->seq);
                        _exit (EXIT_FAILURE);

                } else if (pidChild == -1) {
                        perror("Error during fork..");
                        return EXIT_FAILURE;

                } else if (!l->bg) {
                        /* Parent.. */

                        /* status contains information on how the child has terminated */
                        int status;
                        /* wpid contains the ID of the child whose state has change */
                        int wpid;
                        
                        printf("PID child :%d\n\n\n", pidChild);

                        /* wait for any child */
                        wpid = waitpid(-1, &status, 0); 
                        printf("\n\nchild (pid %d) ", wpid);
                        checkStatus(status);
                }
        }//For
        return EXIT_SUCCESS;
}//execCmd()

int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);
        atexit(freeList);

	while (1) {
		struct cmdline *l;
		int i, j;
		char *prompt = "ensishell>";

		l = readcmd(prompt);

		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);
		if (l->bg) printf("background (&)\n");

                if (execCmd(l, 0) != EXIT_SUCCESS)
                        perror ("hum ?..\n");

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
                        
			printf("seq[%d]: ", i);
                        for (j=0; cmd[j]!=0; j++) {
                                printf("'%s' ", cmd[j]);
                        }//for()
			printf("\n");
		}//for()

	}//while()
}//main
