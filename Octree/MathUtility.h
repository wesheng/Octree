#pragma once

#include "Vec3.h"

class MathUtility
{
public:
    template<typename T>
    inline static T clamp(T val, T min, T max)
    {
        return (val < min) ? min : ((val > max) ? max : val);
    }

    static bool aabb_sphere_intersects(Vec3 min, Vec3 max, Vec3 center, float radius);
    static bool aabb_ray_intersects(Vec3 min, Vec3 max, Vec3 origin, Vec3 direction);
    static bool ray_point_intersects(Vec3 ray_origin, Vec3 ray_direction, Vec3 point, float tolerance = 0.001f);
};