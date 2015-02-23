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

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

int execCmd (struct cmdline * l, int i) {
        pid_t parent = getpid();
        printf("PID parent : %d\n", parent);

        pid_t pidChild =  fork();

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
                        printf("exitted for other reasons.. (see man waitpid)\n");
                }
        }
        return EXIT_SUCCESS;
}//execCmd()

void printJobs () {}

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

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
                        
                        if (execCmd(l, i) != EXIT_SUCCESS)
                                perror ("hum ?..\n");
                        
			printf("seq[%d]: ", i);
                        for (j=0; cmd[j]!=0; j++) {
                                printf("'%s' ", cmd[j]);
                        }//for()
			printf("\n");
		}//for()
	}//while()
}//main
