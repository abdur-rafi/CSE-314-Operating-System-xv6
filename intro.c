#include <stdio.h>
#include <pthread.h>

int sum = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *threadFunc(void *arg){
    for(int i = 0; i < 100000; ++i){
        pthread_mutex_lock(&mutex);
        ++sum;
        pthread_mutex_unlock(&mutex);

    }
    return NULL;
}

int main(){
    pthread_t threads[10];
    for(int i = 0; i < 10; ++i){
        pthread_create(threads + i, NULL, threadFunc, NULL);
    }
    for(int i = 0; i < 10; ++i){
        pthread_join(threads[i], NULL);
    }
    printf("%d", sum);

    return 0;
}