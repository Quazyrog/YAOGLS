#include <sstream>
#include <fstream>
#include <png++/png.hpp>
#include "Misc.hpp"

namespace GL {

void GLError::RaiseIfError()
{
    auto code = glGetError();
    if (code != GL_NO_ERROR) {
        unsigned count = 0;
        while (glGetError() != GL_NO_ERROR)
            ++count;
        if (count)
            throw GLError("OpenGL error #{}: {} (and {} more errors)", code, gluErrorString(code), code);
        else
            throw GLError("OpenGL error #{}: {}", code, gluErrorString(code));
    }
}

std::string ReadFile(const std::filesystem::path &path)
{
    std::string content;
    std::ifstream stream(path);
    if (stream.is_open()) {
        std::stringstream sstr;
        sstr << stream.rdbuf();
        return sstr.str();
    }
    throw Error("unable to open file '{}'", path.string());
}

GLuint LoadTextureImage(const std::filesystem::path &path)
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    GLError::RaiseIfError();

    png::image<png::rgba_pixel> image;
    image.read(path);
    const auto width = image.get_width();
    const auto rounded_width = RoundToSize<uint32_t>(width, 4);
    const auto height = image.get_height();
    std::vector<uint8_t> data(rounded_width * height * 4);
    for (auto r = 0; r < height; ++r) {
        for (auto c = 0; c < width; ++c) {
            const auto pixel_offset = 4 * (r * rounded_width + c);
            data[pixel_offset + 0] = image[c][r].red;
            data[pixel_offset + 1] = image[c][r].green;
            data[pixel_offset + 2] = image[c][r].blue;
            data[pixel_offset + 3] = image[c][r].alpha;
        }
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLint>(width), static_cast<GLint>(height), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    return texture;

DELETE_TEXTURE:
    glDeleteTextures(1, &texture);
    return 0;
}

}
