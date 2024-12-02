//
// Created by guo on 24-12-1.
//

#include "../lib/ThreadPool.h"

ThreadPool::ThreadPool(THREAD_MODE mode) : m_curTaskSize(0), m_spareThreadSize(0), m_isRunning(
        false), m_mode(mode), m_curThreadSize(0) {
    loadConfig("config.ini");
}

ThreadPool::~ThreadPool() {
    m_isRunning = false;
    std::unique_lock<std::mutex> lock(m_TaskQueue_mutex);
    m_notEmpty.notify_all();
    m_isExit.wait(lock, [&]() { return m_tasks.empty(); });
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

void ThreadPool::loadConfig(const std::string &filename) {
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(filename, pt);
    m_initThreadSize = pt.get<unsigned int>("setting.m_initThreadSize");
    m_maxThreadSize = pt.get<unsigned int>("setting.m_maxThreadSize");
    m_maxTaskSize = pt.get<unsigned int>("setting.m_maxTaskSize");
    m_thread_max_spare_time = pt.get<unsigned int>("setting.m_thread_max_spare_time");
}

void ThreadPool::ThreadFunc(int threadID) {
    auto last_time = std::chrono::high_resolution_clock().now();
    std::shared_ptr<Task> task = nullptr;
    for (;;) {
        {

            std::unique_lock<std::mutex> lock(m_TaskQueue_mutex);
            std::cout << "线程tid: " << std::this_thread::get_id() << "尝试获取任务中..." << std::endl;
            while (m_curTaskSize == 0) {
                if (!m_isRunning) {
                    m_threads.erase(threadID);
                    std::cout << "ThreadID: " << threadID << " exit!" << std::endl;
                    m_isExit.notify_all();
                    return;
                }
                if (m_mode == THREAD_MODE::MODE_CACHED) {
                    if (std::cv_status::timeout == m_notEmpty.wait_for(lock, std::chrono::seconds(2))) {
                        auto now_time = std::chrono::high_resolution_clock().now();
                        auto dur_time = std::chrono::duration_cast<std::chrono::seconds>(now_time - last_time);
                        if (dur_time.count() > m_thread_max_spare_time) {
                            m_threads.erase(threadID);
                            m_curTaskSize--;
                            m_spareThreadSize--;
                            std::cout << "ThreadID: " << threadID << " exit!" << std::endl;
                            return;
                        }
                    }
                } else {
                    m_notEmpty.wait(lock);;
                }
            }
            m_spareThreadSize--;
            task = m_tasks.front();
            m_tasks.pop();
            std::cout << "线程tid: " << std::this_thread::get_id() << "获取任务成功..." << std::endl;
            m_curTaskSize--;
            if (m_curTaskSize > 0)
                m_notEmpty.notify_all();
            m_notFull.notify_all();
        }
        if (task != nullptr) {
            task->exec();
        }
        std::cout << "线程tid: " << std::this_thread::get_id() << "退出..." << std::endl;
        m_spareThreadSize++;
        last_time = std::chrono::high_resolution_clock().now();
    }
}


/**提交任务，当队伍满载时提交任务等待3s仍未提交成功打印错误信息*/
Result ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    std::unique_lock<std::mutex> lock(m_TaskQueue_mutex);
    if (!m_notFull.wait_for(lock, std::chrono::seconds(3), [&]() -> bool { return (m_curTaskSize < m_maxTaskSize); })) {
        std::cerr << "任务队列已满，提交失败！" << std::endl;
        return Result(sp, false);
    }
    m_tasks.emplace(sp);
    m_curTaskSize++;
    m_notEmpty.notify_all();
    if (m_mode == THREAD_MODE::MODE_CACHED && m_curTaskSize > m_spareThreadSize &&
        m_curThreadSize < m_maxThreadSize) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
        unsigned int threadID = ptr->getID();
        m_threads.emplace(threadID, std::move(ptr));
        m_curThreadSize++;
        m_spareThreadSize++;
    }
    return Result(sp);
}


void ThreadPool::start(unsigned int size) {
    m_isRunning = true;
    m_initThreadSize = size;
    m_curThreadSize = size;
    for (unsigned int i = 0; i < m_initThreadSize; ++i) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
        unsigned int threadID = ptr->getID();
        m_threads.emplace(threadID, std::move(ptr));
    }
    for (auto &x: m_threads) {
        x.second->start(x.second->getID());
        m_spareThreadSize++;
    }
}
