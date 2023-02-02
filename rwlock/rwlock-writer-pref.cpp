#include "rwlock.h"


void InitalizeReadWriteLock(struct read_write_lock * rw)
{
  //	Write the code for initializing your read-write lock.
  rw->readerCount = 0;
  pthread_mutex_init(&rw->lock, NULL);
  pthread_mutex_init(&rw->writerLock, NULL);
  pthread_cond_init(&rw->cond1, NULL);
  // pthread_cond_init(&rw->waitForReader, NULL);

  rw->writerCount = 0;
}


void ReaderLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the reader.
  pthread_mutex_lock(&rw->lock);
  if(rw->writerCount){
    // pthread_cond_wait()
  }
  if(rw->readerCount == 0){
    pthread_cond_wait(&rw->cond1, &rw->lock);
  }
  ++rw->readerCount;
  pthread_mutex_unlock(&rw->lock);

}

void ReaderUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
  pthread_mutex_lock(&rw->lock);
  pthread_mutex_unlock(&rw->lock);
}

void WriterLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the writer.
  pthread_mutex_lock(&rw->lock);
  pthread_mutex_unlock(&rw->lock);
}

void WriterUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the writer.
  pthread_mutex_lock(&rw->lock);
  pthread_mutex_unlock(&rw->lock);
}
