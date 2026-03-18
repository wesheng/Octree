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
    Octree<5> octree{ bounds, 5 };
    EXPECT_NO_THROW(octree.add({ 0.0f, 0.0f, 0.0f }));
}

TEST(OctreeTest, AddPointOnBounds)
{
    // Octree does allows points on bound edges

    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<5> octree{ bounds, 5 };
    EXPECT_NO_THROW(octree.add({ 50.0f, 50.0f, 50.0f }));
}

TEST(OctreeTest, TryPlaceOutsideBounds)
{
    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<5> octree{ bounds, 5 };
    EXPECT_ANY_THROW(octree.add({ 500.0f, 500.0f, 500.0f }));
}

TEST_P(OctreePointsFixture, AddMultiplePoints)
{
    float size = 100.0f;

    std::random_device rand_device;
    std::default_random_engine rand_engine{ rand_device() };
    std::uniform_real_distribution<float> rand_dist{ -size / 2.0f, size / 2.0f };

    Bounds bounds{ {0.0f, 0.0f, 0.0f}, {size, size, size}  };
    Octree<10> octree{ bounds, 5 };
    for (int i = 0; i < GetParam(); i++)
    {
        Vec3 point{ rand_dist(rand_engine), rand_dist(rand_engine) , rand_dist(rand_engine) };
        EXPECT_NO_THROW(octree.add(point));
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
    Octree<10> octree{ bounds, 5 };
    int index = rand_index(rand_engine);
    Vec3 val;
    for (int i = 0; i < GetParam(); i++)
    {
        Vec3 point{ rand_dist(rand_engine), rand_dist(rand_engine) , rand_dist(rand_engine) };

        if (i == index)
            val = point;

        octree.add(point);
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
    Octree<10> octree{ bounds, 5 };

    Vec3 rand_point{ rand_dist(rand_engine), rand_dist(rand_engine), rand_dist(rand_engine) };
    std::vector<std::tuple<Vec3, float>> points;

    for (int i = 0; i < count; i++)
    {
        Vec3 point{ rand_dist(rand_engine), rand_dist(rand_engine) , rand_dist(rand_engine) };

        points.push_back({ point, Vec3::sqr_distance(rand_point, point) });
        octree.add(point);
    }

    auto nearest = octree.nearest(rand_point, k_nearest);

    std::sort(points.begin(), points.end(), [](std::tuple<Vec3, float> a, std::tuple<Vec3, float> b) {
        return std::get<1>(a) < std::get<1>(b);
        });


    for (int i = 0; i < k_nearest && i < points.size(); i++)
    {
        EXPECT_EQ(std::get<0>(points[i]), nearest[i]);
    }
}

INSTANTIATE_TEST_SUITE_P(ManyPoints, OctreePointsFixture, testing::Values(1, 10, 100, 1000, 10000));
