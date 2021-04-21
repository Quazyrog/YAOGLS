#ifndef OPENGLTUTORIAL_VOXELGRID_HPP
#define OPENGLTUTORIAL_VOXELGRID_HPP

#include <vector>
#include <cstring>
#include <bitset>
#include <tuple>
#include <glm/vec3.hpp>


struct Voxel
{
    unsigned block_id;
};

enum class VoxelFace: unsigned { BACK, FRONT, LEFT, RIGHT, BOTTOM, TOP };
struct RaycastResult
{
    bool hit = false;
    bool started_in_bounds;
    int voxel_x, voxel_y, voxel_z;
    VoxelFace hit_face;
    double distance;
};


class VoxelGrid
{
    struct Chunk
    {
        static constexpr int SIZE = 16;

        Voxel data[SIZE * SIZE * SIZE];

        Chunk() { std::memset(data, 0, sizeof(data)); }

        const Voxel &operator()(unsigned x, unsigned y, unsigned z) const
        {
            return data[(x * SIZE + y) * SIZE + z];
        }

        Voxel &operator()(unsigned x, unsigned y, unsigned z)
        {
            return data[(x * SIZE + y) * SIZE + z];
        }
    };

    std::vector<Chunk> _chunks;
    int _x_chunks, _y_chunks, _z_chunks;
    int _x0, _y0, _z0;

    std::tuple<unsigned, unsigned, unsigned, unsigned> _chunk_index(int x, int y, int z) const
    {
        unsigned xx = (x - _x0) / Chunk::SIZE;
        unsigned dx = (x - _x0) % Chunk::SIZE;
        unsigned yy = (y - _y0) / Chunk::SIZE;
        unsigned dy = (y - _y0) % Chunk::SIZE;
        unsigned zz = (z - _z0) / Chunk::SIZE;
        unsigned dz = (z - _z0) % Chunk::SIZE;
        return {(xx * _y_chunks + yy) * _z_chunks + zz, dx, dy, dz};
    }

    bool _contains_voxel(int x, int y, int z) const
    {
        if (x < min_x() || y < min_y() || z < min_z() || x >= max_x() || y > max_y() || z > max_z())
            return false;
        auto [idx, dx, dy, dz] = _chunk_index(x, y, z);
        return _chunks[idx](dx, dy, dz).block_id != 0;
    }

public:
    VoxelGrid() = default;
    VoxelGrid(unsigned width, unsigned height, unsigned depth, int x0=0, int y0=0, int z0=0);

    const Voxel &operator()(int x, int y, int z) const
    {
        auto [idx, dx, dy, dz] = _chunk_index(x, y, z);
        return _chunks[idx](dx, dy, dz);
    }

    Voxel &operator()(int x, int y, int z)
    {
        auto [idx, dx, dy, dz] = _chunk_index(x, y, z);
        return _chunks[idx](dx, dy, dz);
    }

    std::bitset<6> faces_visibility(int x, int y, int z);

    int min_x() const { return _x0; }
    int max_x() const { return _x0 + Chunk::SIZE * _x_chunks - 1; }
    int min_y() const { return _y0; }
    int max_y() const { return _y0 + Chunk::SIZE * _y_chunks - 1; }
    int min_z() const { return _z0; }
    int max_z() const { return _z0 + Chunk::SIZE * _z_chunks - 1; }

    bool in_bounds(int x, int y, int z)
    {
        return min_x() <= x && x < max_x() && min_y() <= y && y < max_y() && min_z() <= z && z < max_z();
    }

    RaycastResult raycast(glm::vec3 o, glm::vec3 r);
};


#endif //OPENGLTUTORIAL_VOXELGRID_HPP
