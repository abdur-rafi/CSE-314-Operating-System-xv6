#include "rwlock.h"
#include <iostream>

void InitalizeReadWriteLock(struct read_write_lock* rw) {
    //	Write the code for initializing your read-write lock.
    rw->num_readers = 0;
    rw->num_writers = 0;
    sem_init(&rw->binary_lock, 0, 1);
    sem_init(&rw->write_lock, 0, 1);
    pthread_cond_init(&rw->read_cv, NULL);
    pthread_cond_init(&rw->write_cv, NULL);
}

void ReaderLock(struct read_write_lock* rw) {
    //	Write the code for aquiring read-write lock by the reader.
    sem_wait(&rw->binary_lock);
    if (rw->num_writers > 0) pthread_cond_wait(&rw->read_cv, &rw->binary_lock);
    rw->num_readers++;
    if (rw->num_readers == 1) sem_wait(&rw->write_lock);
    sem_post(&rw->binary_lock);
}

void ReaderUnlock(struct read_write_lock* rw) {
    //	Write the code for releasing read-write lock by the reader.
    sem_wait(&rw->binary_lock);
    rw->num_readers--;
    if (rw->num_readers == 0) {
        sem_post(&rw->write_lock);
        if (rw->num_writers > 0) sem_post(&rw->reader_cv);
    }

    sem_post(&rw->binary_lock);
}

void WriterLock(struct read_write_lock* rw) {
    //	Write the code for aquiring read-write lock by the writer.
    sem_wait(&rw->binary_lock);
    rw->num_writers++; // number of waiting writers
    if (rw->num_readers > 0) { // some readers are reading, the first one of them have locked the write_lock
        sem_post(&rw->binary_lock); // let them finish reading
        sem_wait(&rw->reader_cv); // wait for the last reader to release the write_lock
        sem_wait(&rw->binary_lock); // lock the binary_lock again
    }
    sem_wait(&rw->write_lock); // leave a message for the readers that a writer is coming, they cannot read
    rw->num_writers--;
    sem_post(&rw->writer_cv);
    sem_post(&rw->binary_lock);
}

void WriterUnlock(struct read_write_lock* rw) {
    //	Write the code for releasing read-write lock by the writer.
    sem_post(&rw->write_lock);
}
