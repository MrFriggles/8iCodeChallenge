cmake_minimum_required(VERSION 3.5)

project(8i LANGUAGES CXX)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


add_executable(8i_tests tests_main.cpp ../bayerfilter.h ../bayerfilter.cpp)
target_link_libraries(8i_tests PRIVATE Qt5::Widgets)
