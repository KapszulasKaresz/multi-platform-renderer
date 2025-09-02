#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "renderer/rendering_server/inc/rendering_server.hpp"

int main(int argc, const char* argv[])
{
    try {
        auto& l_renderingServer =
            renderer::rendering_server::RenderingServer::getInstance();
        l_renderingServer.setRenderMode(renderer::rendering_server::RENDER_MODE_LOOP);
        l_renderingServer.mainLoop();
    } catch (const std::runtime_error& l_error) {
        std::cout << "Something went wrong: " << l_error.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
