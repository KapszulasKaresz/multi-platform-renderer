#include "renderer/rendering_device/inc/rendering_device.hpp"

namespace renderer {
namespace rendering_device {
bool RenderingDevice::isValid()
{
    return m_valid;
}

uint32_t RenderingDevice::getCurrentFrame()
{
    return m_currentFrame;
}

}   // namespace rendering_device
}   // namespace renderer
