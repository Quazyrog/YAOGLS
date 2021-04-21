cmake_minimum_required(VERSION 3.20)
project(OpenGLTutorial)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(fmt)
find_package(GLEW REQUIRED)
find_package(PNG REQUIRED)
link_libraries(glfw OpenGL::GL OpenGL::GLU ${GLEW_LIBRARIES} fmt::fmt PNG::PNG)

add_executable(tutorial Main.cpp Shaders.cpp Shaders.hpp Utils.cpp Utils.hpp VoxelGrid.cpp VoxelGrid.hpp)
