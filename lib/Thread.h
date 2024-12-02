//
// Created by guo on 24-12-1.
//

#ifndef THREAD_POOL_THREAD_H
#define THREAD_POOL_THREAD_H

#include <functional>

class Thread {
public:
    using Threadfunc = std::function<void(int)>;

    explicit Thread(Threadfunc func);

    ~Thread() = default;

    void start(unsigned int threadID);

    [[nodiscard]] unsigned int getID() const { return m_threadID; }

private:
    Threadfunc func;
    unsigned int m_threadID;
    static unsigned int cur_threadID;
};


#endif //THREAD_POOL_THREAD_H
