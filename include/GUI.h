#pragma once

#include <string>
#include <memory>
#include <chrono>
#include "ImageProcessor.h"

struct GLFWwindow;

class GUI
{
public:
    GUI();
    ~GUI();

    void run();

private:
    // Initialization
    void initGLFW();
    void initImGui();
    void setupStyle();
    void cleanup();

    // Main UI
    void renderFrame();
    void renderSidebar();
    void renderMainArea();
    void renderStatusBar();

    // Panels
    void renderInfoCards();
    void renderImagePreviews();

    // Event Handlers
    void handleLoadImage();
    void handleSaveImage();
    void handleRemoveImage();
    void handleApplyFilter();
    void displayImages();
    void closeImageWindows();

    // Utilities
    std::string openFileDialog();
    void showStatusMessage(const std::string& message);
    void clearStatusMessage();
    void updateWindowPreview(const std::string& windowName, const cv::Mat& image, int x, int y, int width, int height);

    // Members
    GLFWwindow* window;
    std::unique_ptr<ImageProcessor> processor;

    // UI State
    std::string imagePath;
    std::string statusMessage;
    std::chrono::steady_clock::time_point statusMessageTime;
    int selectedFilter;
    int brightnessValue;
    float contrastValue;
    int gaussianKernelSize;
    float gaussianSigma;
    double lastExecutionTime;
    unsigned int threadCount;
    bool hasImageLoaded;
    bool hasProcessedImage;
    bool showOriginal;
    bool showProcessed;

    static constexpr int WINDOW_WIDTH = 1600;
    static constexpr int WINDOW_HEIGHT = 1000;
    static constexpr int SIDEBAR_WIDTH = 300;
    static constexpr int MAIN_AREA_X = 320;
};