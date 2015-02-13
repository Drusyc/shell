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
        int fpid = -1;

        if ((fpid = fork()) == 0) {
                execvp(*(l->seq[i]), *l->seq);
        } else if (l->bg) {
                wait(NULL);
        }
        return 0;
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

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];

                        if (execCmd(l, i) != 0)
                                perror ("FAUX");

			printf("seq[%d]: ", i);
                        for (j=0; cmd[j]!=0; j++) {
                                printf("'%s' ", cmd[j]);
                        }//for()
			printf("\n");
		}//for()
	}//while()
}//main
