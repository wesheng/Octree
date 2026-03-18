#pragma once
#include <sstream>

struct Vec3
{
    float x = 0.0f, y = 0.0f, z = 0.0f;

    const Vec3 operator+(const Vec3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }
    const Vec3 operator-() const {
        return { -x, -y, -z };
    }
    const Vec3 operator-(const Vec3& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }
    const Vec3 operator/(const Vec3& other) const {
        return { x / other.x, y / other.y, z / other.z };
    }
    const Vec3 operator/(const float& constant) const {
        return { x / constant, y / constant, z / constant };
    }

    operator std::string() const {
        std::ostringstream s;
        s << "(" << x << ", " << y << ", " << z << ")";
        return s.str();
    }
};