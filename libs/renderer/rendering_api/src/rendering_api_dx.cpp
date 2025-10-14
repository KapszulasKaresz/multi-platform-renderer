#include "renderer/rendering_api/inc/rendering_api_dx.hpp"

namespace renderer {
namespace rendering_api {
RenderingApiDX::RenderingApiDX() {}

std::shared_ptr<rendering_device::RenderingDevice> RenderingApiDX::getMainRenderingDevice()
{
    return nullptr;
}

void createMainRenderingDeviceWindow(window::Window* f_window) {}

std::shared_ptr<rendering_device::RenderingDevice> RenderingApiDX::createRenderingDevice()
{
    return nullptr;
}

RenderingApiDX& RenderingApiDX::create()
{
    return *this;
}
}   // namespace rendering_api
}   // namespace renderer
