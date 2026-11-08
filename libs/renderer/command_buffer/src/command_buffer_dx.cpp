#include "renderer/command_buffer/inc/command_buffer_dx.hpp"

#include <array>
#include <stdexcept>

#include <d3dcompiler.h>
#include <imgui_impl_dx12.h>

#include "renderer/image/inc/image_dx.hpp"
#include "renderer/material/inc/material_dx.hpp"
#include "renderer/mesh/inc/triangle_mesh_dx.hpp"
#include "renderer/render_target/inc/render_target_dx.hpp"
#include "renderer/render_target/inc/render_target_window_dx.hpp"
#include "renderer/rendering_device/inc/rendering_device_dx.hpp"
#include "renderer/uniform/inc/uniform_storage_buffer_dx.hpp"

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
    m_parentDevice->waitForGPU();
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
    auto l_commandList = selectCommandList();

    ID3D12DescriptorHeap* l_heaps[] = {
        m_parentDevice->getCommonDescriptorHeapManager()->getHeap(),
        m_parentDevice->getCommonSamplerHeapManager()->getHeap()
    };
    l_commandList->SetDescriptorHeaps(2, l_heaps);
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

    render_target::RenderTargetWindowDX* l_currentRenderTargetWindowDX =
        dynamic_cast<render_target::RenderTargetWindowDX*>(m_currentRenderTarget.get());
    if (l_currentRenderTargetWindowDX) {
        beginRendering(l_currentRenderTargetWindowDX, f_renderBeginInfo);
    }
    else {
        render_target::RenderTargetDX* l_currentRenderTargetDX =
            dynamic_cast<render_target::RenderTargetDX*>(m_currentRenderTarget.get());

        if (l_currentRenderTargetDX) {
            beginRendering(l_currentRenderTargetDX, f_renderBeginInfo);
        }
        else {
            throw std::runtime_error(
                "CommandBufferDX::beginRendering(...) unsopported render target format"
            );
        }
    }

    return *this;
}

CommandBufferDX& CommandBufferDX::endRendering()
{
    render_target::RenderTargetWindowDX* l_currentRenderTargetWindowDX =
        dynamic_cast<render_target::RenderTargetWindowDX*>(m_currentRenderTarget.get());
    if (l_currentRenderTargetWindowDX) {
        endRendering(l_currentRenderTargetWindowDX);
    }
    else {
        render_target::RenderTargetDX* l_currentRenderTargetDX =
            dynamic_cast<render_target::RenderTargetDX*>(m_currentRenderTarget.get());

        if (l_currentRenderTargetDX) {
            endRendering(l_currentRenderTargetDX);
        }
        else {
            throw std::runtime_error(
                "CommandBufferDX::endRendering(...) unsopported render target format"
            );
        }
    }

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

    if (l_dxMaterial->getMaterialType() == material::MaterialType::MATERIAL_TYPE_COMPUTE)
    {
        l_commandList->SetComputeRootSignature(l_dxMaterial->getRootSignature());
    }
    else {
        l_commandList->SetGraphicsRootSignature(l_dxMaterial->getRootSignature());
    }

    updateUniforms(f_material);

    return *this;
}

CommandBufferDX& CommandBufferDX::updateUniforms(
    std::shared_ptr<material::Material> f_material
)
{
    material::MaterialDX* l_dxMaterial =
        dynamic_cast<material::MaterialDX*>(f_material.get());

    if (!l_dxMaterial) {
        throw std::runtime_error(
            "CommandBufferDX::updateUniforms(...) non dx material provided"
        );
    }

    auto l_commandList = selectCommandList();

    l_dxMaterial->updateUniforms();

    auto l_CBVOffsets     = l_dxMaterial->getCBVHeapOffsets();
    auto l_UAVOffsets     = l_dxMaterial->getUAVHeapOffsets();
    auto l_SRVOffsets     = l_dxMaterial->getSRVHeapOffsets();
    auto l_samplerOffsets = l_dxMaterial->getSamplerHeapOffsets();

    int l_index = 0;

    for (int i = 0; i < l_CBVOffsets.size(); i++) {
        if (l_dxMaterial->getMaterialType()
            == material::MaterialType::MATERIAL_TYPE_COMPUTE)
        {
            l_commandList->SetComputeRootDescriptorTable(
                l_index,
                m_parentDevice->getCommonDescriptorHeapManager()->getGPUHandle(
                    l_CBVOffsets[i]
                )
            );
        }
        else {
            l_commandList->SetGraphicsRootDescriptorTable(
                l_index,
                m_parentDevice->getCommonDescriptorHeapManager()->getGPUHandle(
                    l_CBVOffsets[i]
                )
            );
        }
        l_index++;
    }

    for (int i = 0; i < l_UAVOffsets.size(); i++) {
        if (l_dxMaterial->getMaterialType()
            == material::MaterialType::MATERIAL_TYPE_COMPUTE)
        {
            l_commandList->SetComputeRootDescriptorTable(
                l_index,
                m_parentDevice->getCommonDescriptorHeapManager()->getGPUHandle(
                    l_UAVOffsets[i]
                )
            );
        }
        else {
            l_commandList->SetGraphicsRootDescriptorTable(
                l_index,
                m_parentDevice->getCommonDescriptorHeapManager()->getGPUHandle(
                    l_UAVOffsets[i]
                )
            );
        }
        l_index++;
    }

    for (int i = 0; i < l_SRVOffsets.size(); i++) {
        if (l_dxMaterial->getMaterialType()
            == material::MaterialType::MATERIAL_TYPE_COMPUTE)
        {
            l_commandList->SetComputeRootDescriptorTable(
                l_index,
                m_parentDevice->getCommonDescriptorHeapManager()->getGPUHandle(
                    l_SRVOffsets[i]
                )
            );
        }
        else {
            l_commandList->SetGraphicsRootDescriptorTable(
                l_index,
                m_parentDevice->getCommonDescriptorHeapManager()->getGPUHandle(
                    l_SRVOffsets[i]
                )
            );
        }
        l_index++;
    }

    for (int i = 0; i < l_samplerOffsets.size(); i++) {
        if (l_dxMaterial->getMaterialType()
            == material::MaterialType::MATERIAL_TYPE_COMPUTE)
        {
            l_commandList->SetComputeRootDescriptorTable(
                l_index,
                m_parentDevice->getCommonSamplerHeapManager()->getGPUHandle(
                    l_samplerOffsets[i]
                )
            );
        }
        else {
            l_commandList->SetGraphicsRootDescriptorTable(
                l_index,
                m_parentDevice->getCommonSamplerHeapManager()->getGPUHandle(
                    l_samplerOffsets[i]
                )
            );
        }
        l_index++;
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
                               .MinDepth = 0.0f,
                               .MaxDepth = 1.0f };
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

CommandBufferDX& CommandBufferDX::renderImGui()
{
    auto l_commandList = selectCommandList();
    ImGui::Render();
    m_renderImGui = true;
    return *this;
}

CommandBufferDX& CommandBufferDX::dispatchCompute(
    uint32_t f_groupCountX,
    uint32_t f_groupCountY,
    uint32_t f_groupCountZ
)
{
    auto l_commandList = selectCommandList();
    l_commandList->Dispatch(f_groupCountX, f_groupCountY, f_groupCountZ);
    return *this;
}

CommandBufferDX& CommandBufferDX::traceRays(
    std::shared_ptr<material::Material> f_material,
    render_target::RenderTarget*        f_renderTarget,
    uint32_t                            f_width,
    uint32_t                            f_height
)
{
    auto* l_material = dynamic_cast<material::MaterialDX*>(f_material.get());
    if (!l_material
        || l_material->getMaterialType() != material::MaterialType::MATERIAL_TYPE_RAY_TRACING
        || !l_material->getRayTracingStateObject())
    {
        throw std::runtime_error(
            "CommandBufferDX::traceRays() requires a created ray-tracing material"
        );
    }
    if (!f_renderTarget) {
        throw std::runtime_error(
            "CommandBufferDX::traceRays() requires a render target"
        );
    }
    auto* l_outputImage =
        dynamic_cast<image::ImageDX*>(f_renderTarget->getImage().get());
    if (!l_outputImage || !l_outputImage->getResource()) {
        throw std::runtime_error(
            "CommandBufferDX::traceRays() requires a DirectX output image"
        );
    }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> l_commandList;
    if (FAILED(selectCommandList()->QueryInterface(IID_PPV_ARGS(&l_commandList)))) {
        throw std::runtime_error(
            "CommandBufferDX::traceRays() requires a DirectX Raytracing command list"
        );
    }

    ID3D12Resource* l_outputResource = l_outputImage->getResource();
    const auto l_previousState = l_material->getRayTracingOutputState(l_outputResource);
    if (l_previousState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
        D3D12_RESOURCE_BARRIER l_toUav = {};
        l_toUav.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        l_toUav.Transition.pResource   = l_outputResource;
        l_toUav.Transition.StateBefore = l_previousState;
        l_toUav.Transition.StateAfter  = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        l_toUav.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        l_commandList->ResourceBarrier(1, &l_toUav);
    }

    l_material->bindRayTracingResources(l_commandList.Get(), l_outputImage);
    l_commandList->SetPipelineState1(l_material->getRayTracingStateObject());
    const auto l_dispatch = l_material->getDispatchRaysDesc(f_width, f_height);
    l_commandList->DispatchRays(&l_dispatch);

    D3D12_RESOURCE_BARRIER l_toCopy = {};
    l_toCopy.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    l_toCopy.Transition.pResource   = l_outputResource;
    l_toCopy.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    l_toCopy.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;
    l_toCopy.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    l_commandList->ResourceBarrier(1, &l_toCopy);
    l_material->setRayTracingOutputState(
        l_outputResource, D3D12_RESOURCE_STATE_COPY_SOURCE
    );
    return *this;
}

CommandBufferDX& CommandBufferDX::copyRenderTarget(
    render_target::RenderTarget* f_source,
    render_target::RenderTarget* f_destination
)
{
    auto* l_sourceImage =
        f_source ? dynamic_cast<image::ImageDX*>(f_source->getImage().get()) : nullptr;
    auto* l_destinationImage = f_destination
                             ? dynamic_cast<image::ImageDX*>(
                                   f_destination->getImage().get()
                               )
                             : nullptr;
    if (!l_sourceImage || !l_destinationImage) {
        throw std::runtime_error(
            "CommandBufferDX::copyRenderTarget() requires DirectX render targets"
        );
    }

    ID3D12DescriptorHeap* l_resourceHeaps[] = {
        m_parentDevice->getCommonDescriptorHeapManager()->getHeap(),
        m_parentDevice->getCommonSamplerHeapManager()->getHeap()
    };
    auto l_commandList = selectCommandList();

    D3D12_SHADER_RESOURCE_VIEW_DESC l_srvDesc = {};
    l_srvDesc.Format = image::ImageDX::convertToDXFormat(l_sourceImage->getFormat());
    l_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    l_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    l_srvDesc.Texture2D.MipLevels = 1;
    auto l_sourceDescriptor = m_copySourceDescriptors.find(l_sourceImage->getResource());
    if (l_sourceDescriptor == m_copySourceDescriptors.end()) {
        const UINT l_offset =
            m_parentDevice->getCommonDescriptorHeapManager()->addSRV(
                l_sourceImage->getResource(), l_srvDesc
            );
        l_sourceDescriptor =
            m_copySourceDescriptors.emplace(l_sourceImage->getResource(), l_offset).first;
    }

    if (m_copySamplerDescriptor == UINT_MAX) {
        D3D12_SAMPLER_DESC l_samplerDesc = {};
        l_samplerDesc.Filter         = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        l_samplerDesc.AddressU       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        l_samplerDesc.AddressV       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        l_samplerDesc.AddressW       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        l_samplerDesc.MaxLOD         = D3D12_FLOAT32_MAX;
        l_samplerDesc.MaxAnisotropy  = 1;
        l_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        m_copySamplerDescriptor =
            m_parentDevice->getCommonSamplerHeapManager()->addSampler(l_samplerDesc);
    }

    constexpr char l_shaderSource[] = R"(
        struct VertexOutput { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
        VertexOutput vertexMain(uint vertexId : SV_VertexID) {
            VertexOutput output;
            float2 uv = float2((vertexId << 1) & 2, vertexId & 2);
            output.uv = uv;
            output.position = float4(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0, 0.0, 1.0);
            return output;
        }
        Texture2D<float4> sourceImage : register(t0);
        SamplerState sourceSampler : register(s0);
        float4 pixelMain(VertexOutput input) : SV_TARGET {
            return sourceImage.SampleLevel(sourceSampler, input.uv, 0);
        }
    )";
    Microsoft::WRL::ComPtr<ID3DBlob> l_compileErrors;
    if (!m_copyVertexShader || !m_copyPixelShader) {
        if (FAILED(D3DCompile(
                l_shaderSource, sizeof(l_shaderSource) - 1, nullptr, nullptr, nullptr,
                "vertexMain", "vs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0,
                &m_copyVertexShader, &l_compileErrors
            )))
        {
            throw std::runtime_error(
                "CommandBufferDX::copyRenderTarget() failed to compile blit vertex shader"
            );
        }
        l_compileErrors.Reset();
        if (FAILED(D3DCompile(
                l_shaderSource, sizeof(l_shaderSource) - 1, nullptr, nullptr, nullptr,
                "pixelMain", "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0,
                &m_copyPixelShader, &l_compileErrors
            )))
        {
            throw std::runtime_error(
                "CommandBufferDX::copyRenderTarget() failed to compile blit pixel shader"
            );
        }
    }

    if (!m_copyRootSignature) {
        D3D12_DESCRIPTOR_RANGE l_ranges[2] = {};
        l_ranges[0] = {
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0,
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
        };
        l_ranges[1] = {
            D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0,
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
        };
        D3D12_ROOT_PARAMETER l_parameters[2] = {};
        for (UINT i = 0; i < 2; ++i) {
            l_parameters[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            l_parameters[i].DescriptorTable.NumDescriptorRanges = 1;
            l_parameters[i].DescriptorTable.pDescriptorRanges = &l_ranges[i];
            l_parameters[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        }
        D3D12_ROOT_SIGNATURE_DESC l_rootSignatureDesc = {};
        l_rootSignatureDesc.NumParameters = 2;
        l_rootSignatureDesc.pParameters   = l_parameters;
        Microsoft::WRL::ComPtr<ID3DBlob> l_rootSignatureBlob;
        if (FAILED(D3D12SerializeRootSignature(
                &l_rootSignatureDesc,
                D3D_ROOT_SIGNATURE_VERSION_1,
                &l_rootSignatureBlob,
                &l_compileErrors
            )))
        {
            throw std::runtime_error(
                "CommandBufferDX::copyRenderTarget() failed to serialize blit root signature"
            );
        }
        if (FAILED(m_parentDevice->getDevice()->CreateRootSignature(
                0, l_rootSignatureBlob->GetBufferPointer(),
                l_rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_copyRootSignature)
            )))
        {
            throw std::runtime_error(
                "CommandBufferDX::copyRenderTarget() failed to create blit root signature"
            );
        }
    }

    auto* l_windowTarget =
        dynamic_cast<render_target::RenderTargetWindowDX*>(f_destination);
    auto* l_offscreenTarget =
        dynamic_cast<render_target::RenderTargetDX*>(f_destination);
    if (!l_windowTarget && !l_offscreenTarget) {
        throw std::runtime_error(
            "CommandBufferDX::copyRenderTarget() received an unsupported destination"
        );
    }

    const auto l_sampleDesc = l_destinationImage->getResource()->GetDesc().SampleDesc;
    const uint64_t l_pipelineKey =
        (static_cast<uint64_t>(image::ImageDX::convertToDXFormat(
             f_destination->getFormat()
         ))
         << 32)
        | (static_cast<uint64_t>(l_sampleDesc.Count) << 16)
        | l_sampleDesc.Quality;
    auto l_pipelineStateIter = m_copyPipelineStates.find(l_pipelineKey);
    if (l_pipelineStateIter == m_copyPipelineStates.end()) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC l_pipelineDesc = {};
    l_pipelineDesc.pRootSignature = m_copyRootSignature.Get();
    l_pipelineDesc.VS = {
        m_copyVertexShader->GetBufferPointer(), m_copyVertexShader->GetBufferSize()
    };
    l_pipelineDesc.PS = {
        m_copyPixelShader->GetBufferPointer(), m_copyPixelShader->GetBufferSize()
    };
    l_pipelineDesc.RasterizerState = {};
    l_pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    l_pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    l_pipelineDesc.RasterizerState.FrontCounterClockwise = FALSE;
    l_pipelineDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    l_pipelineDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    l_pipelineDesc.RasterizerState.SlopeScaledDepthBias =
        D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    l_pipelineDesc.RasterizerState.DepthClipEnable = TRUE;
    l_pipelineDesc.RasterizerState.MultisampleEnable = l_sampleDesc.Count > 1;
    l_pipelineDesc.RasterizerState.AntialiasedLineEnable = FALSE;
    l_pipelineDesc.RasterizerState.ForcedSampleCount = 0;
    l_pipelineDesc.RasterizerState.ConservativeRaster =
        D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    l_pipelineDesc.BlendState.AlphaToCoverageEnable = FALSE;
    l_pipelineDesc.BlendState.IndependentBlendEnable = FALSE;
    for (auto& l_blend : l_pipelineDesc.BlendState.RenderTarget) {
        l_blend = {
            FALSE, FALSE, D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
        };
    }
    l_pipelineDesc.DepthStencilState.DepthEnable = FALSE;
    l_pipelineDesc.DepthStencilState.StencilEnable = FALSE;
    l_pipelineDesc.SampleMask = UINT_MAX;
    l_pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    l_pipelineDesc.NumRenderTargets = 1;
    l_pipelineDesc.RTVFormats[0] =
        image::ImageDX::convertToDXFormat(f_destination->getFormat());
    l_pipelineDesc.SampleDesc = l_sampleDesc;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> l_createdPipeline;
    if (FAILED(m_parentDevice->getDevice()->CreateGraphicsPipelineState(
            &l_pipelineDesc, IID_PPV_ARGS(&l_createdPipeline)
        )))
    {
        throw std::runtime_error(
            "CommandBufferDX::copyRenderTarget() failed to create blit pipeline"
        );
    }
    l_pipelineStateIter = m_copyPipelineStates.emplace(
        l_pipelineKey, std::move(l_createdPipeline)
    ).first;
    }
    ID3D12PipelineState* l_pipelineState = l_pipelineStateIter->second.Get();
    D3D12_RESOURCE_BARRIER l_barriers[2] = {};
    l_barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    l_barriers[0].Transition.pResource = l_sourceImage->getResource();
    l_barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    l_barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    l_barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    const D3D12_RESOURCE_STATES l_destinationBefore =
        l_windowTarget
            ? (l_windowTarget->isMSAA() ? D3D12_RESOURCE_STATE_RESOLVE_SOURCE
                                        : D3D12_RESOURCE_STATE_COPY_SOURCE)
            : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    l_barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    l_barriers[1].Transition.pResource = l_destinationImage->getResource();
    l_barriers[1].Transition.StateBefore = l_destinationBefore;
    l_barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    l_barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    l_commandList->ResourceBarrier(2, l_barriers);

    ID3D12DescriptorHeap* l_heaps[] = {
        m_parentDevice->getCommonDescriptorHeapManager()->getHeap(),
        m_parentDevice->getCommonSamplerHeapManager()->getHeap()
    };
    l_commandList->SetDescriptorHeaps(2, l_heaps);
    l_commandList->SetGraphicsRootSignature(m_copyRootSignature.Get());
    l_commandList->SetPipelineState(l_pipelineState);
    l_commandList->SetGraphicsRootDescriptorTable(
        0, m_parentDevice->getCommonDescriptorHeapManager()->getGPUHandle(
               l_sourceDescriptor->second
           )
    );
    l_commandList->SetGraphicsRootDescriptorTable(
        1, m_parentDevice->getCommonSamplerHeapManager()->getGPUHandle(
               m_copySamplerDescriptor
           )
    );

    ID3D12DescriptorHeap* l_rtvHeap =
        l_windowTarget ? l_windowTarget->getMSAADescriptorHeap()
                       : l_offscreenTarget->getMSAADescriptorHeap();
    auto l_rtvHandle = l_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    l_commandList->OMSetRenderTargets(1, &l_rtvHandle, FALSE, nullptr);
    const auto l_size = f_destination->getSize();
    D3D12_VIEWPORT l_viewport = {
        0.0f, 0.0f, static_cast<float>(l_size.x), static_cast<float>(l_size.y), 0.0f, 1.0f
    };
    D3D12_RECT l_scissor = { 0, 0, l_size.x, l_size.y };
    l_commandList->RSSetViewports(1, &l_viewport);
    l_commandList->RSSetScissorRects(1, &l_scissor);
    l_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    l_commandList->DrawInstanced(3, 1, 0, 0);

    l_barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    l_barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
    l_barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    l_barriers[1].Transition.StateAfter =
        l_windowTarget
            ? (l_windowTarget->isMSAA() ? D3D12_RESOURCE_STATE_RESOLVE_SOURCE
                                        : D3D12_RESOURCE_STATE_COPY_SOURCE)
            : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    l_commandList->ResourceBarrier(2, l_barriers);

    l_commandList->SetDescriptorHeaps(2, l_resourceHeaps);
    return *this;
}

CommandBufferDX& CommandBufferDX::syncStorageBuffer(
    uniform::UniformStorageBuffer* f_buffer
)
{
    auto l_UniformStorageBufferVulkan =
        dynamic_cast<uniform::UniformStorageBufferDX*>(f_buffer);

    auto l_commandList = selectCommandList();

    D3D12_RESOURCE_BARRIER l_uavBarrier = {};
    l_uavBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    l_uavBarrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    l_uavBarrier.UAV.pResource = l_UniformStorageBufferVulkan->getBuffer()->GetResource();

    l_commandList->ResourceBarrier(1, &l_uavBarrier);
    return *this;
}

render_target::RenderTarget* CommandBufferDX::getCurrentRenderTarget() const
{
    return m_currentRenderTarget.get();
}

CommandBufferDX& CommandBufferDX::copyBuffer(
    ID3D12Resource*                    f_srcBuffer,
    ID3D12Resource*                    f_dstBuffer,
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT f_footprint,
    UINT                               f_subresourceIndex,
    bool                               f_transitionResource
)
{
    D3D12_TEXTURE_COPY_LOCATION l_dstLocation = {};
    l_dstLocation.pResource                   = f_dstBuffer;
    l_dstLocation.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    l_dstLocation.SubresourceIndex            = f_subresourceIndex;

    D3D12_TEXTURE_COPY_LOCATION l_srcLocation = {};
    l_srcLocation.pResource                   = f_srcBuffer;
    l_srcLocation.Type                        = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    l_srcLocation.PlacedFootprint             = f_footprint;

    auto l_commandList = selectCommandList();
    l_commandList->CopyTextureRegion(
        &l_dstLocation,
        0,
        0,
        0,        // X, Y, Z destination offset
        &l_srcLocation,
        nullptr   // full resource copy
    );

    if (f_transitionResource) {
        D3D12_RESOURCE_BARRIER l_barrier = {};
        l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        l_barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        l_barrier.Transition.pResource   = f_dstBuffer;
        l_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

        l_commandList->ResourceBarrier(1, &l_barrier);
    }

    return *this;
}

ID3D12GraphicsCommandList* CommandBufferDX::getCommandList()
{
    return selectCommandList();
}

ID3D12GraphicsCommandList* CommandBufferDX::selectCommandList()
{
    if (m_rendering) {
        return m_commandLists[m_parentDevice->getCurrentFrame()].Get();
    }
    return m_commandLists[m_parentDevice->getCurrentFrame()].Get();
}

void CommandBufferDX::beginRendering(
    render_target::RenderTargetDX* f_renderTarget,
    const RenderBeginInfo&         f_renderBeginInfo
)
{
    auto l_commandList = selectCommandList();

    D3D12_RESOURCE_BARRIER l_renderTargetBarrier;
    l_renderTargetBarrier.Type  = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    l_renderTargetBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    l_renderTargetBarrier.Transition.pResource =
        dynamic_cast<image::ImageDX*>(f_renderTarget->getImage().get())->getResource();
    l_renderTargetBarrier.Transition.StateBefore =
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    l_renderTargetBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    l_renderTargetBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    l_commandList->ResourceBarrier(1, &l_renderTargetBarrier);

    D3D12_CPU_DESCRIPTOR_HANDLE
    l_rtvHandle(
        f_renderTarget->getMSAADescriptorHeap()->GetCPUDescriptorHandleForHeapStart()
    );

    D3D12_CPU_DESCRIPTOR_HANDLE
    l_dsvHandle(
        f_renderTarget->getDepthDescriptorHeap()->GetCPUDescriptorHandleForHeapStart()
    );
    l_commandList->OMSetRenderTargets(1, &l_rtvHandle, FALSE, &l_dsvHandle);

    const float l_clearColor[] = { f_renderBeginInfo.m_clearColor.r,
                                   f_renderBeginInfo.m_clearColor.g,
                                   f_renderBeginInfo.m_clearColor.b,
                                   f_renderBeginInfo.m_clearColor.a };

    if (f_renderBeginInfo.m_clear) {
        l_commandList->ClearRenderTargetView(l_rtvHandle, l_clearColor, 0, nullptr);
    }

    l_commandList->ClearDepthStencilView(
        l_dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr
    );
}

void CommandBufferDX::beginRendering(
    render_target::RenderTargetWindowDX* f_renderTarget,
    const RenderBeginInfo&               f_renderBeginInfo
)
{
    auto l_commandList = selectCommandList();

    D3D12_RESOURCE_BARRIER l_renderTargetBarrier;
    l_renderTargetBarrier.Type  = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    l_renderTargetBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    l_renderTargetBarrier.Transition.pResource =
        dynamic_cast<image::ImageDX*>(f_renderTarget->getImage().get())->getResource();
    l_renderTargetBarrier.Transition.StateBefore = f_renderTarget->isMSAA()
                                                     ? D3D12_RESOURCE_STATE_RESOLVE_SOURCE
                                                     : D3D12_RESOURCE_STATE_COPY_SOURCE;
    l_renderTargetBarrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    l_renderTargetBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    l_commandList->ResourceBarrier(1, &l_renderTargetBarrier);

    D3D12_CPU_DESCRIPTOR_HANDLE
    l_rtvHandle(
        f_renderTarget->getMSAADescriptorHeap()->GetCPUDescriptorHandleForHeapStart()
    );

    D3D12_CPU_DESCRIPTOR_HANDLE
    l_dsvHandle(
        f_renderTarget->getDepthDescriptorHeap()->GetCPUDescriptorHandleForHeapStart()
    );
    l_commandList->OMSetRenderTargets(1, &l_rtvHandle, FALSE, &l_dsvHandle);

    const float l_clearColor[] = { f_renderBeginInfo.m_clearColor.r,
                                   f_renderBeginInfo.m_clearColor.g,
                                   f_renderBeginInfo.m_clearColor.b,
                                   f_renderBeginInfo.m_clearColor.a };

    if (f_renderBeginInfo.m_clear) {
        l_commandList->ClearRenderTargetView(l_rtvHandle, l_clearColor, 0, nullptr);
    }

    l_commandList->ClearDepthStencilView(
        l_dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr
    );
}

void CommandBufferDX::endRendering(render_target::RenderTargetDX* f_renderTarget)
{
    auto l_commandList = selectCommandList();

    ID3D12Resource* l_msaaRT =
        dynamic_cast<image::ImageDX*>(f_renderTarget->getImage().get())->getResource();

    {
        D3D12_RESOURCE_BARRIER l_barrier;
        l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        l_barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        l_barrier.Transition.pResource   = l_msaaRT;
        l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        l_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        l_commandList->ResourceBarrier(1, &l_barrier);
    }
}

void CommandBufferDX::endRendering(render_target::RenderTargetWindowDX* f_renderTarget)
{
    auto l_commandList = selectCommandList();

    ID3D12Resource* l_msaaRT =
        dynamic_cast<image::ImageDX*>(f_renderTarget->getImage().get())->getResource();
    ID3D12Resource* l_backBuffer = f_renderTarget->getRenderTarget();

    if (f_renderTarget->isMSAA()) {
        {
            D3D12_RESOURCE_BARRIER l_barrier;
            l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            l_barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            l_barrier.Transition.pResource   = l_msaaRT;
            l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
            l_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            l_commandList->ResourceBarrier(1, &l_barrier);
        }

        {
            D3D12_RESOURCE_BARRIER l_barrier = {};
            l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            l_barrier.Transition.pResource   = l_backBuffer;
            l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RESOLVE_DEST;
            l_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            l_commandList->ResourceBarrier(1, &l_barrier);
        }

        l_commandList->ResolveSubresource(
            /* pDstResource     */ l_backBuffer,
            /* DstSubresource  */ 0,
            /* pSrcResource     */ l_msaaRT,
            /* SrcSubresource  */ 0,
            /* Format          */
            image::ImageDX::convertToDXFormat(f_renderTarget->getFormat())
        );

        {
            D3D12_RESOURCE_BARRIER l_barrier = {};
            l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            l_barrier.Transition.pResource   = l_backBuffer;
            l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_DEST;
            l_barrier.Transition.StateAfter  = m_renderImGui
                                                 ? D3D12_RESOURCE_STATE_RENDER_TARGET
                                                 : D3D12_RESOURCE_STATE_PRESENT;
            l_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            l_commandList->ResourceBarrier(1, &l_barrier);
        }

        if (m_renderImGui) {
            D3D12_CPU_DESCRIPTOR_HANDLE
            l_rtvHandle(
                f_renderTarget->getDescriptorHeap()->GetCPUDescriptorHandleForHeapStart()
            );
            l_rtvHandle.ptr = l_rtvHandle.ptr
                            + (m_parentDevice->getCurrentFrame()
                               * f_renderTarget->getDescriptorSize());

            l_commandList->OMSetRenderTargets(1, &l_rtvHandle, FALSE, nullptr);

            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), l_commandList);

            {
                D3D12_RESOURCE_BARRIER l_barrier = {};
                l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                l_barrier.Transition.pResource   = l_backBuffer;
                l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
                l_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

                l_commandList->ResourceBarrier(1, &l_barrier);
            }

            m_renderImGui = false;
        }
    }
    else {
        {
            D3D12_RESOURCE_BARRIER l_barrier;
            l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            l_barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            l_barrier.Transition.pResource   = l_msaaRT;
            l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;
            l_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            l_commandList->ResourceBarrier(1, &l_barrier);
        }

        {
            D3D12_RESOURCE_BARRIER l_barrier = {};
            l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            l_barrier.Transition.pResource   = l_backBuffer;
            l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;
            l_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            l_commandList->ResourceBarrier(1, &l_barrier);
        }

        l_commandList->CopyResource(l_backBuffer, l_msaaRT);

        {
            D3D12_RESOURCE_BARRIER l_barrier = {};
            l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            l_barrier.Transition.pResource   = l_backBuffer;
            l_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            l_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
            l_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            l_commandList->ResourceBarrier(1, &l_barrier);
        }
    }
}
}   // namespace command_buffer
}   // namespace renderer
