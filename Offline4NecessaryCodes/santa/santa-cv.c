#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define DEERS 9
#define ELVES 6
#define ELVES_GROUP 3

int deerCount = 0, elvesCount = 0, hitchCount = 0, helpCount = 0;

pthread_mutex_t elf_tex;
pthread_mutex_t mutex;
pthread_cond_t santa_cv;
pthread_cond_t reindeer_cv;
pthread_cond_t elf_cv;
pthread_cond_t hitched;
pthread_cond_t occupiedDeer;
pthread_cond_t gettingHelp;

void prepareSleigh() {
    printf("Santa preparing sleigh for deers\n");
    // sleep(1);
}

void getHitched(int id) {
    printf("%d deer getting hitched\n", id);
}

void getHelp(int id) {
    printf("%d elf getting help from santa\n", id);
}

void wait_for_either() {
    while (deerCount < DEERS && elvesCount < ELVES_GROUP) {
        pthread_cond_wait(&santa_cv, &mutex);
    }
}

void* santa(void* arg) {
    while (1) {
        printf("Santa is sleeping\n");
        pthread_mutex_lock(&mutex);
        wait_for_either(); // wait for either elves or deers to arrive
        printf("Santa woke up\n");
        if (deerCount == DEERS) {
            prepareSleigh();
            printf("Sleigh prepared.Waiting for reindeers.\n");
            pthread_cond_broadcast(&reindeer_cv); // signal deers to get hitched
            while (hitchCount < DEERS) // wait for deers to get hitched
                pthread_cond_wait(&hitched, &mutex);
            printf("All hitched. Going to give presents\n");
            deerCount = 0;
            hitchCount = 0;
            printf("Santa returned\n");
            pthread_cond_broadcast(&occupiedDeer); // signal deers so that they can go back
        }
        if (elvesCount == ELVES_GROUP) {
            printf("Santa is helping elves\n");
            pthread_cond_broadcast(&elf_cv); // signal elves to get help
            while (helpCount < ELVES_GROUP) // wait for all elves to get help
                pthread_cond_wait(&gettingHelp, &mutex);
            helpCount = 0;
        }
        printf("Santa goind back to sleep\n");
        sleep(1);
        pthread_mutex_unlock(&mutex);
    }
}

void* reeinder(void* arg) {
    int id = *(int*)arg;
    while (1) {
        pthread_mutex_lock(&mutex);
        printf("%d deer arrived\n", id);
        deerCount++;
        if (deerCount == DEERS) {
            pthread_cond_signal(&santa_cv); // all deers arrived, wake up santa
        }
        pthread_cond_wait(&reindeer_cv, &mutex); // wait for santa to prepare sleigh
        getHitched(id);
        hitchCount++;
        if (hitchCount == DEERS) {
            pthread_cond_signal(&hitched); // all deers got hitched, signal santa to go
        }
        pthread_cond_wait(&occupiedDeer, &mutex); // wait for santa to return
        printf("%d reindeer going back\n", id);
        pthread_mutex_unlock(&mutex);
        sleep(1 + rand() % 5);
    }
}

void* elf(void* arg) {
    int id = *(int*)arg;
    while (1) {
        pthread_mutex_lock(&elf_tex);
        pthread_mutex_lock(&mutex);
        printf("%d elf has come for help\n", id);
        elvesCount++;
        if (elvesCount == ELVES_GROUP)
            pthread_cond_signal(&santa_cv); // elves group arrived, wake up santa
        else
            pthread_mutex_unlock(&elf_tex); // elves group not arrived, allow other elves to come

        pthread_cond_wait(&elf_cv, &mutex); // wait for santa to help
        getHelp(id);
        elvesCount--;
        helpCount++;
        if (elvesCount == 0) {
            printf("Elves group cleared\n");
            pthread_mutex_unlock(&elf_tex); // elves group cleared, allow other elves to come
            pthread_cond_signal(&gettingHelp); // signal santa that all elves got help
        }
        pthread_mutex_unlock(&mutex);
        sleep(1 + rand() % 5);
    }
}

void* alloc_int(int i) { // helper function to allocate memory for thread id
    int* p = malloc(sizeof(int));
    *p = i;
    return p;
}

int main() {
    pthread_t santa_thread;
    pthread_t reindeer_threads[DEERS];
    pthread_t elf_threads[ELVES];

    // initialize mutexes and condition variables
    pthread_mutex_init(&elf_tex, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&santa_cv, NULL);
    pthread_cond_init(&reindeer_cv, NULL);
    pthread_cond_init(&elf_cv, NULL);
    pthread_cond_init(&hitched, NULL);
    pthread_cond_init(&occupiedDeer, NULL);
    pthread_cond_init(&gettingHelp, NULL);
    
    // create threads
    pthread_create(&santa_thread, NULL, santa, NULL);
    for (int i = 0; i < DEERS; i++) {
        pthread_create(&reindeer_threads[i], NULL, reeinder, alloc_int(i + 1));
    }
    for (int i = 0; i < ELVES; i++) {
        pthread_create(&elf_threads[i], NULL, elf, alloc_int(i + 1));
    }
    
    // santa_thread will never exit
    pthread_join(santa_thread, NULL);

    return 0;
}
