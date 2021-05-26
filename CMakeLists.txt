cmake_minimum_required(VERSION 3.20)
project(OpenGLTutorial)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(fmt REQUIRED)
find_package(GLEW REQUIRED)
find_package(PNG REQUIRED)
find_package(yaml-cpp REQUIRED)
link_libraries(glfw OpenGL::GL OpenGL::GLU ${GLEW_LIBRARIES} fmt::fmt PNG::PNG ${YAML_CPP_LIBRARIES})

set(GL_LIB_SOURCES
        Source/GL/GLSL/Types.hpp
        Source/GL/Shaders.cpp Source/GL/Shaders.hpp
        Source/GL/Misc.cpp Source/GL/Misc.hpp
        )

add_executable(tutorial ${GL_LIB_SOURCES}
        Source/Main.cpp
        Source/VoxelGrid.cpp Source/VoxelGrid.hpp
        Source/AssetsRegister.cpp Source/AssetsRegister.hpp
        )
