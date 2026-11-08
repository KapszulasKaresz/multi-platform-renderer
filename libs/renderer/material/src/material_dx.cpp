#include "renderer/material/inc/material_dx.hpp"

#include <cstring>
#include <stdexcept>

#include "renderer/image/inc/image_dx.hpp"
#include "renderer/mesh/inc/mesh.hpp"
#include "renderer/rendering_device/inc/rendering_device_dx.hpp"
#include "renderer/ray_tracing/acceleration_structure/inc/acceleration_structure_dx.hpp"
#include "renderer/texture/inc/texture_dx.hpp"
#include "renderer/uniform/inc/uniform_acceleration_structure_dx.hpp"
#include "renderer/uniform/inc/uniform_collection_dx.hpp"
#include "renderer/uniform/inc/uniform_storage_buffer_dx.hpp"
#include "renderer/utils/inc/utils.hpp"

namespace renderer {
namespace material {
namespace {
constexpr UINT rayTracingTextureCount = 128;
constexpr UINT rayTracingRecordStride =
    D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
constexpr UINT rayTracingRecordAlignment =
    D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
}

MaterialDX::MaterialDX(rendering_device::RenderingDeviceDX* f_parentDevice)
    : Material(),
      m_parentDevice(f_parentDevice)
{}

MaterialDX& MaterialDX::create()
{
    if (!isOriginal()) {
        m_valid = true;
        return *this;
    }

    if (getMaterialType() == MaterialType::MATERIAL_TYPE_RAY_TRACING) {
        createRayTracingRootSignature();
        createRayTracingDescriptors();
        createRayTracingPipelineState();
    }
    else {
        createRootSignature();
    }

    if (getMaterialType() == MaterialType::MATERIAL_TYPE_COMPUTE) {
        createComputePipelineState();
    }
    else if (getMaterialType() == MaterialType::MATERIAL_TYPE_RENDER) {
        createPipelineState();
    }
    else if (getMaterialType() != MaterialType::MATERIAL_TYPE_RAY_TRACING) {
        throw std::runtime_error("MaterialDX::create() unsupported material type");
    }

    m_valid = true;
    return *this;
}

ID3D12RootSignature* MaterialDX::getRootSignature()
{
    if (isOriginal()) {
        return m_rootSignature.Get();
    }
    else {
        return dynamic_cast<MaterialDX*>(m_original.get())->getRootSignature();
    }
}

ID3D12PipelineState* MaterialDX::getPipelineState()
{
    if (isOriginal()) {
        return m_pipelineState.Get();
    }
    else {
        return dynamic_cast<MaterialDX*>(m_original.get())->getPipelineState();
    }
}

ID3D12StateObject* MaterialDX::getRayTracingStateObject()
{
    if (isOriginal()) {
        return m_rayTracingStateObject.Get();
    }
    return getOriginalMaterial()->getRayTracingStateObject();
}

MaterialDX* MaterialDX::getOriginalMaterial()
{
    return isOriginal() ? this : dynamic_cast<MaterialDX*>(m_original.get());
}

const MaterialDX* MaterialDX::getOriginalMaterial() const
{
    return isOriginal() ? this : dynamic_cast<const MaterialDX*>(m_original.get());
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

std::vector<UINT> MaterialDX::getUAVHeapOffsets()
{
    std::vector<UINT> l_ret;
    for (auto l_storageBuffer : m_uniformStorageBuffers) {
        uniform::UniformStorageBufferDX* l_storageBufferDX =
            dynamic_cast<uniform::UniformStorageBufferDX*>(l_storageBuffer.get());

        if (!l_storageBufferDX) {
            throw std::runtime_error(
                "MaterialDX::getUAVHeapOffsets() non dx storage buffer in the material"
            );
        }

        l_ret.push_back(l_storageBufferDX->getHeapOffset());
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

D3D12_DISPATCH_RAYS_DESC MaterialDX::getDispatchRaysDesc(
    UINT f_width, UINT f_height
) const
{
    const MaterialDX* l_material  = getOriginalMaterial();
    const UINT64      l_recordSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    return {
        .RayGenerationShaderRecord = {
            .StartAddress = l_material->m_raygenShaderRecord,
            .SizeInBytes  = l_recordSize
        },
        .MissShaderTable = {
            .StartAddress  = l_material->m_missShaderRecord,
            .SizeInBytes   = l_recordSize,
            .StrideInBytes = l_recordSize
        },
        .HitGroupTable = {
            .StartAddress  = l_material->m_hitGroupShaderRecord,
            .SizeInBytes   = l_recordSize,
            .StrideInBytes = l_recordSize
        },
        .Width  = f_width,
        .Height = f_height,
        .Depth  = 1
    };
}

D3D12_RESOURCE_STATES MaterialDX::getRayTracingOutputState(
    ID3D12Resource* f_resource
) const
{
    const MaterialDX* l_material = getOriginalMaterial();
    auto l_state = l_material->m_rayTracingOutputStates.find(f_resource);
    return l_state == l_material->m_rayTracingOutputStates.end()
         ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS
         : l_state->second;
}

void MaterialDX::setRayTracingOutputState(
    ID3D12Resource* f_resource, D3D12_RESOURCE_STATES f_state
)
{
    MaterialDX* l_material = getOriginalMaterial();
    l_material->m_rayTracingOutputStates[f_resource] = f_state;
}

void MaterialDX::bindRayTracingResources(
    ID3D12GraphicsCommandList4* f_commandList, image::ImageDX* f_outputImage
)
{
    MaterialDX* l_material = getOriginalMaterial();
    if (l_material != this) {
        l_material->bindRayTracingResources(f_commandList, f_outputImage);
        return;
    }

    updateUniforms();
    const auto l_cbvOffsets = getCBVHeapOffsets();
    if (l_cbvOffsets.size() != 2 || m_uniformStorageBuffers.size() != 1
        || m_uniformAccelerationStructures.size() != 1)
    {
        throw std::runtime_error(
            "MaterialDX::bindRayTracingResources() requires two constant buffers, one "
            "geometry buffer, and one acceleration structure"
        );
    }

    auto* l_accelerationStructure =
        dynamic_cast<uniform::UniformAccelerationStructureDX*>(
            m_uniformAccelerationStructures.front().get()
        );
    if (!l_accelerationStructure || !l_accelerationStructure->isValid()) {
        throw std::runtime_error(
            "MaterialDX::bindRayTracingResources() requires a valid DirectX "
            "acceleration-structure uniform"
        );
    }

    auto* l_heap        = m_parentDevice->getCommonDescriptorHeapManager();
    auto* l_samplerHeap = m_parentDevice->getCommonSamplerHeapManager();
    auto* l_geometryBuffer = dynamic_cast<uniform::UniformStorageBufferDX*>(
        m_uniformStorageBuffers.front().get()
    );
    ID3D12Resource* l_geometryResource =
        l_geometryBuffer->getBuffer()->GetResource();
    auto l_geometryState = m_rayTracingGeometryStates.find(l_geometryResource);
    const auto l_currentGeometryState =
        l_geometryState == m_rayTracingGeometryStates.end()
            ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS
            : l_geometryState->second;
    if (l_currentGeometryState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) {
        D3D12_RESOURCE_BARRIER l_geometryBarrier = {};
        l_geometryBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        l_geometryBarrier.Transition.pResource = l_geometryResource;
        l_geometryBarrier.Transition.StateBefore = l_currentGeometryState;
        l_geometryBarrier.Transition.StateAfter =
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        l_geometryBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        f_commandList->ResourceBarrier(1, &l_geometryBarrier);
        m_rayTracingGeometryStates[l_geometryResource] =
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }

    const UINT l_outputOffset = getOrCreateOutputUAV(
        f_outputImage->getResource(),
        image::ImageDX::convertToDXFormat(f_outputImage->getFormat())
    );

    f_commandList->SetComputeRootSignature(m_rootSignature.Get());
    f_commandList->SetComputeRootDescriptorTable(
        0, l_heap->getGPUHandle(l_cbvOffsets[0])
    );
    f_commandList->SetComputeRootDescriptorTable(
        1, l_heap->getGPUHandle(l_cbvOffsets[1])
    );
    f_commandList->SetComputeRootDescriptorTable(
        2, l_heap->getGPUHandle(m_rayTracingGeometrySrv)
    );
    f_commandList->SetComputeRootDescriptorTable(
        3, l_heap->getGPUHandle(l_accelerationStructure->getHeapOffset())
    );
    for (UINT i = 0; i < m_rayTracingTextureSrvRanges.size(); ++i) {
        f_commandList->SetComputeRootDescriptorTable(
            4 + i, l_heap->getGPUHandle(m_rayTracingTextureSrvRanges[i])
        );
        f_commandList->SetComputeRootDescriptorTable(
            7 + i, l_samplerHeap->getGPUHandle(m_rayTracingSamplerRanges[i])
        );
    }
    f_commandList->SetComputeRootDescriptorTable(
        10, l_heap->getGPUHandle(l_outputOffset)
    );
}

UINT MaterialDX::getOrCreateOutputUAV(ID3D12Resource* f_resource, DXGI_FORMAT f_format)
{
    auto l_existing = m_rayTracingOutputUavs.find(f_resource);
    if (l_existing != m_rayTracingOutputUavs.end()) {
        return l_existing->second;
    }

    D3D12_UNORDERED_ACCESS_VIEW_DESC l_uavDesc = {};
    l_uavDesc.Format                           = f_format;
    l_uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_TEXTURE2D;
    m_rayTracingOutputUavs[f_resource] =
        m_parentDevice->getCommonDescriptorHeapManager()->addUAV(f_resource, l_uavDesc);
    return m_rayTracingOutputUavs.at(f_resource);
}

MaterialDX::~MaterialDX()
{
    if (!m_parentDevice || !isOriginal()) {
        return;
    }
    auto* l_heap        = m_parentDevice->getCommonDescriptorHeapManager();
    auto* l_samplerHeap = m_parentDevice->getCommonSamplerHeapManager();
    for (const auto l_range : m_rayTracingTextureSrvRanges) {
        if (l_range != UINT_MAX) {
            l_heap->freeRange(l_range, rayTracingTextureCount);
        }
    }
    for (const auto l_range : m_rayTracingSamplerRanges) {
        if (l_range != UINT_MAX) {
            l_samplerHeap->freeRange(l_range, rayTracingTextureCount);
        }
    }
    if (m_rayTracingGeometrySrv != UINT_MAX) {
        l_heap->free(m_rayTracingGeometrySrv);
    }
    for (const auto& [l_resource, l_offset] : m_rayTracingOutputUavs) {
        (void)l_resource;
        l_heap->free(l_offset);
    }
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
    size_t                             totalTextures = 0;
    for (int i = 0; i < m_uniformCollections.size(); i++) {
        totalTextures += m_uniformCollections[i]->getTextureCount();
    }

    std::vector<D3D12_DESCRIPTOR_RANGE1> l_cbvRanges(m_uniformCollections.size());
    std::vector<D3D12_DESCRIPTOR_RANGE1> l_uavRanges(m_uniformStorageBuffers.size());
    std::vector<D3D12_DESCRIPTOR_RANGE1> l_srvRanges(totalTextures);
    std::vector<D3D12_DESCRIPTOR_RANGE1> l_samplerRanges(totalTextures);

    // CBV ranges
    for (int i = 0; i < m_uniformCollections.size(); i++) {
        l_cbvRanges[i].BaseShaderRegister = i;
        l_cbvRanges[i].RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        l_cbvRanges[i].NumDescriptors     = 1;
        l_cbvRanges[i].RegisterSpace      = 0;
        l_cbvRanges[i].OffsetInDescriptorsFromTableStart = 0;
        l_cbvRanges[i].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

        D3D12_ROOT_PARAMETER1 l_rootParameter{};
        l_rootParameter.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        l_rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        l_rootParameter.DescriptorTable.NumDescriptorRanges = 1;
        l_rootParameter.DescriptorTable.pDescriptorRanges   = &l_cbvRanges[i];
        l_rootParameters.push_back(l_rootParameter);
    }

    // UAV ranges
    for (int i = 0; i < m_uniformStorageBuffers.size(); i++) {
        l_uavRanges[i].BaseShaderRegister = i;
        l_uavRanges[i].RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        l_uavRanges[i].NumDescriptors     = 1;
        l_uavRanges[i].RegisterSpace      = 0;
        l_uavRanges[i].OffsetInDescriptorsFromTableStart = 0;
        l_uavRanges[i].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

        D3D12_ROOT_PARAMETER1 l_rootParameter{};
        l_rootParameter.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        l_rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        l_rootParameter.DescriptorTable.NumDescriptorRanges = 1;
        l_rootParameter.DescriptorTable.pDescriptorRanges   = &l_uavRanges[i];
        l_rootParameters.push_back(l_rootParameter);
    }

    // Texture SRV ranges
    int l_srvIndex             = 0;
    int l_currentSrvRangeIndex = 0;
    for (int i = 0; i < m_uniformCollections.size(); i++) {
        for (int j = 0; j < static_cast<UINT>(m_uniformCollections[i]->getTextureCount());
             j++)
        {
            l_srvRanges[l_currentSrvRangeIndex].BaseShaderRegister = l_srvIndex++;
            l_srvRanges[l_currentSrvRangeIndex].RangeType =
                D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            l_srvRanges[l_currentSrvRangeIndex].NumDescriptors = 1;
            l_srvRanges[l_currentSrvRangeIndex].RegisterSpace  = 0;
            l_srvRanges[l_currentSrvRangeIndex].OffsetInDescriptorsFromTableStart =
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            l_srvRanges[l_currentSrvRangeIndex].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

            D3D12_ROOT_PARAMETER1 l_rootParameter{};
            l_rootParameter.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            l_rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            l_rootParameter.DescriptorTable.NumDescriptorRanges = 1;
            l_rootParameter.DescriptorTable.pDescriptorRanges =
                &l_srvRanges[l_currentSrvRangeIndex];
            l_rootParameters.push_back(l_rootParameter);
            l_currentSrvRangeIndex++;
        }
    }

    int l_samplerIndex             = 0;
    int l_currentSamplerRangeIndex = 0;
    for (int i = 0; i < m_uniformCollections.size(); i++) {
        for (int j = 0; j < static_cast<UINT>(m_uniformCollections[i]->getTextureCount());
             j++)
        {
            l_samplerRanges[l_currentSamplerRangeIndex].RangeType =
                D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            l_samplerRanges[l_currentSamplerRangeIndex].BaseShaderRegister =
                l_samplerIndex++;
            l_samplerRanges[l_currentSamplerRangeIndex].NumDescriptors = 1;
            l_samplerRanges[l_currentSamplerRangeIndex].RegisterSpace  = 0;
            l_samplerRanges[l_currentSamplerRangeIndex].OffsetInDescriptorsFromTableStart =
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


            D3D12_ROOT_PARAMETER1 l_rootParameterSampler{};
            l_rootParameterSampler.ParameterType =
                D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            l_rootParameterSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            l_rootParameterSampler.DescriptorTable.NumDescriptorRanges = 1;
            l_rootParameterSampler.DescriptorTable.pDescriptorRanges =
                &l_samplerRanges[l_currentSamplerRangeIndex];
            l_rootParameters.push_back(l_rootParameterSampler);
            l_currentSamplerRangeIndex++;
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

void MaterialDX::createRayTracingRootSignature()
{
    constexpr UINT l_rootParameterCount = 11;
    std::array<D3D12_DESCRIPTOR_RANGE, l_rootParameterCount> l_ranges{};
    std::array<D3D12_ROOT_PARAMETER, l_rootParameterCount>   l_parameters{};

    const auto setTable = [&](UINT f_index,
                              D3D12_DESCRIPTOR_RANGE_TYPE f_type,
                              UINT f_register,
                              UINT f_space,
                              UINT f_count) {
        l_ranges[f_index].RangeType                         = f_type;
        l_ranges[f_index].NumDescriptors                    = f_count;
        l_ranges[f_index].BaseShaderRegister                = f_register;
        l_ranges[f_index].RegisterSpace                     = f_space;
        l_ranges[f_index].OffsetInDescriptorsFromTableStart =
            D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        l_parameters[f_index].ParameterType =
            D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        l_parameters[f_index].DescriptorTable.NumDescriptorRanges = 1;
        l_parameters[f_index].DescriptorTable.pDescriptorRanges = &l_ranges[f_index];
        l_parameters[f_index].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    };

    setTable(0, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 0, 1);
    setTable(1, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 1);
    setTable(2, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2, 1);
    setTable(3, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 3, 1);
    setTable(4, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 4, rayTracingTextureCount);
    setTable(5, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 5, rayTracingTextureCount);
    setTable(6, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, rayTracingTextureCount);
    setTable(7, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 4, rayTracingTextureCount);
    setTable(8, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 5, rayTracingTextureCount);
    setTable(9, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 6, rayTracingTextureCount);
    setTable(10, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 7, 1);

    D3D12_ROOT_SIGNATURE_DESC l_rootSignatureDesc = {};
    l_rootSignatureDesc.NumParameters = l_rootParameterCount;
    l_rootSignatureDesc.pParameters   = l_parameters.data();
    l_rootSignatureDesc.Flags         = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    Microsoft::WRL::ComPtr<ID3DBlob> l_signature;
    Microsoft::WRL::ComPtr<ID3DBlob> l_error;
    if (FAILED(D3D12SerializeRootSignature(
            &l_rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &l_signature,
            &l_error
        )))
    {
        throw std::runtime_error(
            "MaterialDX::createRayTracingRootSignature() failed to serialize root "
            "signature"
        );
    }
    if (FAILED(m_parentDevice->getDevice()->CreateRootSignature(
            0,
            l_signature->GetBufferPointer(),
            l_signature->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature)
        )))
    {
        throw std::runtime_error(
            "MaterialDX::createRayTracingRootSignature() failed to create root "
            "signature"
        );
    }
}

void MaterialDX::createRayTracingDescriptors()
{
    if (m_uniformCollections.size() != 2 || m_uniformStorageBuffers.size() != 1
        || m_uniformAccelerationStructures.size() != 1)
    {
        throw std::runtime_error(
            "MaterialDX::createRayTracingDescriptors() requires two constant buffers, "
            "one geometry buffer, and one acceleration structure"
        );
    }

    const std::array<const std::vector<std::shared_ptr<texture::Texture>>*, 3>
        l_textureLists = {
            &m_rayTracingAlbedoTextures,
            &m_rayTracingNormalTextures,
            &m_rayTracingMetallicRoughnessTextures
        };
    const size_t l_textureCount = l_textureLists.front()->size();
    if (l_textureCount == 0 || l_textureCount > rayTracingTextureCount) {
        throw std::runtime_error(
            "MaterialDX::createRayTracingDescriptors() requires between 1 and 128 "
            "textures"
        );
    }
    for (const auto* l_list : l_textureLists) {
        if (l_list->size() != l_textureCount) {
            throw std::runtime_error(
                "MaterialDX::createRayTracingDescriptors() texture arrays are "
                "inconsistent"
            );
        }
    }

    auto* l_resourceHeap = m_parentDevice->getCommonDescriptorHeapManager();
    auto* l_samplerHeap = m_parentDevice->getCommonSamplerHeapManager();
    for (UINT l_array = 0; l_array < l_textureLists.size(); ++l_array) {
        m_rayTracingTextureSrvRanges[l_array] =
            l_resourceHeap->allocateRange(rayTracingTextureCount);
        m_rayTracingSamplerRanges[l_array] =
            l_samplerHeap->allocateRange(rayTracingTextureCount);

        for (UINT i = 0; i < rayTracingTextureCount; ++i) {
            const auto& l_texture =
                (*l_textureLists[l_array])[std::min<size_t>(i, l_textureCount - 1)];
            auto* l_textureDX = dynamic_cast<texture::TextureDX*>(l_texture.get());
            if (!l_textureDX || !l_textureDX->isValid()) {
                throw std::runtime_error(
                    "MaterialDX::createRayTracingDescriptors() requires valid DirectX "
                    "textures"
                );
            }
            l_textureDX->writeDescriptors(
                l_resourceHeap->getCPUHandle(m_rayTracingTextureSrvRanges[l_array] + i),
                l_samplerHeap->getCPUHandle(m_rayTracingSamplerRanges[l_array] + i)
            );
        }
    }

    auto* l_storageBuffer = dynamic_cast<uniform::UniformStorageBufferDX*>(
        m_uniformStorageBuffers.front().get()
    );
    if (!l_storageBuffer || !l_storageBuffer->getBuffer()) {
        throw std::runtime_error(
            "MaterialDX::createRayTracingDescriptors() requires a created DirectX "
            "geometry buffer"
        );
    }
    D3D12_SHADER_RESOURCE_VIEW_DESC l_geometrySrv = {};
    l_geometrySrv.Format                     = DXGI_FORMAT_R32_TYPELESS;
    l_geometrySrv.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    l_geometrySrv.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    l_geometrySrv.Buffer.FirstElement        = 0;
    l_geometrySrv.Buffer.NumElements         =
        static_cast<UINT>((l_storageBuffer->getSize() + sizeof(uint32_t) - 1)
                          / sizeof(uint32_t));
    l_geometrySrv.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
    m_rayTracingGeometrySrv = l_resourceHeap->addSRV(
        l_storageBuffer->getBuffer()->GetResource(), l_geometrySrv
    );

    auto* l_accelerationStructure =
        dynamic_cast<uniform::UniformAccelerationStructureDX*>(
            m_uniformAccelerationStructures.front().get()
        );
    if (!l_accelerationStructure || !l_accelerationStructure->isValid()) {
        throw std::runtime_error(
            "MaterialDX::createRayTracingDescriptors() requires a valid DirectX "
            "acceleration structure"
        );
    }
}

void MaterialDX::createRayTracingPipelineState()
{
    const auto l_libraryData = utils::readFile(m_shaderLocation + ".dxil");
    D3D12_DXIL_LIBRARY_DESC l_libraryDesc = {};
    l_libraryDesc.DXILLibrary = {
        l_libraryData.data(),
        l_libraryData.size()
    };
    const std::array<D3D12_EXPORT_DESC, 3> l_exports = {
        D3D12_EXPORT_DESC{ L"raygenMain", nullptr, D3D12_EXPORT_FLAG_NONE },
        D3D12_EXPORT_DESC{ L"missMain", nullptr, D3D12_EXPORT_FLAG_NONE },
        D3D12_EXPORT_DESC{ L"closestHitMain", nullptr, D3D12_EXPORT_FLAG_NONE }
    };
    l_libraryDesc.NumExports = static_cast<UINT>(l_exports.size());
    l_libraryDesc.pExports   = l_exports.data();

    D3D12_HIT_GROUP_DESC l_hitGroup = {};
    l_hitGroup.HitGroupExport      = L"HitGroup";
    l_hitGroup.Type                = D3D12_HIT_GROUP_TYPE_TRIANGLES;
    l_hitGroup.ClosestHitShaderImport = L"closestHitMain";

    D3D12_RAYTRACING_SHADER_CONFIG l_shaderConfig = {
        .MaxPayloadSizeInBytes   = 64,
        .MaxAttributeSizeInBytes = 8
    };
    D3D12_GLOBAL_ROOT_SIGNATURE l_globalRootSignature = {
        .pGlobalRootSignature = m_rootSignature.Get()
    };
    D3D12_RAYTRACING_PIPELINE_CONFIG l_pipelineConfig = {
        .MaxTraceRecursionDepth = 4
    };
    const std::array<D3D12_STATE_SUBOBJECT, 5> l_subobjects = {
        D3D12_STATE_SUBOBJECT{
            D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &l_libraryDesc
        },
        D3D12_STATE_SUBOBJECT{
            D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &l_hitGroup
        },
        D3D12_STATE_SUBOBJECT{
            D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &l_shaderConfig
        },
        D3D12_STATE_SUBOBJECT{
            D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &l_pipelineConfig
        },
        D3D12_STATE_SUBOBJECT{
            D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &l_globalRootSignature
        }
    };
    D3D12_STATE_OBJECT_DESC l_stateObjectDesc = {};
    l_stateObjectDesc.Type                  = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    l_stateObjectDesc.NumSubobjects          = static_cast<UINT>(l_subobjects.size());
    l_stateObjectDesc.pSubobjects            = l_subobjects.data();

    Microsoft::WRL::ComPtr<ID3D12Device5> l_device;
    if (FAILED(m_parentDevice->getDevice().As(&l_device))
        || FAILED(l_device->CreateStateObject(
            &l_stateObjectDesc, IID_PPV_ARGS(&m_rayTracingStateObject)
        )))
    {
        throw std::runtime_error(
            "MaterialDX::createRayTracingPipelineState() failed to create DXR pipeline"
        );
    }

    Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> l_properties;
    if (FAILED(m_rayTracingStateObject.As(&l_properties))) {
        throw std::runtime_error(
            "MaterialDX::createRayTracingPipelineState() failed to access shader "
            "identifiers"
        );
    }
    const void* l_raygenIdentifier = l_properties->GetShaderIdentifier(L"raygenMain");
    const void* l_missIdentifier = l_properties->GetShaderIdentifier(L"missMain");
    const void* l_hitGroupIdentifier = l_properties->GetShaderIdentifier(L"HitGroup");
    if (!l_raygenIdentifier || !l_missIdentifier || !l_hitGroupIdentifier) {
        throw std::runtime_error(
            "MaterialDX::createRayTracingPipelineState() failed to find shader exports"
        );
    }

    constexpr UINT l_recordCount = 3;
    constexpr UINT l_tableStride = rayTracingRecordAlignment;
    constexpr UINT l_tableSize   = l_recordCount * l_tableStride;
    D3D12_RESOURCE_DESC l_tableDesc = {};
    l_tableDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    l_tableDesc.Width               = l_tableSize;
    l_tableDesc.Height              = 1;
    l_tableDesc.DepthOrArraySize    = 1;
    l_tableDesc.MipLevels           = 1;
    l_tableDesc.SampleDesc.Count    = 1;
    l_tableDesc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12MA::ALLOCATION_DESC l_allocationDesc = {};
    l_allocationDesc.HeapType                 = D3D12_HEAP_TYPE_UPLOAD;
    if (FAILED(m_parentDevice->getMemoryAllocator()->CreateResource(
            &l_allocationDesc,
            &l_tableDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            &m_shaderTable,
            IID_NULL,
            nullptr
        )))
    {
        throw std::runtime_error(
            "MaterialDX::createRayTracingPipelineState() failed to create shader table"
        );
    }

    std::byte* l_mappedData = nullptr;
    D3D12_RANGE l_readRange = { 0, 0 };
    if (FAILED(m_shaderTable->GetResource()->Map(
            0, &l_readRange, reinterpret_cast<void**>(&l_mappedData)
        )))
    {
        throw std::runtime_error(
            "MaterialDX::createRayTracingPipelineState() failed to map shader table"
        );
    }
    std::memcpy(l_mappedData, l_raygenIdentifier, rayTracingRecordStride);
    std::memcpy(
        l_mappedData + l_tableStride, l_missIdentifier, rayTracingRecordStride
    );
    std::memcpy(
        l_mappedData + 2 * l_tableStride, l_hitGroupIdentifier,
        rayTracingRecordStride
    );
    m_shaderTable->GetResource()->Unmap(0, nullptr);

    const D3D12_GPU_VIRTUAL_ADDRESS l_tableAddress =
        m_shaderTable->GetResource()->GetGPUVirtualAddress();
    m_raygenShaderRecord    = l_tableAddress;
    m_missShaderRecord      = l_tableAddress + l_tableStride;
    m_hitGroupShaderRecord  = l_tableAddress + 2 * l_tableStride;
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

void MaterialDX::createComputePipelineState()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC l_psoDesc = {};

    l_psoDesc.pRootSignature = m_rootSignature.Get();

    D3D12_SHADER_BYTECODE l_csBytecode;
    std::vector<char> l_csBytecodeData = utils::readFile(m_shaderLocation + ".comp.dxil");
    l_csBytecode.pShaderBytecode       = l_csBytecodeData.data();
    l_csBytecode.BytecodeLength        = l_csBytecodeData.size();
    l_psoDesc.CS                       = l_csBytecode;


    if (FAILED(m_parentDevice->getDevice()->CreateComputePipelineState(
            &l_psoDesc, IID_PPV_ARGS(&m_pipelineState)
        )))
    {
        throw std::runtime_error(
            "MaterialDX::createComputePipelineState() failed to create pipeline"
        );
    }
}

std::array<D3D12_INPUT_ELEMENT_DESC, 5> MaterialDX::getInputElementDescription()
{
    std::array<D3D12_INPUT_ELEMENT_DESC, 5> l_desc = {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION",
                                 0,    DXGI_FORMAT_R32G32B32_FLOAT,
                                 0, offsetof(mesh::Vertex, m_position),
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        D3D12_INPUT_ELEMENT_DESC{   "NORMAL",
                                 0,    DXGI_FORMAT_R32G32B32_FLOAT,
                                 0, offsetof(mesh::Vertex,   m_normal),
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        D3D12_INPUT_ELEMENT_DESC{  "TANGENT",
                                 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
                                 0, offsetof(mesh::Vertex,  m_tangent),
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD",
                                 0,       DXGI_FORMAT_R32G32_FLOAT,
                                 0, offsetof(mesh::Vertex, m_texCoord),
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        D3D12_INPUT_ELEMENT_DESC{    "COLOR",
                                 0,    DXGI_FORMAT_R32G32B32_FLOAT,
                                 0, offsetof(mesh::Vertex,    m_color),
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    return l_desc;
}

}   // namespace material
}   // namespace renderer
