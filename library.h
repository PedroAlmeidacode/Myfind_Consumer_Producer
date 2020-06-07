//
// Created by pedro on 21/05/20.
//

#ifndef MYFIND_CONSUMIDORPRODUTOR_LIBRARY_H
#define MYFIND_CONSUMIDORPRODUTOR_LIBRARY_H

#endif //MYFIND_CONSUMIDORPRODUTOR_LIBRARY_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


#define N 10
#define NProds 1
#define NCons 5

typedef int (*PARAM)(char * value, const char *path, char * d_name);

typedef struct args {
    PARAM opt;
    char *value;
} ARGS;

typedef struct data {
    ARGS args[10];
    char *path; // path to search from, in dept
    int n_args;
}DATA;


typedef struct threads {
    struct matches *matches;
    pthread_t thread_id;  //id of the thread
    struct threads *next; // threads created when found a directory, nexy = tail->next
    int n_matches; // numnero de matches encontrados ou seja numero de strings em matches
} THREADS;



typedef struct matches {
    char *match; // strings that match the search in this thread
    struct matches *next;
} MATCHES;

char *strremove(char *str, const char *sub);
int isDirectory(const char *path);
void parse_args(int argc, char *argv[], DATA *data);