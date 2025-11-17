#ifndef COMMAND_BUFFER_DX_HPP
#define COMMAND_BUFFER_DX_HPP

#include <array>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "renderer/command_buffer/inc/command_buffer.hpp"
#include "renderer/rendering_device/inc/rendering_device.hpp"

namespace renderer {
namespace image {
class ImageDX;
}   // namespace image

namespace rendering_device {
class RenderingDeviceDX;
}   // namespace rendering_device

namespace render_target {
class RenderTargetDX;
class RenderTargetWindowDX;
}   // namespace render_target

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
    CommandBufferDX& renderImGui() override final;

    CommandBufferDX& copyBuffer(
        ID3D12Resource*                    f_srcBuffer,
        ID3D12Resource*                    f_dstBuffer,
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT f_footprint,
        UINT                               f_subresourceIndex
    );

    ID3D12GraphicsCommandList* getCommandList();

protected:
    ID3D12GraphicsCommandList* selectCommandList();

    void beginRendering(
        render_target::RenderTargetDX* f_renderTarget,
        const RenderBeginInfo&         f_renderBeginInfo
    );
    void beginRendering(
        render_target::RenderTargetWindowDX* f_renderTarget,
        const RenderBeginInfo&               f_renderBeginInfo
    );

    void endRendering(render_target::RenderTargetDX* f_renderTarget);
    void endRendering(render_target::RenderTargetWindowDX* f_renderTarget);

    bool m_rendering{ false };

    bool m_singleUse{ false };
    bool m_used{ false };

    std::shared_ptr<render_target::RenderTarget> m_currentRenderTarget{ nullptr };

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };

    std::array<
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>,
        rendering_device::maxFramesInFlight>
                                                m_commandLists{};
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_initialPipelineState{ nullptr };
};
}   // namespace command_buffer
}   // namespace  renderer

#endif
