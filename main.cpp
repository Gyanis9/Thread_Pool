//
// Created by guo on 24-12-1.
//
#include <iostream>
#include "lib/Any.hpp"
#include "lib/ThreadPool_final.hpp"

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
/*
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
};*/


int add2(int num1, int num2) {
    return num1 + num2;
}

int add3(int num1, int num2, int num3) {
    return num1 + num2 + num3;
}

int add4(int num1, int num2, int num3, int num4) {
    return num1 + num2 + num3 + num4;
}

int main() {
    {
        ThreadPool_final pool;
        pool.start(4);
        std::future<int> result1 = pool.submitTask(add2, 1, 2);
        std::future<int> result2 = pool.submitTask(add3, 1, 2, 3);
        std::future<int> result3 = pool.submitTask(add4, 1, 2, 3, 4);
        std::cout << "resul1: " << result1.get() << std::endl;
        std::cout << "resul2: " << result2.get() << std::endl;
        std::cout << "resul3: " << result3.get() << std::endl;
    }
    getchar();
}
