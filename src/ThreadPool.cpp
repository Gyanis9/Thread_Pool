//
// Created by guo on 24-12-1.
//

#include "../lib/ThreadPool.h"

ThreadPool::ThreadPool(unsigned int ThreadSize, THREAD_MODE mode) : m_initThreadSize(
        ThreadSize), m_curTaskSize(0), m_spareThreadSize(0), m_maxThreadSize(MAX_THREAD_SIZE), m_isRunning(
        false), m_mode(mode), m_curThreadSize(0), m_maxTaskSize(MAX_Task_SIZE) {
}

ThreadPool::~ThreadPool() {

}

void ThreadPool::setThreadMode(THREAD_MODE mode) {
    if (checkState()) {
        std::cout << "thread pool is running,can not change mode!" << std::endl;
    } else {
        this->m_mode = mode;
        std::cout << "thread pool set success，cur thread pool mode is:"
                  << (this->m_mode == THREAD_MODE::MODE_CACHED ? std::string(
                          "cached mode") : std::string("fixed mode")) << std::endl;
    }
}

bool ThreadPool::checkState() const {
    return m_isRunning;
}

void ThreadPool::setMaxThreadSize(unsigned int size) {
    if (checkState()) {
        std::cout << "thread pool is running，can not change！" << std::endl;
    } else {
        this->m_maxThreadSize = size;
        std::cout << "thread pool max size change success,cur thread pool size is:" << this->m_maxThreadSize
                  << std::endl;
    }
}

void ThreadPool::loadConfig() {

}

[[noreturn]] void ThreadPool::ThreadFunc() {
    std::shared_ptr<Task> task = nullptr;
    for (;;) {
        {
            std::unique_lock<std::mutex> lock(m_TaskQueue_mutex);
            std::cout << "tid: " << std::this_thread::get_id() << " try get task..." << std::endl;
            m_notEmpty.wait(lock, [&]() -> bool { return m_curTaskSize > 0; });
            task = m_tasks.front();
            m_tasks.pop();
            std::cout << "tid: " << std::this_thread::get_id() << " get task success..." << std::endl;
            m_curTaskSize--;
            if (m_curTaskSize > 0)
                m_notEmpty.notify_all();
            m_notFull.notify_all();
        }
        if (task != nullptr) {
            task->run();
        }
        std::cout << "tid: " << std::this_thread::get_id() << " exit..." << std::endl;
    }
}


void ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    std::unique_lock<std::mutex> lock(m_TaskQueue_mutex);
    if (!m_notFull.wait_for(lock, std::chrono::seconds(3), [&]() -> bool { return (m_curTaskSize < m_maxTaskSize); })) {
        std::cerr << "task queue is full, submit task error" << std::endl;
        return;
    }
    m_tasks.push(std::move(sp));
    m_curTaskSize++;
    m_notEmpty.notify_all();
}


void ThreadPool::start(unsigned int size) {
    m_isRunning = true;
    m_initThreadSize = size;
    m_curThreadSize = size;
    for (unsigned int i = 0; i < m_initThreadSize; ++i)
        m_threads.emplace_back(std::make_unique<Thread>([this] { ThreadFunc(); }));
    for (unsigned int i = 0; i < m_initThreadSize; ++i) {
        m_threads[i]->start();
        m_spareThreadSize++;
    }
}
