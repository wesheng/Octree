#pragma once

#include <array>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <Vec3.h>
#include <Bounds.h>
#include <algorithm>
#include <tuple>

/**
 * @brief Recursive data structure for storing points within octants in a 3D space.
 * @tparam MinCapacity 
 */
template<size_t MinCapacity>
class Octree {
private:
    using Octants = std::array<Octree<MinCapacity>, 8>;
public:
    Octree() = default;

    Octree(Bounds bounds, unsigned max_depth) : bounds_(bounds), max_depth_(max_depth)
    {}

    inline void add(Vec3 point)
    {
        if (!bounds_.contains(point))
        {
            throw std::exception("Point not in bounds.");
        }

        if (children_ != nullptr)
        {
            // find child to insert
            insert_children(point);
        }
        else
        {
            // insert to points, or subdivide.
            try_insert_point(point);
        }
        num_points_++;
    }

    inline bool has(Vec3 point)
    {
        if (bounds_.contains(point))
        {
            if (children_ != nullptr)
            {
                unsigned index = get_child_index(point);
                return (*children_)[index].has(point);
            }
            else
            {
                for (auto p : points_)
                {
                    if (p == point) return true;
                }
            }
        }

        return false;
    }

    inline std::vector<Vec3> nearest(Vec3 point, int k_count)
    {
        std::vector<std::tuple<Vec3, float>> candidates;
        nearest(point, k_count, candidates);

        std::sort(candidates.begin(), candidates.end(), [](std::tuple<Vec3, float> a, std::tuple<Vec3, float> b) {
            return std::get<1>(a) < std::get<1>(b);
            });

        // return slice
        std::vector<Vec3> output;
        for (int i = 0; i < std::min(k_count, static_cast<int>(num_points_)); i++)
        {
            output.push_back(std::get<0>(candidates[i]));
        }

        return output;
    }

    operator std::string()
    {
        return to_string(0);
    }

private:
    inline void nearest(Vec3 point, int k_count, std::vector<std::tuple<Vec3, float>>& candidates)
    {
        if (children_ != nullptr)
        {
            // find the octant point is residing in and test those
            unsigned index = get_child_index(point);
            (*children_)[index].nearest(point, k_count, candidates);

            if (candidates.size() > 0)
            {
                float furthest_sqr_dst = std::get<1>(candidates.back());

                Octants& children = *children_;
                // if point is near another quadrant check those too
                for (int i = 0; i < children.size(); i++)
                {
                    if (i == index) continue;

                    if (children[i].bounds_.intersects_sphere(point, furthest_sqr_dst))
                    {
                        children[i].nearest(point, k_count, candidates);
                    }
                }
            }
        }
        else
        {
            // brute force nearest neighbors
            std::vector<std::tuple<Vec3, float>> sqr_distances;
            for (Vec3 p : points_)
            {
                candidates.push_back({ p, Vec3::sqr_distance(point, p) });
            }
        }

    }

    inline void try_insert_point(Vec3 point)
    {
        points_.push_back(point);
        if (points_.size() > MinCapacity && max_depth_ > 0)
        {
            subdivide();
        }
    }

    inline unsigned get_child_index(Vec3 point)
    {
        bool greater_x = point.x > bounds_.center.x;
        bool greater_y = point.y > bounds_.center.y;
        bool greater_z = point.z > bounds_.center.z;

        return (greater_x << 0) + (greater_y << 1) + (greater_z << 2);
    }

    inline void insert_children(Vec3 point)
    {
        unsigned index = get_child_index(point);
        (*children_)[index].add(point);
    }

    inline void subdivide()
    {
        if (children_ == nullptr)
        {
            children_ = std::make_unique<std::array<Octree<MinCapacity>, 8>>();
            auto& children = *children_;
            
            Vec3 half_size = bounds_.get_half_size();
            Vec3 center = bounds_.center;
            Vec3 min = bounds_.get_min();
            Vec3 max = bounds_.get_max();

            unsigned new_depth = max_depth_ - 1;

            for (int i = 0; i < 8; i++)
            {
                // 7 == 0b111; each digit corresponds to an axis.
                Vec3 child_min{
                    ((i & (1 << 0)) == 0) ? min.x : center.x,
                    ((i & (1 << 1)) == 0) ? min.y : center.y,
                    ((i & (1 << 2)) == 0) ? min.z : center.z
                };
                Vec3 child_max{
                    ((i & (1 << 0)) == 0) ? center.x : max.x,
                    ((i & (1 << 1)) == 0) ? center.y : max.y,
                    ((i & (1 << 2)) == 0) ? center.z : max.z
                };

                Bounds b;
                b.set_extents(child_min, child_max);
                children[i] = Octree{ b, new_depth };
            }
            

            for (Vec3 point : points_)
            {
                insert_children(point);
            }
            points_.clear();
        }
    }

    inline void set_bounds(Bounds bounds)
    {
        bounds_ = bounds;
    }

    inline std::string to_string(int current_depth)
    {
        std::string indents(current_depth, '\t');
        std::ostringstream s;
        s << indents << "Node at " << static_cast<std::string>(bounds_.center) << " with size " << static_cast<std::string>(bounds_.size) << std::endl;
        s << indents << "- Number of Points: " << num_points_ << std::endl;
        if (children_.get() != nullptr)
        {
            for (auto& child : (*children_))
            {
                s << child.to_string(current_depth + 1);
            }
        }
        return s.str();
    }

private:
    Bounds bounds_;
    unsigned max_depth_ = 0;
    unsigned num_points_ = 0;

    std::unique_ptr<Octants> children_;
    std::vector<Vec3> points_;
};