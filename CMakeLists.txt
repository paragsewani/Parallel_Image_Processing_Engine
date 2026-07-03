cmake_minimum_required(VERSION 3.20)

project(Parallel_Image_Processing_Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)

add_executable(Parallel_Image_Processing_Engine

    src/main.cpp

    src/ThreadPool.cpp
    src/ImageProcessor.cpp
    src/Filters.cpp
    src/Benchmark.cpp
    src/GUI.cpp

    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp

    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(Parallel_Image_Processing_Engine PRIVATE

    include

    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(
    Parallel_Image_Processing_Engine

    PRIVATE

    ${OpenCV_LIBS}

    glfw

    OpenGL::GL
)