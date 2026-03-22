# Octree
A recursive implementation of [Octrees](https://en.wikipedia.org/wiki/Octree) using the C++20 compiler.

Note that modern C++ features are utilized in this project, including use of managed pointers.

The project is set up with the following subprojects:
- `Octree` - Main implementation
- `Demo` - Executable to showcase Octree implementation using OpenGL
- `Tests` - Ensure quality and consistency of Octree implementation

## Features
- Ability to search for nearest points close to a passed position (K-nearest neighbor)
    - Use of Sphere-AABB testing to determine octants near the passed position
- Raycast for intersecting points (and octants)
- Storage of identifiers to points for future retrieval
- Ability to iterate through octants
- Error handling for points outside of Octree bounds

## Building
This project uses Cmake as a build configuration tool. Typically the command for configuring and generating a project goes along the lines of:

```bash
cmake -S . -B ./out
cmake --build ./out
```

Demo and Test dependencies will be downloaded automatically via FetchContent.

The project was tested using the MSVC compiler on Windows.

## Demo
The Demo is an minimal, interactive OpenGL 4.6 implementation (w/ DSA) created to showcase the Octree implementation. It uses a rendered ray to demonstrate raycasting and nearby neighbors. The ray can be set to a static position or to follow camera position and rotation.

The Octree can also be regenerated with new points & depth settings.

- WASD / Shift / Space to move
- Mouse to look around (camera rotation)
- Esc to lock camera rotation

It uses the following libraries:
- [GLFW](https://github.com/glfw/glfw.git)
- [GLM](https://github.com/g-truc/glm.git)
- [Glad](https://github.com/Dav1dde/glad.git)
- [imgui](https://github.com/ocornut/imgui)

Note that the demo *does* make several optimizations for smoother framerate during rendering.
- Utilizes instanced rendering for the Octree grid and Points
	- Rendering the grid is disabled for large number of points
- Caches Octree operation results unless ray is following camera

## Tests
This project uses [GoogleTest](https://github.com/google/googletest) as the test suite. Tests focus on the Octree itself; tests for utility classes are out of scope for this project.

## Future Work

- Performance of this implementation compared to other implementations
- Searching nearest neighbors incurs the largest performance cost
- Support for storing triangles and other structures for meshes
- 