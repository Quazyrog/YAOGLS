#include <iostream>
#include "Shaders.hpp"



struct Config
{
    int antialiasing = 4;
    int resolution_width = 1366;
    int resolution_height = 768;
    bool fullscreen = false;

    std::filesystem::path resource_root = "/home/quazyrog/Desktop/OpenGL/Resources";
};


GLFWwindow *InitMainWindow(const char *title, Config config)
{
    if (!glfwInit())
        throw Error("unable to initialize GLFW");

    auto monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(config.resolution_width, config.resolution_height, title, monitor, nullptr);
    if (!window) {
        glfwTerminate();
        throw Error("unable to create main window");
    }

    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw Error("unable to initialize GLEW");
    }

    return window;
}

static constexpr GLfloat VERTEX_DATA[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    0.0f,  1.0f, 0.0f,
};


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

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLError::RaiseIfError();

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX_DATA), VERTEX_DATA, GL_STATIC_DRAW);
    GLError::RaiseIfError();

    ShaderProgram shader_program;
    try {
        auto vertex_shader = Shader::FromSourceFile<VertexShader>(config.resource_root / "Shader.vert");
        shader_program.attach(vertex_shader);
        auto fragment_shader = Shader::FromSourceFile<FragmentShader>(config.resource_root / "Shader.frag");
        shader_program.attach(fragment_shader);
        shader_program.link();
        shader_program.detach(fragment_shader);
        shader_program.detach(vertex_shader);
        glUseProgram(shader_program.id());
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

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(0);
        GLError::RaiseIfError();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}