# Octree
A recursive implementation of [Octrees](https://en.wikipedia.org/wiki/Octree) using C++20.

Note that modern C++ features are utilized in this project, including use of STL functions and managed pointers.

The project is set up with the following subprojects:
- `Octree` - Main implementation
- `Demo` - Executable to showcase Octree implementation using OpenGL
- `Tests` - Ensure quality and consistency of Octree implementation

## Features
- Raycast for intersecting points (and octants)
	- ~60 fps with 1,200,000 (Debug) and 3,500,000 (Release) points
	- Tested with depth of 3 on AMD Ryzen 5 5600X without searching nearby neighbors
- Ability to search for nearest points close to a passed position (K-nearest neighbor)
    - Use of Sphere-AABB testing to determine octants near the passed position
- Storage of identifiers to points for future retrieval
- Ability to iterate through octants via range-based for loop

## Building
This project uses CMake as a build configuration tool. Typically the command for configuring and generating a project goes along the lines of:

```bash
cmake -S . -B ./out
cmake --build ./out
```

Demo and Test dependencies will be downloaded automatically via FetchContent. You can see a list of dependencies under each subproject below.

The project was tested using the MSVC compiler on Windows.

## Demo
The Demo is a minimal, interactive OpenGL 4.6 implementation (w/ DSA) created to showcase the Octree. It uses a rendered ray to demonstrate raycasting and nearby neighbors against a randomized set of points, representing entities. The ray can be set to a static position or to follow camera position and rotation.

The Octree can also be regenerated with new points & depth settings.

- WASD / Shift / Space to move
- Mouse to look around (camera rotation)
- Esc to toggle locking camera rotation

It uses the following libraries:
- [GLFW](https://github.com/glfw/glfw.git)
- [GLM](https://github.com/g-truc/glm.git)
- [Glad](https://github.com/Dav1dde/glad.git)
- [imgui](https://github.com/ocornut/imgui)

Note that the demo does make several optimizations for smoother framerate during rendering:
- Utilizes instanced rendering for the Octree grid and Points
	- Rendering the grid is *disabled* when using more than 100,000 points
- Caches Octree operation results unless ray is following camera

Additionally, depth testing is not enabled by default as it affects grid rendering, but you can enable it through the imgui interface.

### Description of Demo GUI options
- Octree
	- `Show Octree Grid` - Toggles rendering the Octree grid. Affects performance.
	- `Grid Size` - Sets the size & bounds of the Octree grid
	- `Max Depth` - Sets the maximum recursive depth of the Octree.
	- `Remake Octree` - Recreates the Octree using the `Grid Size` and `Max Depth` parameters. Also regenerates points.
- Ray
	- `Origin` - Sets the start location of the ray
	- `Rotation` - Sets the rotation and direction of the ray (in euler angles & degrees)
	- `Orient Ray to Camera` - Sets the `Origin` and `Rotation` to the current Camera position and look direction.
	- `Lock Ray to Camera` - Forces the ray to follow the camera location and orientation.
	- `Cast Tolerance` - Sets the leeway in which a point is considered to be intersecting the ray
	- `Num Nearest Points` - Sets the amount of points used for nearest neighbor calculation. Affects performance; higher values requires more computations.
- Other
	- `Depth Test` - Enables GL depth testing. Affects grid rendering -- raycasted octants may not be visably marked.

## Tests
This project uses [GoogleTest](https://github.com/google/googletest) as the test suite. Tests focus on the Octree itself; tests for utility classes are out of scope for this project.

## Future Work

- Benchmark performance of this implementation compared to other Octree implementations
- Add frustum casting to cull for visible points for improved rendering performance
- Support for storing triangles and other structures for meshes
- Ability to move and remove points