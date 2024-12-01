// Created by guo on 24-11-27.
//
#include "ThreadPool.h"

unsigned int Thread::m_generateID = 0;

/**----------------------------ThreadPool----------------------------------------------*/
ThreadPool::ThreadPool() : m_initThreadSize(4), m_taskSize(0), m_taskQueMAXThreadHold(TASK_MAX_THREADS),
                           m_poolMode(PoolMode::MODE_FIXED), isPoolRunning(false), m_idleThreadSize(0),
                           MAXThreadSizeHold(THREAD_MAX_HOLD), m_curThreadSize(0) {

}

ThreadPool::~ThreadPool() {
    isPoolRunning = false;
    std::unique_lock<std::mutex> lock(m_taskQueMex);
    m_notEmpty.notify_all();
    m_exit.wait(lock, [&]() -> bool { return m_threads.empty(); });
}

void ThreadPool::setThreadSizeThreadHold(int thread_hold) {
    if (checkRunningState()) {
        std::cout << "线程池已启动，无法修改！" << std::endl;
        return;
    }
    if (m_poolMode == PoolMode::MODE_CACHED) {
        MAXThreadSizeHold = thread_hold;
    } else {
        std::cout << "线程池模式固定，无法修改线程数量！" << std::endl;
    }
}

bool ThreadPool::checkRunningState() const {
    return isPoolRunning;
}

[[maybe_unused]] void ThreadPool::setMode(PoolMode mode) {
    if (checkRunningState()) {
        std::cout << "线程池已启动，无法更改模式！" << std::endl;
        return;
    }
    m_poolMode = mode;
}


[[maybe_unused]] void ThreadPool::setTaskMaxQueThreadHold(int threadHold) {
    m_taskQueMAXThreadHold = threadHold;
}

void ThreadPool::start(int initThreadSize) {
    isPoolRunning = true;

    m_initThreadSize = initThreadSize;
    m_curThreadSize = initThreadSize;
    for (int i = 0; i < m_initThreadSize; ++i) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        int threadID = ptr->getID();
        m_threads.emplace(threadID, std::move(ptr));
    }
    for (int i = 0; i < m_initThreadSize; ++i) {
        m_threads[i]->start();
        m_idleThreadSize++;
    }
}

Result ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    std::unique_lock<std::mutex> lock(m_taskQueMex);
    if (!m_notFull.wait_for(lock, std::chrono::seconds(2),
                            [&]() -> bool { return m_taskSize < m_taskQueMAXThreadHold; })) {
        std::cerr << "task queue is full, submit task error!" << std::endl;
        return Result(sp, false);
    }
    m_taskQu.emplace(sp);
    m_taskSize++;
    m_notEmpty.notify_all();
    if (m_poolMode == PoolMode::MODE_CACHED && m_taskSize > m_idleThreadSize && m_curThreadSize < MAXThreadSizeHold) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        int threadID = ptr->getID();
        m_threads.emplace(threadID, std::move(ptr));
        m_curThreadSize++;
        m_threads[std::make_unique<Thread>(
                std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1))->getID()]->start();
        m_idleThreadSize++;
    }
    return Result(sp);
}

void ThreadPool::threadFunc(int threadID) {

    auto last_time = std::chrono::high_resolution_clock().now();
    std::shared_ptr<Task> task= nullptr;
   for(;;) {
        {
            {
                std::unique_lock<std::mutex> lock(m_taskQueMex);
                std::cout << "tid: " << std::this_thread::get_id() << " 尝试获取任务..." << std::endl;
                {
                    while ( m_taskQu.size() == 0) {
                        if (!isPoolRunning) {
                            m_threads.erase(threadID);
                            std::cout << "threadID: " << std::this_thread::get_id() << " exit" << std::endl;
                            m_exit.notify_all();
                            return;
                        }
                        if (m_poolMode == PoolMode::MODE_CACHED) {
                            if (std::cv_status::no_timeout == m_notEmpty.wait_for(lock, std::chrono::seconds(1))) {
                                auto now_time = std::chrono::high_resolution_clock().now();
                                auto dur = std::chrono::duration_cast<std::chrono::seconds>(now_time - last_time);
                                if (dur.count() >= THREAD_MAX_IDLE_TIME && m_curThreadSize > m_initThreadSize) {
                                    m_threads.erase(threadID);
                                    m_curThreadSize--;
                                    m_idleThreadSize--;
                                    std::cout << "tid = " << std::this_thread::get_id() << " exit!" << std::endl;
                                    return ;
                                }
                            }
                        } else {
                            m_notEmpty.wait(lock);
                        }

                    }
                    m_idleThreadSize--;
                    std::cout << "tid: " << std::this_thread::get_id() << " 获取任务成功..." << std::endl;
                    task = m_taskQu.front();
                    m_taskQu.pop();
                    m_taskSize--;
                    if (m_taskSize > 0) {
                        m_notEmpty.notify_all();
                    }
                    m_notFull.notify_all();
                }
                if (task != nullptr) {
                    task->exec();
                }
                m_idleThreadSize++;
                last_time = std::chrono::high_resolution_clock().now();
            }
        }
    }
}

/**-----------------------------------------------Task----------------------------------------------*/
void Task::exec() {
    {
        if (m_result != nullptr)
            m_result->setVal(run());
    }
}

void Task::setResult(Result *res) {
    {
        this->m_result = res;
    }
}

Task::Task() : m_result(nullptr) {}


/**-----------------------------------------------Result--------------------------------------------------------*/
Result::Result(std::shared_ptr<Task>
               task, bool
               isValid) : m_task(task), m_isValid(isValid) {
    m_task->setResult(this);
}

void Result::setVal(Any any) {
    this->m_any = std::move(any);
    m_sem.post();
}

Any Result::get() {
    if (!m_isValid)
        return "";
    m_sem.wait();
    return std::move(m_any);
}


