#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <pthread.h>

#define MAX_PROCESSES 100

typedef struct {
    int id;
    int number;
} process_number;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int total;
    process_number numbers[MAX_PROCESSES];
} shared_data_t;

void child_process(shared_data_t *shared, int id) {
    pthread_mutex_lock(&shared->mutex);
    printf("Proces %d. unesite broj: ", id);
    fflush(stdout);
    if (scanf("%d", &shared->numbers[id].number) != 1) {
        fprintf(stderr, "Proces %d. greška pri unosu broja.\n", id);
    }
    shared->numbers[id].id = id;
    shared->count++;
    if (shared->count == shared->total) {
        pthread_cond_broadcast(&shared->cond);
    } else {
        while (shared->count < shared->total) {
            pthread_cond_wait(&shared->cond, &shared->mutex);
        }
    }
    pthread_mutex_unlock(&shared->mutex);
    printf("Proces %d. uneseni broj je %d\n", id, shared->numbers[id].number);
    fflush(stdout);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_processes>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    if (N <= 0 || N > MAX_PROCESSES) {
        fprintf(stderr, "Number of processes must be between 1 and %d\n", MAX_PROCESSES);
        return 1;
    }

    shared_data_t shared = static_cast<shared_data_t>(mmap(NULL, sizeof(shared_data_t), 
                                 PROT_READ | PROT_WRITE, 
                                 MAP_SHARED | MAP_ANONYMOUS, 
                                 -1, 0));
    if (shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    shared->count = 0;
    shared->total = N;

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    if (pthread_mutex_init(&shared->mutex, &mutex_attr) != 0) {
        perror("pthread_mutex_init");
        return 1;
    }
    pthread_mutexattr_destroy(&mutex_attr);

    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    if (pthread_cond_init(&shared->cond, &cond_attr) != 0) {
        perror("pthread_cond_init");
        return 1;
    }
    pthread_condattr_destroy(&cond_attr);

    for (int i = 0; i < N; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            for (int j = 0; j < i; j++) {
                wait(NULL);
            }
            return 1;
        } else if (pid == 0) {
            child_process(shared, i);
            exit(0);
        }
    }

    for (int i = 0; i < N; i++) {
        wait(NULL);
    }

    pthread_mutex_destroy(&shared->mutex);
    pthread_cond_destroy(&shared->cond);
    munmap(shared, sizeof(shared_data_t));

    return 0;
}
