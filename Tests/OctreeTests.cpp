#include <gtest/gtest.h>
#include <gtest/gtest-param-test.h>
#include <Octree.h>
#include <random>
#include <algorithm>
#include <tuple>

class OctreePointsFixture : public testing::TestWithParam<int> {
};

TEST(OctreeTest, AddPointInBounds)
{
    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<int> octree{ bounds, 5, 5 };
    EXPECT_TRUE(octree.add({ 0.0f, 0.0f, 0.0f }, 0));
}

TEST(OctreeTest, AddPointOnBounds)
{
    // Octree does allows points on bound edges

    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<int> octree{ bounds, 5, 5 };
    EXPECT_TRUE(octree.add({ 50.0f, 50.0f, 50.0f }, 0));
}

TEST(OctreeTest, TryPlaceOutsideBounds)
{
    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<int> octree{ bounds, 5, 5 };
    EXPECT_FALSE(octree.add({ 500.0f, 500.0f, 500.0f }, 0));
}

TEST_P(OctreePointsFixture, AddMultiplePoints)
{
    float size = 100.0f;

    std::random_device rand_device;
    std::default_random_engine rand_engine{ rand_device() };
    std::uniform_real_distribution<float> rand_dist{ -size / 2.0f, size / 2.0f };

    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {size, size, size}  };
    Octree<int> octree{ bounds, 10, 5 };
    for (int i = 0; i < GetParam(); i++)
    {
        Vec3 point{ rand_dist(rand_engine), rand_dist(rand_engine) , rand_dist(rand_engine) };
        EXPECT_TRUE(octree.add(point, i));
    }
}

TEST_P(OctreePointsFixture, HasPoint)
{
    float size = 100.0f;
    int count = GetParam();

    std::random_device rand_device;
    std::default_random_engine rand_engine{ rand_device() };
    std::uniform_real_distribution<float> rand_dist{ -size / 2.0f, size / 2.0f };
    std::uniform_int_distribution<> rand_index{ 0, count - 1 };

    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {size, size, size} };
    Octree<int> octree{ bounds, 10, 5 };
    int index = rand_index(rand_engine);
    Vec3 val;
    for (int i = 0; i < GetParam(); i++)
    {
        Vec3 point{ rand_dist(rand_engine), rand_dist(rand_engine) , rand_dist(rand_engine) };

        if (i == index)
            val = point;

        octree.add(point, i);
    }

    EXPECT_TRUE(octree.has(val));
}

TEST_P(OctreePointsFixture, NearestPoint)
{
    float size = 100.0f;
    int count = GetParam();

    int k_nearest = 5;

    std::random_device rand_device;
    std::default_random_engine rand_engine{ rand_device() };
    std::uniform_real_distribution<float> rand_dist{ -size / 2.0f, size / 2.0f };
    std::uniform_int_distribution<> rand_index{ 0, count - 1 };

    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {size, size, size} };
    Octree<int> octree{ bounds, 10, 5 };

    Vec3 rand_point{ rand_dist(rand_engine), rand_dist(rand_engine), rand_dist(rand_engine) };
    std::vector<std::pair<Vec3, float>> points;

    for (int i = 0; i < count; i++)
    {
        Vec3 point{ rand_dist(rand_engine), rand_dist(rand_engine) , rand_dist(rand_engine) };

        points.push_back({ point, Vec3::sqr_distance(rand_point, point) });
        octree.add(point, i);
    }

    auto nearest = octree.nearest(rand_point, k_nearest);

    EXPECT_GT(nearest.size(), 0);

    std::sort(points.begin(), points.end(), [](std::pair<Vec3, float> a, std::pair<Vec3, float> b) {
        return a.second < b.second;
        });


    for (int i = 0; i < k_nearest && i < points.size(); i++)
    {
        EXPECT_EQ(points[i].first, nearest[i].first);
    }
}

INSTANTIATE_TEST_SUITE_P(ManyPoints, OctreePointsFixture, testing::Values(1, 5, 10, 100, 1000, 10000, 100000));

TEST(OctreeTest, NearestInAnotherOctant)
{
    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<int> octree{ bounds, 5, 5 };

    Vec3 test_point = { -10.0f, -10.0f, -10.0f };
    Vec3 point = { 10.0f, 10.0f, 10.0f };
    for (int i = 0; i < 10; i++)
    {
        octree.add(point, i);
    }

    std::vector<std::pair<Vec3, int>> nearest;
    EXPECT_NO_THROW(nearest = octree.nearest(test_point, 1));
    EXPECT_GE(nearest.size(), 1);
    EXPECT_TRUE(Vec3::approx(nearest.front().first, point));
}

TEST(OctreeTest, RayIntersectsRootNodes)
{
    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<int> octree{ bounds, 5, 5 };

    Vec3 point = { 10.0f, 10.0f, 10.0f };
    octree.add(point, 0);

    Vec3 ray_position{ 0.0f, 0.0f, 100.0f };
    Vec3 ray_direction{ 0.0f, 0.0f, -1.0f };

    std::vector<const Octree<int>*> intersections;
    EXPECT_NO_THROW(intersections = octree.intersects_nodes(ray_position, ray_direction));
    EXPECT_GE(intersections.size(), 1);
}

TEST(OctreeTest, RayNotIntersectsRootNodes)
{
    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<int> octree{ bounds, 5, 5 };

    Vec3 point = { 10.0f, 10.0f, 10.0f };
    octree.add(point, 0);

    // offsetted to the side so that it's parellel to aabb.
    Vec3 ray_position{ 100.0f, 0.0f, 100.0f };
    Vec3 ray_direction{ 0.0f, 0.0f, -1.0f };

    std::vector<const Octree<int>*> intersections;
    EXPECT_NO_THROW(intersections = octree.intersects_nodes(ray_position, ray_direction));
    EXPECT_LE(intersections.size(), 0);
}

TEST(OctreeTest, RayFacingAwayNodes)
{
    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<int> octree{ bounds, 5, 5 };

    Vec3 point = { 10.0f, 10.0f, 10.0f };
    octree.add(point, 0);

    Vec3 ray_position{ 0.0f, 0.0f, 100.0f };
    Vec3 ray_direction{ 0.0f, 0.0f, 1.0f };

    std::vector<const Octree<int>*> intersections;
    EXPECT_NO_THROW(intersections = octree.intersects_nodes(ray_position, ray_direction));
    EXPECT_LE(intersections.size(), 0);
}

TEST(OctreeTest, RayIntersectsPoint)
{
    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<int> octree{ bounds, 5, 5 };

    Vec3 point = { 10.0f, 10.0f, 10.0f };
    octree.add(point, 0);

    Vec3 ray_position{ 10.0f, 10.0f, 100.0f };
    Vec3 ray_direction{ 0.0f, 0.0f, -1.0f };

    std::vector<std::pair<Vec3, int>> intersections;
    EXPECT_NO_THROW(intersections = octree.intersects_points(ray_position, ray_direction));
    EXPECT_GT(intersections.size(), 0);
    EXPECT_TRUE(Vec3::approx(intersections.front().first, point));
}

TEST(OctreeTest, RayDoesNotIntersectPoint)
{
    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<int> octree{ bounds, 5, 5 };

    Vec3 point = { 10.0f, 10.0f, 10.0f };
    octree.add(point, 0);

    Vec3 ray_position{ 0.0f, 0.0f, 100.0f };
    Vec3 ray_direction{ 0.0f, 0.0f, -1.0f };

    std::vector<std::pair<Vec3, int>> intersections;
    EXPECT_NO_THROW(intersections = octree.intersects_points(ray_position, ray_direction));
    EXPECT_LE(intersections.size(), 0);
}

TEST(OctreeTest, IterateOctree)
{
    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<int> octree{ bounds, 5, 5 };

    Vec3 point = { 10.0f, 10.0f, 10.0f };
    octree.add(point, 0);

    for (auto node : octree)
    {
        EXPECT_NO_THROW(node->get_bounds());
    }
}