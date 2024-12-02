//
// Created by guo on 24-12-2.
//

#ifndef THREAD_POOL_THREADPOOL_FINAL_HPP
#define THREAD_POOL_THREADPOOL_FINAL_HPP

#include <mutex>
#include <iostream>
#include <vector>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <memory>
#include "Thread.h"
#include <thread>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <unordered_map>
#include <functional>
#include <future>
#include "thread_mode.h"

class ThreadPool_final {

public:
    explicit ThreadPool_final(THREAD_MODE mode = THREAD_MODE::MODE_CACHED) : m_curTaskSize(0), m_spareThreadSize(0),
                                                                             m_isRunning(
                                                                                     false), m_mode(mode),
                                                                             m_curThreadSize(0) {
        loadConfig("config.ini");
    }

    ~ThreadPool_final() {
        m_isRunning = false;
        std::unique_lock<std::mutex> lock(m_TaskQueue_mutex);
        m_notEmpty.notify_all();
        m_isExit.wait(lock, [&]() { return m_tasks.empty(); });
    }

    void start(unsigned int size = 4) {
        m_isRunning = true;
        m_initThreadSize = size;
        m_curThreadSize = size;
        for (unsigned int i = 0; i < m_initThreadSize; ++i) {
            auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool_final::ThreadFunc, this, std::placeholders::_1));
            unsigned int threadID = ptr->getID();
            m_threads.emplace(threadID, std::move(ptr));
        }
        for (auto &x: m_threads) {
            x.second->start(x.second->getID());
            m_spareThreadSize++;
        }
    }

    template<typename Func, typename ...Args>
    auto submitTask(Func &&func, Args &&...args) -> std::future<decltype(func(args...))> {
        using Rtype = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<Rtype()>>(
                std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        std::future<Rtype> result = task->get_future();
        std::unique_lock<std::mutex> lock(m_TaskQueue_mutex);
        if (!m_notFull.wait_for(lock, std::chrono::seconds(3),
                                [&]() -> bool { return (m_curTaskSize < m_maxTaskSize); })) {
            std::cerr << "任务队列已满，提交失败！" << std::endl;
            auto task_error = std::make_shared<std::packaged_task<Rtype()>>([]() -> Rtype { return Rtype(); });
            (*task_error)();
            return task_error->get_future();
        }
        m_tasks.emplace([task]() { (*task)(); });
        m_curTaskSize++;
        m_notEmpty.notify_all();
        if (m_mode == THREAD_MODE::MODE_CACHED && m_curTaskSize > m_spareThreadSize) {
            if (m_curThreadSize >= m_maxThreadSize) {
                std::cerr << "已经达到最大线程数，无法创建新线程。" << std::endl;
            } else {
                std::cout << "正在创建新线程....." << std::endl;
                auto ptr = std::make_unique<Thread>(
                        std::bind(&ThreadPool_final::ThreadFunc, this, std::placeholders::_1));
                unsigned int threadID = ptr->getID();
                m_threads.emplace(threadID, std::move(ptr));
                m_curThreadSize++;
                m_spareThreadSize++;
            }
        }
        return result;
    }

    void setMaxThreadSize(unsigned int size) {
        if (checkState()) {
            std::cout << "thread pool is running，can not change！" << std::endl;
        } else {
            this->m_maxThreadSize = size;
            std::cout << "thread pool max size change success,cur thread pool size is:" << this->m_maxThreadSize
                      << std::endl;
        }
    }

    void setThreadMode(THREAD_MODE mode) {
        if (checkState()) {
            std::cout << "thread pool is running,can not change mode!" << std::endl;
        } else {
            this->m_mode = mode;
            std::cout << "thread pool set success，cur thread pool mode is:"
                      << (this->m_mode == THREAD_MODE::MODE_CACHED ? std::string(
                              "cached mode") : std::string("fixed mode")) << std::endl;
        }
    }

    ThreadPool_final(const ThreadPool_final &) = delete;

    ThreadPool_final &operator=(const ThreadPool_final &) = delete;


protected:
    [[nodiscard]] bool checkState() const {
        return m_isRunning;
    }


    void loadConfig(const std::string &filename = "config.ini") {
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(filename, pt);
        m_initThreadSize = pt.get<unsigned int>("setting.m_initThreadSize");
        m_maxThreadSize = pt.get<unsigned int>("setting.m_maxThreadSize");
        m_maxTaskSize = pt.get<unsigned int>("setting.m_maxTaskSize");
        m_thread_max_spare_time = pt.get<unsigned int>("setting.m_thread_max_spare_time");
        std::cout << "配置已成功重载。" << std::endl;
    }

    void ThreadFunc(int threadID) {
        auto last_time = std::chrono::high_resolution_clock().now();

        for (;;) {
            Task task = nullptr;
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
                        if (m_notEmpty.wait_for(lock, std::chrono::seconds(2)) == std::cv_status::timeout) {
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
                task();
            }
            std::cout << "线程tid: " << std::this_thread::get_id() << "退出..." << std::endl;
            m_spareThreadSize++;
            last_time = std::chrono::high_resolution_clock().now();
        }
    }


private:
    using Task = std::function<void()>;
    std::unordered_map<int, std::unique_ptr<Thread>> m_threads;
    std::atomic<unsigned int> m_initThreadSize{};/**初始程数量数量*/
    std::atomic<unsigned int> m_curThreadSize;/**当前线程数量*/
    std::atomic<unsigned int> m_spareThreadSize;/**空闲线程数量*/
    std::atomic<unsigned int> m_maxThreadSize{};/**线程最大数量*/
    std::atomic<bool> m_isRunning;/**线程池是否运行*/
    THREAD_MODE m_mode;/**线程池运行模式*/

    std::queue<Task> m_tasks;
    std::atomic<unsigned int> m_curTaskSize;/**当前任务数量*/
    std::atomic<unsigned int> m_maxTaskSize{};/**任务最大数量*/


    unsigned int m_thread_max_spare_time;
    /**进程通信*/
    std::mutex m_TaskQueue_mutex;
    std::condition_variable m_notEmpty;
    std::condition_variable m_notFull;
    std::condition_variable m_isExit;
};


#endif //THREAD_POOL_THREADPOOL_FINAL_HPP
