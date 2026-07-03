#include <iostream>

#include "ImageProcessor.h"
#include "Benchmark.h"

using namespace std;

int main()
{
    ImageProcessor processor;

    if (!processor.loadImage("assets/input/test.jpg"))
    {
        return -1;
    }

    double sequentialTime = processor.convertToGraySequential();

    processor.saveImage("assets/output/sequential.jpg");

    processor.reset();

    processor.convertToGrayParallel();

    double parallelTime = processor.getLastExecutionTime();

    processor.saveImage("assets/output/parallel.jpg");

    cout << "\n========== Benchmark ==========\n";
    cout << "Sequential : " << sequentialTime << " ms\n";
    cout << "Parallel   : " << parallelTime << " ms\n";
    cout << "Speedup    : "
         << Benchmark::calculateSpeedup(sequentialTime, parallelTime)
         << "x\n";

    return 0;
}