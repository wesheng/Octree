#include "MathUtility.h"

bool Octrees::MathUtility::aabb_sphere_intersects(Vec3 min, Vec3 max, Vec3 center, float radius)
{
    Vec3 clamped = {
        MathUtility::clamp(center.x, min.x, max.x),
        MathUtility::clamp(center.y, min.y, max.y),
        MathUtility::clamp(center.z, min.z, max.z)
    };
    float sqr_distance = Vec3::sqr_distance(clamped, center);
    return sqr_distance <= radius * radius;
}

bool Octrees::MathUtility::aabb_ray_intersects(Vec3 min, Vec3 max, Vec3 origin, Vec3 direction)
{
    // ray - aabb test
    // use slab method
    // https://en.wikipedia.org/wiki/Slab_method

    Vec3 t0 = (min - origin) / direction;
    Vec3 t1 = (max - origin) / direction;
    float t_near = Vec3::min(t0, t1).max_component();
    float t_far = Vec3::max(t0, t1).min_component();
    return t_near <= t_far && t_far >= 0; // t_far >= 0 to ensure ray isn't facing away from aabb.
}

bool Octrees::MathUtility::ray_point_intersects(Vec3 ray_origin, Vec3 ray_direction, Vec3 point, float tolerance)
{
    // ray-sphere intersection. we're treating the points as spheres as provided by tolerance.
    // https://iquilezles.org/articles/intersectors/
    // that code is for SDFs, but we're just strictly checking if intersect or not.
    Vec3 dif = ray_origin - point;
    float b = Vec3::dot(dif, ray_direction);
    float c = Vec3::dot(dif, dif) - (tolerance * tolerance);
    float h = b * b - c;
    return h >= 0.0f;
}