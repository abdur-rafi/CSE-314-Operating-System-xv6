#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock* rw) {
    //	Write the code for initializing your read-write lock.
    rw->num_readers = 0;
    sem_init(&rw->binary_lock, 0, 1);
    sem_init(&rw->write_lock, 0, 1);
}

void ReaderLock(struct read_write_lock* rw) {
    //	Write the code for aquiring read-write lock by the reader.
    sem_wait(&rw->binary_lock);
    rw->num_readers++;
    if (rw->num_readers == 1) sem_wait(&rw->write_lock);
    sem_post(&rw->binary_lock);
}

void ReaderUnlock(struct read_write_lock* rw) {
    //	Write the code for releasing read-write lock by the reader.
    sem_wait(&rw->binary_lock);
    rw->num_readers--;
    if (rw->num_readers == 0) sem_post(&rw->write_lock);
    sem_post(&rw->binary_lock);
}

void WriterLock(struct read_write_lock* rw) {
    //	Write the code for aquiring read-write lock by the writer.
    sem_wait(&rw->write_lock);
}

void WriterUnlock(struct read_write_lock* rw) {
    //	Write the code for releasing read-write lock by the writer.
    sem_post(&rw->write_lock);
}
