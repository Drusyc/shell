
#ifndef __LISTE_H_
#define __LISTE_H

#include <unistd.h> // fork()..

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

void initList (list_t **list);
void addList (list_t **list, pid_t procPid, char * procCmd);
void printList (list_t *list);
void freeList (list_t **list);


#endif // __LISTE_H_
