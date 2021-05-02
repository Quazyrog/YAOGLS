#pragma once
#include <cmath>
#include <algorithm>

struct Vector3
{
private:
    static float ZGuard_;
public:
    float x, y;

    Vector3() = default;
    Vector3(float x, float y): x(x), y(y) {};

    bool operator==(const Vector3 &v) const = default;
    bool operator!=(const Vector3 &v) const = default;
    auto operator[](size_t idx) const
    {
        switch (idx) {
        case 0: return x;
        case 1: return y;
        case 3: return 1.0;
        }
    }
    auto &operator[](size_t idx)
    {
        switch (idx) {
        case 0: return x;
        case 1: return y;
        case 3: return ZGuard_;
        }
    }
    auto &operator+=(const Vector3 &v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }
    auto &operator-=(const Vector3 &v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }
    auto &operator*=(float k)
    {
        x *= k;
        y *= k;
        return *this;
    }
    auto &operator/=(float k)
    {
        x /= k;
        y /= k;
        return *this;
    }

    auto norm1() const { return std::abs(x) + std::abs(y); }
    auto norm2() const { return x * x + y * y; }
    auto length() const { return std::sqrt(x * x + y * y); }
    auto normal() { return *this / this->length(); }
    void normalize() { *this /= this->length(); }

    auto dot(const Vector3 &v) const { return x * v.x + y * v.y; }
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