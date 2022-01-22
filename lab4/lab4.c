#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define NUMBER_SECONDS 2
#define TRUE 1

void* print_lines(void* param) {
    while (TRUE) {
        printf("Hello! I'm a %s\n", (char*)param);
    }
}

int main() {
    pthread_t thread;
    int error_code = pthread_create(&thread, NULL, print_lines, "Child");
    if (0 != error_code) {
        printf("Thread creation error: %s", strerror(error_code));
        return EXIT_FAILURE;
    }

    sleep(NUMBER_SECONDS);

    const char *msg = "Parent: Trying to cancel child thread\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    error_code = pthread_cancel(thread);

    if (0 != error_code) {
        printf("Thread cancel error: %s", strerror(error_code));
        return EXIT_FAILURE;
    }

    error_code = pthread_join(thread, NULL);

    if(0 != error_code){
        printf("Error if joining thread: %s", strerror(error_code));
        return EXIT_FAILURE;
    }

    msg = "Parent: Cancelled child thread\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    pthread_exit(NULL);
    return EXIT_SUCCESS;
}
