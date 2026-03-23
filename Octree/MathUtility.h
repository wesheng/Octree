#pragma once

#include "Vec3.h"

namespace Octrees
{
    /**
    * @brief Math utility, including several intersection tests.
    */
    class MathUtility
    {
    public:
        /**
         * @brief Clamps a value between a minimum and maximum value.
         * @tparam T Comparable type
         * @param val Value to compare
         * @param min Minimum value
         * @param max Maximum value
         * @return The clamped value
         */
        template<typename T>
        [[nodiscard]]
        inline static T clamp(T val, T min, T max)
        {
            return (val < min) ? min : ((val > max) ? max : val);
        }

        /**
         * @brief Tests whether a sphere intersects with an axis-aligned bounding box.
         * @param min The minimum bounds of the box
         * @param max The maximum bounds of the box
         * @param center The center/origin of the sphere
         * @param radius The size of the sphere
         * @return True if the sphere intersects with the AABB.
         */
        [[nodiscard]]
        static bool aabb_sphere_intersects(Vec3 min, Vec3 max, Vec3 sphere_center, float radius);

        /**
         * @brief Tests whether a ray intersects with an axis-aligned bounding box.
         * @param min The minimum bounds of the box
         * @param max The maximum bounds of the box
         * @param origin The origin of the ray
         * @param direction The direction of the ray
         * @return True if the ray intersects the AABB.
         */
        [[nodiscard]]
        static bool aabb_ray_intersects(Vec3 min, Vec3 max, Vec3 ray_origin, Vec3 ray_direction);

        /**
         * @brief Tests whether a ray intersects with a point, given tolerance.
         * @param ray_origin The origin of the ray
         * @param ray_direction The direction of the ray
         * @param point The location of the point
         * @param tolerance The leeway amount to determine how close the point is to the ray
         * @return True if the ray intersects the point.
         * @remark This is effectively a ray-sphere intersection test.
         */
        [[nodiscard]]
        static bool ray_point_intersects(Vec3 ray_origin, Vec3 ray_direction, Vec3 point, float tolerance = 0.001f);
    };
}