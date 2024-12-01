//
// Created by guo on 24-12-1.
//

#ifndef THREAD_POOL_TASK_H
#define THREAD_POOL_TASK_H


class Task {
public:
    Task() = default;

    virtual void run() = 0;

    virtual ~Task() = default;
};

#endif //THREAD_POOL_TASK_H
