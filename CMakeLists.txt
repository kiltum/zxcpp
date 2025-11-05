cmake_minimum_required(VERSION 4.0)
project(zxcpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "/Users/kiltum/qt610")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(zxcpp main.cpp)
target_link_libraries(zxcpp
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

