#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock* rw) {
    //	Write the code for initializing your read-write lock.
    rw->num_readers = 0;
    rw->num_writers = 0;
    pthread_mutex_init(&rw->mutex_lock, NULL);
    pthread_mutex_init(&rw->write_lock, NULL);
    pthread_cond_init(&rw->read_cv, NULL);
}

void ReaderLock(struct read_write_lock* rw) {
    //	Write the code for aquiring read-write lock by the reader.
    pthread_mutex_lock(&rw->mutex_lock);
    if (rw->num_writers > 0) pthread_cond_wait(&rw->read_cv, &rw->mutex_lock);
    rw->num_readers++;
    if (rw->num_readers == 1) pthread_mutex_lock(&rw->write_lock);
    pthread_mutex_unlock(&rw->mutex_lock);
}

void ReaderUnlock(struct read_write_lock* rw) {
    //	Write the code for releasing read-write lock by the reader.
    pthread_mutex_lock(&rw->mutex_lock);
    rw->num_readers--;
    if (rw->num_readers == 0) pthread_mutex_unlock(&rw->write_lock);
    pthread_mutex_unlock(&rw->mutex_lock);
}

void WriterLock(struct read_write_lock* rw) {
    //	Write the code for aquiring read-write lock by the writer.
    pthread_mutex_lock(&rw->mutex_lock);
    rw->num_writers++;
    pthread_mutex_unlock(&rw->mutex_lock);
    pthread_mutex_lock(&rw->write_lock);
}

void WriterUnlock(struct read_write_lock* rw) {
    //	Write the code for releasing read-write lock by the writer.
    pthread_mutex_lock(&rw->mutex_lock);
    rw->num_writers--;
    if (rw->num_writers == 0) pthread_cond_broadcast(&rw->read_cv);
    pthread_mutex_unlock(&rw->mutex_lock);
    pthread_mutex_unlock(&rw->write_lock);
}
