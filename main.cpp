#include <iostream>
#include "ThreadPool.h"

class MyTask : public Task {
public:
    void run() override {
        std::cout << "begin threadFunc" << ", threadID: " << std::this_thread::get_id() << std::endl;
        std::cout << "end threadFunc" << ", threadID: " << std::this_thread::get_id() << std::endl;
    }
};


int main() {
    ThreadPool pool;
    pool.start(4);
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    while (true) {}
}
