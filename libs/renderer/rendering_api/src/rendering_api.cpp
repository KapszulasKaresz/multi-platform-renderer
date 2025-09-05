#include "renderer/rendering_api/inc/rendering_api.hpp"

namespace renderer {
namespace rendering_api {
std::shared_ptr<rendering_device::RenderingDevice> RenderingApi::getMainRenderingDevice()
{
    if (!m_mainRenderingDevice) {
        m_mainRenderingDevice = createRenderingDevice();
    }

    return m_mainRenderingDevice;
}

}   // namespace rendering_api
}   // namespace renderer
