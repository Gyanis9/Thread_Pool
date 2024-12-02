//
// Created by guo on 24-12-2.
//

#ifndef THREAD_POOL_SEMAPHORE_H
#define THREAD_POOL_SEMAPHORE_H

#include <mutex>
#include <condition_variable>
#include <atomic>

class Semaphore {
public:
    explicit Semaphore(unsigned int limit = 0);

    void post();

    void wait();

    ~Semaphore() = default;

private:
    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::atomic<unsigned int> m_res;
};


#endif //THREAD_POOL_SEMAPHORE_H
