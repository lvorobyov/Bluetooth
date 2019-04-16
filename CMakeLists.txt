cmake_minimum_required(VERSION 3.13)
project(Bluetooth)

set(CMAKE_CXX_STANDARD 17)
include_directories($ENV{WORK}/include)
add_definitions(-D_UNICODE -DUNICODE)

add_executable(app main.cpp)
target_link_libraries(app ws2_32 bthprops)