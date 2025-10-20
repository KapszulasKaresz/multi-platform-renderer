#include "renderer/render_target/inc/render_target_window_dx.hpp"

#include <stdexcept>

#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace render_target {
RenderTargetWindowDX::RenderTargetWindowDX(
    rendering_device::RenderingDeviceDX* f_parentDevice
)
    : RenderTargetWindow(),
      m_parentDevice(f_parentDevice)
{}

RenderTargetWindowDX& RenderTargetWindowDX::setWindow(window::Window* f_window)
{
    m_window = f_window;
    return *this;
}

std::shared_ptr<image::Image> RenderTargetWindowDX::getImage()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<image::Image> RenderTargetWindowDX::getDepthImage()
{
    throw std::logic_error("Function not yet implemented");
}

glm::ivec2 RenderTargetWindowDX::getSize() const
{
    throw std::logic_error("Function not yet implemented");
}

RenderTargetWindowDX& RenderTargetWindowDX::create()
{
    m_valid = true;
    return *this;
}
}   // namespace render_target
}   // namespace renderer
