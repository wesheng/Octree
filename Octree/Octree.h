#pragma once

#include <array>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>

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
        return (point.x > min.x && point.x < max.x) &&
            (point.y > min.y && point.y < max.y) &&
            (point.z > min.z && point.z < max.z);
    }
};

/**
 * @brief Recursive data structure for storing points within nodes in a 3D space.
 * @tparam MinCapacity 
 */
template<size_t MinCapacity>
class Octree {
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

    operator std::string()
    {
        return to_string(0);
    }

private:

    inline void try_insert_point(Vec3 point)
    {
        points_.push_back(point);
        if (points_.size() > MinCapacity && max_depth_ > 0)
        {
            subdivide();
        }
    }

    inline void insert_children(Vec3 point)
    {
        bool greater_x = point.x > bounds_.center.x;
        bool greater_y = point.y > bounds_.center.y;
        bool greater_z = point.z > bounds_.center.z;

        unsigned index = (greater_x << 0) + (greater_y << 1) + (greater_z << 2);
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

    std::unique_ptr<std::array<Octree<MinCapacity>, 8>> children_;
    std::vector<Vec3> points_;
};