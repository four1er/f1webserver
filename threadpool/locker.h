#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <semaphore.h>

#include <cstddef>
#include <exception>
class Locker {
 public:
  Locker() {
    if (pthread_mutex_init(&m_mutex, NULL) != 0) {
      throw std::exception();
    }
  }
  ~Locker() { pthread_mutex_destroy(&m_mutex); }

  bool lock() { return pthread_mutex_lock(&m_mutex); }

  bool unlock() { return pthread_mutex_unlock(&m_mutex); }

 private:
  pthread_mutex_t m_mutex;
};

class Sem {
 public:
  Sem() {
    if (sem_init(&m_sem, 0, 0) != 0) {
      throw std::exception();
    }
  }

  Sem(int num) {
    if (sem_init(&m_sem, num, 0) != 0) {
      throw std::exception();
    }
  }

  ~Sem() { sem_destroy(&m_sem); }

  bool wait() { return sem_wait(&m_sem) == 0; }

  bool post() { return sem_post(&m_sem) == 0; }

 private:
  sem_t m_sem;
};

#endif