#pragma once

#include <array>
#include <memory>
#include <vector>
#include <Bounds.h>
#include <tuple>
#include <ranges>
#include <stack>
#include <queue>
#include <iterator>
#include "MathUtility.h"
#include "Vec3.h"

namespace Octrees
{
    /**
    * @brief Recursive data structure for storing points within octants in a 3D space.
    * @tparam T Identifier type (index, pointer, entity id, etc.)
    */
    template<typename T>
    class Octree {
        using Octants = std::array<Octree<T>, 8>;
        using PointPair = std::pair<Vec3, T>;

        // Used for priority queue.
        struct ComparePointPairDistance 
        {
            bool operator()(const std::pair<PointPair, float> l, const std::pair<PointPair, float> r)
            {
                return l.second < r.second;
            }
        };
        using PointPairDistance = std::pair<PointPair, float>;
        using PriorityQueuePointPairDistance = std::priority_queue<PointPairDistance, std::vector<PointPairDistance>, ComparePointPairDistance>;
    public:
        // iterators - https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
        // https://stackoverflow.com/questions/72405122/creating-an-iterator-with-c20-concepts-for-custom-container
        // https://stackoverflow.com/questions/6366684/c-implementing-a-custom-iterator-for-binary-trees-long
        /**
         * @brief Octant iterator that searches depth first.
         */
        struct OctantIterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;

            OctantIterator(Octree<T>* octree)
            {
                find_children(octree);
            }
            OctantIterator() = default;

            const Octree<T>* operator*() const { return stack_.top(); }
            const Octree<T>& operator&() const { return *stack_.top(); }

            OctantIterator& operator++()
            {
                Octree<T>* node = stack_.top();
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
            void find_children(Octree<T>* octree) {
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
            std::stack<Octree<T>*> stack_;
        };

        /**
         * @brief Creates an empty Octree.
         */
        Octree() = default;

        /**
         * @brief Creates a Octree
         * @param bounds
         * @param min_capacity
         * @param max_depth
         */
        Octree(Bounds bounds, unsigned min_capacity = 10, unsigned max_depth = 5) : bounds_(bounds), min_capacity_(min_capacity), max_depth_(max_depth)
        {}

        /**
         * @brief Adds a point to the Octree. The point should be within bounds of the Octree.
         * @param point The position.
         * @param entity The identifier associated with the position.
         * @return True if the insertion was successful.
         */
        bool add(Vec3 point, T entity)
        {
            bool can_add = bounds_.contains(point);
            if (can_add)
            {
                if (children_ != nullptr)
                {
                    // find child to insert
                    can_add = insert_children(point, entity);
                }
                else
                {
                    // insert to points, or subdivide.
                    try_insert_point(point, entity);
                }
                num_points_++;
            }

            return can_add;
        }

        /**
         * @brief Checks whether a point exists in the Octree.
         * @param point The point
         * @return If the point exists.
         */
        [[nodiscard]]
        bool has(Vec3 point)
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
                    for (PointPair p : points_)
                    {
                        if (p.first == point) return true;
                    }
                }
            }
            return false;
        }

        /**
         * @brief Retrieves a sorted list of the nearest points of a specified location.
         * @param point The location to start searching from.
         * @param k_count The number of points to find.
         * @return A sorted vector (from closest to furthest) containing the nearby points.
         */
        [[nodiscard]]
        std::vector<PointPair> nearest(Vec3 point, int k_count)
        {
            // K-NN Implementation partially based on the work described by Jun Zhu et. al.:
            // Zhu, J., Li, H., Wang, Z., Wang, S., & Zhang, T. (2024). i-Octree: A Fast, Lightweight, and Dynamic Octree for Proximity Search (arXiv:2309.08315; Version 2). arXiv. https://doi.org/10.48550/arXiv.2309.08315

            PriorityQueuePointPairDistance candidates;
            nearest(point, k_count, candidates);

            std::vector<PointPair> output;
            while (!candidates.empty())
            {
                output.push_back(candidates.top().first);
                candidates.pop();
            }
            std::reverse(output.begin(), output.end());

            return output;
        }

        /**
         * @brief Raycasts the Octree for leaf nodes that intersect the ray.
         * @param ray_origin The origin of the ray
         * @param ray_direction The direction of the ray
         * @return A list of Octree nodes.
         */
        [[nodiscard]]
        std::vector<const Octree*> intersects_nodes(Vec3 ray_origin, Vec3 ray_direction)
        {
            // returns const Octree * to prevent manipulation of data
            std::vector<const Octree*> output;
            intersects_nodes(ray_origin, ray_direction, output);
            return output;
        }

        /**
         * @brief Raycasts the Octree for points that intersect the ray.
         * @param ray_origin The origin of the ray
         * @param ray_direction The direction of the ray
         * @param tolerance A tolerance to use for points that are near the ray.
         * @return A list of points.
         * @remarks Utilizes intersects_nodes() to find points.
         */
        [[nodiscard]]
        std::vector<PointPair> intersects_points(Vec3 ray_origin, Vec3 ray_direction, float tolerance = 0.001f)
        {
            std::vector<const Octree*> nodes = intersects_nodes(ray_origin, ray_direction);
            std::vector<PointPair> output;
            for (auto node : nodes)
            {
                for (PointPair point : node->points_)
                {
                    if (MathUtility::ray_point_intersects(ray_origin, ray_direction, point.first))
                    {
                        output.push_back(point);
                    }
                }
            }
            return output;
        }

        /**
         * @brief Retrieves the bounds of the Octree.
         * @return The bounds
         */
        [[nodiscard]]
        inline Bounds get_bounds() const { return bounds_; }

        /**
         * @brief Retrieves the points stored within this node specifically.
         * @return The list of points
         * @remarks Does not return points stored inside child octants.
         */
        [[nodiscard]]
        inline std::vector<PointPair> get_points() const { return points_; }

        /**
         * @brief Gets the current recursive depth level of this node.
         * @return The depth level
         */
        [[nodiscard]]
        inline int get_depth_level() const { return depth_level_; }

        /**
         * @brief Determines whether this node is a leaf (no children) or not (has children).
         * @return Whether the node is a leaf or not.
         */
        [[nodiscard]]
        inline bool is_leaf() const
        {
            return children_ == nullptr;
        }

        auto begin() { return OctantIterator{ this }; }
        auto end() { return OctantIterator{}; }

    private:
        void nearest(Vec3 point, int k_count, std::vector<std::pair<PointPair, float>>& candidates)
        {
            if (!is_leaf())
            {
                Octants& children = *children_;

                // find the octant point is residing in and test those
                unsigned index = get_child_index(point);
                children[index].nearest(point, k_count, candidates);

                // if no candidates then use max distance.
                float furthest_sqr_dst = std::numeric_limits<float>::max();
                if (candidates.size() > 0)
                {
                    furthest_sqr_dst = std::get<1>(candidates.back());
                }

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
            else
            {
                for (PointPair p : points_)
                {
                    candidates.push_back({ p, Vec3::sqr_distance(point, p.first) });
                }
            }
        }

        void nearest(Vec3 point, int k_count, PriorityQueuePointPairDistance& candidates)
        {
            // if we are still discovering candidates then use max distance.
            // otherwise if candidates is full use the top.
            float furthest_sqr_dst = std::numeric_limits<float>::max();
            if (candidates.size() >= k_count)
            {
                furthest_sqr_dst = candidates.top().second;
            }

            if (!is_leaf())
            {
                Octants& children = *children_;

                // find the octant point is residing in and test those
                unsigned index = get_child_index(point);
                children[index].nearest(point, k_count, candidates);

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
            else
            {
                for (PointPair p : points_)
                {
                    float distance = Vec3::sqr_distance(point, p.first);
                    if (distance < furthest_sqr_dst)
                    {
                        candidates.push({ p, distance });
                        if (candidates.size() > k_count)
                        {
                            candidates.pop();
                        }
                    }

                }
            }
        }

        void intersects_nodes(Vec3 ray_origin, Vec3 ray_direction, std::vector<const Octree*>& candidates)
        {
            if (is_leaf())
            {
                if (bounds_.intersects_ray(ray_origin, ray_direction))
                {
                    candidates.push_back(this);
                }
            }
            else
            {
                for (auto& child : (*children_))
                {
                    if (child.bounds_.intersects_ray(ray_origin, ray_direction))
                    {
                        // a check is done here to avoid having to test intersection on leaf nodes twice.
                        if (child.is_leaf())
                        {
                            candidates.push_back(&child);
                        }
                        else
                        {
                            child.intersects_nodes(ray_origin, ray_direction, candidates);
                        }
                    }
                }
            }
        }

        void try_insert_point(Vec3 point, T entity)
        {
            points_.push_back({ point, entity });
            if (points_.size() > min_capacity_ && max_depth_ > 0)
            {
                subdivide();
            }
        }

        unsigned get_child_index(Vec3 point)
        {
            bool greater_x = point.x > bounds_.center.x;
            bool greater_y = point.y > bounds_.center.y;
            bool greater_z = point.z > bounds_.center.z;

            return (greater_x << 0) + (greater_y << 1) + (greater_z << 2);
        }

        bool insert_children(Vec3 point, T entity)
        {
            unsigned index = get_child_index(point);
            return (*children_)[index].add(point, entity);
        }

        void subdivide()
        {
            if (children_ == nullptr)
            {
                children_ = std::make_unique<Octants>();
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
                    children[i] = Octree{ b, min_capacity_, new_depth };
                    children[i].depth_level_ = depth_level_ + 1;
                }

                for (PointPair point : points_)
                {
                    insert_children(point.first, point.second);
                }
                points_.clear();
            }
        }

        inline void set_bounds(Bounds bounds)
        {
            bounds_ = bounds;
        }

    private:
        Bounds bounds_;
        unsigned min_capacity_ = 0;
        unsigned max_depth_ = 0;
        unsigned num_points_ = 0;
        int depth_level_ = 0;

        std::unique_ptr<Octants> children_;
        std::vector<PointPair> points_;
    };
}