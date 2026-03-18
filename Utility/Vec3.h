#pragma once
#include <sstream>

struct Vec3
{
    float x = 0.0f, y = 0.0f, z = 0.0f;

    Vec3 operator+(const Vec3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }
    Vec3 operator-() const {
        return { -x, -y, -z };
    }
    Vec3 operator-(const Vec3& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }
    Vec3 operator/(const Vec3& other) const {
        return { x / other.x, y / other.y, z / other.z };
    }
    Vec3 operator/(const float& scalar) const {
        return { x / scalar, y / scalar, z / scalar };
    }

    bool operator==(const Vec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool approx(const Vec3& other, float tolerance = 0.001f) const {
        Vec3 dif = *this - other;
        return abs(dif.x) < tolerance && abs(dif.y) < tolerance && abs(dif.z) < tolerance;
    }

    inline float sqr_magnitude() const {
        return x * x + y * y + z * z;
    }
    inline float magnitude() const {
        return sqrt(sqr_magnitude());
    }

    static float sqr_distance(const Vec3& a, const Vec3& b)
    {
        return (a - b).sqr_magnitude();
    }

    static float distance(const Vec3& a, const Vec3& b)
    {
        return (a - b).magnitude();
    }


    operator std::string() const {
        std::ostringstream s;
        s << "(" << x << ", " << y << ", " << z << ")";
        return s.str();
    }
};