#include <cstdlib>
#include <GLFW/glfw3.h>

int main(int argc, const char* argv[])
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* w = glfwCreateWindow(640, 480, "No OpenGL", NULL, NULL);
    while(!glfwWindowShouldClose(w)) {
        glfwPollEvents();
    }
    glfwDestroyWindow(w);
    glfwTerminate();
    return EXIT_SUCCESS;
}