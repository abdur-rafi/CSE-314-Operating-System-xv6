#include <stdio.h>
#include <pthread.h>
#include "../zemaphore/zemaphore.h"
#include<unistd.h>
#include <stdlib.h>

#define DEERS 9
#define ELVES 6
#define ELVES_GROUP 3

struct zemaphore mutex, santaVar, reindeersVar, elfMax, elfVar, hitched, occupiedDeer, gettingHelp;
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
    codeUnlock();
    // sleep(5);
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
void reindeerFreed(){
    signalDeers();
}
void waitForDeersToGetHitched(){
    for(int i = 0; i < DEERS; ++i)
        zem_down(&hitched);
}
void deerGotHitched(){
    zem_up(&hitched);
}
void freeDeers(){
    for(int i = 0; i < DEERS; ++i){
        zem_up(&occupiedDeer);
    }
}

void handleReindeer(){
    prepareSleigh();
    codeLock();
    printf("Sleigh prepared.Waiting for reindeers.\n");
    signalDeers();
    codeUnlock();
    waitForDeersToGetHitched();
    codeLock();
    printf("All hitched. Going to give presents\n");
    codeUnlock();
    // sleep(5);
    codeLock();
    deerCount = 0;
    printf("Santa returned\n");
    // reindeerFreed();
    // signalDeers();
    freeDeers();
    // codeUnlock();
}

void helpElves(){
    printf("Santa is helping elves\n");
}
void getHelp(int id){
    printf("%d elf got help from santa\n", id);
}



void locksInit(){
    zem_init(&mutex, 1);
    zem_init(&santaVar, 0);
    zem_init(&reindeersVar, 0);
    zem_init(&elfMax, 1);
    zem_init(&elfVar, 0);
    zem_init(&hitched, 0);
    zem_init(&occupiedDeer, 0);
    zem_init(&gettingHelp, 0);


}

void waitForHelpComplete(){
    zem_down(&gettingHelp);
}
void helpComplete(){
    zem_up(&gettingHelp);
}

void handleElves(){
    helpElves();
    for(int i = 0; i < ELVES_GROUP; ++i){
        signalElf();
        codeUnlock();
        waitForHelpComplete();
        codeLock();
    }
    // codeUnlock();
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
        // codeLock();
        printf("Santa going back to sleep\n");
        codeUnlock();
        // sleep(3);
    }
}

void getHitched(int id){
    printf("%d deer getting hitched\n", id);
    deerGotHitched();
}

void reindeerOccupied(){
    zem_down(&occupiedDeer);
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
        codeLock();
        getHitched(deerId);
        codeUnlock();

        // sleep(3);
        reindeerOccupied();
        codeLock();
        printf("%d reindeer going back\n", deerId);
        codeUnlock();
        // sleep(3);

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
        codeLock();
        getHelp(elfId);
        helpComplete();
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
    for(int i = 0; i < ELVES; ++i){
        int* id = (int *)malloc(sizeof(int));
        *id = i + 1;
        pthread_create(elfThreads + i,NULL, elfThreadFunc,id);
    }
    for(int i = 0; i < DEERS; ++i){
        int* id = (int *)malloc(sizeof(int));
        *id = i + 1;
        pthread_create(deerThreads + i,NULL, reindeerThreadFunc,id);
    }
    
    pthread_join(santaThread, NULL);
    // for(int i = 0; i < DEERS; ++i){
    //     pthread_join(deerThreads[i], NULL);
    // }

    

    return 0;
}
