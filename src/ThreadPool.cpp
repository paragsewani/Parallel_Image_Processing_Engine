#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads)
{
    for (size_t i = 0; i < numThreads; i++)
    {
        workers.emplace_back([this]()
        {
            while (true)
            {
                function<void()> task;

                {
                    unique_lock<mutex> lock(queueMutex);

                    taskCondition.wait(lock, [this]()
                    {
                        return stop || !tasks.empty();
                    });

                    if (stop && tasks.empty())
                    {
                        return;
                    }

                    task = move(tasks.front());
                    tasks.pop();

                    activeTasks++;
                }

                task();

                {
                    unique_lock<mutex> lock(queueMutex);

                    activeTasks--;

                    if (tasks.empty() && activeTasks == 0)
                    {
                        finishedCondition.notify_one();
                    }
                }
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    {
        unique_lock<mutex> lock(queueMutex);
        stop = true;
    }

    taskCondition.notify_all();

    for (thread &worker : workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void ThreadPool::submit(function<void()> task)
{
    {
        unique_lock<mutex> lock(queueMutex);

        tasks.push(move(task));
    }

    taskCondition.notify_one();
}

void ThreadPool::wait()
{
    unique_lock<mutex> lock(queueMutex);

    finishedCondition.wait(lock, [this]()
    {
        return tasks.empty() && activeTasks == 0;
    });
}