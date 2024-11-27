//
// Created by guo on 24-11-27.
//
#include "ThreadPool.h"

#include <utility>


const int TASK_MAX_THREADS = 1024;

ThreadPool::ThreadPool() : m_initThreadSize(4), m_taskSize(0), m_taskQueMAXThreadHold(TASK_MAX_THREADS),
                           m_poolMode(PoolMode::MODE_FIXED) {

}

ThreadPool::~ThreadPool() {

}

void ThreadPool::start(int initThreadSize) {
    m_initThreadSize = initThreadSize;
    for (int i = 0; i < m_initThreadSize; ++i)
        m_threads.emplace_back(std::move(std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this))));
    for (int i = 0; i < m_initThreadSize; ++i)
        m_threads[i]->start();
}

[[maybe_unused]] void ThreadPool::setMode(PoolMode mode) {
    m_poolMode = mode;
}


[[maybe_unused]] void ThreadPool::setTaskMaxQueThreadHold(int threadHold) {
    m_taskQueMAXThreadHold = threadHold;
}


void ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    std::unique_lock<std::mutex> lock(m_taskQueMex);
    if (!m_notFull.wait_for(lock, std::chrono::seconds(1),
                            [&]() -> bool { return m_taskQu.size() < m_taskQueMAXThreadHold; })) {
        std::cerr << "task queue is full, submit task error!" << std::endl;
    }
    m_taskQu.emplace(sp);
    m_taskSize++;
    m_notEmpty.notify_all();
}

[[noreturn]] void ThreadPool::threadFunc() {

    std::shared_ptr<Task> task;
    for (;;) {
        {
            {
                std::unique_lock<std::mutex> lock(m_taskQueMex);
                m_notEmpty.wait_for(lock, std::chrono::seconds(1), [&]() -> bool { return !m_taskQu.empty(); });
                task = m_taskQu.front();
                m_taskQu.pop();
                m_taskSize--;
                if (m_taskSize > 0) {
                    m_notEmpty.notify_all();
                }
                m_notFull.notify_all();
            }
            if (task != nullptr) {
                task->run();
            }
        }
    }
}


/**Thread实现*/
Thread::Thread(ThreadFunc func) : m_func(std::move(func)) {

}

void Thread::start() {
    std::thread t(m_func);
    t.detach();
}

Thread::~Thread() {

}
