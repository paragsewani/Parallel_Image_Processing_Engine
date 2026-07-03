#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <utility>

using namespace std;

class ThreadPool
{
public:
    explicit ThreadPool(size_t numThreads);

    ~ThreadPool();

    // Prevent copying
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Allow moving (optional but good practice)
    ThreadPool(ThreadPool&&) = default;
    ThreadPool& operator=(ThreadPool&&) = default;

    template<typename F>
    void submit(F&& task)
    {
        {
            unique_lock<mutex> lock(queueMutex);

            tasks.emplace(forward<F>(task));
        }

        taskCondition.notify_one();
    }

    void wait();

private:
    vector<thread> workers;

    queue<function<void()>> tasks;

    mutex queueMutex;

    condition_variable taskCondition;
    condition_variable finishedCondition;

    atomic<int> activeTasks{0};

    bool stop = false;
};