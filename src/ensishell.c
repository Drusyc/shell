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

#include "liste.h"

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif


/*
 * Affiche le status d'un processus terminé
 */
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

list_t * bgProcList = NULL;

int execCmd (struct cmdline * l, int y) {
        //pid_t parent = getpid();
        //printf("PID parent : %d\n", parent);

        pid_t pidChild = -1;
        int i;
        int nbPipes = 0;
        /* compte le nombre de commande pipé */
	for (i=0; l->seq[i]!=0; i++) 
                nbPipes++;

        int tuyau[2][nbPipes-1];

        for(i=0; i < nbPipes-1;i++)
                pipe(tuyau[i]);

	for (i=0; l->seq[i]!=0; i++) {
                if (pidChild != 0 && strcmp(*l->seq[i], "jobs") == 0) {
                        printList(bgProcList);
                        return EXIT_SUCCESS;
                }//fi

                pidChild =  fork();

                if (pidChild != 0 && l->bg) {
                        addList(&bgProcList, pidChild, *l->seq[i]);
                }//fi

                if (pidChild == 0) {
                        /* Child .. */

                        printf("%s (PID :%d)\n\n", *l->seq[i], getpid());
                        /* Check if there is a pipe (next command).. */
                        /* if we are the first command */
                        if (i == 0 && l->seq[i+1] != 0) {
                                dup2(tuyau[0][1], 1);
                        }
                        /* if we are not the first command */
                        else if (i == nbPipes) {
                                dup2(tuyau[nbPipes-1][0], 0);
                        }
                        /* */
                        else if (l->seq[i+1] !=0) {
                                dup2(tuyau[i][0],0);
                                dup2(tuyau[i+1][1],1);
                        }
                        
                        /* Ferme les tuyaux */
                        for(i=0; i < nbPipes-1; i++) {
                                close(tuyau[i][0]);
                                close(tuyau[i][1]);
                        }

                        char ** cmd = l->seq[i];
                        execvp(cmd[0], cmd);
                        //execvp(*(l->seq[i]), *l->seq);
                        _exit (EXIT_FAILURE);

                } else if (pidChild == -1) {
                        perror("Error during fork..");
                        return EXIT_FAILURE;
                }
        }//for()

        /* Parent.. */
        if (pidChild > 0) {
                /* Ferme les tuyaux */
                for(i=0; i < nbPipes-1; i++) {
                        close(tuyau[i][0]);
                        close(tuyau[i][1]);
                }

		for (i=0; l->seq[i]!=0; i++) {
                        if (!l->bg) {
                                /* status contains information on how the child has terminated */
                                int status;
                                /* wpid contains the ID of the child whose state has change */
                                int wpid;
                                
                                /* wait for child */
                                wpid = waitpid(-1, &status, 0); 
                                printf("\n\nchild (pid %d) ", wpid);
                                checkStatus(status);
                        }
                }//for()
        }//fi

        return EXIT_SUCCESS;
}//execCmd()

int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

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
