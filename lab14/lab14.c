#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>

#define NUMBER_OF_LINES 10

#define SECOND_SEMAPHORE 1
#define FIRST_SEMAPHORE 0

sem_t semaphores[2];

void print_error(const char *prefix, int code) {
    if (NULL == prefix) {
        prefix = "error";
    }
    char buf[256];
    if (0 != strerror_r(code, buf, sizeof(buf))) {
        strcpy(buf, "(unable to generate error!)");
    }
    fprintf(stderr, "%s: %s\n", prefix, buf);
}

void wait_semaphore(sem_t *sem) {
    if ( -1 == sem_wait(sem)) {
        perror("Unable to wait semaphore");
    }
}

void post_semaphore(sem_t *sem) {
    if (-1 == sem_post(sem)) {
        perror("Unable to wait semaphore");
    }
}

void print_messages(int first_sem, int second_sem, const char *message) {
    if (NULL == message) {
        fprintf(stderr, "print_messages: message was null");
        return;
    }
    size_t msg_length = strlen(message);

    for (int i = 0; i < NUMBER_OF_LINES; i++) {
        wait_semaphore(&semaphores[first_sem]);
        write(STDOUT_FILENO, message, msg_length);
        post_semaphore(&semaphores[second_sem]);
    }
}

void *second_print(void *param) {
    print_messages(SECOND_SEMAPHORE, FIRST_SEMAPHORE, "Child\n");
    return NULL;
}

int main() {
    int error_code = sem_init(&semaphores[FIRST_SEMAPHORE], 0, 1);
    if (-1 == error_code) {
        perror("Unable to init semaphore");
        return EXIT_FAILURE;
    }

    error_code = sem_init(&semaphores[SECOND_SEMAPHORE], 0, 0);
    if (-1 == error_code) {
        perror("Unable to init semaphore");
        sem_destroy(&semaphores[FIRST_SEMAPHORE]);
        return EXIT_FAILURE;
    }

    pthread_t thread;
    error_code = pthread_create(&thread, NULL, second_print, NULL);
    if (0 != error_code) {
        print_error("Unable to create thread", error_code);
        sem_destroy(&semaphores[FIRST_SEMAPHORE]);
        sem_destroy(&semaphores[SECOND_SEMAPHORE]);
        return EXIT_FAILURE;
    }

    print_messages(FIRST_SEMAPHORE, SECOND_SEMAPHORE, "Parent\n");

    error_code = pthread_join(thread, NULL);
    if (0 != error_code) {
        print_error("Unable to join thread", error_code);
    }

    sem_destroy(&semaphores[FIRST_SEMAPHORE]);
    sem_destroy(&semaphores[SECOND_SEMAPHORE]);
    pthread_exit(NULL);
    return EXIT_SUCCESS;
}
