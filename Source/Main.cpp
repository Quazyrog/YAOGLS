#include <iostream>
#include <cmath>
#include <png++/png.hpp>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <map>
#include "GL/Shaders.hpp"
#include "VoxelGrid.hpp"
#include "AssetsRegister.hpp"


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

    std::filesystem::path resource_root = "/home/quazyrog/Desktop/OpenGL_/Resources";
};

namespace Cube {
struct Vertex
{
    glm::vec3 position;
    glm::vec2 tex_coord;
    GLint face_index;
};
constexpr Vertex VERTICES[] = {
    // Front face
    {{-1, -1, +1}, {0, 1}, 0},
    {{+1, -1, +1}, {1, 1}, 0},
    {{-1, +1, +1}, {0, 0}, 0},
    {{+1, +1, +1}, {1, 0}, 0},

    // Back face
    {{+1, -1, -1}, {0, 1}, 1},
    {{-1, -1, -1}, {1, 1}, 1},
    {{+1, +1, -1}, {0, 0}, 1},
    {{-1, +1, -1}, {1, 0}, 1},

    // Left face
    {{-1, -1, -1}, {0, 1}, 2},
    {{-1, -1, +1}, {1, 1}, 2},
    {{-1, +1, -1}, {0, 0}, 2},
    {{-1, +1, +1}, {1, 0}, 2},

    // Right face
    {{+1, -1, +1}, {0, 1}, 3},
    {{+1, -1, -1}, {1, 1}, 3},
    {{+1, +1, +1}, {0, 0}, 3},
    {{+1, +1, -1}, {1, 0}, 3},

    // Bottom face
    {{-1, -1, -1}, {0, 1}, 4},
    {{+1, -1, -1}, {1, 1}, 4},
    {{-1, -1, +1}, {0, 0}, 4},
    {{+1, -1, +1}, {1, 0}, 4},

    // Top face
    {{-1, +1, +1}, {0, 1}, 5},
    {{+1, +1, +1}, {1, 1}, 5},
    {{-1, +1, -1}, {0, 0}, 5},
    {{+1, +1, -1}, {1, 0}, 5},
};
GLuint INDICES[] = {
     0,  1,  2,   2,  1,  3,
     4,  5,  6,   6,  5,  7,
     8,  9, 10,  10,  9, 11,
    12, 13, 14,  14, 13, 15,
    16, 17, 18,  18, 17, 19,
    20, 21, 22,  22, 21, 23
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
constexpr GLfloat UI_VERTICES[] = {
    -0.05, -0.05, 0.0,   0.0, 0.0,
     0.05, -0.05, 0.0,   1.0, 0.0,
    -0.05,  0.05, 0.0,   0.0, 1.0,
     0.05,  0.05, 0.0,   1.0, 1.0,
};


VoxelGrid Grid(48, 24, 48, -24, -8, -24);
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
        auto worldspace_x = std::cos(hangle) * velocity.x + std::sin(hangle) * velocity.z;
        auto worldspace_z = -std::sin(hangle) * velocity.x + std::cos(hangle) * velocity.z;
        position += speed * glm::vec3(worldspace_x, velocity.y, worldspace_z);
    }

    auto compute_view_matrix() const
    {
        glm::mat4 mat(1.0);
        mat = glm::rotate(mat, -vangle, glm::vec3(1, 0, 0));
        mat = glm::rotate(mat, -hangle, glm::vec3(0, 1, 0));
        mat = glm::translate(mat, -position);
        return mat;
    }

    glm::vec3 compute_look_vector() const {
        double x = std::cos(vangle) * std::sin(-hangle);
        double y = std::sin(vangle);
        double z = -std::cos(vangle) * std::cos(-hangle);
        return {x, y, z};
    }
} CameraState;
float ScreenRatio;


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
    ScreenRatio = (float)width / (float)height;
    ProjectionMatrix = glm::perspective(glm::radians(45.0f), ScreenRatio, 0.1f, 500.0f);
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
    if (action != GLFW_PRESS)
        return;
    if (ControlState.cursor_locked) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            auto r = Grid.raycast(CameraState.position, CameraState.compute_look_vector());
            if (r.hit) {
                switch (r.hit_face) {
                case VoxelFace::BACK:
                    r.voxel_z -= 1;
                    break;
                case VoxelFace::FRONT:
                    r.voxel_z += 1;
                    break;
                case VoxelFace::LEFT:
                    r.voxel_x -= 1;
                    break;
                case VoxelFace::RIGHT:
                    r.voxel_x += 1;
                    break;
                case VoxelFace::BOTTOM:
                    r.voxel_y -= 1;
                    break;
                case VoxelFace::TOP:
                    r.voxel_y += 1;
                    break;
                }
            }
            if (r.hit || r.voxel_y == Grid.min_y())
                Grid(r.voxel_x, r.voxel_y, r.voxel_z).block_id = 1;
        }
    } else {
        if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)
            ControlState.cursor_locked = true;
    }
}

static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (ControlState.cursor_locked) {
        if (std::abs(xpos) + std::abs(ypos) < 10) {
            CameraState.vangle -= ypos / 300.0;
            CameraState.hangle -= xpos / 300.0;
        }
        glfwSetCursorPos(window, 0, 0);
    }
}

GLFWwindow *InitMainWindow(const char *title, Config config)
{
    if (!glfwInit())
        throw GL::Error("unable to initialize GLFW");

    auto monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    glfwWindowHint(GLFW_SAMPLES, config.antialiasing);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(config.resolution_width, config.resolution_height, title, monitor, nullptr);
    if (!window) {
        glfwTerminate();
        throw GL::Error("unable to create main window");
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
        throw GL::Error("unable to initialize GLEW");
    }

    return window;
}


auto CompileShader(const std::filesystem::path& vsh_path, const std::filesystem::path& fsh_path)
{
    GL::ShaderProgram program;
    auto vertex_shader = GL::Shader::FromSourceFile<GL::VertexShader>(vsh_path);
    program.attach(vertex_shader);
    auto fragment_shader = GL::Shader::FromSourceFile<GL::FragmentShader>(fsh_path);
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
    AssetsRegister assets;

    for (int z = -4; z < 15; ++z)
        Grid(0, -6, z).block_id = 1 + (abs(z) % 4);

    try {
        MainWindow = InitMainWindow("Hello World", config);
    } catch (GL::Error &e) {
        std::cerr << "ERROR: " << e.message() << std::endl;
        return 1;
    }

    if (config.print_system_info)
        PrintSystemInfo();

    assets.read_pack(config.resource_root / "AssetsPack" / "MANIFEST.yml");

    GLuint cube_vao;
    {
        glGenVertexArrays(1, &cube_vao);
        glBindVertexArray(cube_vao);

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Cube::VERTICES), Cube::VERTICES, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Cube::Vertex), (void*)offsetof(Cube::Vertex, position));
        glEnableVertexAttribArray(0);
        glVertexAttribIPointer(1, 1, GL_INT, sizeof(Cube::Vertex), (void*)offsetof(Cube::Vertex, face_index));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Cube::Vertex), (void*)offsetof(Cube::Vertex, tex_coord));
        glEnableVertexAttribArray(2);

        GLuint ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Cube::INDICES), Cube::INDICES, GL_STATIC_DRAW);

        glBindVertexArray(0);
        GL::GLError::RaiseIfError();
    }

    GLuint floor_vao;
    {
        glGenVertexArrays(1, &floor_vao);
        glBindVertexArray(floor_vao);
        GL::GLError::RaiseIfError();

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(FLOOR_VERTICES), FLOOR_VERTICES, GL_STATIC_DRAW);
        GL::GLError::RaiseIfError();

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
        glEnableVertexAttribArray(0);
        GL::GLError::RaiseIfError();

        glBindVertexArray(0);
    }

    GLuint ui_vao;
    {
        glGenVertexArrays(1, &ui_vao);
        glBindVertexArray(ui_vao);
        GL::GLError::RaiseIfError();

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(UI_VERTICES), UI_VERTICES, GL_STATIC_DRAW);
        GL::GLError::RaiseIfError();

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *) 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        GL::GLError::RaiseIfError();

        glBindVertexArray(0);
    }

    auto indicator_texture = GL::LoadTextureImage(config.resource_root / "Textures" / "sight.png");
    glBindTexture(GL_TEXTURE_2D, indicator_texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    GL::GLError::RaiseIfError();


    GL::ShaderProgram cube_shader, floor_shader, ui_shader;
    try {
        auto shaders_path = config.resource_root / "Shaders";
        cube_shader = CompileShader(shaders_path / "Voxel.vert", shaders_path / "Voxel.frag");
        floor_shader = CompileShader(shaders_path / "Floor.vert", shaders_path / "Floor.frag");
        ui_shader = CompileShader(shaders_path / "UI.vert", shaders_path / "UI.frag");
    } catch (GL::ShaderCompilationError &e) {
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 0);
    while (!glfwWindowShouldClose(MainWindow)) {
        ApplyControlState();
        CameraState.move(0.025);
        auto view_matrix = CameraState.compute_view_matrix();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(cube_shader.id());
        glBindVertexArray(cube_vao);
        cube_shader["ModelMatrix"] = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.5)), glm::vec3(0.5));
        cube_shader["ViewMatrix"] = view_matrix;
        cube_shader["ProjectionMatrix"] = ProjectionMatrix;
        auto attr_position = cube_shader["Position"];
        auto attr_atlas = cube_shader["AtlasArray"];
        auto attr_textures = cube_shader["FaceTextures"];
        for (int x = Grid.min_x(); x < Grid.max_x(); ++x) {
            for (int y = Grid.min_y(); y < Grid.max_y(); ++y) {
                for (int z = Grid.min_z(); z < Grid.max_z(); ++z) {
                    const auto &voxel = Grid(x, y, z);
                    if (voxel.block_id == 0)
                        continue;
                    const auto &block_model = assets.block_by_id(voxel.block_id);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D_ARRAY, block_model.used_texture_array_id());
                    attr_atlas = 0;
                    attr_textures = block_model.faces_textures_indices();
                    GL::GLError::RaiseIfError();
                    attr_position = glm::vec3(x, y, z);
                    glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, nullptr);
                }
            }
        }
        GL::GLError::RaiseIfError();

        glUseProgram(floor_shader.id());
        glBindVertexArray(floor_vao);
        floor_shader["ViewMatrix"] = view_matrix;
        floor_shader["ProjectionMatrix"] = ProjectionMatrix;
        auto fpos = floor_shader["Position"];
        for (int x = Grid.min_x(); x < Grid.max_x(); ++x) {
            for (int z = Grid.min_z(); z < Grid.max_z(); ++z) {
                fpos = glm::vec3(x, Grid.min_y(), z);
                glDrawArrays(GL_LINES, 0, 20);
            }
        }
        GL::GLError::RaiseIfError();

        glDisable(GL_DEPTH_TEST);
        glUseProgram(ui_shader.id());
        ui_shader["ScreenRatio"] = ScreenRatio;
        glBindVertexArray(ui_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, indicator_texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glEnable(GL_DEPTH_TEST);
        GL::GLError::RaiseIfError();

        glBindVertexArray(0);
        glfwSwapBuffers(MainWindow);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}