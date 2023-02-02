#include <stdio.h>
#include <pthread.h>
#include "../zemaphore/zemaphore.h"
#include<unistd.h>
#include <stdlib.h>

#define DEERS 9
#define ELVES 6
#define ELVES_GROUP 3

struct zemaphore mutex, santa_var, reindeers_var;
int deerCount = 0, elvesCount = 0;

void santaSleep(){
    zem_down(&santa_var);
}
void signalSanta(){
    zem_up(&santa_var);
}
void codeLock(){
    zem_down(&mutex);
}
void codeUnlock(){
    zem_up(&mutex);
}
void prepareSleigh(){
    printf("Santa preparing sleigh for deers\n");
    sleep(1);
    printf("Sleigh prepared\n");
}
int allDeersArrived(){
    return deerCount == DEERS;
}
int awakenByElves(){

}
void signalDeers(){
    for(int i = 0; i < DEERS; ++i)
        zem_up(&reindeers_var);
}
void sleepDeer(){
    zem_down(&reindeers_var);
}
void handleReindeer(){
    prepareSleigh();
    signalDeers();
    codeUnlock();
}

void handleElves(){

}

void locksInit(){
    zem_init(&mutex, 1);
    zem_init(&santa_var, 0);
    zem_init(&reindeers_var, 0);

}

void *santaThreadFunc(void *args){
    while(1){
        santaSleep();
        codeLock();
        printf("Santa woke up\n");
        if(allDeersArrived()){
            handleReindeer();
        }
        if(awakenByElves()){
            handleElves();
        }
        codeUnlock();
        printf("Santa back to sleeping\n");
    }
}

void getHitched(int id){
    printf("%d deer getting hitched\n", id);
}

void *reindeerThreadFunc(void *data){
    int deerId = *((int *)data);
    codeLock();
    printf("%d deer arrived\n", deerId);
    deerCount += 1;
    if(allDeersArrived()){
        signalSanta();
    }
    codeUnlock();
    sleepDeer();
    getHitched(deerId);
}


int main(){
    printf("hello\n");
    locksInit();
    pthread_t santaThread;
    pthread_t deerThreads[DEERS];
    pthread_create(&santaThread, NULL, santaThreadFunc, NULL);
    for(int i = 0; i < DEERS; ++i){
        int* id = (int *)malloc(sizeof(int));
        *id = i + 1;
        pthread_create(deerThreads + i,NULL, reindeerThreadFunc,id);
    }
    pthread_join(santaThread, NULL);
    for(int i = 0; i < DEERS; ++i){
        pthread_join(deerThreads[i], NULL);
    }
    return 0;
}
