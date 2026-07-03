#pragma once

#include <opencv2/opencv.hpp>

namespace Filters
{
    void grayscale(
        const cv::Mat& input,
        cv::Mat& output,
        int startRow,
        int endRow
    );

    void negative(
        const cv::Mat& input,
        cv::Mat& output,
        int startRow,
        int endRow
    );

    void brightness(
        const cv::Mat& input,
        cv::Mat& output,
        int startRow,
        int endRow,
        int value
    );

    void contrast(
        const cv::Mat& input,
        cv::Mat& output,
        int startRow,
        int endRow,
        double alpha
    );

    void gaussianBlur(
        const cv::Mat& input,
        cv::Mat& output
    );

    void sobel(
        const cv::Mat& input,
        cv::Mat& output
    );
}