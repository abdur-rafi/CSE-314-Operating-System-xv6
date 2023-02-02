#include <stdio.h>
#include <pthread.h>
#include "../zemaphore/zemaphore.h"
#include<unistd.h>
#include <stdlib.h>

#define DEERS 9
#define ELVES 6
#define ELVES_GROUP 3

struct zemaphore mutex, santaVar, reindeersVar, elfMax, elfVar;
int deerCount = 0, elvesCount = 0;

void santaSleep(){
    zem_down(&santaVar);
}
void signalSanta(){
    zem_up(&santaVar);
}
void codeLock(){
    zem_down(&mutex);
}
void codeUnlock(){
    zem_up(&mutex);
}
void elfMaxLock(){
    zem_down(&elfMax);
}
void elfMaxUnlock(){
    zem_up(&elfMax);
}
void sleepElf(){
    zem_down(&elfVar);
}
void signalElf(){
    zem_up(&elfVar);
}
void prepareSleigh(){
    printf("Santa preparing sleigh for deers\n");
    sleep(1);
    printf("Sleigh prepared\n");
}
int allDeersArrived(){
    return deerCount == DEERS;
}
int groupOfElvesFull(){
    return elvesCount == ELVES_GROUP;
}
void signalDeers(){
    for(int i = 0; i < DEERS; ++i)
        zem_up(&reindeersVar);
}
void sleepDeer(){
    zem_down(&reindeersVar);
}
void handleReindeer(){
    prepareSleigh();
    signalDeers();
    deerCount = 0;
}

void helpElves(){
    printf("Santa is helping elves\n");
}
void getHelp(int id){
    printf("%d elf is getting help from santa\n", id);
}



void locksInit(){
    zem_init(&mutex, 1);
    zem_init(&santaVar, 0);
    zem_init(&reindeersVar, 0);
    zem_init(&elfMax, 1);
    zem_init(&elfVar, 0);


}

void handleElves(){
    helpElves();
    for(int i = 0; i < ELVES_GROUP; ++i)
        signalElf();
}


void *santaThreadFunc(void *args){
    printf("Santa is sleeping\n");
    while(1){
        santaSleep();
        codeLock();
        printf("Santa woke up\n");
        if(allDeersArrived()){
            handleReindeer();
        }
        if(groupOfElvesFull()){
            handleElves();
        }
        sleep(3);
        printf("Santa back to sleeping\n");
        codeUnlock();
    }
}

void getHitched(int id){
    printf("%d deer getting hitched\n", id);
}

void *reindeerThreadFunc(void *data){
    int deerId = *((int *)data);

    while(1){
        codeLock();
        printf("%d deer arrived\n", deerId);
        deerCount += 1;
        if(allDeersArrived()){
            signalSanta();
        }
        codeUnlock();
        sleepDeer();
        getHitched(deerId);
        sleep(rand() % 5 + 1);
    }
    
}

void* elfThreadFunc(void *data){
    int elfId = *((int *)data);

    while(1){
        elfMaxLock();
        codeLock();
        printf("%d elf has come for help\n", elfId);
        elvesCount++;
        if(groupOfElvesFull()){
            signalSanta();
        }
        else{
            elfMaxUnlock();
        }
        codeUnlock();
        sleepElf();
        getHelp(elfId);
        codeLock();
        --elvesCount;
        if(elvesCount == 0){
            printf("Elves group cleared\n");
            elfMaxUnlock();
        }
        codeUnlock();
    }
}



int main(){
    printf("hello\n");
    locksInit();
    pthread_t santaThread;
    pthread_t deerThreads[DEERS];
    pthread_t elfThreads[ELVES];

    pthread_create(&santaThread, NULL, santaThreadFunc, NULL);
    for(int i = 0; i < DEERS; ++i){
        int* id = (int *)malloc(sizeof(int));
        *id = i + 1;
        pthread_create(deerThreads + i,NULL, reindeerThreadFunc,id);
    }
    for(int i = 0; i < ELVES; ++i){
        int* id = (int *)malloc(sizeof(int));
        *id = i + 1;
        pthread_create(elfThreads + i,NULL, elfThreadFunc,id);
    }
    pthread_join(santaThread, NULL);
    // for(int i = 0; i < DEERS; ++i){
    //     pthread_join(deerThreads[i], NULL);
    // }

    

    return 0;
}
