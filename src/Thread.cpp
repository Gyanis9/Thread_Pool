//
// Created by guo on 24-12-1.
//

#include "../lib/Thread.h"
#include <thread>

Thread::Thread(Thread::Threadfunc func) : func(std::move(func)) {

}

void Thread::start() {
    std::thread t(func);
    t.detach();
}