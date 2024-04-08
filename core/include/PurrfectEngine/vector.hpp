#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <cmath>

template <typename T>
class vec2 {
public:
    T x, y;

    vec2() : x(0), y(0) {}
    vec2(T x, T y) : x(x), y(y) {}

    vec2 operator+(const vec2& other) const {
        return vec2(x + other.x, y + other.y);
    }
    vec2 operator-(const vec2& other) const {
        return vec2(x - other.x, y - other.y);
    }
    vec2 operator*(T scalar) const {
        return vec2(x * scalar, y * scalar);
    }
    vec2 operator/(T scalar) const {
        return vec2(x / scalar, y / scalar);
    }
    vec2 operator+=(const vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    vec2 operator-=(const vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    T magnitude() const {
        return std::sqrt(x * x + y * y);
    }
    vec2 normalize() const {
        T mag = magnitude();
        return *this / mag;
    }


    T dot(const vec2& other) const {
        return x * other.x + y * other.y;
    }

    vec2 cross(const vec2& other) const {
        return x * other.y - y * other.x;
    }

    T angle() const {
        return std::atan2(y, x);
    }
};

typedef vec2<float> vec2f;
typedef vec2<double> vec2d;

template <typename T>
class vec3 {
public:
    T x, y, z;

    vec3() : x(0), y(0), z(0) {}
    vec3(T x, T y, T z) : x(x), y(y), z(z) {}

    vec3 operator+(const vec3& other) const {
        return vec3(x + other.x, y + other.y, z + other.z);
    }

    vec3 operator-(const vec3& other) const {
        return vec3(x - other.x, y - other.y, z - other.z);
    }

    vec3 operator*(T scalar) const {
        return vec3(x * scalar, y * scalar, z * scalar);
    }

    vec3 operator/(T scalar) const {
        return vec3(x / scalar, y / scalar, z / scalar);
    }

    vec3 operator+=(const vec3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    vec3 operator-=(const vec3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    T magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    vec3 normalize() const {
        T mag = magnitude();
        return *this / mag;
    }

    T dot(const vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    vec3 cross(const vec3& other) const {
        return vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }
};

typedef vec3<float> vec3f;
typedef vec3<double> vec3d;


template <typename T>
class vec4 {
public:
    T x, y, z, w;

    vec4() : x(0), y(0), z(0), w(0) {}
    vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

    vec4 operator+(const vec4& other) const {
        return vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    vec4 operator-(const vec4& other) const {
        return vec4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    vec4 operator*(T scalar) const {
        return vec4(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    vec4 operator/(T scalar) const {
        return vec4(x / scalar, y / scalar, z / scalar, w / scalar);
    }

    vec4 operator+=(const vec4& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    vec4 operator-=(const vec4& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    T magnitude() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    vec4 normalize() const {
        T mag = magnitude();
        return *this / mag;
    }

    T dot(const vec4& other) const {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }
};

typedef vec4<float> vec4f;
typedef vec4<double> vec4d;
#endif // VECTOR_HPP
