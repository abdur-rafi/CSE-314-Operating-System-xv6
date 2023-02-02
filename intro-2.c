#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define N 100
pthread_cond_t cond_locks[N];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int done[N];
void* func(void* arg) {
    int* i = (int*)arg;
    while (1) {
        pthread_mutex_lock(&lock);
        while (done[*i] == 0) {
            pthread_cond_wait(cond_locks + *i, &lock);
        }
        printf("%d ", *i);
        if(*i==N-1) printf("\n");
        done[(*i + 1) % N] = 1;
        done[*i] = 0;
        pthread_cond_signal(cond_locks + (*i + 1) % N);
        pthread_mutex_unlock(&lock);
    }
}

void init_global_vars() {
    for (int i = 0; i < N; ++i) {
        pthread_cond_init(cond_locks + i, NULL);
    }
    done[0] = 1;
    for (int i = 1; i < N; ++i) {
        done[i] = 0;
    }
}

int main() {
    pthread_t threads[N];
    init_global_vars();
    for (int i = 0; i < N; ++i) {
        int* arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(threads + i, NULL, func, arg);
    }
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }
}