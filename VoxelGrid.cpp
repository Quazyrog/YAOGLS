#include <cmath>
#include <glm/geometric.hpp>
#include <limits>
#include "VoxelGrid.hpp"
#include "fmt/format.h"

namespace {

template<class Float>
constexpr Float UntilGridHit(Float origin, Float speed, Float min_speed=0.001)
{
    if (std::abs(speed) < min_speed)
        return std::numeric_limits<Float>::infinity();
    if (speed > 0) {
        return (std::ceil(origin) - origin) / speed;
    }
    return (origin - std::floor(origin)) / -speed;
}

}

VoxelGrid::VoxelGrid(unsigned int width, unsigned int height, unsigned int depth, int x0, int y0, int z0) :
    _x0(x0), _y0(y0), _z0(z0)
{
    _x_chunks = (width + Chunk::SIZE - 1) / Chunk::SIZE;
    _y_chunks = (height + Chunk::SIZE - 1) / Chunk::SIZE;
    _z_chunks = (depth + Chunk::SIZE - 1) / Chunk::SIZE;
    _chunks.resize(_x_chunks * _y_chunks * _z_chunks);
}

std::bitset<6> VoxelGrid::faces_visibility(int x, int y, int z)
{
    std::bitset<6> result;
    result[0] = _contains_voxel(x, y, z - 1);
    result[1] = _contains_voxel(x, y, z + 1);
    result[2] = _contains_voxel(x - 1, y, z);
    result[3] = _contains_voxel(x + 1, y, z);
    result[4] = _contains_voxel(x, y - 1, z);
    result[5] = _contains_voxel(x, y + 1, z);
    return result;
}

RaycastResult VoxelGrid::raycast(glm::vec3 o, glm::vec3 r)
{
    constexpr float MAX_D = 32.0;
    constexpr unsigned MAX_I = 100;
    constexpr float EPSILON = 0.01;
//    fmt::print("Raycast ({}, {}, {}) --> [{}. {}. {}]:\n", o.x, o.y, o.z, r.x, r.y, r.z);
    r = glm::normalize(r);

    RaycastResult result;
    result.voxel_x = static_cast<int>(o.x);
    result.voxel_y = static_cast<int>(o.y);
    result.voxel_z = static_cast<int>(o.z);
    result.started_in_bounds  = in_bounds(result.voxel_x, result.voxel_y, result.voxel_z);

    unsigned i = 0;
    float dist = 0.0;
    while (dist < MAX_D && i < MAX_I) {
        ++i;
        auto d = std::min(UntilGridHit(o.x, r.x), std::min(UntilGridHit(o.y, r.y), UntilGridHit(o.z, r.z))) + EPSILON;
        dist += d;
        o += d * r;

        auto x = static_cast<int>(o.x);
        auto y = static_cast<int>(o.y);
        auto z = static_cast<int>(o.z);
//        fmt::print("  - ({}, {}, {})\n", x, y, z);

        if (result.started_in_bounds && !in_bounds(x, y, z))
            return result;
        if (result.voxel_x < x)
            result.hit_face = VoxelFace::LEFT;
        else if (result.voxel_x > x)
            result.hit_face = VoxelFace::RIGHT;
        else if (result.voxel_y < y)
            result.hit_face = VoxelFace::BOTTOM;
        else if (result.voxel_y > y)
            result.hit_face = VoxelFace::TOP;
        else if (result.voxel_z < z)
            result.hit_face = VoxelFace::BACK;
        else if (result.voxel_z > z)
            result.hit_face = VoxelFace::FRONT;
        result.voxel_x = x;
        result.voxel_y = y;
        result.voxel_z = z;
        if (_contains_voxel(x, y, z)) {
            result.hit = true;
            return result;
        }
    }

    return result;
}
