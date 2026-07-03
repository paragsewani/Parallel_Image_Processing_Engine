#include "Benchmark.h"

Benchmark::Benchmark()
{
    elapsedTime = 0.0;
}

void Benchmark::start()
{
    startTime = chrono::high_resolution_clock::now();
}

double Benchmark::stop()
{
    endTime = chrono::high_resolution_clock::now();

    elapsedTime =
        chrono::duration<double, milli>(endTime - startTime).count();

    return elapsedTime;
}

double Benchmark::getElapsedTime() const
{
    return elapsedTime;
}

double Benchmark::calculateSpeedup(double sequential, double parallel)
{
    if (parallel == 0.0)
        return 0.0;

    return sequential / parallel;
}

void Benchmark::reset()
{
    elapsedTime = 0.0;
}