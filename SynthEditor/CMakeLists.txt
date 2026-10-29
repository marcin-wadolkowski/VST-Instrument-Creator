cmake_minimum_required(VERSION 3.21)
project(SynthEditor VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(SynthEditor
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/instrumentcanvas.h
    src/instrumentcanvas.cpp
    src/instrumentmodel.h
)
target_link_libraries(SynthEditor PRIVATE Qt6::Widgets)

if(MSVC)
    target_compile_options(SynthEditor PRIVATE /W4)
else()
    target_compile_options(SynthEditor PRIVATE -Wall -Wextra -Wpedantic)
endif()
