#pragma once

#include <chrono>

using namespace std;

class Benchmark
{
public:
    Benchmark();

    void start();

    double stop();

    double getElapsedTime() const;

    static double calculateSpeedup(double sequential, double parallel);

    void reset();

private:
    chrono::high_resolution_clock::time_point startTime;
    chrono::high_resolution_clock::time_point endTime;

    double elapsedTime;
};