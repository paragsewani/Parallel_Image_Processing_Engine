#pragma once

#include <string>
#include <functional>
#include <thread>
#include <algorithm>

#include <opencv2/opencv.hpp>

#include "ThreadPool.h"
#include "Benchmark.h"

using namespace std;

class ImageProcessor
{
public:
    ImageProcessor();

    bool loadImage(const string& path);

    bool saveImage(const string& path);

    void convertToGrayParallel();

    void negativeParallel();

    void brightnessParallel(int value);

    void contrastParallel(double alpha);

    void gaussianBlur();

    void sobelEdgeDetection();

    double getLastExecutionTime() const;

    const cv::Mat& getInputImage() const;

    const cv::Mat& getOutputImage() const;

    void reset();

    double convertToGraySequential();

private:

    template<typename F>
    void processRows(F&& task)
    {
        if (inputImage.empty())
            return;

        int rows = inputImage.rows;

        int chunkSize = (rows + threadCount - 1) / threadCount;

        for (unsigned int i = 0; i < threadCount; i++)
        {
            int startRow = i * chunkSize;

            if (startRow >= rows)
                break;

            int endRow = min(startRow + chunkSize, rows);

            pool.submit([=, &task]()
            {
                task(startRow, endRow);
            });
        }

        pool.wait();
    }

    // IMPORTANT: declaration order matters

    unsigned int threadCount;

    ThreadPool pool;

    Benchmark benchmark;

    cv::Mat inputImage;

    cv::Mat outputImage;
};