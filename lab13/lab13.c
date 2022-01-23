#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUMBER_OF_LINES 10

#define MAIN_THREAD 0
#define CHILD_THREAD 1
#define SUCCESS 0
#define FAILURE -1

typedef struct pthreadParameters {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int cur_printing_thread;
}pthreadParameters;

void print_error(const char *prefix, int code) {
    if (NULL == prefix) {
        prefix = "error";
    }
    char buffer[256];
    if (0 != strerror_r(code, buffer, sizeof(buffer))) {
        strcpy(buffer, "(unable to generate error!)");
    }
    fprintf(stderr, "%s: %s\n", prefix, buffer);
}

int lock_mutex(pthread_mutex_t *mutex) {
    int error_code = pthread_mutex_lock(mutex);
    if (SUCCESS != error_code) {
        print_error("Unable to lock mutex", error_code);
        return error_code;
    }
    return SUCCESS;
}

int unlock_mutex(pthread_mutex_t *mutex) {
    int error_code = pthread_mutex_unlock(mutex);
    if (SUCCESS != error_code) {
        print_error("Unable to unlock mutex", error_code);
        return error_code;
    }
    return SUCCESS;
}

int wait_cond(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    int error_code = pthread_cond_wait(cond, mutex);
    if (SUCCESS != error_code) {
        print_error("Unable to wait cond variable", error_code);
        return error_code;
    }
    return SUCCESS;
}

int signal_cond(pthread_cond_t *cond) {
    int error_code = pthread_cond_signal(cond);
    if (SUCCESS != error_code) {
        print_error("Unable to signal cond", error_code);
        return error_code;
    }
    return SUCCESS;
}

void print_messages(struct pthreadParameters *parameters, const char *message, int calling_thread) {
    if (NULL == message) {
        fprintf(stderr, "print_messages: invalid parameter\n");
        return;
    }
    size_t msg_length = strlen(message);

    if (SUCCESS != lock_mutex(&parameters->mutex)){
        return;
    }

    for (int i = 0; i < NUMBER_OF_LINES; i++) {
        while (calling_thread != parameters->cur_printing_thread) {
            if (SUCCESS != wait_cond(&parameters->cond, &parameters->mutex)){ // анлочит мбютекс при вызове и лочит его обратно при завершении вызова(успешном)
                return;
            }
        }

        if (-1 == write(STDOUT_FILENO, message, msg_length)){
            perror("write error");
            return;
        }
        parameters->cur_printing_thread = (parameters->cur_printing_thread == MAIN_THREAD) ? CHILD_THREAD : MAIN_THREAD;
        if (SUCCESS != signal_cond(&parameters->cond)){ //говорит нити пробудится
            return;
        }
    }

    if (SUCCESS != unlock_mutex(&parameters->mutex)){
        return;
    }
}

void *second_print(void *param) {
    if (NULL == param){
        fprintf(stderr, "Param is NULL\n");
        return NULL;
    }
    print_messages(param, "Child\n", CHILD_THREAD);
    return NULL;
}

int init(struct pthreadParameters *parameters) {
    int error_code = pthread_mutex_init(&parameters->mutex, NULL);
    if (0 != error_code) {
        print_error("Unable to init mutex", error_code);
        return FAILURE;
    }

    error_code = pthread_cond_init(&parameters->cond, NULL);
    if (0 != error_code) {
        print_error("Unable to init cond", error_code);
        pthread_mutex_destroy(&parameters->mutex);
        return FAILURE;
    }
    parameters->cur_printing_thread = MAIN_THREAD;
    return SUCCESS;
}

void cleanup(struct pthreadParameters *parameters) {
    pthread_mutex_destroy(&parameters->mutex);
    pthread_cond_destroy(&parameters->cond);
}

int main() {
    struct pthreadParameters parameters;
    if (SUCCESS != init(&parameters)) {
        return EXIT_FAILURE;
    }

    pthread_t thread;
    int error_code = pthread_create(&thread, NULL, second_print, &parameters);
    if (0 != error_code) {
        print_error("Unable to create thread", error_code);
        cleanup(&parameters);
        return EXIT_FAILURE;
    }

    print_messages(&parameters,"Parent\n", MAIN_THREAD);

    cleanup(&parameters);
    pthread_exit(NULL);
    return EXIT_SUCCESS;
}
