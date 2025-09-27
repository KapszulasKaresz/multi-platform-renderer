#include "renderer/rendering_device/inc/rendering_device.hpp"

namespace renderer {
namespace rendering_device {
bool RenderingDevice::isValid()
{
    return m_valid;
}

uint32_t RenderingDevice::getCurrentFrame() const
{
    return m_currentFrame;
}

int RenderingDevice::getMaxFramesInFlight() const
{
    return m_maxFramesInFlight;
}

}   // namespace rendering_device
}   // namespace renderer
