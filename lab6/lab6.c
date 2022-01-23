#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define RATIO 200000
#define BUFFER_SIZE 4096
#define MAX_NUM_OF_LINES 100

#define TRUE 1
#define FALSE 0

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

void exact_usleep(unsigned time_left) {
    do {
        time_left = usleep(time_left);
    } while (time_left > 0);
}

void *sleep_and_print(void *param) {
    if (NULL == param) {
        fprintf(stderr, "wait_and_print: invalid param\n");
        return NULL;
    }

    char *str = (char *)param;
    size_t length = strlen(str);

    exact_usleep(RATIO * length);
    write(STDOUT_FILENO, str, length);

    free(str);
    return NULL;
}

char *read_line(int *is_eof) {
    char *str = NULL;
    size_t length = 0;

    while (1) {
        char *ptr = (char *)realloc(str, (length + BUFFER_SIZE) * sizeof(char));
        if (NULL == ptr) {
            perror("read_strings: Unable to (re)allocate memory for string");
            break;
        }
        str = ptr; // realloc may return different pointer, so we need to update str

        errno = 0;  //errno is unique to calling thread (furthermore, we call this function before spawning any threads)
        char *check = fgets(str + length, BUFFER_SIZE, stdin);
        if (check == NULL) {
            if (errno != 0) {
                perror("read_strings: Unable to read from stdin");
            }
            *is_eof = TRUE;
            break;
        }

        length += strlen(check); // 'check' will be null-terminated
        if (str[length - 1] == '\n') {
            break;
        }
    }

    return str;
}

int main() {
    int is_eof = FALSE;
    int number_of_lines = 0;
    char *lines[MAX_NUM_OF_LINES];

    while (number_of_lines < MAX_NUM_OF_LINES && !is_eof) {
        lines[number_of_lines] = read_line(&is_eof);
        if (lines[number_of_lines] == NULL) {
            break;
        }
        number_of_lines++;
    }

    for (int i = 0; i < number_of_lines; i++) {
        pthread_t thread;
        int error_code = pthread_create(&thread, NULL, sleep_and_print, lines[i]);
        if (0 != error_code) {
            print_error("Unable to create thread", error_code);
            free(lines[i]);
        }
    }

    pthread_exit(NULL);
    return EXIT_SUCCESS;
}
