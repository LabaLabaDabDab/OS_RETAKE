#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define NUMBER_OF_LINES 10
#define NO_ERROR 0
#define WRITE_ERROR (-1)

void * printLines(void * param){
    if (NULL == param){
        fprintf(stderr, "Error print lines: param was NULL\n");
        return NULL;
    }
    for (int i = 0; i < NUMBER_OF_LINES; ++i) {
        if (WRITE_ERROR == write(STDOUT_FILENO, (char*)param, strlen((char*)param))) {
            perror("write error");
            return NULL;
        }
    }
    return NULL;
}

int main(int argc, char ** argv) {
    pthread_t thread;
    int errorCode = pthread_create(&thread, NULL, printLines, "child\n");
    if (NO_ERROR != errorCode){
        fprintf(stderr, "Unable to create thread: %s\n", strerror(errorCode));
        return EXIT_FAILURE;
    }
    void *a = NULL;
    errorCode = pthread_join(thread, &a);
    if (NO_ERROR != errorCode){
        fprintf(stderr, "Unable to join thread: %s\n", strerror(errorCode));
        return EXIT_FAILURE;
    }
    printf("%p\n",  a);
    printLines("parent\n");
    pthread_exit(NULL);
    return EXIT_SUCCESS;
}
