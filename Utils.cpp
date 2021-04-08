#include <sstream>
#include <fstream>
#include "Utils.hpp"

void GLError::RaiseIfError()
{
    auto code = glGetError();
    if (code != GL_NO_ERROR) {
        unsigned count = 0;
        while (glGetError() != GL_NO_ERROR)
            ++count;
        if (count)
            throw Error("OpenGL error #{}: {} (and {} more errors)", code, gluErrorString(code), code);
        else
            throw Error("OpenGL error #{}: {}", code, gluErrorString(code));
    }
}

std::string ReadFile(const std::filesystem::path &path)
{
    std::string content;
    std::ifstream stream(path);
    if (stream.is_open()){
        std::stringstream sstr;
        sstr << stream.rdbuf();
        return sstr.str();
    }
    throw Error("unable to open file '{}'", path.string());
}
