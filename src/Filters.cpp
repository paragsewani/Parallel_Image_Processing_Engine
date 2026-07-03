#include "Filters.h"

#include <algorithm>

using namespace std;

namespace Filters
{

void grayscale(
    const cv::Mat& input,
    cv::Mat& output,
    int startRow,
    int endRow)
{
    for (int i = startRow; i < endRow; i++)
    {
        for (int j = 0; j < input.cols; j++)
        {
            cv::Vec3b pixel = input.at<cv::Vec3b>(i, j);

            unsigned char gray =
                static_cast<unsigned char>(
                    0.114 * pixel[0] +
                    0.587 * pixel[1] +
                    0.299 * pixel[2]);

            output.at<cv::Vec3b>(i, j) =
                cv::Vec3b(gray, gray, gray);
        }
    }
}

void negative(
    const cv::Mat& input,
    cv::Mat& output,
    int startRow,
    int endRow)
{
    for (int i = startRow; i < endRow; i++)
    {
        for (int j = 0; j < input.cols; j++)
        {
            cv::Vec3b pixel = input.at<cv::Vec3b>(i, j);

            output.at<cv::Vec3b>(i, j)[0] = 255 - pixel[0];
            output.at<cv::Vec3b>(i, j)[1] = 255 - pixel[1];
            output.at<cv::Vec3b>(i, j)[2] = 255 - pixel[2];
        }
    }
}

void brightness(
    const cv::Mat& input,
    cv::Mat& output,
    int startRow,
    int endRow,
    int value)
{
    for (int i = startRow; i < endRow; i++)
    {
        for (int j = 0; j < input.cols; j++)
        {
            cv::Vec3b pixel = input.at<cv::Vec3b>(i, j);

            for (int c = 0; c < 3; c++)
            {
                int temp = pixel[c] + value;

                temp = std::clamp(temp, 0, 255);

                output.at<cv::Vec3b>(i, j)[c] =
                    static_cast<unsigned char>(temp);
            }
        }
    }
}

void contrast(
    const cv::Mat& input,
    cv::Mat& output,
    int startRow,
    int endRow,
    double alpha)
{
    for (int i = startRow; i < endRow; i++)
    {
        for (int j = 0; j < input.cols; j++)
        {
            cv::Vec3b pixel = input.at<cv::Vec3b>(i, j);

            for (int c = 0; c < 3; c++)
            {
                int temp = static_cast<int>(pixel[c] * alpha);

                temp = std::clamp(temp, 0, 255);

                output.at<cv::Vec3b>(i, j)[c] =
                    static_cast<unsigned char>(temp);
            }
        }
    }
}

void gaussianBlur(
    const cv::Mat& input,
    cv::Mat& output)
{
    cv::GaussianBlur(
        input,
        output,
        cv::Size(9, 9),
        2.0);
}

void sobel(
    const cv::Mat& input,
    cv::Mat& output)
{
    cv::Mat gray;

    cv::cvtColor(
        input,
        gray,
        cv::COLOR_BGR2GRAY);

    cv::Mat gradX;
    cv::Mat gradY;

    cv::Sobel(
        gray,
        gradX,
        CV_16S,
        1,
        0);

    cv::Sobel(
        gray,
        gradY,
        CV_16S,
        0,
        1);

    cv::Mat absX;
    cv::Mat absY;

    cv::convertScaleAbs(gradX, absX);
    cv::convertScaleAbs(gradY, absY);

    cv::Mat result;

    cv::addWeighted(
        absX,
        0.5,
        absY,
        0.5,
        0.0,
        result);

    cv::cvtColor(
        result,
        output,
        cv::COLOR_GRAY2BGR);
}

}