#pragma once
#include <filesystem>
#include <string>
#include <map>
#include <vector>
#include <GL/gl.h>
#include <png++/png.hpp>


class AssetsRegister
{
    struct TextureAtlas_
    {
        std::string name;
        png::image<png::rgb_pixel> image;
        size_t resolution, x_stride, y_stride;
        GLuint block_texture_array;
        std::map<std::pair<size_t, size_t>, GLuint> loaded_textures;
    };

    std::map<std::string, TextureAtlas_> atlases_;
    std::vector<std::unique_ptr<class BlockModel>> registered_blocks_;

    GLuint get_texture_index_(TextureAtlas_ &source, size_t x, size_t y);

public:
    void read_pack(const std::filesystem::path &manifest_path);

    const BlockModel &block_by_id(uint32_t id) const
    { return *registered_blocks_.at(id - 1); }
};


class BlockModel
{
    friend class AssetsRegister;
    uint32_t id_;
    std::string name_;

    GLuint textures_array_;
    std::array<GLint, 6> textures_indices_;

public:
    GLint used_texture_array_id() const noexcept
    { return static_cast<GLint>(textures_array_); }

    const auto &faces_textures_indices() const noexcept
    { return textures_indices_; }
};
