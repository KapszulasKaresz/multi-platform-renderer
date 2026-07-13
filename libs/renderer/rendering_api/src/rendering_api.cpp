#include "renderer/rendering_api/inc/rendering_api.hpp"

namespace renderer {
namespace rendering_api {
rendering_device::RenderingDevice* RenderingApi::getMainRenderingDevice()
{
    if (!m_mainRenderingDevice) {
        m_mainRenderingDevice = createRenderingDevice();
    }

    return m_mainRenderingDevice.get();
}

RenderingApi& RenderingApi::setUseImGui(bool f_use)
{
    m_useImgui = f_use;
    return *this;
}

RenderingAPIType RenderingApi::getRenderingAPIType() const
{
    return m_apiType;
}


}   // namespace rendering_api
}   // namespace renderer
