#include <iostream>
#include "ThreadPool.h"


using uLong = unsigned long long;

class MyTask : public Task {
public:
    MyTask(uLong begin, uLong end) : m_begin(begin), m_end(end) {}

    Any run() override {
        std::cout << "tid:" << std::this_thread::get_id() << "     "
                  << "begin!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        uLong sum = 0;
        for (uLong i = m_begin; i <= m_end; i++)
            sum += i;
        std::cout << "tid:" << std::this_thread::get_id() << "    "
                  << "end!" << std::endl;

        return sum;
    }

    ~MyTask() = default;

private:
    uLong m_begin;
    uLong m_end;
};


int main() {
    {
        ThreadPool pool;
        pool.setMode(PoolMode::MODE_CACHED);
        pool.start(6);
        Result res1 = pool.submitTask(std::make_shared<MyTask>(0, 100000000));
        Result res2 = pool.submitTask(std::make_shared<MyTask>(100000000, 200000000));
        Result res3 = pool.submitTask(std::make_shared<MyTask>(300000000, 400000000));
        uLong sum1 = res1.get().cast_<uLong>();
        std::cout << sum1 << std::endl;
        uLong sum2 = res2.get().cast_<uLong>();
        std::cout << sum2 << std::endl;
        uLong sum3 = res3.get().cast_<uLong>();
        std::cout << sum3 << std::endl;
        std::cout << "sum = " << sum1 + sum2 + sum3 << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
}
