cmake_minimum_required(VERSION 2.8.4)
project(Ultra_Air_Arcade)

#Include EIgen headers (NOT a library. Eigen is a header-only files)
SET( EIGEN3_INCLUDE_DIR "$ENV{EIGEN3_INCLUDE_DIR}" )
IF( NOT EIGEN3_INCLUDE_DIR )
    MESSAGE( FATAL_ERROR "Please point the environment variable EIGEN3_INCLUDE_DIR to the include directory of your Eigen3 installation.")
ENDIF()
INCLUDE_DIRECTORIES ( "${EIGEN3_INCLUDE_DIR}" )

include_directories(/home/lamcgowa/glfw-3.1.1/include)
link_directories(/home/lamcgowa/glfw-3.1.1/build/src)

# FIND OPENGL
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})

set(GCC_COVERAGE_COMPILE_FLAGS "-g -ansi -pedantic -Wno-deprecated -std=c++11 -O3")
set(GCC_COVERAGE_LINK_FLAGS "-DGL_GLEXT_PROTOTYPES -lglfw3 -lrt -lXrandr -lXinerama -lXi -lXcursor -lGL -lm -ldl -lXrender -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 -lxcb-present -lxcb-sync -lxshmfence -lXxf86vm -lXfixes -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp")

add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})
add_definitions(${GCC_COVERAGE_LINK_FLAGS})


set(SOURCE_FILES main.cpp Camera.cpp  GLSL.cpp  main.cpp  MatrixStack.cpp  Program.cpp  ShapeObj.cpp  tiny_obj_loader.cc)

add_executable(Ultra_Air_Arcade ${SOURCE_FILES})

#link libraries to the executable
message(STATUS ${GLFW_LIBRARIES})


target_link_libraries(Ultra_Air_Arcade ${OPENGL_LIBRARIES} glfw3 ${GLFW_STATIC_LIBRARIES})