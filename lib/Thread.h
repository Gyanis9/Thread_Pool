//
// Created by guo on 24-12-1.
//

#ifndef THREAD_POOL_THREAD_H
#define THREAD_POOL_THREAD_H

#include <functional>

class Thread {
public:
    using Threadfunc = std::function<void()>;

    explicit Thread(Threadfunc func);

    ~Thread() = default;

    void start();

private:
    Threadfunc func;

};


#endif //THREAD_POOL_THREAD_H
