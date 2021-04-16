#include <iostream>
#include <cmath>
#include <png++/png.hpp>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include "Shaders.hpp"


struct Config
{
    bool print_system_info = true;

    int antialiasing = 4;
    int resolution_width = 1366;
    int resolution_height = 768;
    float field_of_view = 45.0;
    bool fullscreen = false;

    struct {
        int x0 = -20, x1 = 21;
        int z0 = -20, z1 = 21;
    } world_bounds;

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
constexpr GLfloat FLOOR_VERTICES[] = {
    -1.0, -1.0,  -1.0, +1.0,
    -0.5, -1.0,  -0.5, +1.0,
     0.0, -1.0,   0.0, +1.0,
    +0.5, -1.0,  +0.5, +1.0,
    +1.0, -1.0,  +1.0, +1.0,

    -1.0, -1.0,  +1.0, -1.0,
    -1.0, -0.5,  +1.0, -0.5,
    -1.0,  0.0,  +1.0,  0.0,
    -1.0, +0.5,  +1.0, +0.5,
    -1.0, +1.0,  +1.0, +1.0,
};


glm::mat4 ProjectionMatrix;
GLFWwindow *MainWindow;
struct ControlState {
    bool wireframe_mode = false;
    bool cursor_locked = false;
    bool cull_face = true;

    bool operator==(const ControlState &other) const = default;
    bool operator!=(const ControlState &other) const = default;
} ControlState;
struct CameraState {
    float vangle = 0;
    float hangle = 0;

    glm::vec3 position{0};
    glm::vec3 velocity{0};

    void move(float speed)
    {
        if (glm::l1Norm(velocity) < 0.001)
            return;
        velocity = glm::normalize(velocity);
        auto worldspace_x = std::cos(-hangle) * velocity.x + std::sin(-hangle) * velocity.z;
        auto worldspace_z = -std::sin(-hangle) * velocity.x + std::cos(-hangle) * velocity.z;
        position += speed * glm::vec3(worldspace_x, velocity.y, worldspace_z);
    }

    auto compute_view_matrix() const
    {
        glm::mat4 mat(1.0);
        mat = glm::rotate(mat, vangle, glm::vec3(1, 0, 0));
        mat = glm::rotate(mat, hangle, glm::vec3(0, 1, 0));
        mat = glm::translate(mat, -position);
        return mat;
    }
} CameraState;


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
            ControlState.cursor_locked = false;
            break;
        case GLFW_KEY_F1:
            ControlState.wireframe_mode = !ControlState.wireframe_mode;
            ControlState.cull_face = !ControlState.wireframe_mode;
            break;

        // Arrows
        case GLFW_KEY_W:
            CameraState.velocity.z = -1;
            break;
        case GLFW_KEY_S:
            CameraState.velocity.z = 1;
            break;
        case GLFW_KEY_A:
            CameraState.velocity.x = -1;
            break;
        case GLFW_KEY_D:
            CameraState.velocity.x = 1;
            break;
        case GLFW_KEY_SPACE:
        case GLFW_KEY_Q:
            CameraState.velocity.y = 1;
            break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_E:
            CameraState.velocity.y = -1;
            break;
        }
    }

    if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_W:
            CameraState.velocity.z = 0;
            break;
        case GLFW_KEY_S:
            CameraState.velocity.z = 0;
            break;
        case GLFW_KEY_A:
            CameraState.velocity.x = 0;
            break;
        case GLFW_KEY_D:
            CameraState.velocity.x = 0;
            break;
        case GLFW_KEY_SPACE:
        case GLFW_KEY_Q:
            CameraState.velocity.y = 0;
            break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_E:
            CameraState.velocity.y = 0;
            break;
        }
    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS) {
        ControlState.cursor_locked = true;
    }
}

static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (ControlState.cursor_locked) {
        if (std::abs(xpos) + std::abs(ypos) < 10) {
            CameraState.vangle += ypos / 300.0;
            CameraState.hangle += xpos / 300.0;
        }
        glfwSetCursorPos(window, 0, 0);
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

    WindowResizeCallback(window, config.resolution_width, config.resolution_height);
    glfwSetWindowSizeCallback(window, WindowResizeCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPositionCallback);

    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw Error("unable to initialize GLEW");
    }

    return window;
}


auto CompileShader(const std::filesystem::path& vsh_path, const std::filesystem::path& fsh_path)
{
    ShaderProgram program;
    auto vertex_shader = Shader::FromSourceFile<VertexShader>(vsh_path);
    program.attach(vertex_shader);
    auto fragment_shader = Shader::FromSourceFile<FragmentShader>(fsh_path);
    program.attach(fragment_shader);
    program.link();
    program.detach(fragment_shader);
    program.detach(vertex_shader);
    return program;
}


void ApplyControlState()
{
    static struct ControlState current_control_state;
    if (current_control_state == ControlState)
        return;
    current_control_state = ControlState;

    glPolygonMode(GL_FRONT_AND_BACK, ControlState.wireframe_mode ? GL_LINE : GL_FILL);

    if (ControlState.cull_face)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    if (ControlState.cursor_locked)
        glfwSetInputMode(MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(MainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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


    GLuint cube_vao;
    {
        glGenVertexArrays(1, &cube_vao);
        glBindVertexArray(cube_vao);
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

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
        glEnableVertexAttribArray(0);
        GLError::RaiseIfError();
    }

    GLuint floor_vao;
    {
        glGenVertexArrays(1, &floor_vao);
        glBindVertexArray(floor_vao);
        GLError::RaiseIfError();

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(FLOOR_VERTICES), FLOOR_VERTICES, GL_STATIC_DRAW);
        GLError::RaiseIfError();

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
        glEnableVertexAttribArray(0);
        GLError::RaiseIfError();
    }


    ShaderProgram cube_shader, floor_shader;
    try {
        auto shaders_path = config.resource_root / "Shaders";
        cube_shader = CompileShader(shaders_path / "Voxel.vert", shaders_path / "Voxel.frag");
        floor_shader = CompileShader(shaders_path / "GridTile.vert", shaders_path / "GridTile.frag");
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
        ApplyControlState();
        CameraState.move(0.0125);
        auto view_matrix = CameraState.compute_view_matrix();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ++i;

        glUseProgram(cube_shader.id());
        glBindVertexArray(cube_vao);
        cube_shader["ModelMatrix"] = glm::rotate(glm::scale(glm::mat4(1.0), glm::vec3(0.1)), glm::radians(i / 2.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        cube_shader["ViewMatrix"] = view_matrix;
        cube_shader["ProjectionMatrix"] = ProjectionMatrix;
        cube_shader["Position"] = glm::vec3(0, 0, -1.5);
        auto colour = cube_shader["voxel_colour"];
        for (auto i = 0; i <= 6; ++i) {
            colour = glm::vec3((i+1) & 1, (i+1) & 2, (i+1) & 4);
            glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT,(void *) (sizeof(Cube::FACES[0]) * 4 * i));
        }
        GLError::RaiseIfError();

        glUseProgram(floor_shader.id());
        glBindVertexArray(floor_vao);
        floor_shader["ViewMatrix"] = view_matrix;
        floor_shader["ProjectionMatrix"] = ProjectionMatrix;
        auto fpos = cube_shader["Position"];
        for (int x = config.world_bounds.x0; x < config.world_bounds.x1; ++x) {
            for (int z = config.world_bounds.z0; z < config.world_bounds.z1; ++z) {
                fpos = glm::vec3(x, -10.0, z);
                glDrawArrays(GL_LINES, 0, 20);
            }
        }
        GLError::RaiseIfError();

        glBindVertexArray(0);
        glfwSwapBuffers(MainWindow);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}