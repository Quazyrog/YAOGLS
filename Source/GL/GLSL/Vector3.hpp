#pragma once
#include <cmath>
#include <algorithm>

struct Vector3
{
private:
    static float WGuard_;
public:
    float x, y, z;

    Vector3() = default;
    constexpr Vector3(float x, float y, float z): x(x), y(y), z(z) {};

    bool operator==(const Vector3 &v) const = default;
    bool operator!=(const Vector3 &v) const = default;
    auto operator[](size_t idx) const
    {
        switch (idx) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return 1.0f;
        }
    }
    auto &operator[](size_t idx)
    {
        switch (idx) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return WGuard_;
        }
    }
    auto &operator+=(const Vector3 &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    auto &operator-=(const Vector3 &v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    auto &operator*=(float k)
    {
        x *= k;
        y *= k;
        z *= k;
        return *this;
    }
    auto &operator/=(float k)
    {
        x /= k;
        y /= k;
        z /= k;
        return *this;
    }

    auto norm1() const { return std::abs(x) + std::abs(y) + std::abs(z); }
    auto norm2() const { return x * x + y * y + z * z; }
    auto length() const { return std::sqrt(x * x + y * y + z * z); }
    auto normal() { auto a = *this; a /= length(); return a; }
    void normalize() { *this /= this->length(); }

    auto cross(const Vector3 &v) const
    {
        return Vector3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
            );
    }
    auto dot(const Vector3 &v) const { return x * v.x + y * v.y + z * v.z; }
};

inline auto operator+(const Vector3 &a, const Vector3 &b)
{
    Vector3 r = a;
    r += b;
    return r;
}

inline auto operator-(const Vector3 &a, const Vector3 &b)
{
    Vector3 r = a;
    r -= b;
    return r;
}

inline auto operator*(float k, const Vector3 &a)
{
    Vector3 r = a;
    r *= k;
    return r;
}

inline auto operator*(const Vector3 &a, float k)
{
    Vector3 r = a;
    r *= k;
    return r;
}

inline auto operator/(float k, const Vector3 &a)
{
    Vector3 r = a;
    r /= k;
    return r;
}

inline auto operator/(const Vector3 &a, float k)
{
    Vector3 r = a;
    r /= k;
    return r;
}