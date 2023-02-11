#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <semaphore.h>

using namespace std;

struct read_write_lock
{
    pthread_mutex_t mutex_lock;
    pthread_mutex_t write_lock;
    pthread_cond_t read_cv;
    pthread_cond_t write_cv;
    int num_readers;
    int num_writers;
};

void InitalizeReadWriteLock(struct read_write_lock * rw);
void ReaderLock(struct read_write_lock * rw);
void ReaderUnlock(struct read_write_lock * rw);
void WriterLock(struct read_write_lock * rw);
void WriterUnlock(struct read_write_lock * rw);
