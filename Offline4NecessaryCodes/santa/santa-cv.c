#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define DEERS 9
#define ELVES 6
#define ELVES_GROUP 3

int deerCount = 0, elvesCount = 0;

pthread_mutex_t elf_tex;
pthread_mutex_t mutex;
pthread_cond_t santa_cv;
pthread_cond_t reindeer_cv;
pthread_cond_t elf_cv;

void prepareSleigh() {
    printf("Santa preparing sleigh for deers\n");
    sleep(1);
    printf("Sleigh prepared\n");
}

void getHitched(int id) {
    printf("%d deer getting hitched\n", id);
}

void getHelp(int id) {
    printf("%d elf getting help from santa\n", id);
}

void* santa(void* arg) {
    while (1) {
        printf("Santa is sleeping\n");
        pthread_mutex_lock(&mutex);
        while (deerCount < DEERS && elvesCount < ELVES_GROUP) {
            pthread_cond_wait(&santa_cv, &mutex);
        }
        printf("Santa woke up\n");
        if (deerCount == DEERS) {
            prepareSleigh();
            pthread_cond_broadcast(&reindeer_cv);
            deerCount = 0;
        } else if (elvesCount == ELVES_GROUP) {
            printf("Santa is helping elves\n");
            sleep(1);
            pthread_mutex_unlock(&mutex);
            pthread_cond_broadcast(&elf_cv);
        }
        pthread_mutex_unlock(&mutex);
        sleep(3);
        printf("Santa back to sleeping\n");
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
            pthread_cond_signal(&santa_cv);
        }
        pthread_cond_wait(&reindeer_cv, &mutex);
        getHitched(id);
        sleep(1);
        pthread_mutex_unlock(&mutex);
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
            pthread_cond_signal(&santa_cv);
        else
            pthread_mutex_unlock(&elf_tex);
        pthread_cond_wait(&elf_cv, &mutex);
        getHelp(id);
        sleep(1);
        elvesCount--;
        if (elvesCount == 0) {
            printf("Elves group cleared\n");
            pthread_mutex_unlock(&elf_tex);
        }
        pthread_mutex_unlock(&mutex);
    }
}

void* alloc_int(int i) {
    int* p = malloc(sizeof(int));
    *p = i;
    return p;
}

int main() {
    pthread_t santa_thread;
    pthread_t reindeer_threads[DEERS];
    pthread_t elf_threads[ELVES];

    pthread_mutex_init(&elf_tex, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&santa_cv, NULL);
    pthread_cond_init(&reindeer_cv, NULL);
    pthread_cond_init(&elf_cv, NULL);

    pthread_create(&santa_thread, NULL, santa, NULL);
    for (int i = 0; i < DEERS; i++) {
        pthread_create(&reindeer_threads[i], NULL, reeinder, alloc_int(i + 1));
    }
    for (int i = 0; i < ELVES; i++) {
        pthread_create(&elf_threads[i], NULL, elf, alloc_int(i + 1));
    }

    pthread_join(santa_thread, NULL);

    return 0;
}
