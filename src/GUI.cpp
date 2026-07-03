#include "GUI.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>

// Avoid conflicts with Windows SDK byte type
namespace {
    using Windows_BYTE = unsigned char;
}

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define byte Windows_BYTE
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#undef byte

GUI::GUI()
    : window(nullptr),
      processor(std::make_unique<ImageProcessor>()),
      selectedFilter(0),
      brightnessValue(0),
      contrastValue(1.0f),
      gaussianKernelSize(5),
      gaussianSigma(1.0f),
      lastExecutionTime(0.0),
      threadCount(0),
      hasImageLoaded(false),
      hasProcessedImage(false),
      showOriginal(true),
      showProcessed(true)
{
    initGLFW();
    initImGui();
    setupStyle();
    threadCount = processor->getThreadCount();
}

GUI::~GUI()
{
    cleanup();
}

void GUI::initGLFW()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                             "Parallel Image Processing Engine", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
}

void GUI::initImGui()
{
    if (!window)
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GUI::setupStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FramePadding = ImVec2(10, 8);
    style.WindowPadding = ImVec2(15, 15);
    style.ItemSpacing = ImVec2(10, 10);
    style.ItemInnerSpacing = ImVec2(8, 8);

    // Scale font by 12%
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);
    if (font == nullptr)
    {
        // Fallback: use default font
        io.FontGlobalScale = 1.12f;
    }

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
}

void GUI::cleanup()
{
    closeImageWindows();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

std::string GUI::openFileDialog()
{
    OPENFILENAMEA ofn = {};
    char szFile[260] = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0JPG Files\0*.jpg;*.jpeg\0PNG Files\0*.png\0BMP Files\0*.bmp\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
    {
        return std::string(ofn.lpstrFile);
    }
    return "";
}

void GUI::handleLoadImage()
{
    std::string filepath = openFileDialog();
    if (!filepath.empty())
    {
        if (processor->loadImage(filepath))
        {
            hasImageLoaded = true;
            hasProcessedImage = false;
            imagePath = filepath;

            size_t lastSlash = filepath.find_last_of("/\\");
            std::string filename = (lastSlash == std::string::npos) ? filepath : filepath.substr(lastSlash + 1);

            showStatusMessage("Image Loaded: " + filename);
            std::cout << "Image loaded: " << filepath << "\n";
        }
        else
        {
            showStatusMessage("Failed to load image");
            hasImageLoaded = false;
        }
    }
}

void GUI::handleSaveImage()
{
    if (!hasProcessedImage)
    {
        showStatusMessage("No processed image to save");
        return;
    }

    // Save processed image to assets/output
    std::string outputPath = "D:/Parallel_Image_Processing_Engine/assets/output/processed.jpg";
    
    if (processor->saveImage(outputPath))
    {
        // Also save the original loaded image to assets/input
        size_t lastSlash = imagePath.find_last_of("/\\");
        std::string filename = (lastSlash == std::string::npos) ? imagePath : imagePath.substr(lastSlash + 1);
        std::string inputPath = "D:/Parallel_Image_Processing_Engine/assets/input/" + filename;
        
        cv::imwrite(inputPath, processor->getInputImage());
        
        showStatusMessage("Image Saved: processed.jpg & input saved");
        std::cout << "Processed image saved to: " << outputPath << "\n";
        std::cout << "Original image saved to: " << inputPath << "\n";
    }
    else
    {
        showStatusMessage("Failed to save image");
    }
}

void GUI::handleRemoveImage()
{
    closeImageWindows();
    hasImageLoaded = false;
    hasProcessedImage = false;
    imagePath = "";
    lastExecutionTime = 0.0;
    selectedFilter = 0;
    brightnessValue = 0;
    contrastValue = 1.0f;
    gaussianKernelSize = 5;
    gaussianSigma = 1.0f;
    showStatusMessage("Image removed");
}

void GUI::handleApplyFilter()
{
    if (!hasImageLoaded)
    {
        showStatusMessage("No image loaded");
        return;
    }

    processor->reset();

    switch (selectedFilter)
    {
    case 0: // Grayscale
        processor->convertToGrayParallel();
        break;
    case 1: // Negative
        processor->negativeParallel();
        break;
    case 2: // Brightness
        processor->brightnessParallel(brightnessValue);
        break;
    case 3: // Contrast
        processor->contrastParallel(static_cast<double>(contrastValue));
        break;
    case 4: // Gaussian Blur
        processor->gaussianBlur();
        break;
    case 5: // Sobel
        processor->sobelEdgeDetection();
        break;
    }

    lastExecutionTime = processor->getLastExecutionTime();
    hasProcessedImage = true;
    showStatusMessage("Filter Applied: " + std::to_string(static_cast<int>(lastExecutionTime)) + " ms");
}

void GUI::closeImageWindows()
{
    cv::destroyAllWindows();
}

void GUI::displayImages()
{
    if (!hasImageLoaded)
        return;

    if (showOriginal)
    {
        updateWindowPreview("Original Image", processor->getInputImage(), 50, 100, 800, 500);
    }

    if (showProcessed && hasProcessedImage)
    {
        updateWindowPreview("Processed Image", processor->getOutputImage(), 900, 100, 800, 500);
    }
}

void GUI::updateWindowPreview(const std::string& windowName, const cv::Mat& image, int x, int y, int width, int height)
{
    if (image.empty())
        return;

    cv::Mat resized;
    float imgAspect = static_cast<float>(image.cols) / static_cast<float>(image.rows);
    float boxAspect = static_cast<float>(width) / static_cast<float>(height);

    int newWidth, newHeight;
    if (imgAspect > boxAspect)
    {
        newWidth = width;
        newHeight = static_cast<int>(width / imgAspect);
    }
    else
    {
        newHeight = height;
        newWidth = static_cast<int>(height * imgAspect);
    }

    cv::resize(image, resized, cv::Size(newWidth, newHeight));

    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, newWidth, newHeight);
    cv::moveWindow(windowName, x, y);
    cv::imshow(windowName, resized);
}

void GUI::showStatusMessage(const std::string& message)
{
    statusMessage = message;
    statusMessageTime = std::chrono::steady_clock::now();
}

void GUI::clearStatusMessage()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - statusMessageTime).count();
    if (elapsed > 3)
    {
        statusMessage = "";
    }
}

void GUI::renderStatusBar()
{
    clearStatusMessage();

    ImGui::SetNextWindowPos(ImVec2(0, WINDOW_HEIGHT - 35), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, 35), ImGuiCond_Always);
    
    ImGui::Begin("##StatusBar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  Status: ");
    ImGui::SameLine();

    if (!statusMessage.empty())
    {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), statusMessage.c_str());
    }
    else if (hasImageLoaded)
    {
        ImGui::TextColored(ImVec4(0.5f, 0.7f, 0.9f, 1.0f), "Ready - Image loaded");
    }
    else
    {
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.5f, 1.0f), "Waiting for image");
    }

    ImGui::SameLine(WINDOW_WIDTH - 280);
    if (hasImageLoaded)
    {
        size_t lastSlash = imagePath.find_last_of("/\\");
        std::string filename = (lastSlash == std::string::npos) ? imagePath : imagePath.substr(lastSlash + 1);
        const cv::Mat& img = processor->getInputImage();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s  |  %d x %d  |  %u threads", 
                          filename.c_str(), img.cols, img.rows, threadCount);
    }

    ImGui::End();
}

void GUI::renderInfoCards()
{
    ImGui::Spacing();
    
    // Get available content region
    ImVec2 contentAvail = ImGui::GetContentRegionAvail();
    float availWidth = contentAvail.x - 20;
    float cardWidth = (availWidth / 3.0f) - 8;

    // Row 1: Image Status, Resolution, Channels
    ImGui::PushID("InfoCardRow1");
    
    ImGui::BeginChild("##CardImageStatus", ImVec2(cardWidth, 90), true);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Image Loaded");
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), hasImageLoaded ? "Yes" : "No");
    ImGui::EndChild();
    ImGui::PopID();

    ImGui::SameLine();

    ImGui::PushID("InfoCardResolution");
    ImGui::BeginChild("##CardResolution", ImVec2(cardWidth, 90), true);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Resolution");
    if (hasImageLoaded)
    {
        const cv::Mat& img = processor->getInputImage();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%d x %d", img.cols, img.rows);
    }
    else
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "-");
    }
    ImGui::EndChild();
    ImGui::PopID();

    ImGui::SameLine();

    ImGui::PushID("InfoCardChannels");
    ImGui::BeginChild("##CardChannels", ImVec2(cardWidth, 90), true);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Channels");
    if (hasImageLoaded)
    {
        const cv::Mat& img = processor->getInputImage();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%d (%s)", img.channels(),
                          img.channels() == 3 ? "BGR" : img.channels() == 1 ? "Gray" : "BGRA");
    }
    else
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "-");
    }
    ImGui::EndChild();
    ImGui::PopID();

    ImGui::Spacing();
    ImGui::Spacing();

    // Row 2: Threads, Execution Time, Speedup
    ImGui::PushID("InfoCardRow2");
    
    ImGui::BeginChild("##CardThreads", ImVec2(cardWidth, 90), true);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Available Threads");
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%u", threadCount);
    ImGui::EndChild();
    ImGui::PopID();

    ImGui::SameLine();

    ImGui::PushID("InfoCardExecTime");
    ImGui::BeginChild("##CardExecTime", ImVec2(cardWidth, 90), true);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Last Execution Time");
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%.3f ms", lastExecutionTime);
    ImGui::EndChild();
    ImGui::PopID();

    ImGui::SameLine();

    ImGui::PushID("InfoCardSpeedup");
    ImGui::BeginChild("##CardSpeedup", ImVec2(cardWidth, 90), true);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Speedup");
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "—");
    ImGui::EndChild();
    ImGui::PopID();
}

void GUI::renderImagePreviews()
{
    ImGui::Spacing();
    ImGui::Spacing();

    ImVec2 contentAvail = ImGui::GetContentRegionAvail();
    float availWidth = contentAvail.x - 20;
    float previewWidth = (availWidth / 2.0f) - 5;
    float previewHeight = contentAvail.y - 40;

    // Original Image Preview
    ImGui::PushID("OriginalImagePreview");
    ImGui::BeginChild("##OriginalPreview", ImVec2(previewWidth, previewHeight), true);
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "ORIGINAL IMAGE");
    ImGui::Separator();

    if (hasImageLoaded)
    {
        const cv::Mat& img = processor->getInputImage();
        ImGui::TextWrapped("Dimensions: %d x %d", img.cols, img.rows);
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.7f, 0.9f, 1.0f), "Click 'Show Images' button to display");
    }
    else
    {
        ImGui::TextWrapped("Load an image to see preview");
    }

    ImGui::EndChild();
    ImGui::PopID();

    ImGui::SameLine();

    // Processed Image Preview
    ImGui::PushID("ProcessedImagePreview");
    ImGui::BeginChild("##ProcessedPreview", ImVec2(previewWidth, previewHeight), true);
    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.7f, 1.0f), "PROCESSED IMAGE");
    ImGui::Separator();

    if (hasProcessedImage)
    {
        const cv::Mat& img = processor->getOutputImage();
        ImGui::TextWrapped("Dimensions: %d x %d", img.cols, img.rows);
        ImGui::Spacing();
        ImGui::Text("Processing Time: %.3f ms", lastExecutionTime);
    }
    else if (hasImageLoaded)
    {
        ImGui::TextWrapped("Apply a filter to process the image");
    }
    else
    {
        ImGui::TextWrapped("Load and process an image");
    }

    ImGui::EndChild();
    ImGui::PopID();
}

void GUI::renderMainArea()
{
    ImGui::SetNextWindowPos(ImVec2(MAIN_AREA_X, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH - MAIN_AREA_X - 20, WINDOW_HEIGHT - 50), ImGuiCond_FirstUseEver);

    ImGui::Begin("##MainPanel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    renderInfoCards();
    renderImagePreviews();

    ImGui::End();
}

void GUI::renderSidebar()
{
    ImGui::SetNextWindowPos(ImVec2(10, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(SIDEBAR_WIDTH, WINDOW_HEIGHT - 50), ImGuiCond_FirstUseEver);

    ImGui::Begin("##Sidebar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

    ImGui::SeparatorText("FILE OPERATIONS");
    ImGui::Spacing();

    // Load Button - Blue
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
    ImGui::PushID("LoadImageButton");
    if (ImGui::Button("Load Image##FileOps", ImVec2(-1, 40)))
    {
        handleLoadImage();
    }
    ImGui::PopID();
    ImGui::PopStyleColor(2);

    // Save Button - Green
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.85f, 0.4f, 1.0f));
    ImGui::PushID("SaveImageButton");
    if (ImGui::Button("Save Image##FileOps", ImVec2(-1, 40)))
    {
        if (hasProcessedImage)
        {
            handleSaveImage();
        }
        else
        {
            showStatusMessage("No processed image to save");
        }
    }
    ImGui::PopID();
    ImGui::PopStyleColor(2);

    // Remove Button - Red
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
    ImGui::PushID("RemoveImageButton");
    if (ImGui::Button("Remove Image##FileOps", ImVec2(-1, 40)))
    {
        handleRemoveImage();
    }
    ImGui::PopID();
    ImGui::PopStyleColor(2);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SeparatorText("FILTER SELECTION");
    ImGui::Spacing();

    ImGui::PushID("FilterSelection");
    ImGui::RadioButton("Grayscale##Filter", &selectedFilter, 0);
    ImGui::RadioButton("Negative##Filter", &selectedFilter, 1);
    ImGui::RadioButton("Brightness##Filter", &selectedFilter, 2);
    ImGui::RadioButton("Contrast##Filter", &selectedFilter, 3);
    ImGui::RadioButton("Gaussian Blur##Filter", &selectedFilter, 4);
    ImGui::RadioButton("Sobel##Filter", &selectedFilter, 5);
    ImGui::PopID();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SeparatorText("FILTER PARAMETERS");
    ImGui::Spacing();

    ImGui::PushID("FilterParams");
    if (selectedFilter == 2) // Brightness
    {
        ImGui::SliderInt("Brightness##Slider", &brightnessValue, -100, 100);
    }
    else if (selectedFilter == 3) // Contrast
    {
        ImGui::SliderFloat("Contrast##Slider", &contrastValue, 0.5f, 3.0f);
    }
    else if (selectedFilter == 4) // Gaussian Blur
    {
        ImGui::SliderInt("Kernel Size##Gaussian", &gaussianKernelSize, 1, 31);
        if (gaussianKernelSize % 2 == 0)
            gaussianKernelSize++;
        ImGui::SliderFloat("Sigma##Gaussian", &gaussianSigma, 0.1f, 5.0f);
    }
    ImGui::PopID();

    ImGui::Spacing();
    ImGui::Spacing();

    // Apply Filter Button - Blue
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.5f, 0.9f, 1.0f));

    bool canApplyFilter = hasImageLoaded;
    if (!canApplyFilter)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    }

    ImGui::PushID("ApplyFilterButton");
    if (ImGui::Button("Apply Filter##Processing", ImVec2(-1, 45)))
    {
        if (canApplyFilter)
            handleApplyFilter();
    }
    ImGui::PopID();

    if (!canApplyFilter)
    {
        ImGui::PopStyleColor();
        ImGui::PopItemFlag();
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::Spacing();

    // Display Options
    ImGui::SeparatorText("DISPLAY OPTIONS");
    ImGui::Spacing();

    ImGui::PushID("DisplayOptions");
    ImGui::Checkbox("Show Original##Display", &showOriginal);
    ImGui::Checkbox("Show Processed##Display", &showProcessed);
    ImGui::PopID();

    ImGui::Spacing();
    ImGui::PushID("ShowImagesButton");
    if (ImGui::Button("Show Images##Display", ImVec2(-1, 35)))
    {
        displayImages();
    }
    ImGui::PopID();

    ImGui::Spacing();
    ImGui::Spacing();

    // Exit Button - Gray
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::PushID("ExitButton");
    if (ImGui::Button("Exit##App", ImVec2(-1, 40)))
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    ImGui::PopID();
    ImGui::PopStyleColor(2);

    ImGui::End();
}

void GUI::renderFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render all panels
    renderSidebar();
    renderMainArea();
    renderStatusBar();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::run()
{
    if (!window)
    {
        std::cerr << "GUI window not initialized\n";
        return;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderFrame();

        // Handle OpenCV window events
        int key = cv::waitKey(1);
        if (key == 27) // ESC key
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glfwSwapBuffers(window);
    }
}
