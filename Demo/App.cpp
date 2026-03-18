#include "App.h"
#include <array>
#include <Octree.h>
#include <random>

int main()
{
    std::random_device rand_device;
    std::default_random_engine rand_engine { rand_device()};
    std::uniform_real_distribution<float> rand_dist{ -50.0f, 50.0f };
    
    std::array<Vec3, 5000> points;

    Bounds b{ {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
    Octree<10> octree{ b, 5 };
    for (int i = 0; i < points.size(); i++)
    {
        points[i] = Vec3{ rand_dist(rand_engine), rand_dist(rand_engine), rand_dist(rand_engine) };
        octree.add(points[i]);
    }
    std::cout << "Generated " << points.size() << " points." << std::endl;

    std::cout << static_cast<std::string>(octree) << std::endl;

	return 0;
}
