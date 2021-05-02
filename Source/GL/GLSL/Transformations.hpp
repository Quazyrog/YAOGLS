#pragma once
#include "Matrix.hpp"
#include "Vector3.hpp"


namespace GL::GLSL {

#ifndef TRANSFORMATIONS_INSTANTIATE_TEMPLATES__
extern template class Matrix<4, 4, float>;
#else
template class Matrix<4, 4, float>;
#endif
using Matrix4 = Matrix<4, 4, float>;

Matrix4 Identity3D(float scale = 1.0);
Matrix4 Scaling3D(float sx, float sy, float sz);
Matrix4 Translation3D(float tx, float ty, float tz);
Matrix4 Rotation3D(Vector3 axis, float angle);
Matrix4 Perspective3D(float fovy, float aspect, float near, float far);

}