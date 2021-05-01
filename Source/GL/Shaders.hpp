#pragma once
#include "Misc.hpp"
#include <cassert>
#include <memory>
#include <glm/gtc/type_ptr.hpp>

namespace GL {

class Shader
{
protected:
    std::vector<std::shared_ptr<std::string>> _sources;
    std::string _log;
    GLuint _id = 0;
    GLenum _type;
    bool _compiled;
    bool _auto_free;
    std::string _name;

    Shader(GLenum type, std::string name, bool auto_free_source = true) :
        _type(type),
        _auto_free(auto_free_source),
        _name(std::move(name))
    {}

    void _delete() noexcept;

public:
    template<class ShaderClass>
    static ShaderClass FromSourceFile(const std::filesystem::path &source_path)
    {
        ShaderClass shader(source_path.filename().string());
        shader.add_source(source_path);
        shader.compile();
        return shader;
    }

    Shader(const Shader &other) = delete;

    Shader(Shader &&other)
    { *this = std::move(other); }

    Shader &operator=(Shader &&other);

    ~Shader()
    { _delete(); }

    GLuint id() const noexcept
    { return _id; }

    void create();

    bool is_compiled() const noexcept
    { return _compiled; }

    const std::string &compilation_log() const noexcept
    { return _log; }

    const std::string &name() const noexcept
    { return _name; }

    void add_source(std::shared_ptr<std::string> code_ptr);

    void add_source(std::string code)
    { add_source(std::make_shared<std::string>(std::move(code))); }

    void add_source(const std::filesystem::path &source_file_path)
    { add_source(ReadFile(source_file_path)); }

    void free_source() noexcept
    { _sources.clear(); }

    void compile();
};

class VertexShader : public Shader
{
public:
    VertexShader(std::string name = "unnamed vertex shader", bool auto_free_source = true) :
        Shader(GL_VERTEX_SHADER, name, auto_free_source)
    {}
};

class FragmentShader : public Shader
{
public:
    FragmentShader(std::string name = "unnamed fragment shader", bool auto_free_source = true) :
        Shader(GL_FRAGMENT_SHADER, name, auto_free_source)
    {}
};


class UniformValue
{
    friend class ShaderProgram;

    GLuint _program_id = 0;
    GLint _location = -1;

    UniformValue(GLuint program, GLint location) :
        _program_id(program),
        _location(location)
    {}

public:
    UniformValue() = default;
    void operator=(const UniformValue &other) = delete;

    void operator=(int v)
    { glUniform1i(_location, v); }

    void operator=(float v)
    { glUniform1f(_location, v); }

    void operator=(const glm::vec2 &v)
    { glUniform2f(_location, v[0], v[1]); }

    void operator=(const glm::vec3 &v)
    { glUniform3f(_location, v[0], v[1], v[2]); }

    void operator=(const glm::vec4 &v)
    { glUniform4f(_location, v[0], v[1], v[2], v[3]); }

    void operator=(const glm::mat4 &v)
    { glUniformMatrix4fv(_location, 1, GL_FALSE, glm::value_ptr(v)); }
};

class ShaderProgram
{
    GLuint _id = 0;
    std::string _log;
    bool _linked = false;

public:
    ShaderProgram() = default;
    ShaderProgram(const ShaderProgram &other) = delete;

    ShaderProgram(ShaderProgram &&other)
    { *this = std::move(other); }

    ShaderProgram &operator=(ShaderProgram &&other);
    ~ShaderProgram();

    GLuint id() const noexcept
    { return _id; }

    bool is_linked() const noexcept
    { return _linked; }

    const std::string &link_log() const noexcept
    { return _log; }

    void attach(const Shader &shader);
    void detach(const Shader &shader);

    void link();

    UniformValue operator[](const char *uniform_name)
    {
        auto location = glGetUniformLocation(_id, uniform_name);
        GLError::RaiseIfError();
        return UniformValue(_id, location);
    }
};

}