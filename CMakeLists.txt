cmake_minimum_required(VERSION 3.10)

project(AppSrcOpenCV)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)

set(SOURCE_FILES appsrc_opencv.cpp)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
    ${GST_APP_INCLUDE_DIRS}
)

add_executable(AppSrcOpenCV ${SOURCE_FILES})
target_link_libraries(AppSrcOpenCV
    ${OpenCV_LIBS}
    ${GST_LIBRARIES}
    ${GST_APP_LIBRARIES}
)
