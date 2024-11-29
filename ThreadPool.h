//
// Created by guo on 24-11-21.
//

#ifndef THREAD_POOL_THREADPOOL_H
#define THREAD_POOL_THREADPOOL_H

#include <iostream>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>

const int TASK_MAX_THREADS = INT32_MAX;
const int THREAD_MAX_HOLD = 10;
const int THREAD_MAX_IDLE_TIME = 60;

/**上帝类Any*/
class Any {
public:
    Any() = default;

    ~Any() = default;

    Any(const Any &) = delete;

    Any &operator=(const Any &) = delete;

    Any(Any &&) = default;

    Any &operator=(Any &&) = default;

    template<typename T>
    Any(T data) : m_base(std::make_unique<Derive < T>>

    (data)) {}

    template<typename T>
    T cast_() {
        Derive <T> *pd = dynamic_cast<Derive <T> *>(m_base.get());
        if (pd == nullptr) {
            throw "type is not match!";
        }
        return pd->m_date;
    }

private:
    class Base {
    public:
        Base() = default;

        virtual ~Base() = default;
    };

    template<typename T>
    class Derive : public Base {
    public:
        Derive(T data) : m_date(data) {}

        ~Derive() = default;

        T m_date;
    };

private:
    std::unique_ptr<Base> m_base;
};

/**信号量实现*/
class Semaphore {
public:
    Semaphore(int res = 0) : m_res(res) {}

    ~Semaphore() = default;

    /**使用资源*/
    void wait() {
        std::unique_lock<std::mutex> lock(m_myx);
        m_cond.wait(lock, [&]() -> bool { return m_res > 0; });
        --m_res;
    }

    /**增加资源*/
    void post() {
        std::unique_lock<std::mutex> lock(m_myx);
        ++m_res;
        m_cond.notify_all();
    }

private:
    int m_res;
    std::mutex m_myx;
    std::condition_variable m_cond;
};

class Result;

class Task {
public:
    Task();

    ~Task() = default;

    virtual Any run() = 0;

    void exec();

    void setResult(Result *res);

private:
    Result *m_result;
};

/**实现接受提交到线程池的task任务执行完成后的返回值类型result*/
class Result {
public:
    Result(std::shared_ptr<Task> task, bool isValid = true);

    ~Result() = default;

    void setVal(Any any);

    Any get();

private:
    Any m_any;/**存储任务的返回值*/
    Semaphore m_sem;
    std::shared_ptr<Task> m_task;
    std::atomic<bool> m_isValid;
};

/**任务抽象基类,子类继承run()函数运行*/


/**线程类型*/
class Thread {
public:
    using ThreadFunc = std::function<void(int)>;

    explicit Thread(ThreadFunc func) : m_func(std::move(func)), m_threadID(m_generateID++) {}

    ~Thread() = default;

    void start() {
        std::thread t(m_func, m_threadID);
        t.detach();
    }

    unsigned int getID() const {
        return m_threadID;
    }

private:
    ThreadFunc m_func;
    static unsigned int m_generateID;
    unsigned int m_threadID;
};


/**线程池支持的模式*/
enum class PoolMode {
    MODE_FIXED,
    MODE_CACHED,
};

/**线程池类型*/
class ThreadPool {
public:
    ThreadPool();

    ~ThreadPool();

    void start(int initThreadSize = 4);

    void setThreadSizeThreadHold(int thread_hold);

    [[maybe_unused]] void setTaskMaxQueThreadHold(int threadHold);

    Result submitTask(std::shared_ptr<Task> sp);

    [[maybe_unused]] void setMode(PoolMode mode);

    /**防止拷贝构造和赋值构造*/
    ThreadPool(const ThreadPool &) = delete;

    ThreadPool &operator=(const ThreadPool &) = delete;

private:
    /**实际运行的线程函数*/
    void threadFunc(int threadID);

    bool checkRunningState() const;

private:
    std::unordered_map<unsigned int, std::unique_ptr<Thread>> m_threads;/**线程列表*/
    size_t m_initThreadSize;/**初始线程数量*/
    std::atomic<int> m_curThreadSize;/**当前线程总数量*/
    std::atomic<int> m_idleThreadSize;/**空闲线程数量*/
    size_t MAXThreadSizeHold;/**初始线程数量上限阈值*/
    std::atomic<bool> isPoolRunning;/**线程池运行标志*/

    std::queue<std::shared_ptr<Task>> m_taskQu;/**任务队列*/
    std::atomic<int> m_taskSize;/**任务数量*/
    int m_taskQueMAXThreadHold;/**任务队列最大阈值*/

    PoolMode m_poolMode;/**线程池模式*/

    /**------------进程通信------------*/
    std::mutex m_taskQueMex;
    std::condition_variable m_notEmpty;
    std::condition_variable m_notFull;
    std::condition_variable m_exit;
};


#endif //THREAD_POOL_THREADPOOL_H
