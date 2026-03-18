# Octree
A recursive implementation of [Octrees](https://en.wikipedia.org/wiki/Octree) using the C++20 compiler.

Note that modern C++ features are utilized in this project, including use of managed pointers.

The project is set up with the following subprojects:
- `Octree` - Main implementation
- `Demo` - Executable to showcase Octree implementation
- `Utility` - Classes to streamline Octree implementation
- `Tests` - Ensure quality and consistency of Octree implementation

## Features
- Ability to search for nearest points close to a passed position (K-nearest neighbor)
    - Use of Sphere-AABB testing to determine Octants near the passed position

## Building
This project uses Cmake as a build configuration tool. Typically the command for generating a project goes along the lines of:

```bash
cmake --build ./out
```

## Tests
This project uses [GoogleTest](https://github.com/google/googletest) as the test suite. Tests focus on the Octree itself; tests for Utility classes are out of scope for this project.