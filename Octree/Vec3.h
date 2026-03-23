#pragma once

#include <algorithm>
#include <cmath>

namespace Octrees
{
    /**
    * @brief Utility Vector3
    * @remarks Contains only functions required for Octree calculation.
    */
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

        [[nodiscard]]
        bool approx(const Vec3& other, float tolerance = 0.001f) const {
            Vec3 dif = *this - other;
            return abs(dif.x) < tolerance && abs(dif.y) < tolerance && abs(dif.z) < tolerance;
        }

        [[nodiscard]]
        static bool approx(const Vec3& a, const Vec3& b, float tolerance = 0.001f) {
            return a.approx(b, tolerance);
        }

        [[nodiscard]]
        inline float sqr_magnitude() const {
            return x * x + y * y + z * z;
        }

        [[nodiscard]]
        inline float magnitude() const {
            return std::sqrt(sqr_magnitude());
        }

        [[nodiscard]]
        inline float min_component() const {
            return std::min(x, std::min(y, z));
        }

        [[nodiscard]]
        inline float max_component() const {
            return std::max(x, std::max(y, z));
        }

        //

        [[nodiscard]]
        static float sqr_distance(const Vec3& a, const Vec3& b)
        {
            return (a - b).sqr_magnitude();
        }

        [[nodiscard]]
        static float distance(const Vec3& a, const Vec3& b)
        {
            return (a - b).magnitude();
        }

        [[nodiscard]]
        static float dot(const Vec3& a, const Vec3& b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        [[nodiscard]]
        static Vec3 min(const Vec3& a, const Vec3& b)
        {
            return {
                std::min(a.x, b.x),
                std::min(a.y, b.y),
                std::min(a.z, b.z)
            };
        }

        [[nodiscard]]
        static Vec3 max(const Vec3& a, const Vec3& b)
        {
            return {
                std::max(a.x, b.x),
                std::max(a.y, b.y),
                std::max(a.z, b.z)
            };
        }
    };
}