#include "renderer/rendering_device/inc/rendering_device.hpp"

#include <stdexcept>

namespace renderer {
namespace rendering_device {

RenderingDevice& RenderingDevice::setTargetMSAASamples(uint32_t f_sampleCount)
{
    if (isValid()) {
        throw std::
            runtime_error(
                "RenderingDevice::setTargetMSAASamples(uint32_t f_sampleCount) msaa can "
                "only " "be set before device creation"
            );
    }

    m_MaxMSAASamples = f_sampleCount;
    return *this;
}

RenderingDevice& RenderingDevice::enableImgui(bool f_enabled)
{
    if (isValid()) {
        throw std::runtime_error(
            "RenderingDevice::enableImgui() imgui can only be enables before device "
            "creation"
        );
    }

    m_useImGui = f_enabled;
    return *this;
}

uint32_t RenderingDevice::getCurrentFrame() const
{
    return m_currentFrame;
}

int RenderingDevice::getMaxFramesInFlight() const
{
    return maxFramesInFlight;
}

uint32_t RenderingDevice::getMaxMSAASamples()
{
    return m_MaxMSAASamples;
}

bool RenderingDevice::isImGuiEnabled() const
{
    return m_useImGui;
}

}   // namespace rendering_device
}   // namespace renderer
