#include "renderer/material/inc/material_dx.hpp"

#include <stdexcept>

#include "renderer/image/inc/image_dx.hpp"
#include "renderer/mesh/inc/mesh.hpp"
#include "renderer/rendering_device/inc/rendering_device_dx.hpp"
#include "renderer/uniform/inc/uniform_collection_dx.hpp"
#include "renderer/utils/inc/utils.hpp"

namespace renderer {
namespace material {

MaterialDX::MaterialDX(rendering_device::RenderingDeviceDX* f_parentDevice)
    : Material(),
      m_parentDevice(f_parentDevice)
{}

MaterialDX& MaterialDX::create()
{
    createRootSignature();
    createPipelineState();
    m_valid = true;
    return *this;
}

ID3D12RootSignature* MaterialDX::getRootSignature()
{
    return m_rootSignature.Get();
}

ID3D12PipelineState* MaterialDX::getPipelineState()
{
    return m_pipelineState.Get();
}

std::vector<UINT> MaterialDX::getCBVHeapOffsets()
{
    std::vector<UINT> l_ret;
    for (auto l_uniform : m_uniformCollections) {
        uniform::UniformCollectionDX* l_uniformDX =
            dynamic_cast<uniform::UniformCollectionDX*>(l_uniform.get());

        if (!l_uniformDX) {
            throw std::runtime_error(
                "MaterialDX::getCBVHeapOffsets() non dx uniform in the material"
            );
        }

        l_ret.push_back(l_uniformDX->getHeapOffset());
    }

    return l_ret;
}

std::vector<UINT> MaterialDX::getSRVHeapOffsets()
{
    std::vector<UINT> l_ret;
    for (auto l_uniform : m_uniformCollections) {
        uniform::UniformCollectionDX* l_uniformDX =
            dynamic_cast<uniform::UniformCollectionDX*>(l_uniform.get());

        if (!l_uniformDX) {
            throw std::runtime_error(
                "MaterialDX::getSRVHeapOffsets() non dx uniform in the material"
            );
        }

        auto l_textureOffsets = l_uniformDX->getTextureHeapOffsets();
        l_ret.insert(l_ret.end(), l_textureOffsets.begin(), l_textureOffsets.end());
    }

    return l_ret;
}

std::vector<UINT> MaterialDX::getSamplerHeapOffsets()
{
    std::vector<UINT> l_ret;
    for (auto l_uniform : m_uniformCollections) {
        uniform::UniformCollectionDX* l_uniformDX =
            dynamic_cast<uniform::UniformCollectionDX*>(l_uniform.get());

        if (!l_uniformDX) {
            throw std::runtime_error(
                "MaterialDX::getSamplerHeapOffsets() non dx uniform in the material"
            );
        }

        auto l_samplerOffsets = l_uniformDX->getTextureHeapSamplerOffsets();
        l_ret.insert(l_ret.end(), l_samplerOffsets.begin(), l_samplerOffsets.end());
    }

    return l_ret;
}

void MaterialDX::createRootSignature()
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE l_featureData = {};
    l_featureData.HighestVersion                    = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(m_parentDevice->getDevice()->CheckFeatureSupport(
            D3D12_FEATURE_ROOT_SIGNATURE, &l_featureData, sizeof(l_featureData)
        )))
    {
        l_featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    std::vector<D3D12_ROOT_PARAMETER1> l_rootParameters;
    // CBV ranges
    for (int i = 0; i < m_uniformCollections.size(); i++) {
        D3D12_DESCRIPTOR_RANGE1 l_range{};

        l_range.BaseShaderRegister                = i;
        l_range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        l_range.NumDescriptors                    = 1;
        l_range.RegisterSpace                     = 0;
        l_range.OffsetInDescriptorsFromTableStart = 0;
        l_range.Flags                             = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

        D3D12_ROOT_PARAMETER1 l_rootParameter{};
        l_rootParameter.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        l_rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        l_rootParameter.DescriptorTable.NumDescriptorRanges = 1;
        l_rootParameter.DescriptorTable.pDescriptorRanges   = &l_range;
        l_rootParameters.push_back(l_rootParameter);
    }

    // Texture SRV ranges
    int l_srvIndex = 0;
    for (int i = 0; i < m_uniformCollections.size(); i++) {
        for (int j = 0; j < static_cast<UINT>(m_uniformCollections[0]->getTextureCount());
             j++)
        {
            D3D12_DESCRIPTOR_RANGE1 l_srvRange = {};
            l_srvRange.BaseShaderRegister      = l_srvIndex++;
            l_srvRange.RangeType               = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            l_srvRange.NumDescriptors          = 1;
            l_srvRange.RegisterSpace           = 0;
            l_srvRange.OffsetInDescriptorsFromTableStart =
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            l_srvRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

            D3D12_ROOT_PARAMETER1 l_rootParameter{};
            l_rootParameter.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            l_rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            l_rootParameter.DescriptorTable.NumDescriptorRanges = 1;
            l_rootParameter.DescriptorTable.pDescriptorRanges   = &l_srvRange;
            l_rootParameters.push_back(l_rootParameter);
        }
    }

    int l_samplerIndex = 0;
    for (int i = 0; i < m_uniformCollections.size(); i++) {
        for (int j = 0; j < static_cast<UINT>(m_uniformCollections[0]->getTextureCount());
             j++)
        {
            D3D12_DESCRIPTOR_RANGE1 l_samplerRange = {};
            l_samplerRange.RangeType               = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            l_samplerRange.BaseShaderRegister      = l_samplerIndex++;
            l_samplerRange.NumDescriptors          = 1;
            l_samplerRange.RegisterSpace           = 0;
            l_samplerRange.OffsetInDescriptorsFromTableStart =
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


            D3D12_ROOT_PARAMETER1 l_rootParameterSampler{};
            l_rootParameterSampler.ParameterType =
                D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            l_rootParameterSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            l_rootParameterSampler.DescriptorTable.NumDescriptorRanges = 1;
            l_rootParameterSampler.DescriptorTable.pDescriptorRanges   = &l_samplerRange;
            l_rootParameters.push_back(l_rootParameterSampler);
        }
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC l_rootSignatureDesc;
    l_rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    l_rootSignatureDesc.Desc_1_1.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    l_rootSignatureDesc.Desc_1_1.NumParameters     = l_rootParameters.size();
    l_rootSignatureDesc.Desc_1_1.pParameters       = l_rootParameters.data();
    l_rootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
    l_rootSignatureDesc.Desc_1_1.pStaticSamplers   = nullptr;

    ID3DBlob* l_signature;
    ID3DBlob* l_error;
    if (FAILED((D3D12SerializeVersionedRootSignature(
            &l_rootSignatureDesc, &l_signature, &l_error
        )))
        || FAILED((m_parentDevice->getDevice()->CreateRootSignature(
            0,
            l_signature->GetBufferPointer(),
            l_signature->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature)
        ))))
    {
        throw std::runtime_error(
            "MaterialDX::createRootSignature() failed to create root signature"
        );
    }
    m_rootSignature->SetName(L"Material Root Signature");

    if (l_signature) {
        l_signature->Release();
        l_signature = nullptr;
    }
}

void MaterialDX::createPipelineState()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC l_psoDesc = {};

    l_psoDesc.InputLayout = {
        getInputElementDescription().data(),
        static_cast<unsigned int>(getInputElementDescription().size())
    };

    l_psoDesc.pRootSignature = m_rootSignature.Get();

    D3D12_SHADER_BYTECODE l_vsBytecode;
    std::vector<char> l_vsBytecodeData = utils::readFile(m_shaderLocation + ".vert.dxil");
    l_vsBytecode.pShaderBytecode       = l_vsBytecodeData.data();
    l_vsBytecode.BytecodeLength        = l_vsBytecodeData.size();
    l_psoDesc.VS                       = l_vsBytecode;

    D3D12_SHADER_BYTECODE psBytecode;
    std::vector<char> l_psBytecodeData = utils::readFile(m_shaderLocation + ".frag.dxil");
    psBytecode.pShaderBytecode         = l_psBytecodeData.data();
    psBytecode.BytecodeLength          = l_psBytecodeData.size();
    l_psoDesc.PS                       = psBytecode;

    D3D12_RASTERIZER_DESC l_rasterDesc;
    l_rasterDesc.FillMode              = D3D12_FILL_MODE_SOLID;
    l_rasterDesc.CullMode              = D3D12_CULL_MODE_NONE;
    l_rasterDesc.FrontCounterClockwise = FALSE;
    l_rasterDesc.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;
    l_rasterDesc.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    l_rasterDesc.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    l_rasterDesc.DepthClipEnable       = TRUE;
    l_rasterDesc.MultisampleEnable     = FALSE;
    l_rasterDesc.AntialiasedLineEnable = FALSE;
    l_rasterDesc.ForcedSampleCount     = 0;
    l_rasterDesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    l_psoDesc.RasterizerState          = l_rasterDesc;
    l_psoDesc.PrimitiveTopologyType    = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    D3D12_BLEND_DESC l_blendDesc;
    l_blendDesc.AlphaToCoverageEnable                                   = FALSE;
    l_blendDesc.IndependentBlendEnable                                  = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC l_defaultRenderTargetBlendDesc = {
        FALSE,
        FALSE,
        D3D12_BLEND_ONE,
        D3D12_BLEND_ZERO,
        D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE,
        D3D12_BLEND_ZERO,
        D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
        l_blendDesc.RenderTarget[i] = l_defaultRenderTargetBlendDesc;
    }
    l_psoDesc.BlendState = l_blendDesc;

    D3D12_DEPTH_STENCIL_DESC l_depthStencilDesc = {};
    l_depthStencilDesc.DepthEnable              = TRUE;
    l_depthStencilDesc.DepthWriteMask           = D3D12_DEPTH_WRITE_MASK_ALL;
    l_depthStencilDesc.DepthFunc                = D3D12_COMPARISON_FUNC_LESS;
    l_depthStencilDesc.StencilEnable            = FALSE;

    l_psoDesc.DepthStencilState = l_depthStencilDesc;
    l_psoDesc.DSVFormat  = image::ImageDX::convertToDXFormat(image::IMAGE_FORMAT_DEPTH);
    l_psoDesc.SampleMask = UINT_MAX;

    l_psoDesc.NumRenderTargets = 1;
    l_psoDesc.RTVFormats[0]    = DXGI_FORMAT_B8G8R8A8_UNORM;
    l_psoDesc.SampleDesc.Count = m_parentDevice->getMaxMSAASamples();

    if (FAILED(m_parentDevice->getDevice()->CreateGraphicsPipelineState(
            &l_psoDesc, IID_PPV_ARGS(&m_pipelineState)
        )))
    {
        throw std::runtime_error(
            "MaterialDX::createPipelineState() failed to create pipeline"
        );
    }
}

std::array<D3D12_INPUT_ELEMENT_DESC, 4> MaterialDX::getInputElementDescription()
{
    std::array<D3D12_INPUT_ELEMENT_DESC, 4> l_desc = {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION",
                                 0, DXGI_FORMAT_R32G32B32_FLOAT,
                                 0,  0,
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        D3D12_INPUT_ELEMENT_DESC{   "NORMAL",
                                 0, DXGI_FORMAT_R32G32B32_FLOAT,
                                 0, 12,
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD",
                                 0,    DXGI_FORMAT_R32G32_FLOAT,
                                 0, 24,
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        D3D12_INPUT_ELEMENT_DESC{    "COLOR",
                                 0, DXGI_FORMAT_R32G32B32_FLOAT,
                                 0, 32,
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    return l_desc;
}

}   // namespace material
}   // namespace renderer
