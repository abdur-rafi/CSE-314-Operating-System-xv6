#include <stdio.h>
#include <pthread.h>

int N = 3;

void *func(void *arg){
    int *i = (int*) arg;
    while(1){
        printf("%d ", *i);
    }
}

int main(){
    pthread_t threads[N];
    for(int i = 0; i < N; ++i){
        pthread_create(threads + i, NULL, func, &i);
    }
    for (int i = 0; i < N; i++){
        pthread_join(threads[i], NULL);
    }

    
}