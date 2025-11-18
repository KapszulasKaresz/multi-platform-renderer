#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

#include <stdexcept>

#include <imgui_impl_dx12.h>
#include <imgui_impl_glfw.h>

#include "renderer/command_buffer/inc/command_buffer_dx.hpp"
#include "renderer/image/inc/image_dx.hpp"
#include "renderer/material/inc/material_dx.hpp"
#include "renderer/mesh/inc/triangle_mesh_dx.hpp"
#include "renderer/render_target/inc/render_target_dx.hpp"
#include "renderer/render_target/inc/render_target_window_dx.hpp"
#include "renderer/rendering_api/inc/rendering_api_dx.hpp"
#include "renderer/rendering_server/inc/rendering_server.hpp"
#include "renderer/texture/inc/texture_dx.hpp"
#include "renderer/uniform/inc/uniform_collection_dx.hpp"
#include "renderer/window/inc/glfw_window.hpp"
#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace rendering_device {

RenderingDeviceDX::RenderingDeviceDX(rendering_api::RenderingApiDX* f_parentApi)
    : RenderingDevice(),
      m_parentApi(f_parentApi)
{}

std::shared_ptr<render_target::RenderTargetWindow>
    RenderingDeviceDX::getRenderTargetWindow()
{
    if (!isValid()) {
        throw std::runtime_error(
            "RenderingDeviceDX::getRenderTargetWindow() device needs to be valid"
        );
    }

    if (!m_window) {
        throw std::
            runtime_error(
                "RenderingDeviceDX::getRenderTargetWindow() needs a window "
                "attached to the device"
            );
    }

    if (!m_renderTargetWindow) {
        throw std::
            runtime_error(
                "RenderingDeviceDX::getRenderTargetWindow() no render target window "
                "is available"
            );
    }

    return m_renderTargetWindow;
}

std::shared_ptr<image::Image> RenderingDeviceDX::createImage()
{
    return std::make_shared<image::ImageDX>(this);
}

std::shared_ptr<material::Material> RenderingDeviceDX::createMaterial()
{
    return std::make_shared<material::MaterialDX>(this);
}

std::shared_ptr<command_buffer::CommandBuffer> RenderingDeviceDX::createCommandBuffer()
{
    return getRenderingCommandBuffer();
}

std::shared_ptr<command_buffer::CommandBuffer>
    RenderingDeviceDX::getRenderingCommandBuffer()
{
    if (!m_renderingCommandBuffer) {
        m_renderingCommandBuffer = std::make_shared<command_buffer::CommandBufferDX>(this);
        m_renderingCommandBuffer->setRendering(true).create();
    }
    return m_renderingCommandBuffer;
}

std::shared_ptr<mesh::TriangleMesh> RenderingDeviceDX::createTriangleMesh()
{
    return std::make_shared<mesh::TriangleMeshDX>(this);
}

std::shared_ptr<uniform::UniformCollection> RenderingDeviceDX::createUniformCollection()
{
    return std::make_shared<uniform::UniformCollectionDX>(this);
}

std::shared_ptr<texture::Texture> RenderingDeviceDX::createTexture()
{
    return std::make_shared<texture::TextureDX>(this);
}

std::shared_ptr<render_target::RenderTarget> RenderingDeviceDX::createRenderTarget()
{
    return std::make_shared<render_target::RenderTargetDX>(this);
}

bool RenderingDeviceDX::preFrame()
{
    waitForGPU();
    m_frameIndex   = m_renderTargetWindow->getSwapchain()->GetCurrentBackBufferIndex();
    m_currentFrame = m_renderTargetWindow->getSwapchain()->GetCurrentBackBufferIndex();

    if (m_useImGui) {
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplDX12_NewFrame();
        ImGui::NewFrame();
    }
    return true;
}

void RenderingDeviceDX::postFrame()
{
    m_renderTargetWindow->getSwapchain()->Present(1, 0);
    if (m_window->isResized()) {
        waitForGPU();
        m_window->resizeHandled();
        m_renderTargetWindow->resizeSwapChain();
    }
}

void RenderingDeviceDX::finishRendering()
{
    waitForGPU();
}

RenderingDeviceDX& RenderingDeviceDX::setWindow(window::Window* f_window)
{
    m_window = f_window;
    return *this;
}

RenderingDeviceDX& RenderingDeviceDX::create()
{
    createAdapter();
    createDevice();
    createAllocator();
    createCommandQueue();
    createSyncObjects();
    m_valid = true;
    createRenderTargetWindow();
    createDescriptorHeapManager();
    if (m_useImGui) {
        initImGui();
    }
    return *this;
}

void RenderingDeviceDX::waitForGPU()
{
    if (FAILED(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]))) {
        throw std::runtime_error(
            "RenderingDeviceDX::waitForGPU() Failed to signal command queue"
        );
    }

    if (FAILED(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent)))
    {
        throw std::runtime_error("RenderingDeviceDX::waitForGPU() Failed to set fence");
    }
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    m_fenceValues[m_frameIndex]++;
}

rendering_api::RenderingApiDX* RenderingDeviceDX::getParentApi()
{
    return m_parentApi;
}

ID3D12CommandQueue* RenderingDeviceDX::getCommandQueue()
{
    return m_commandQueue.Get();
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> RenderingDeviceDX::getCommandAllocator(
    int f_index
)
{
    if (f_index < 0) {
        return m_commandAllocators[m_currentFrame];
    }
    return m_commandAllocators[f_index];
}

Microsoft::WRL::ComPtr<ID3D12Device> RenderingDeviceDX::getDevice()
{
    return m_device;
}

D3D12MA::Allocator* RenderingDeviceDX::getMemoryAllocator()
{
    return m_allocator.Get();
}

utils::DescriptorHeapManagerDX* RenderingDeviceDX::getCommonDescriptorHeapManager()
{
    return m_commonDescriptorHeap.get();
}

utils::DescriptorHeapManagerDX* RenderingDeviceDX::getCommonSamplerHeapManager()
{
    return m_commonSamplerHeap.get();
}

void RenderingDeviceDX::executeCommandList(ID3D12GraphicsCommandList* f_commandList)
{
    ID3D12CommandList* l_ppCommandLists[] = { f_commandList };
    m_commandQueue->ExecuteCommandLists(_countof(l_ppCommandLists), l_ppCommandLists);
}

RenderingDeviceDX::~RenderingDeviceDX()
{
    waitForGPU();
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    waitForGPU();
}

void RenderingDeviceDX::createAdapter()
{
    IDXGIAdapter4* l_adapter4 = nullptr;
    for (UINT l_index = 0;
         m_parentApi->getFactory()->EnumAdapterByGpuPreference(
             l_index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&l_adapter4)
         )
         != DXGI_ERROR_NOT_FOUND;
         ++l_index)
    {
        DXGI_ADAPTER_DESC3 l_desc;
        l_adapter4->GetDesc3(&l_desc);

        if (l_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            l_adapter4->Release();
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(
                l_adapter4, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr
            )))
        {
            m_adapter = l_adapter4;
            break;
        }

        l_adapter4->Release();
    }
}

void RenderingDeviceDX::createDevice()
{
    if (FAILED(D3D12CreateDevice(
            m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)
        )))
    {
        throw std::runtime_error(
            "RenderingDeviceDX::createDevice() Failed to create D3D12 device!"
        );
    }

    if (m_parentApi->isDebugEnabled()) {
        if (FAILED(m_device.As(&m_debugDevice))) {
            throw std::runtime_error(
                "RenderingDeviceDX::createDevice() Failed to create D3D12 debug device!"
            );
        }
    }
}

void RenderingDeviceDX::createAllocator()
{
    D3D12MA::ALLOCATOR_DESC l_allocatorDesc = { .Flags =
                                                    D3D12MA_RECOMMENDED_ALLOCATOR_FLAGS,
                                                .pDevice  = m_device.Get(),
                                                .pAdapter = m_adapter.Get() };
    if (FAILED(D3D12MA::CreateAllocator(&l_allocatorDesc, &m_allocator))) {
        throw std::runtime_error(
            "RenderingDeviceDX::createAllocator() failed to create DMA allocator"
        );
    }
}

void RenderingDeviceDX::createCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC l_queueDesc{ .Type  = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                          .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE };
    if (FAILED(m_device->CreateCommandQueue(&l_queueDesc, IID_PPV_ARGS(&m_commandQueue))))
    {
        throw std::runtime_error(
            "RenderingDeviceDX::createCommandQueue() Failed to create command queue!"
        );
    }

    for (UINT n = 0; n < maxFramesInFlight; n++) {
        if (FAILED(m_device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])
            )))
        {
            throw std::runtime_error(
                "RenderingDeviceDX::createCommandQueue() Failed to create command "
                "allocator!"
            );
        }
    }
}

void RenderingDeviceDX::createSyncObjects()
{
    if (FAILED(m_device->CreateFence(
            m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)
        )))
    {
        throw std::runtime_error(
            "RenderingDeviceDX::createSyncObjects() Failed to create fence!"
        );
    }
    m_fenceValues[m_frameIndex]++;

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr) {
        if (FAILED((HRESULT_FROM_WIN32(GetLastError())))) {
            throw std::runtime_error(
                "RenderingDeviceDX::createSyncObjects() Failed to create fence event!"
            );
        }
    }
}

void RenderingDeviceDX::createRenderTargetWindow()
{
    if (!m_window) {
        return;
    }
    m_renderTargetWindow = std::make_shared<render_target::RenderTargetWindowDX>(this);
    m_renderTargetWindow->setWindow(m_window)
        .setFormat(image::ImageFormat::IMAGE_FORMAT_BGRA8)
        .setColorSpace(image::ColorSpace::COLOR_SPACE_SRGB_NON_LINEAR)
        .setDepthBuffer(true)
        .create();
}

void RenderingDeviceDX::createDescriptorHeapManager()
{
    // TODO do something with the size
    m_commonDescriptorHeap = std::make_shared<utils::DescriptorHeapManagerDX>(
        this, 256, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );
    m_commonSamplerHeap = std::make_shared<utils::DescriptorHeapManagerDX>(
        this, 256, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
    );
}

void RenderingDeviceDX::initImGui()
{
    window::GLFWWindow* l_glfwWindow = dynamic_cast<window::GLFWWindow*>(m_window);

    if (!l_glfwWindow) {
        throw std::runtime_error(
            "RenderingDeviceDX::initImGui() currently only GLFW imgui is supported"
        );
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(l_glfwWindow->getNativeHandle(), true);

    ImGui_ImplDX12_InitInfo l_init_info = {};
    l_init_info.Device                  = m_device.Get();
    l_init_info.CommandQueue            = m_commandQueue.Get();
    l_init_info.NumFramesInFlight       = maxFramesInFlight;
    l_init_info.RTVFormat =
        image::ImageDX::convertToDXFormat(m_renderTargetWindow->getFormat());


    l_init_info.DSVFormat            = DXGI_FORMAT_UNKNOWN;
    l_init_info.SrvDescriptorHeap    = m_commonDescriptorHeap->getHeap();
    l_init_info.SrvDescriptorAllocFn = &imguiAllocation;
    l_init_info.SrvDescriptorFreeFn  = &imguiFree;

    ImGui_ImplDX12_Init(&l_init_info);
}

void RenderingDeviceDX::imguiAllocation(
    ImGui_ImplDX12_InitInfo*,
    D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
    D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle
)
{
    RenderingDeviceDX* l_device = dynamic_cast<RenderingDeviceDX*>(
        rendering_server::RenderingServer::getInstance().getMainRenderingDevice()
    );

    auto l_id = l_device->getCommonDescriptorHeapManager()->addEmpty();
    out_cpu_handle->ptr =
        l_device->getCommonDescriptorHeapManager()->getCPUHandle(l_id).ptr;
    out_gpu_handle->ptr =
        l_device->getCommonDescriptorHeapManager()->getGPUHandle(l_id).ptr;
    return;
}

void RenderingDeviceDX::imguiFree(
    ImGui_ImplDX12_InitInfo*,
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
    D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle
)
{
    RenderingDeviceDX* l_device = dynamic_cast<RenderingDeviceDX*>(
        rendering_server::RenderingServer::getInstance().getMainRenderingDevice()
    );

    l_device->getCommonDescriptorHeapManager()->free(cpu_handle, gpu_handle);
}
}   // namespace rendering_device
}   // namespace renderer
