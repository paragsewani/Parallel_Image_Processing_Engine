cmake_minimum_required(VERSION 3.20)

project(Parallel_Image_Processing_Engine VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV CONFIG REQUIRED)

 add_executable(Parallel_Image_Processing_Engine
    src/main.cpp
    src/ThreadPool.cpp
    src/ImageProcessor.cpp
    src/Filters.cpp
    src/Benchmark.cpp
)
  

target_include_directories(Parallel_Image_Processing_Engine PRIVATE
    include
)

target_link_libraries(Parallel_Image_Processing_Engine PRIVATE
    ${OpenCV_LIBS}
)