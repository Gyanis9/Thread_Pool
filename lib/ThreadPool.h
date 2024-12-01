//
// Created by guo on 24-12-1.
//

#ifndef THREAD_POOL_THREADPOOL_H
#define THREAD_POOL_THREADPOOL_H
#include <mutex>
#include <iostream>
#include <vector>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <memory>
#include "Thread.h"
#include "Task.h"
#include <thread>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

const unsigned int MAX_THREAD_SIZE = 10;
const unsigned int MAX_Task_SIZE = 1024;

enum class THREAD_MODE {
    MODE_CACHED,
    MODE_FIXED,
};

class ThreadPool {

public:
    explicit ThreadPool(unsigned int ThreadSize = 4, THREAD_MODE mode = THREAD_MODE::MODE_CACHED);

    ~ThreadPool();

    void start(unsigned int size = 4);

    void submitTask(std::shared_ptr<Task> sp);

    void setMaxThreadSize(unsigned int size);

    void setThreadMode(THREAD_MODE mode);


protected:
    [[nodiscard]] bool checkState() const;

    void loadConfig();

    [[noreturn]] [[noreturn]] void ThreadFunc();


private:

    std::vector<std::unique_ptr<Thread>> m_threads;
    std::atomic<unsigned int> m_initThreadSize;/**初始程数量数量*/
    std::atomic<unsigned int> m_curThreadSize;/**当前线程数量*/
    std::atomic<unsigned int> m_spareThreadSize;/**空闲线程数量*/
    std::atomic<unsigned int> m_maxThreadSize;/**线程最大数量*/
    std::atomic<bool> m_isRunning;/**线程池是否运行*/
    THREAD_MODE m_mode;/**线程池运行模式*/

    std::queue<std::shared_ptr<Task>> m_tasks;
    std::atomic<unsigned int> m_curTaskSize;/**当前任务数量*/
    std::atomic<unsigned int> m_maxTaskSize;/**任务最大数量*/


    /**进程通信*/
    std::mutex m_TaskQueue_mutex;
    std::condition_variable m_notEmpty;
    std::condition_variable m_notFull;
    std::condition_variable m_isExit;


};


#endif //THREAD_POOL_THREADPOOL_H
