#include "Shaders.hpp"

Shader &Shader::operator=(Shader &&other)
{
    if (this == &other)
        return *this;

    _delete();
    _id = other._id;

    _sources = std::move(other._sources);
    _log = std::move(other._log);
    _name = std::move(other._name);
    _compiled = other._compiled;
    _auto_free = other._auto_free;

    return *this;
}

void Shader::create()
{
    if (!(_id = glCreateShader(_type))) {
        GLError::RaiseIfError();
        throw GLError("shader creation failed (unknown error)");
    }
}

void Shader::_delete() noexcept
{
    if (_id) {
        glDeleteShader(_id);
        _id = 0;
    }
}

void Shader::add_source(std::shared_ptr<std::string> code_ptr)
{
    assert(code_ptr != nullptr);
    _compiled = false;
    _sources.push_back(std::move(code_ptr));
}

void Shader::compile()
{
    if (_compiled)
        return;
    create();

    std::vector<const char *> source_pointers;
    source_pointers.reserve(_sources.size());
    for (const auto &src: _sources)
        source_pointers.push_back(src->c_str());
    glShaderSource(_id, source_pointers.size(), source_pointers.data(), nullptr);

    glCompileShader(_id);

    int log_len;
    glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        char log[log_len + 1];
        glGetShaderInfoLog(_id, log_len, nullptr, log);
        _log.assign(log, log_len);
    }

    int result;
    glGetShaderiv(_id, GL_COMPILE_STATUS, &result);
    if (result != GL_TRUE)
        throw ShaderCompilationError(fmt::format("failed to compile shader '{}'", _name), _log);
    _compiled = true;
    if (_auto_free)
        free_source();
    GLError::RaiseIfError();
}

ShaderProgram &ShaderProgram::operator=(ShaderProgram &&other)
{
    if (_id)
        glDeleteProgram(_id);
    _id = other._id;
    other._id = 0;
    _log = std::move(other._log);
    return *this;
}

ShaderProgram::~ShaderProgram()
{
    if (_id)
        glDeleteProgram(_id);
}

void ShaderProgram::attach(const Shader &shader)
{
    if (!_id)
        _id = glCreateProgram();
    assert(shader.id() != 0);
    glAttachShader(_id, shader.id());
    _linked = false;
    GLError::RaiseIfError();
}

void ShaderProgram::detach(const Shader &shader)
{
    glDetachShader(_id, shader.id());
    _linked = false;
    GLError::RaiseIfError();
}

void ShaderProgram::link()
{
    if (!_id)
        throw GLError("no shaders were attached to this program");
    if (_linked)
        return;

    glLinkProgram(_id);

    int log_len;
    glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        char log[log_len + 1];
        glGetProgramInfoLog(_id, log_len, nullptr, log);
        _log.assign(log, log_len);
    }

    int result;
    glGetProgramiv(_id, GL_LINK_STATUS, &result);
    if (result != GL_TRUE)
        throw ShaderCompilationError(fmt::format("failed to link program"), _log);
    _linked = true;
    GLError::RaiseIfError();
}
