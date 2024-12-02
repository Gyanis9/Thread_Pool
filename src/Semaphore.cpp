//
// Created by guo on 24-12-2.
//

#include "../lib/Semaphore.h"

Semaphore::Semaphore(unsigned int limit) : m_res(limit) {

}

void Semaphore::post() {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_res++;
    m_cv.notify_all();
}

void Semaphore::wait() {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait(lock, [&]() -> bool { return m_res > 0; });
    m_res--;
}