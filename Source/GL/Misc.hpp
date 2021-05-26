#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <fmt/format.h>
#include <exception>
#include <filesystem>
#include <string_view>

namespace GL {

class Error : public std::exception
{
protected:
    std::string _message;

public:
    Error(std::string message) :
        _message(std::move(message))
    {}

    template<class ...Args>
    Error(std::string_view format, Args... args):
        _message(fmt::format(format, std::forward<Args>(args)...))
    {}

    const std::string &message() noexcept
    { return _message; }

    virtual const char *what() const noexcept override
    { return "Error"; }

    virtual void raise() const
    { throw *this; }

    virtual Error *clone() const
    { return new Error(*this); }
};

class GLError : public Error
{
public:
    static void RaiseIfError();
    using Error::Error;

    const char *what() const noexcept override
    { return "GLError"; }
};

class ShaderCompilationError : public GLError
{
private:
    std::string _compilation_log;
public:
    ShaderCompilationError(std::string message, std::string log) :
        GLError(std::move(message)),
        _compilation_log(std::move(log))
    {}

    const char *what() const noexcept override
    { return "ShaderCompilationError"; }

    const std::string &compilation_log() const noexcept
    { return _compilation_log; }
};


template<typename IntegralType>
constexpr IntegralType RoundToSize(IntegralType value, IntegralType size)
{
    if (value % size != 0)
        return value + (size - value % size);
    return value;
}

std::string ReadFile(const std::filesystem::path &path);
std::tuple<size_t, size_t, std::vector<uint8_t>> LoadImageData(const std::filesystem::path &path);
GLuint LoadTextureImage(const std::filesystem::path &path);

}
