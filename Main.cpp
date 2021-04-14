#include <iostream>
#include <cmath>
#include <png++/png.hpp>
#include <functional>
#include "Shaders.hpp"


struct Config
{
    bool print_system_info = true;

    int antialiasing = 4;
    int resolution_width = 1366;
    int resolution_height = 768;
    bool fullscreen = false;

    std::filesystem::path resource_root = "/home/quazyrog/Desktop/OpenGL/Resources";
};

namespace Cube {
GLfloat VERTICES[] = {
    -1.0, -1.0, -1.0,
    -1.0, -1.0,  1.0,
    -1.0,  1.0, -1.0,
    -1.0,  1.0,  1.0,
     1.0, -1.0, -1.0,
     1.0, -1.0,  1.0,
     1.0,  1.0, -1.0,
     1.0,  1.0,  1.0,
};

constexpr unsigned int FACES[] = {
    1, 5, 3, 7,
    0, 2, 4, 6,
    1, 3, 0, 2,
    4, 6, 5, 7,
    4, 5, 0, 1,
    7, 6, 3, 2,
};

}


void PrintSystemInfo()
{
    int iv;
    fmt::print("Vendor:  {}\n", (char*)glGetString(GL_VENDOR));
    fmt::print("Renderer:  {}\n", (char*)glGetString(GL_RENDERER));
    fmt::print("Version:  {}\n", (char*)glGetString(GL_VERSION));
    fmt::print("GLSL Version:  {}\n", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &iv);
    fmt::print("Max Vertex Attribs: {}\n", iv);
}


void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    static bool wireframe_mode = false;
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        if (wireframe_mode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        wireframe_mode = !wireframe_mode;
    }
}


GLFWwindow *InitMainWindow(const char *title, Config config)
{
    if (!glfwInit())
        throw Error("unable to initialize GLFW");

    auto monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    glfwWindowHint(GLFW_SAMPLES, config.antialiasing);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(config.resolution_width, config.resolution_height, title, monitor, nullptr);
    if (!window) {
        glfwTerminate();
        throw Error("unable to create main window");
    }

    glfwSetKeyCallback(window, KeyCallback);

    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw Error("unable to initialize GLEW");
    }

    return window;
}

glm::vec3 ColorFrom(int x, int y, int z)
{
    return glm::vec3(0.5 + x / 30.0, 0.5 + y / 6.0, 0.5 + z / 30.0);
}


int main(void)
{
    Config config;

    GLFWwindow *window;
    try {
        window = InitMainWindow("Hello World", config);
    } catch (Error &e) {
        std::cerr << "ERROR: " << e.message() << std::endl;
        return 1;
    }

    if (config.print_system_info)
        PrintSystemInfo();


    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLError::RaiseIfError();

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Cube::VERTICES), Cube::VERTICES, GL_STATIC_DRAW);
    GLError::RaiseIfError();

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Cube::FACES), Cube::FACES, GL_STATIC_DRAW);
    GLError::RaiseIfError();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    GLError::RaiseIfError();


    ShaderProgram shader_program;
    try {
        auto vertex_shader = Shader::FromSourceFile<VertexShader>(config.resource_root / "Voxel.vert");
        shader_program.attach(vertex_shader);
        auto fragment_shader = Shader::FromSourceFile<FragmentShader>(config.resource_root / "Voxel.frag");
        shader_program.attach(fragment_shader);
        shader_program.link();
        shader_program.detach(fragment_shader);
        shader_program.detach(vertex_shader);
    } catch (ShaderCompilationError &e) {
        std::cerr << e.what() << ": " << e.message() << "\n";
        for (auto c: e.compilation_log()) {
            if (c == '\n')
                std::cerr << "    \n";
            else
                std::cerr << c;
        }
        std::cerr << std::endl;
        return 1;
    }


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glEnable(GL_CULL_FACE);
    glClearDepth(-1.0);
    glClearColor(0, 0, 0, 0);
    int i = 0;
    while (!glfwWindowShouldClose(window)) {
        ++i;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_program.id());
        glBindVertexArray(vao);

        auto position = shader_program["worldspace_position"];
        auto colour = shader_program["voxel_colour"];
        int j = 0;
        for (int x = 10; x >= -10; --x) {
            for (int z = -10; z <= 10; ++z) {
                for (int y = -10; y <= 10; ++y) {
                    if (j > i) goto BAD;
                    j += 1;
                    position = glm::vec3(x, y, z);
                    for (auto i = 1; i <= 6; ++i) {
                        colour = glm::vec3((i+1) & 1, (i+1) & 2, (i+1) & 4);
                        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT,
                                       (void *) (sizeof(Cube::FACES[0]) * 4 * i));
                    }
                }
            }
        }
        BAD:

        glBindVertexArray(0);
        GLError::RaiseIfError();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}