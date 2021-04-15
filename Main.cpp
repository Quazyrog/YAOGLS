#include <iostream>
#include <cmath>
#include <png++/png.hpp>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include "Shaders.hpp"


struct Config
{
    bool print_system_info = true;

    int antialiasing = 4;
    int resolution_width = 1366;
    int resolution_height = 768;
    float field_of_view = 45.0;
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


glm::mat4 ProjectionMatrix;
GLFWwindow *MainWindow;
struct {
    bool wireframe_mode = false;
    bool cursor_locked = false;
    bool cull_face = true;
} ControlState;
bool ControlStateChanged = true;


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

void WindowResizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    ProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 500.0f);
}

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
        case GLFW_KEY_W:
            ControlState.wireframe_mode = !ControlState.wireframe_mode;
            ControlState.cull_face = !ControlState.wireframe_mode;
            ControlStateChanged = true;
            break;
        }
    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

    WindowResizeCallback(window, config.resolution_width, config.resolution_height);
    glfwSetWindowSizeCallback(window, WindowResizeCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw Error("unable to initialize GLEW");
    }

    return window;
}


int main(void)
{
    Config config;

    try {
        MainWindow = InitMainWindow("Hello World", config);
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
    glClearColor(0, 0, 0, 0);
    int i = 0;
    while (!glfwWindowShouldClose(MainWindow)) {
        if (ControlStateChanged) {
            glPolygonMode(GL_FRONT_AND_BACK, ControlState.wireframe_mode ? GL_LINE : GL_FILL);
            if (ControlState.cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
            ControlStateChanged = false;
        }

        ++i;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_program.id());
        glBindVertexArray(vao);
        shader_program["ModelMatrix"] = glm::rotate(glm::scale(glm::mat4(1.0), glm::vec3(0.1)), glm::radians(i / 2.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        shader_program["ViewMatrix"] = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, sin(i/100.0)));
        shader_program["ProjectionMatrix"] = ProjectionMatrix;

        shader_program["Position"] = glm::vec3(0, 0, -1.5);
        auto colour = shader_program["voxel_colour"];
        for (auto i = 0; i <= 6; ++i) {
            colour = glm::vec3((i+1) & 1, (i+1) & 2, (i+1) & 4);
            glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT,(void *) (sizeof(Cube::FACES[0]) * 4 * i));
        }

        glBindVertexArray(0);
        GLError::RaiseIfError();
        glfwSwapBuffers(MainWindow);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}