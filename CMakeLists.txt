cmake_minimum_required(VERSION 2.8.4)
project(Ultra_Air_Arcade)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/main.cpp)
add_executable(Ultra_Air_Arcade ${SOURCE_FILES})