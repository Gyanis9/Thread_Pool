//
// Created by guo on 24-12-1.
//
#include <iostream>

#include <iostream>
#include "lib/ThreadPool.h"

class MyTask : public Task {
public:
    MyTask() = default;

    void run() override {
        std::cout << "tie: " << std::this_thread::get_id() << " begin!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "tie: " << std::this_thread::get_id() << " end!" << std::endl;
    }

    ~MyTask() override = default;
};


int add(int num1, int num2) {
    return num1 + num2;
}

int add(int num1, int num2, int num3) {
    return num1 + num2 + num3;
}

int add(int num1, int num2, int num3, int num4) {
    return num1 + num2 + num3 + num4;
}

int main() {
    ThreadPool pool;
    pool.setMaxThreadSize(4);
    pool.start();
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    getchar();
}
