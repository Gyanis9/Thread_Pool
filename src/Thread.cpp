//
// Created by guo on 24-12-1.
//

#include "../lib/Thread.h"
#include <thread>

unsigned int Thread::cur_threadID = 0;

Thread::Thread(Thread::Threadfunc func) : func(std::move(func)), m_threadID(cur_threadID++) {
}

void Thread::start(unsigned int threadID) {
    std::thread t(func, threadID);
    t.detach();
}