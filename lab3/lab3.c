#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define THREADS_NUMBER 4
#define ARRAY_LENGTH 8
#define OFFSET 2
#define NO_ERROR 0

void * print_lines(void* param) {
    if( NULL == param){
        fprintf(stderr, "Error print lines: param was NULL\n");
        return NULL;
    }
    char** lines = (char **) param;
    for ( char ** line = lines; NULL != *line; line++) {
        printf("%s\n", *line);
    }
    return NULL;
}

int main() {
    pthread_t threads[THREADS_NUMBER];
    char* numbers[ARRAY_LENGTH] = {"Child 1", NULL, "Child 2", NULL, "Child 3", NULL, "Child 4", NULL};
    int error_code;
    int count_created_threads = THREADS_NUMBER;
    for (int i = 0; i < THREADS_NUMBER; i++) {
        error_code = pthread_create(&threads[i], NULL, print_lines, &numbers[i * OFFSET]);
        if (NO_ERROR != error_code) {
            printf("Thread creation error: %s", strerror(error_code));
            count_created_threads = i;
            break;
        }
    }
    for (int j = 0; j < count_created_threads; j++) {
        error_code = pthread_join(threads[j], NULL);
        if (NO_ERROR != error_code) {
            printf("Thread join error: %s", strerror(error_code));
            return EXIT_FAILURE;
        }
    }
    printf("%s\n", "Parent Thread");
    pthread_exit(NULL);
    return EXIT_SUCCESS;
}
