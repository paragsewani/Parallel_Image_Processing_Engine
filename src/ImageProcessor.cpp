#include "ImageProcessor.h"

#include "Filters.h"

#include <iostream>
#include <thread>
#include <algorithm>

using namespace std;

ImageProcessor::ImageProcessor()
    : threadCount(max(1u, thread::hardware_concurrency())),
      pool(threadCount)
{
}

bool ImageProcessor::loadImage(const string& path)
{
    inputImage = cv::imread(path, cv::IMREAD_COLOR);

    if (inputImage.empty())
    {
        cout << "Failed to load image\n";
        return false;
    }

    outputImage = inputImage.clone();

    cout << "Image Loaded Successfully\n";
    cout << "Width   : " << inputImage.cols << endl;
    cout << "Height  : " << inputImage.rows << endl;
    cout << "Threads : " << threadCount << endl;

    return true;
}

bool ImageProcessor::saveImage(const string& path)
{
    if (outputImage.empty())
        return false;

    return cv::imwrite(path, outputImage);
}

double ImageProcessor::getLastExecutionTime() const
{
    return benchmark.getElapsedTime();
}

void ImageProcessor::convertToGrayParallel()
{
    outputImage = inputImage.clone();

    benchmark.start();

    processRows(
        [&](int startRow, int endRow)
        {
            Filters::grayscale(
                inputImage,
                outputImage,
                startRow,
                endRow);
        });

    benchmark.stop();
}

void ImageProcessor::negativeParallel()
{
    outputImage = inputImage.clone();
    benchmark.start();

    processRows(
        [&](int startRow, int endRow)
        {
            Filters::negative(
                inputImage,
                outputImage,
                startRow,
                endRow);
        });

    benchmark.stop();
}

void ImageProcessor::brightnessParallel(int value)
{
    outputImage = inputImage.clone();
    benchmark.start();

    processRows(
        [&](int startRow, int endRow)
        {
            Filters::brightness(
                inputImage,
                outputImage,
                startRow,
                endRow,
                value);
        });

    benchmark.stop();
}

void ImageProcessor::contrastParallel(double alpha)
{
    outputImage = inputImage.clone();
    benchmark.start();

    processRows(
        [&](int startRow, int endRow)
        {
            Filters::contrast(
                inputImage,
                outputImage,
                startRow,
                endRow,
                alpha);
        });

    benchmark.stop();
}

void ImageProcessor::gaussianBlur()
{
    benchmark.start();

    Filters::gaussianBlur(
        inputImage,
        outputImage);

    benchmark.stop();
}

void ImageProcessor::sobelEdgeDetection()
{
    benchmark.start();

    Filters::sobel(
        inputImage,
        outputImage);

    benchmark.stop();
}

const cv::Mat& ImageProcessor::getInputImage() const
{
    return inputImage;
}

const cv::Mat& ImageProcessor::getOutputImage() const
{
    return outputImage;
}

void ImageProcessor::reset()
{
    if (!inputImage.empty())
    {
        outputImage = inputImage.clone();
    }
}

double ImageProcessor::convertToGraySequential()
{
    benchmark.start();

    outputImage = inputImage.clone();

    for (int i = 0; i < inputImage.rows; i++)
    {
        for (int j = 0; j < inputImage.cols; j++)
        {
            cv::Vec3b pixel = inputImage.at<cv::Vec3b>(i, j);

            unsigned char gray =
                static_cast<unsigned char>(
                    0.114 * pixel[0] +
                    0.587 * pixel[1] +
                    0.299 * pixel[2]);

            outputImage.at<cv::Vec3b>(i, j) =
                cv::Vec3b(gray, gray, gray);
        }
    }

    return benchmark.stop();
}