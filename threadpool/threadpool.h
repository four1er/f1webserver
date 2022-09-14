#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>

#include <cstdio>
#include <exception>
#include <list>

#include "./locker.h"

template <typename T> class Threadpool {
  public:
    Threadpool(int thread_number = 8, int max_requests = 10000);
    ~Threadpool();
    bool append(T *request);

  private:
    static void *worker(void *arg);
    void run();

  private:
    int m_thread_number;
    int m_max_requests;
    pthread_t *m_threads; // 线程池数组
    std::list<T *> m_work_queue;
    Locker m_queue_locker;
    Sem m_request_num;
    bool m_stop;
};

template <typename T>
Threadpool<T>::Threadpool(int thread_number, int max_requests)
    : m_thread_number(thread_number), m_max_requests(max_requests),
      m_stop(false), m_threads(nullptr) {
    if (thread_number <= 0 || max_requests <= 0) {
        throw std::exception();
    }
    m_threads = new pthread_t[thread_number];
    if (!m_threads) {
        throw std::exception();
    }

    // 创建thread，并设置detach
    for (int i = 0; i < thread_number; i++) {
        printf("create the %dth thread\n", i);
        if (pthread_create(&m_threads[i], NULL, worker, this) != 0) {
            delete[] m_threads;
            throw std::exception();
        }

        if (pthread_detach(m_threads[i])) {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T> Threadpool<T>::~Threadpool() {
    delete[] m_threads;
    m_threads = nullptr;
}

template <typename T> bool Threadpool<T>::append(T *request) {
    m_queue_locker.lock();
    if (m_work_queue.size() >= m_max_requests) {
        m_queue_locker.unlock();
        return false;
    }
    m_work_queue.push_back(request);
    m_queue_locker.unlock();
    m_request_num.post();
    return true;
}

template <typename T> void *Threadpool<T>::worker(void *arg) {
    Threadpool *pool = (Threadpool *)arg;
    pool->run();
    return pool;
}

template <typename T> void Threadpool<T>::run() {
    while (!m_stop) {
        m_request_num.wait();
        m_queue_locker.lock();
        if (m_work_queue.empty()) {
            m_queue_locker.unlock();
            continue;
        }
        T *request = m_work_queue.front();
        m_work_queue.pop_front();
        m_queue_locker.unlock();
        if (!request) {
            continue;
        }
        request->process();
    }
}

#endif