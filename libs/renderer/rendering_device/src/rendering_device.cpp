#include "renderer/rendering_device/inc/rendering_device.hpp"

#include <stdexcept>

namespace renderer {
namespace rendering_device {
RenderingDevice& RenderingDevice::setTargetMSAASamples(uint32_t f_sampleCount)
{
    if (isValid()) {
        throw std::runtime_error(
            "RenderingDevice::setTargetMSAASamples(uint32_t f_sampleCount) msaa can only "
            "be set before device creation"
        );
    }

    m_MaxMSAASamples = f_sampleCount;
    return *this;
}

uint32_t RenderingDevice::getCurrentFrame() const
{
    return m_currentFrame;
}

int RenderingDevice::getMaxFramesInFlight() const
{
    return m_maxFramesInFlight;
}

uint32_t RenderingDevice::getMaxMSAASamples()
{
    return m_MaxMSAASamples;
}

}   // namespace rendering_device
}   // namespace renderer
