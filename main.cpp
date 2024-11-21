#include <iostream>
#include "ThreadPool.hpp"

int main() {
    ThreadPool thread_pool(4);
    for (int i = 0; i < 1000; ++i) {
        auto rsfuture = thread_pool.enques([](int a, int b) -> int {
            std::cout << "当线程：" << std::this_thread::get_id() << std::endl;
            return a + b;
        }, 10 * i, 10 * i);
        std::cout << "thread rs:" << rsfuture.get() << std::endl;
    }
    return 0;
}
