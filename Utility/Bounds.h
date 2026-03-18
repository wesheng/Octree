#pragma once
#include "Vec3.h"
#include "Mathf.h"

struct Bounds
{
    Vec3 center{ 0.0f, 0.0f, 0.0f };
    Vec3 size{ 0.0f, 0.0f, 0.0f };

    inline Vec3 get_half_size() {
        return size / 2.0f;
    }

    inline Vec3 get_min()
    {
        return center - get_half_size();
    }

    inline Vec3 get_max()
    {
        return center + get_half_size();
    }

    inline void set_extents(Vec3 min, Vec3 max)
    {
        size = max - min;
        center = min + get_half_size();
    }

    inline bool contains(Vec3 point)
    {
        Vec3 min = get_min();
        Vec3 max = get_max();
        return (point.x >= min.x && point.x <= max.x) &&
            (point.y >= min.y && point.y <= max.y) &&
            (point.z >= min.z && point.z <= max.z);
    }

    inline bool intersects_sphere(Vec3 center, float radius)
    {
        // sphere - aabb test
        Vec3 min = get_min();
        Vec3 max = get_max();
        Vec3 clamped = {
            Mathf::clamp(center.x, min.x, max.x),
            Mathf::clamp(center.y, min.y, max.y),
            Mathf::clamp(center.z, min.z, max.z)
        };
        float sqr_distance = Vec3::sqr_distance(clamped, center);
        return sqr_distance <= radius * radius;
    }
};
