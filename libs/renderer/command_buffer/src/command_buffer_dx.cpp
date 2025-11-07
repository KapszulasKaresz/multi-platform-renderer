#include "renderer/command_buffer/inc/command_buffer_dx.hpp"

#include <stdexcept>

#include "renderer/material/inc/material_dx.hpp"
#include "renderer/mesh/inc/triangle_mesh_dx.hpp"
#include "renderer/render_target/inc/render_target_window_dx.hpp"
#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace command_buffer {
CommandBufferDX::CommandBufferDX(rendering_device::RenderingDeviceDX* f_parentDevice)
    : CommandBuffer(),
      m_parentDevice(f_parentDevice)
{}

CommandBufferDX& CommandBufferDX::setRendering(bool f_rendering)
{
    m_rendering = f_rendering;
    return *this;
}

CommandBufferDX& CommandBufferDX::setUsage(bool f_usage)
{
    m_singleUse = f_usage;
    return *this;
}

CommandBufferDX& CommandBufferDX::create()
{
    for (int i = 0; i < rendering_device::maxFramesInFlight; i++) {
        if (FAILED(m_parentDevice->getDevice()->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_parentDevice->getCommandAllocator(i).Get(),
                nullptr,
                IID_PPV_ARGS(&m_commandLists[i])
            )))
        {
            throw std::runtime_error(
                "CommandBufferDX::create() failed to create command list"
            );
        }
        m_commandLists[i]->Close();
    }
    m_valid = true;
    return *this;
}

CommandBufferDX& CommandBufferDX::submit()
{
    m_parentDevice->executeCommandList(selectCommandList());
    return *this;
}

CommandBufferDX& CommandBufferDX::reset()
{
    m_parentDevice->getCommandAllocator()->Reset();
    auto l_commandList = selectCommandList();
    l_commandList->Reset(m_parentDevice->getCommandAllocator().Get(), nullptr);
    return *this;
}

CommandBufferDX& CommandBufferDX::begin()
{
    return *this;
}

CommandBufferDX& CommandBufferDX::end()
{
    auto l_commandList = selectCommandList();
    if (FAILED(l_commandList->Close())) {
        throw std::runtime_error("CommandBufferDX::end() failed to close command buffer");
    }
    return *this;
}

CommandBufferDX& CommandBufferDX::beginRendering(const RenderBeginInfo& f_renderBeginInfo)
{
    if (m_currentRenderTarget) {
        throw std::runtime_error(
            "CommandBufferDX::beginRendering(...) you need to end the previous "
            "rendering before you can start a new one"
        );
    }
    m_currentRenderTarget = f_renderBeginInfo.m_renderTarget;

    render_target::RenderTargetWindowDX* l_currentRenderTargetDX =
        dynamic_cast<render_target::RenderTargetWindowDX*>(m_currentRenderTarget.get());
    if (!l_currentRenderTargetDX) {
        throw std::runtime_error(
            "CommandBufferDX::beginRendering(...) unsopported render target format"
        );
    }
    auto l_commandList = selectCommandList();

    D3D12_RESOURCE_BARRIER l_renderTargetBarrier;
    l_renderTargetBarrier.Type  = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    l_renderTargetBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    l_renderTargetBarrier.Transition.pResource =
        l_currentRenderTargetDX->getRenderTarget();
    l_renderTargetBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    l_renderTargetBarrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    l_renderTargetBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    l_commandList->ResourceBarrier(1, &l_renderTargetBarrier);

    D3D12_CPU_DESCRIPTOR_HANDLE
    l_rtvHandle(
        l_currentRenderTargetDX->getDescriptorHeap()->GetCPUDescriptorHandleForHeapStart()
    );
    l_rtvHandle.ptr = l_rtvHandle.ptr
                    + (m_parentDevice->getCurrentFrame()
                       * l_currentRenderTargetDX->getDescriptorSize());
    l_commandList->OMSetRenderTargets(1, &l_rtvHandle, FALSE, nullptr);

    const float l_clearColor[] = { f_renderBeginInfo.m_clearColor.r,
                                   f_renderBeginInfo.m_clearColor.g,
                                   f_renderBeginInfo.m_clearColor.b,
                                   f_renderBeginInfo.m_clearColor.a };

    l_commandList->ClearRenderTargetView(l_rtvHandle, l_clearColor, 0, nullptr);
    return *this;
}

CommandBufferDX& CommandBufferDX::endRendering()
{
    render_target::RenderTargetWindowDX* l_currentRenderTargetDX =
        dynamic_cast<render_target::RenderTargetWindowDX*>(m_currentRenderTarget.get());
    if (!l_currentRenderTargetDX) {
        throw std::runtime_error(
            "CommandBufferDX::endRendering() unsopported render target format"
        );
    }
    auto l_commandList = selectCommandList();

    D3D12_RESOURCE_BARRIER l_presentBarrier;
    l_presentBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    l_presentBarrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    l_presentBarrier.Transition.pResource   = l_currentRenderTargetDX->getRenderTarget();
    l_presentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    l_presentBarrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    l_presentBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    l_commandList->ResourceBarrier(1, &l_presentBarrier);

    m_currentRenderTarget = nullptr;
    return *this;
}

CommandBufferDX& CommandBufferDX::useMaterial(
    std::shared_ptr<material::Material> f_material
)
{
    material::MaterialDX* l_dxMaterial =
        dynamic_cast<material::MaterialDX*>(f_material.get());

    if (!l_dxMaterial) {
        throw std::runtime_error(
            "CommandBufferDX::useMaterial(...) non dx material provided"
        );
    }
    auto l_commandList = selectCommandList();

    l_commandList->SetPipelineState(l_dxMaterial->getPipelineState());

    l_dxMaterial->updateUniforms();

    l_commandList->SetGraphicsRootSignature(l_dxMaterial->getRootSignature());
    auto l_descriptorHeaps = l_dxMaterial->getDescriptorHeaps();
    l_commandList->SetDescriptorHeaps(l_descriptorHeaps.size(), l_descriptorHeaps.data());

    for (int i = 0; i < l_descriptorHeaps.size(); i++) {
        D3D12_GPU_DESCRIPTOR_HANDLE
        l_cbvHandle(l_descriptorHeaps[i]->GetGPUDescriptorHandleForHeapStart());
        l_commandList->SetGraphicsRootDescriptorTable(i, l_cbvHandle);
    }
    return *this;
}

CommandBufferDX& CommandBufferDX::useViewport(const ViewportInfo& f_viewportInfo)
{
    auto l_commandList = selectCommandList();

    glm::vec2 l_size = f_viewportInfo.m_size;

    if (f_viewportInfo.m_fullScreen) {
        if (!m_currentRenderTarget) {
            throw std::runtime_error(
                    "CommandBufferDX::useViewport(...) cannot use fullscreen "
                    "viewport without a render target bound"
                );
        }
        l_size = m_currentRenderTarget->getSize();
    }

    D3D12_VIEWPORT l_viewport{ .TopLeftX = f_viewportInfo.m_positions.x,
                               .TopLeftY = f_viewportInfo.m_positions.y,
                               .Width    = l_size.x,
                               .Height   = l_size.y,
                               .MinDepth = 0.1f,
                               .MaxDepth = 1000.0f };
    D3D12_RECT     l_surfaceSize{ .left   = 0,
                                  .top    = 0,
                                  .right  = static_cast<LONG>(l_size.x),
                                  .bottom = static_cast<LONG>(l_size.y) };

    l_commandList->RSSetViewports(1, &l_viewport);
    l_commandList->RSSetScissorRects(1, &l_surfaceSize);
    return *this;
}

CommandBufferDX& CommandBufferDX::draw(std::shared_ptr<mesh::TriangleMesh> f_mesh)
{
    mesh::TriangleMeshDX* l_triangleMeshDX =
        dynamic_cast<mesh::TriangleMeshDX*>(f_mesh.get());

    if (!l_triangleMeshDX) {
        throw std::runtime_error(
            "CommandBufferDX::draw((...) the mesh wasn't created using a dx device"
        );
    }
    auto l_commandList = selectCommandList();

    l_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    l_commandList->IASetVertexBuffers(0, 1, &l_triangleMeshDX->getVertexBufferView());
    l_commandList->IASetIndexBuffer(&l_triangleMeshDX->getIndexBufferView());

    l_commandList->DrawIndexedInstanced(l_triangleMeshDX->getIndicieCount(), 1, 0, 0, 0);

    return *this;
}

ID3D12GraphicsCommandList* CommandBufferDX::selectCommandList()
{
    if (m_rendering) {
        return m_commandLists[m_parentDevice->getCurrentFrame()].Get();
    }
    return m_commandLists[0].Get();
}
}   // namespace command_buffer
}   // namespace renderer
