//
// Created by guo on 24-11-21.
//

#ifndef THREAD_POOL_THREADPOOL_HPP
#define THREAD_POOL_THREADPOOL_HPP

#include <iostream>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <functional>
#include <queue>


class ThreadPool {

public:
    explicit ThreadPool(int thread_nums = 4);

    template<typename F, typename ...Arg>
    auto enques(F &&f, Arg &&...arg) -> std::future<typename std::result_of<F(Arg...)>::type>;

    ~ThreadPool();
private:
    /**工作函数*/
    void worker();
    bool is_stop;
    std::condition_variable cv;
    std::mutex mx;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> my_queue;
};

ThreadPool::ThreadPool(int thread_nums) : is_stop(false) {
    for (int i = 0; i < thread_nums; ++i) {
        workers.emplace_back([this]() {
            this->worker();
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> varname(mx);
        is_stop = true;
    }
    cv.notify_all();
    for (auto &t: workers)
        t.join();
}

void ThreadPool::worker() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mx);
            cv.wait(lock, [this] { return this->is_stop || !this->my_queue.empty(); });
            if (is_stop && my_queue.empty())
                return;
            task = std::move(this->my_queue.front());
            this->my_queue.pop();
        }
        task();
    }
}

template<typename F, typename... Arg>
auto ThreadPool::enques(F &&f, Arg &&... arg) -> std::future<typename std::result_of<F(Arg...)>::type> {
    using functype = typename std::result_of<F(Arg...)>::type;
    auto task = std::make_shared<std::packaged_task<functype()>>(
            std::bind(std::forward<F>(f), std::forward<Arg>(arg)...));
    auto rsfuture = task->get_future();
    {
        std::lock_guard<std::mutex> lock_guard(this->mx);
        if (is_stop)
            throw std::runtime_error("出错:线程池已经停止了！");
        my_queue.emplace([task]() {
            (*task)();
        });
    }
    cv.notify_one();
    return rsfuture;
}


#endif //THREAD_POOL_THREADPOOL_HPP
