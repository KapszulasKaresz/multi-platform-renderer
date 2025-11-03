#ifndef COMMAND_BUFFER_DX_HPP
#define COMMAND_BUFFER_DX_HPP

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/command_buffer/inc/command_buffer.hpp"

namespace renderer {
namespace image {
class ImageDX;
}   // namespace image

namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace command_buffer {
class CommandBufferDX : public CommandBuffer {
public:
    CommandBufferDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    CommandBufferDX& setRendering(bool f_rendering);
    CommandBufferDX& setUsage(bool f_usage);
    CommandBufferDX& create();

    CommandBufferDX& submit() override final;
    CommandBufferDX& reset() override final;
    CommandBufferDX& begin() override final;
    CommandBufferDX& end() override final;
    CommandBufferDX& beginRendering(
        const RenderBeginInfo& f_renderBeginInfo
    ) override final;
    CommandBufferDX& endRendering() override final;
    CommandBufferDX& useMaterial(
        std::shared_ptr<material::Material> f_material
    ) override final;
    CommandBufferDX& useViewport(const ViewportInfo& f_viewportInfo) override final;
    CommandBufferDX& draw(std::shared_ptr<mesh::TriangleMesh> f_mesh) override final;

protected:
    bool m_rendering{ false };

    bool m_singleUse{ false };
    bool m_used{ false };

    std::shared_ptr<render_target::RenderTarget> m_currentRenderTarget{ nullptr };

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };
};
}   // namespace command_buffer
}   // namespace  renderer

#endif
