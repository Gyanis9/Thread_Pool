//
// Created by guo on 24-11-21.
//

#ifndef THREAD_POOL_THREADPOOL_H
#define THREAD_POOL_THREADPOOL_H

#include <iostream>
#include <vector>
#include <memory>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>

/**任务抽象基类,子类继承run()函数运行*/
class Task {
public:
    virtual void run() = 0;
};

/**线程池支持的模式*/
enum class PoolMode {
    MODE_FIXED,
    MODE_CACHED,
};

/**线程类型*/
class Thread {
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func);

    ~Thread();

    void start();

private:
    ThreadFunc m_func;
};
/**
 * example;
 * ThreadPool pool;
 * pool.start(4);
 *
 * class MyTask:public Task{
 * public:
 * void run(){线程代码}
 *}
 *
 * pool.submitTask(std::make_shared<MyTask>());
 *
 * */
/**线程池类型*/
class ThreadPool {
public:
    ThreadPool();

    ~ThreadPool();

    void start(int initThreadSize = 4);

    [[maybe_unused]] void setTaskMaxQueThreadHold(int threadHold);

    void submitTask(std::shared_ptr<Task> sp);

    [[maybe_unused]] void setMode(PoolMode mode);

    /**防止拷贝构造和赋值构造*/
    ThreadPool(const ThreadPool &) = delete;

    ThreadPool &operator=(const ThreadPool &) = delete;

private:
    /**实际运行的线程函数*/
    [[noreturn]] void threadFunc();

private:
    std::vector<std::unique_ptr<Thread>> m_threads;/**线程列表*/
    size_t m_initThreadSize;/**初始线程数量*/
    std::queue<std::shared_ptr<Task>> m_taskQu;/**任务队列*/
    std::atomic<int> m_taskSize;/**任务数量*/
    int m_taskQueMAXThreadHold;/**任务队列最大阈值*/
    PoolMode m_poolMode;/**线程池模式*/


    /**------------进程通信------------*/
    std::mutex m_taskQueMex;
    std::condition_variable m_notEmpty;
    std::condition_variable m_notFull;
};


#endif //THREAD_POOL_THREADPOOL_H
