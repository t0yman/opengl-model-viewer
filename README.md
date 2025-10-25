
# Simple OpenGL Model Viewer

A real-time 3D model viewer built with modern OpenGL to demonstrate graphics programming fundamentals.

![Cube Demo](docs/cube_demo.mp4)

![Pyramid Demo](docs/pyramid_demo.mp4)

![Tetrahedron Demo](docs/tetrahedron_demo.mp4)

## Features

- 3D Model Loading: OBJ file parser supporting vertex positions and normals
- Phong Lighting: Full Phong reflection model with ambient, diffuse, and specular components
- Interactive Camera: Obital camera system with smooth sphereical coordinate controls
- Modern OpenGL: Uses OpenGL 3.3 Core Profile with vertex and fragment shaders
- Cross-Platform: Builds on Windows, macOS, and Linux via CMake

## Controls

- W/S: Zoom in/out
- A/D: Rotate camera horizontally around model
- Q/E: Rotate camera vertically
- ESC: Exit application

## Technical Highlights

### Phong Lighting Implementation

The fragment shader implements the classic Phong reflection model with:
- Ambient lighting for base illumination
- Diffuse lighting based on surface-to-light angle
- Specular highlights with configurable shininess
- Proper normal transformation in world space

### Spherical Camera System

Camera position is calculated using spherical coordinates and converted to Cartesian coordinates. This provides intuitive orbital controls while avoiding gimbal lock issues.

## Building

### Prerequisites

- CMake 3.10.3 or higher
- C++11 compatible compiler
- vcpkg package manager

### Build Instructions

```bash
# clone the repository
git clone https://github.com/t0yman/opengl-model-viewer.git
cd opengl-model-viewer

# install dependencies via vcpkg
vcpkg install

mkdir build && cd build
cmake ..
cmake --build .

./opengl-model-viewer
```

## Dependencies

- GLFW: Window creation and input handling
- GLAD: OpenGL function loader
- GLM: Mathematics library for computer graphics

## Future Enhancements

- Multiple mesh support for complex models
- Texture mapping with UV coordinates
- Additional lighting models (Blinn-Phong, PBR)
- Multiple light sources
- Shadow mapping
- Material system with per-object properties