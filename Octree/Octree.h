#pragma once

#include <array>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <Vec3.h>
#include <Bounds.h>
#include <tuple>

/**
 * @brief Recursive data structure for storing points within octants in a 3D space.
 */
class Octree {
private:
    using Octants = std::array<Octree, 8>;
public:
    Octree() = default;

    Octree(Bounds bounds, unsigned min_capacity = 10, unsigned max_depth = 5) : bounds_(bounds), min_capacity_(min_capacity), max_depth_(max_depth)
    {}

    void add(Vec3 point);
    bool has(Vec3 point);
    std::vector<Vec3> nearest(Vec3 point, int k_count);

    operator std::string()
    {
        return to_string(0);
    }

private:
    void nearest(Vec3 point, int k_count, std::vector<std::tuple<Vec3, float>>& candidates);
    void try_insert_point(Vec3 point);
    unsigned get_child_index(Vec3 point);
    void insert_children(Vec3 point);
    void subdivide();

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
    unsigned min_capacity_ = 0;
    unsigned max_depth_ = 0;
    unsigned num_points_ = 0;

    std::unique_ptr<Octants> children_;
    std::vector<Vec3> points_;
};