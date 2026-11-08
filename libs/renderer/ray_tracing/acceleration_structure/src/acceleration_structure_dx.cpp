#include "renderer/ray_tracing/acceleration_structure/inc/acceleration_structure_dx.hpp"

#include <cstring>
#include <stdexcept>
#include <unordered_map>

#include <glm/glm.hpp>

#include "renderer/command_buffer/inc/command_buffer_dx.hpp"
#include "renderer/mesh/inc/triangle_mesh_dx.hpp"
#include "renderer/rendering_device/inc/rendering_device_dx.hpp"
#include "renderer/scene/node/inc/node.hpp"

namespace renderer {
namespace ray_tracing {
namespace {
Microsoft::WRL::ComPtr<D3D12MA::Allocation> createBuffer(
    rendering_device::RenderingDeviceDX* f_device,
    UINT64                              f_size,
    D3D12_HEAP_TYPE                     f_heapType,
    D3D12_RESOURCE_FLAGS                f_flags,
    D3D12_RESOURCE_STATES               f_initialState
)
{
    D3D12_RESOURCE_DESC l_resourceDesc = {};
    l_resourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    l_resourceDesc.Width               = f_size;
    l_resourceDesc.Height              = 1;
    l_resourceDesc.DepthOrArraySize    = 1;
    l_resourceDesc.MipLevels           = 1;
    l_resourceDesc.Format               = DXGI_FORMAT_UNKNOWN;
    l_resourceDesc.SampleDesc.Count     = 1;
    l_resourceDesc.Layout               = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    l_resourceDesc.Flags                = f_flags;

    D3D12MA::ALLOCATION_DESC l_allocationDesc = {};
    l_allocationDesc.HeapType                 = f_heapType;

    Microsoft::WRL::ComPtr<D3D12MA::Allocation> l_allocation;
    if (FAILED(f_device->getMemoryAllocator()->CreateResource(
            &l_allocationDesc,
            &l_resourceDesc,
            f_initialState,
            nullptr,
            &l_allocation,
            IID_NULL,
            nullptr
        )))
    {
        throw std::runtime_error(
            "AccelerationStructureDX failed to create an acceleration-structure buffer"
        );
    }
    return l_allocation;
}
}   // namespace

AccelerationStructureDX::AccelerationStructureDX(
    rendering_device::RenderingDeviceDX* f_parentDevice
)
    : m_parentDevice(f_parentDevice)
{}

AccelerationStructureDX& AccelerationStructureDX::setScene(scene::Node* f_rootNode)
{
    m_rootNode = f_rootNode;
    return *this;
}

AccelerationStructureDX& AccelerationStructureDX::create()
{
    if (!m_parentDevice || !m_parentDevice->isValid()) {
        throw std::runtime_error(
            "AccelerationStructureDX::create() requires a valid DirectX device"
        );
    }
    if (!m_rootNode) {
        throw std::runtime_error(
            "AccelerationStructureDX::create() requires a scene root node"
        );
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 l_options = {};
    if (FAILED(m_parentDevice->getDevice()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS5, &l_options, sizeof(l_options)
        ))
        || l_options.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
    {
        throw std::runtime_error(
            "AccelerationStructureDX::create() requires DirectX Raytracing support"
        );
    }

    scene::RayTracingVisitor l_visitor;
    m_rootNode->applyVisitor(&l_visitor);
    const auto& l_meshInstances = l_visitor.getMeshInstances();
    if (l_meshInstances.empty()) {
        throw std::runtime_error(
            "AccelerationStructureDX::create() found no triangle mesh instances"
        );
    }

    buildBottomLevelAccelerationStructures(l_meshInstances);
    buildTopLevelAccelerationStructure(l_meshInstances);
    m_valid = true;
    return *this;
}

ID3D12Resource* AccelerationStructureDX::getAccelerationStructure() const
{
    return m_accelerationStructure
         ? m_accelerationStructure->GetResource()
         : nullptr;
}

void AccelerationStructureDX::buildBottomLevelAccelerationStructures(
    const std::vector<scene::RayTracingVisitor::MeshInstance>& f_meshInstances
)
{
    std::unordered_map<mesh::TriangleMesh*, uint32_t> l_meshIndices;
    std::vector<std::shared_ptr<mesh::TriangleMesh>> l_meshes;
    for (const auto& l_instance : f_meshInstances) {
        if (!l_meshIndices.contains(l_instance.m_mesh.get())) {
            l_meshIndices.emplace(
                l_instance.m_mesh.get(), static_cast<uint32_t>(l_meshes.size())
            );
            l_meshes.push_back(l_instance.m_mesh);
        }
    }

    Microsoft::WRL::ComPtr<ID3D12Device5> l_device;
    if (FAILED(m_parentDevice->getDevice().As(&l_device))) {
        throw std::runtime_error(
            "AccelerationStructureDX::create() failed to access the DXR device"
        );
    }

    m_bottomLevelAccelerationStructures.clear();
    m_bottomLevelScratchBuffers.clear();
    m_bottomLevelAccelerationStructures.reserve(l_meshes.size());
    m_bottomLevelScratchBuffers.reserve(l_meshes.size());

    std::vector<D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC> l_buildDescs;
    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> l_geometries;
    l_buildDescs.reserve(l_meshes.size());
    l_geometries.reserve(l_meshes.size());

    for (const auto& l_mesh : l_meshes) {
        auto* l_meshDX = dynamic_cast<mesh::TriangleMeshDX*>(l_mesh.get());
        if (!l_meshDX || l_mesh->getIndicieCount() < 3
            || l_mesh->getIndicieCount() % 3 != 0 || l_mesh->getVertexCount() == 0)
        {
            throw std::runtime_error(
                "AccelerationStructureDX::create() requires indexed DirectX triangle "
                "meshes"
            );
        }

        l_geometries.emplace_back();
        auto& l_geometry = l_geometries.back();
        l_geometry.Type   = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        l_geometry.Flags  = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        l_geometry.Triangles.Transform3x4 = 0;
        l_geometry.Triangles.IndexFormat  = DXGI_FORMAT_R32_UINT;
        l_geometry.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        l_geometry.Triangles.IndexCount =
            static_cast<UINT>(l_mesh->getIndicieCount());
        l_geometry.Triangles.VertexCount =
            static_cast<UINT>(l_mesh->getVertexCount());
        l_geometry.Triangles.IndexBuffer =
            l_meshDX->getIndexBufferView().BufferLocation;
        l_geometry.Triangles.VertexBuffer.StartAddress =
            l_meshDX->getVertexBufferView().BufferLocation;
        l_geometry.Triangles.VertexBuffer.StrideInBytes =
            l_meshDX->getVertexBufferView().StrideInBytes;

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS l_inputs = {};
        l_inputs.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        l_inputs.Flags          = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        l_inputs.NumDescs       = 1;
        l_inputs.DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY;
        l_inputs.pGeometryDescs = &l_geometry;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO l_prebuildInfo = {};
        l_device->GetRaytracingAccelerationStructurePrebuildInfo(
            &l_inputs, &l_prebuildInfo
        );
        if (l_prebuildInfo.ResultDataMaxSizeInBytes == 0
            || l_prebuildInfo.ScratchDataSizeInBytes == 0)
        {
            throw std::runtime_error(
                "AccelerationStructureDX::create() received invalid BLAS build sizes"
            );
        }

        m_bottomLevelAccelerationStructures.push_back(createBuffer(
            m_parentDevice,
            l_prebuildInfo.ResultDataMaxSizeInBytes,
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE
        ));
        m_bottomLevelScratchBuffers.push_back(createBuffer(
            m_parentDevice,
            l_prebuildInfo.ScratchDataSizeInBytes,
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS
        ));

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC l_buildDesc = {};
        l_buildDesc.Inputs                   = l_inputs;
        l_buildDesc.DestAccelerationStructureData =
            m_bottomLevelAccelerationStructures.back()->GetResource()
                ->GetGPUVirtualAddress();
        l_buildDesc.ScratchAccelerationStructureData =
            m_bottomLevelScratchBuffers.back()->GetResource()->GetGPUVirtualAddress();
        l_buildDescs.push_back(l_buildDesc);
    }

    auto l_commandBuffer = m_parentDevice->createCommandBuffer();
    auto* l_commandBufferDX =
        dynamic_cast<command_buffer::CommandBufferDX*>(l_commandBuffer.get());
    if (!l_commandBufferDX) {
        throw std::runtime_error(
            "AccelerationStructureDX::create() failed to create a DirectX command buffer"
        );
    }
    l_commandBufferDX->reset().begin();

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> l_commandList;
    if (FAILED(l_commandBufferDX->getCommandList()->QueryInterface(
            IID_PPV_ARGS(&l_commandList)
        )))
    {
        throw std::runtime_error(
            "AccelerationStructureDX::create() failed to access a DXR command list"
        );
    }

    for (const auto& l_buildDesc : l_buildDescs) {
        l_commandList->BuildRaytracingAccelerationStructure(&l_buildDesc, 0, nullptr);
    }

    std::vector<D3D12_RESOURCE_BARRIER> l_barriers;
    l_barriers.reserve(m_bottomLevelAccelerationStructures.size());
    for (const auto& l_blas : m_bottomLevelAccelerationStructures) {
        D3D12_RESOURCE_BARRIER l_barrier = {};
        l_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        l_barrier.UAV.pResource          = l_blas->GetResource();
        l_barriers.push_back(l_barrier);
    }
    l_commandList->ResourceBarrier(
        static_cast<UINT>(l_barriers.size()), l_barriers.data()
    );

    l_commandBufferDX->end().submit();
    m_bottomLevelScratchBuffers.clear();
}

void AccelerationStructureDX::buildTopLevelAccelerationStructure(
    const std::vector<scene::RayTracingVisitor::MeshInstance>& f_meshInstances
)
{
    std::unordered_map<mesh::TriangleMesh*, uint32_t> l_meshIndices;
    for (const auto& l_instance : f_meshInstances) {
        if (!l_meshIndices.contains(l_instance.m_mesh.get())) {
            l_meshIndices.emplace(
                l_instance.m_mesh.get(), static_cast<uint32_t>(l_meshIndices.size())
            );
        }
    }

    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> l_instances(f_meshInstances.size());
    for (size_t i = 0; i < f_meshInstances.size(); ++i) {
        const auto& l_instance = f_meshInstances[i];
        auto&       l_dxInstance = l_instances[i];
        std::memset(&l_dxInstance, 0, sizeof(l_dxInstance));
        const glm::mat4& l_transform = l_instance.m_transform;
        for (uint32_t row = 0; row < 3; ++row) {
            for (uint32_t column = 0; column < 4; ++column) {
                l_dxInstance.Transform[row][column] = l_transform[column][row];
            }
        }
        l_dxInstance.InstanceID =
            (l_instance.m_meshId << 12) | (l_instance.m_materialId & 0xFFFu);
        l_dxInstance.InstanceMask = 0xFF;
        l_dxInstance.InstanceContributionToHitGroupIndex = 0;
        l_dxInstance.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;
        l_dxInstance.AccelerationStructure =
            m_bottomLevelAccelerationStructures[l_meshIndices.at(l_instance.m_mesh.get())]
                ->GetResource()
                ->GetGPUVirtualAddress();
    }

    m_instanceBuffer = createBuffer(
        m_parentDevice,
        sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * l_instances.size(),
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ
    );
    void*       l_mappedData = nullptr;
    D3D12_RANGE l_readRange  = { 0, 0 };
    if (FAILED(m_instanceBuffer->GetResource()->Map(
            0, &l_readRange, &l_mappedData
        )))
    {
        throw std::runtime_error(
            "AccelerationStructureDX::create() failed to map the TLAS instance buffer"
        );
    }
    std::memcpy(
        l_mappedData, l_instances.data(),
        sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * l_instances.size()
    );
    m_instanceBuffer->GetResource()->Unmap(0, nullptr);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS l_inputs = {};
    l_inputs.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    l_inputs.Flags          = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    l_inputs.NumDescs       = static_cast<UINT>(l_instances.size());
    l_inputs.DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY;
    l_inputs.InstanceDescs  =
        m_instanceBuffer->GetResource()->GetGPUVirtualAddress();

    Microsoft::WRL::ComPtr<ID3D12Device5> l_device;
    if (FAILED(m_parentDevice->getDevice().As(&l_device))) {
        throw std::runtime_error(
            "AccelerationStructureDX::create() failed to access the DXR device"
        );
    }
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO l_prebuildInfo = {};
    l_device->GetRaytracingAccelerationStructurePrebuildInfo(&l_inputs, &l_prebuildInfo);
    if (l_prebuildInfo.ResultDataMaxSizeInBytes == 0
        || l_prebuildInfo.ScratchDataSizeInBytes == 0)
    {
        throw std::runtime_error(
            "AccelerationStructureDX::create() received invalid TLAS build sizes"
        );
    }

    m_accelerationStructure = createBuffer(
        m_parentDevice,
        l_prebuildInfo.ResultDataMaxSizeInBytes,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE
    );
    m_scratchBuffer = createBuffer(
        m_parentDevice,
        l_prebuildInfo.ScratchDataSizeInBytes,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    );
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC l_buildDesc = {};
    l_buildDesc.Inputs                   = l_inputs;
    l_buildDesc.DestAccelerationStructureData =
        m_accelerationStructure->GetResource()->GetGPUVirtualAddress();
    l_buildDesc.ScratchAccelerationStructureData =
        m_scratchBuffer->GetResource()->GetGPUVirtualAddress();

    auto l_commandBuffer = m_parentDevice->createCommandBuffer();
    auto* l_commandBufferDX =
        dynamic_cast<command_buffer::CommandBufferDX*>(l_commandBuffer.get());
    if (!l_commandBufferDX) {
        throw std::runtime_error(
            "AccelerationStructureDX::create() failed to create a DirectX command buffer"
        );
    }
    l_commandBufferDX->reset().begin();

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> l_commandList;
    if (FAILED(l_commandBufferDX->getCommandList()->QueryInterface(
            IID_PPV_ARGS(&l_commandList)
        )))
    {
        throw std::runtime_error(
            "AccelerationStructureDX::create() failed to access a DXR command list"
        );
    }
    l_commandList->BuildRaytracingAccelerationStructure(&l_buildDesc, 0, nullptr);
    l_commandBufferDX->end().submit();
    m_instanceBuffer.Reset();
    m_scratchBuffer.Reset();
}
}   // namespace ray_tracing
}   // namespace renderer
