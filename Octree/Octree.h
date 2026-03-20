#pragma once

#include <array>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <Vec3.h>
#include <Bounds.h>
#include <tuple>
#include <ranges>
#include <stack>
#include <iterator>

/**
 * @brief Recursive data structure for storing points within octants in a 3D space.
 */
class Octree {
    using Octants = std::array<Octree, 8>;
public:
    // iterators - https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
    // https://stackoverflow.com/questions/72405122/creating-an-iterator-with-c20-concepts-for-custom-container
    // https://stackoverflow.com/questions/6366684/c-implementing-a-custom-iterator-for-binary-trees-long
    /**
     * @brief Octant iterator that searches depth first
     */
    struct OctantIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Octree;
        using pointer = const Octree*;
        using reference = const Octree&;

        OctantIterator(Octree* octree)
        { 
            find_children(octree);
        }
        OctantIterator() = default;

        const Octree* operator*() const { return stack_.top(); }
        const Octree& operator&() const { return *stack_.top(); }

        OctantIterator& operator++()
        {
            Octree* node = stack_.top();
            stack_.pop();
            return *this;
        }
        OctantIterator operator++(int)
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const OctantIterator& other) {
            bool this_empty = stack_.empty();
            bool other_empty = other.stack_.empty();
            // if both is empty then true
            // if one is empty then false (it's not possible to query the top()
            // compare that both tops are the same
            return
                (this_empty && other_empty) ||
                ((!this_empty && !other_empty) &&
                    (stack_.top() == other.stack_.top())); 
        }
        bool operator!=(const OctantIterator& other) {
            return !(*this == other);
        }


    private:
        void find_children(Octree* octree) {
            Octants& children = *(octree->children_);
            stack_.push(octree);
            if (octree->children_)
            {
                for (auto& child : children)
                {
                    find_children(&child);
                }
            }
        }
    private:
        std::stack<Octree*> stack_;

    };

    Octree() = default;

    Octree(Bounds bounds, unsigned min_capacity = 10, unsigned max_depth = 5) : bounds_(bounds), min_capacity_(min_capacity), max_depth_(max_depth)
    {}

    void add(Vec3 point);
    bool has(Vec3 point);
    std::vector<Vec3> nearest(Vec3 point, int k_count);
    /**
     * @brief Raycasts the Octree for leaf nodes that intersect the ray. 
     * @param ray_origin The origin of the ray
     * @param ray_direction The direction of the ray
     * @return A list of Octree nodes.
     */
    std::vector<const Octree*> intersects_nodes(Vec3 ray_origin, Vec3 ray_direction);
    std::vector<Vec3> intersects_points(Vec3 ray_origin, Vec3 ray_direction, float tolerance = 0.001f);
    
    inline Bounds get_bounds() const { return bounds_; }
    inline std::vector<Vec3> get_points() const { return points_; }
    inline int get_depth_level() const { return depth_level_; }


    operator std::string()
    {
        return to_string(0);
    }

    auto begin() { return OctantIterator{ this }; }
    auto end() { return OctantIterator{}; }

private:
    void nearest(Vec3 point, int k_count, std::vector<std::tuple<Vec3, float>>& candidates);
    void intersects_nodes(Vec3 ray_origin, Vec3 ray_direction, std::vector<const Octree*>& candidates);
    void try_insert_point(Vec3 point);
    unsigned get_child_index(Vec3 point);
    void insert_children(Vec3 point);
    void subdivide();

    inline bool is_leaf() {
        return children_ == nullptr;
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
    unsigned min_capacity_ = 0;
    unsigned max_depth_ = 0;
    unsigned num_points_ = 0;
    int depth_level_ = 0;

    std::unique_ptr<Octants> children_;
    std::vector<Vec3> points_;
};