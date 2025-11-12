# Multi Platform Renderer

## Goal of this project

The goal of this project is to provide a rendering engine in which the graphics API is interchangeable. The user shouldn't be able to notice the underlying graphics API they are working with. 

## Building
### 1. Preparation

This project uses the [conan](https://conan.io/) package manager to get it's dependencies, so as a first step install conan on your system.

```
pip install conan
```
Then create a build profile which uses C++ 20 or newer. (Also recommend using ninja for cmake although not mandatory). The project was tested with the MSVC and GCC compiler. 

### 2. Getting the dependencies
After setting up the build configuration run the conan install command which will collect the dependencies for the project. (You can set the output folder to `_build` as it is already in the `.gitignore` of the repository).
```
conan install . -of=_build --build=missing 
```
There is one dependency ([D3D12 Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator)) that is currently not available as a conan package so you need to source that manually, you can find more info about it [here](libs/renderer/external/readme.md)

### 3. Building
Once you have the dependencies you can use the conan build command to build the project and compile the shaders.

```
conan build . -of=_build
```

## Features
| Feature                 | Vulkan | DirectX |
| :---------------------- | :----: | ------: |
| Vertex input            |   ✅    |       ✅ |
| Custom shader materials |   ✅    |       ✅ |
| Uniform handling        |   ✅    |       ✅ |
| On-screen rendering     |   ✅    |       ✅ |
| Off-screen rendering    |   ✅    |       ❌ |
| Textures                |   ✅    |       🔜 |
| Mip-map generation      |   ✅    |       ❌ |
| MSAA                    |   ✅    |       ❌ |