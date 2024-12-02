//
// Created by guo on 24-12-1.
//

#ifndef THREAD_POOL_TASK_H
#define THREAD_POOL_TASK_H

#include "Any.hpp"
#include "Semaphore.h"

class Result;


class Task {
public:
    Task() : m_res(nullptr) {}

    virtual Any run() = 0;

    virtual ~Task() = default;

    void setResult(Result *res) {
        m_res = res;
    }

    void exec();

private:
    Result *m_res = nullptr;
};

class Result {
public:
    Result(std::shared_ptr<Task> task, bool isValid = true) : m_task(std::move(task)), m_isValid(isValid) {
        m_task->setResult(this);
    }

    void setAny(Any any) {
        this->m_any = std::move(any);
        m_sem.post();
    }

    Any get() {
        if (!m_isValid)
            return " ";
        m_sem.wait();
        return std::move(m_any);
    }

    ~Result() = default;

private:
    Semaphore m_sem;
    Any m_any;
    std::shared_ptr<Task> m_task;
    std::atomic_bool m_isValid{};
};


#endif //THREAD_POOL_TASK_H
