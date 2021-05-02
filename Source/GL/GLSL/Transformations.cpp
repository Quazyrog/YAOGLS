#define TRANSFORMATIONS_INSTANTIATE_TEMPLATES__
#include <cmath>
#include "Transformations.hpp"

namespace GL::GLSL {

Matrix4 Identity4(float scale)
{
    Matrix4 mat;
    for (auto i = 0; i < 4; ++i)
        mat(i, i) = scale;
    return mat;
}

Matrix4 Scaling3D(float sx, float sy, float sz)
{
    Matrix4 mat;
    mat(0, 0) = sx;
    mat(1, 1) = sy;
    mat(2, 2) = sz;
    mat(3, 3) = 1.0f;
    return mat;
}

Matrix4 Translation3D(float tx, float ty, float tz)
{
    Matrix4 mat;
    mat(0, 3) = tx;
    mat(1, 3) = ty;
    mat(2, 3) = tz;
    mat(3, 3) = 1.0f;
    return mat;
}

Matrix4 Rotation3D(Vector3 axis, float angle)
{
    axis.normalize();
    auto sin = std::sin(angle);
    auto cos = std::cos(angle);
    auto one_minus_cos = 1.0f - cos;
    auto x = axis.x;
    auto y = axis.y;
    auto z = axis.z;

    Matrix4 mat;
    mat(3, 3) = 1.0f;

    mat(0,0) = cos + x * x * one_minus_cos;
    mat(1,1) = cos + y * y * one_minus_cos;
    mat(2,2) = cos + z * z * one_minus_cos;

    mat(1, 0) = y * x * one_minus_cos + z * sin;
    mat(0, 1) = y * x * one_minus_cos - z * sin;

    mat(2, 0) = z * x * one_minus_cos - y * sin;
    mat(0, 2) = z * x * one_minus_cos + y * sin;

    mat(2, 1) = z * y * one_minus_cos + x * sin;
    mat(1, 2) = z * y * one_minus_cos - x * sin;

    return mat;
}

Matrix4 Perspective3D(float fovy, float aspect, float near, float far)
{
    auto height = std::sin(fovy);
    auto width = aspect * height;
    Matrix4 mat;
    mat(0, 0) = near / width;
    mat(1, 1) = near / height;
    mat(2, 2) = (near + far) / (near - far);
    mat(2, 3) = 2 * near * far / (near - far);
    mat(3, 2) = -1;
    return mat;
}

}
