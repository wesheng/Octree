#include "Octree.h"

void Octree::add(Vec3 point)
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

bool Octree::has(Vec3 point)
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

std::vector<Vec3> Octree::nearest(Vec3 point, int k_count)
{
    std::vector<std::tuple<Vec3, float>> candidates;
    nearest(point, k_count, candidates);

    std::sort(candidates.begin(), candidates.end(), [](std::tuple<Vec3, float> a, std::tuple<Vec3, float> b) {
        return std::get<1>(a) < std::get<1>(b);
        });

    // return slice
    std::vector<Vec3> output;
    for (int i = 0; i < std::min(k_count, static_cast<int>(candidates.size())); i++)
    {
        output.push_back(std::get<0>(candidates[i]));
    }

    return output;
}

void Octree::nearest(Vec3 point, int k_count, std::vector<std::tuple<Vec3, float>>& candidates)
{
    if (children_ != nullptr)
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
        // brute force nearest neighbors
        std::vector<std::tuple<Vec3, float>> sqr_distances;
        for (Vec3 p : points_)
        {
            candidates.push_back({ p, Vec3::sqr_distance(point, p) });
        }
    }
}

void Octree::try_insert_point(Vec3 point)
{
    points_.push_back(point);
    if (points_.size() > min_capacity_ && max_depth_ > 0)
    {
        subdivide();
    }
}

unsigned Octree::get_child_index(Vec3 point)
{
    bool greater_x = point.x > bounds_.center.x;
    bool greater_y = point.y > bounds_.center.y;
    bool greater_z = point.z > bounds_.center.z;

    return (greater_x << 0) + (greater_y << 1) + (greater_z << 2);
}

void Octree::insert_children(Vec3 point)
{
    unsigned index = get_child_index(point);
    (*children_)[index].add(point);
}

void Octree::subdivide()
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
        }


        for (Vec3 point : points_)
        {
            insert_children(point);
        }
        points_.clear();
    }
}