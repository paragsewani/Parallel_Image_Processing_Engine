#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;

int main()
{
    cout << "========== Parallel Image Processing Engine ==========\n\n";

    string imagePath = "assets/input/test.jpg";

    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);

    if (image.empty())
    {
        cout << "Failed to load image!\n";
        return -1;
    }

    cout << "Image Loaded Successfully!\n\n";

    cout << "Width    : " << image.cols << endl;
    cout << "Height   : " << image.rows << endl;
    cout << "Channels : " << image.channels() << endl;

    cv::imwrite("assets/output/output.jpg", image);

    cout << "\nImage saved to assets/output/output.jpg\n";

    return 0;
}