cmake_minimum_required(VERSION 2.8.4)
project(Ultra_Air_Arcade)

add_subdirectory(lib/glfw3)
include_directories(${OPENGL_INCLUDE_DIRS} lib/glfw3/include)

find_package(OpenGL REQUIRED)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/main.cpp)

add_executable(Ultra_Air_Arcade ${SOURCE_FILES})

target_link_libraries(Ultra_Air_Arcade glfw ${OPENGL_LIBRARIES} ${GLFW_STATIC_LIBRARIES})