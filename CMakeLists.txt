cmake_minimum_required(VERSION 3.5)

project(8i LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Qt flags
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

file(COPY resources/Barbie.png  resources/Beard.png  resources/Bottle.png  resources/CleanPlate.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(8i main.cpp beyerfilter.cpp beyerfilter.h)

target_link_libraries(8i PRIVATE Qt5::Widgets)
