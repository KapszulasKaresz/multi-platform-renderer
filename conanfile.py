from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout


class MultiPlatformRenderer(ConanFile):
    name = "MultiPlatformRenderer"
    version = "0.0.1"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("glfw/3.4")
        self.requires("glm/1.0.1")
        self.requires("vulkan-memory-allocator/3.3.0")
        self.requires("d3d12-memory-allocator/3.0.1")
        self.requires("stb/cci.20240531")
        self.requires("imgui/1.92.4")

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()