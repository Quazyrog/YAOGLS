#pragma once
#include <array>
#include <cstring>
#include <iostream>
#include "Transformations.hpp"

namespace GL {
class UniformValue;
}

namespace GL::GLSL {

template<size_t M, size_t N, typename Float = float>
class Matrix
{
    friend class GL::UniformValue;
protected:
    std::array<Float, M * N> data_;

    static constexpr auto index_(size_t row, size_t column) noexcept
    { return M * column + row; }

public:
    Matrix()
    {
        for (auto &e: data_)
            e = 0;
    }

    auto nrows() const
    { return M; }
    auto ncols() const
    { return N; }
    auto operator()(size_t row, size_t column) const noexcept
    { return data_[index_(row, column)]; }
    auto &operator()(size_t row, size_t column) noexcept
    { return data_[index_(row, column)]; }
};

template<size_t K, size_t L, size_t M, typename Float = float>
auto operator*(const Matrix<K, L, Float> &a, const Matrix<L, M, Float> &b)
{
    Matrix<K, M, Float> r;
    for (auto k = 0; k < K; ++k) {
        for (auto m = 0; m < M; ++m) {
            Float e = 0;
            for (auto l = 0; l < L; ++l)
                e += a(k, l) * b(l, m);
            r(k, m) = e;
        }
    }
    return r;
}

template<size_t K, size_t M, typename Vector, typename Float = float>
auto operator*(const Matrix<K, M, Float> &a, const Vector &v)
{
    Vector r;
    for (auto k = 0; k < K; ++k) {
        Float e = 0;
        for (auto m = 0; m < M; ++m)
            e += v[m] * a(k, m);
        r[k] = e;
    }
    return r;
}

}