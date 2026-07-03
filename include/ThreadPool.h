#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

using namespace std;

class ThreadPool
{
public:
    explicit ThreadPool(size_t numThreads);

    ~ThreadPool();

    void submit(function<void()> task);

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