#include <yaml-cpp/yaml.h>
#include <GL/glew.h>
#include "AssetsRegister.hpp"
#include "GL/Misc.hpp"


void AssetsRegister::read_pack(const std::filesystem::path &manifest_path)
{
    const auto manifest = YAML::LoadFile(manifest_path);
    const auto root_path = manifest_path.parent_path();
    const auto prefix = manifest["prefix"].as<std::string>();

    for (const auto &pack_object: manifest["objects"]) {
        const auto type = pack_object["type"].as<std::string>();

        if (type == "BLOCKS_TEXTURE_ATLAS") {
            TextureAtlas_ atlas;
            const auto name = pack_object["name"].as<std::string>();;
            atlas.name = name;
            atlas.resolution = pack_object["resolution"].as<size_t>();
            atlas.x_stride = pack_object["x_stride"].as<size_t>();
            atlas.y_stride = pack_object["y_stride"].as<size_t>();
            atlas.image.read(root_path / pack_object["file"].as<std::string>());

            glGenTextures(1, &atlas.block_texture_array);
            glBindTexture(GL_TEXTURE_2D_ARRAY, atlas.block_texture_array);
            // fixme: proper layer count
            glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, static_cast<GLsizei>(atlas.resolution),
                           static_cast<GLsizei>(atlas.resolution), 256);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            GL::GLError::RaiseIfError();

            atlases_[name] = std::move(atlas);

        } else if (type == "BLOCKS_REGISTER") {
            const auto reg_data = YAML::LoadFile(root_path / pack_object["file"].as<std::string>());
            auto &used_atlas = atlases_.at(reg_data["textures_source"].as<std::string>());
            for (const auto &block: reg_data["blocks"]) {
                std::unique_ptr<BlockModel> model = std::make_unique<BlockModel>();
                model->id_ = registered_blocks_.size() + 1;
                model->name_ = block["name"].as<std::string>();
                model->textures_array_ = used_atlas.block_texture_array;
                for (const auto &tex: block["textures"]) {
                    const char *FACE_SHORT_ID = "bflrdu";
                    const auto tile_x = tex.second[0].as<size_t>();
                    const auto tile_y = tex.second[1].as<size_t>();
                    for (const auto face_char: tex.first.as<std::string>()) {
                        if (auto ptr = strchr(FACE_SHORT_ID, face_char)) {
                            auto face_index = ptr - FACE_SHORT_ID;
                            model->textures_indices_[face_index] = get_texture_index_(used_atlas, tile_x, tile_y);
                        } else {
                            // todo: exception
                        }
                    }
                }
                registered_blocks_.push_back(std::move(model));
            }
        }
    }
}

GLuint AssetsRegister::get_texture_index_(AssetsRegister::TextureAtlas_ &source, const size_t x, const size_t y)
{
    auto it = source.loaded_textures.find({x, y});
    if (it != source.loaded_textures.end())
        return it->second;
    // fixme: alignment
    std::vector<uint8_t> tile_rgb(3 * source.resolution * source.resolution);

    // Load pixel data
    const auto copy_x0 = (x - 1) * source.resolution;
    const auto copy_x1 = std::min<size_t>(copy_x0 + source.resolution, source.image.get_width());
    const auto copy_y0 = (y - 1) * source.resolution;
    const auto copy_y1 = std::min<size_t>(copy_y0 + source.resolution, source.image.get_height());
    size_t pixels = 0;
    for (size_t px = copy_x0; px < copy_x1; ++px) {
        for (size_t py = copy_y0; py < copy_y1; ++py) {
            tile_rgb[3 * pixels + 0] = source.image[px][py].red;
            tile_rgb[3 * pixels + 1] = source.image[px][py].green;
            tile_rgb[3 * pixels + 2] = source.image[px][py].blue;
            ++pixels;
        }
    }

    // Upload pixels to GPU
    const auto index = static_cast<GLsizei>(source.loaded_textures.size());
    glBindTexture(GL_TEXTURE_2D_ARRAY, source.block_texture_array);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index,
                    static_cast<GLsizei>(source.resolution), static_cast<GLsizei>(source.resolution), 1,
                    GL_RGB, GL_UNSIGNED_BYTE, tile_rgb.data());
    GL::GLError::RaiseIfError();

    // Store and return tile load location
    source.loaded_textures.insert(it, {{x, y}, index});
    return index;
}


