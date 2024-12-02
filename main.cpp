//
// Created by guo on 24-12-1.
//
#include <iostream>
#include "lib/ThreadPool.h"
#include "lib/Any.hpp"

/*class MyTask : public Task {
public:
    MyTask() = default;

    void run() override {
        std::cout << "线程tie: " << std::this_thread::get_id() << "开始!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "线程tie: " << std::this_thread::get_id() << "结束!" << std::endl;
    }

    ~MyTask() override = default;
};*/

class MyTask : public Task {
public:
    MyTask(int begin, int end) : begin(begin), end(end) {}

    Any run() override {
        std::cout << "线程tie: " << std::this_thread::get_id() << "开始!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "线程tie: " << std::this_thread::get_id() << "结束!" << std::endl;
        int sum = 0;
        for (int i = begin; i < end; ++i)
            sum += i;
        return sum;
    }

    ~MyTask() override = default;

private:
    int begin, end;
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
    auto result1 = pool.submitTask(std::make_shared<MyTask>(1, 10));
    auto result2 = pool.submitTask(std::make_shared<MyTask>(10, 20));
    auto result3 = pool.submitTask(std::make_shared<MyTask>(20, 30));
    auto result4 = pool.submitTask(std::make_shared<MyTask>(30, 40));
    auto result5 = pool.submitTask(std::make_shared<MyTask>(30, 40));
    auto result6 = pool.submitTask(std::make_shared<MyTask>(30, 40));
    auto result7 = pool.submitTask(std::make_shared<MyTask>(30, 40));
    std::cout << result1.get().cast_<int>() << std::endl;
    std::cout << result2.get().cast_<int>() << std::endl;
    std::cout << result3.get().cast_<int>() << std::endl;
    std::cout << result4.get().cast_<int>() << std::endl;
    getchar();

}
