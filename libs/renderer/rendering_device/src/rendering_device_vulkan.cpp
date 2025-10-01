#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

#include <array>

#include "renderer/command_buffer/inc/command_buffer_vulkan.hpp"
#include "renderer/image/inc/image_vulkan.hpp"
#include "renderer/material/inc/material_vulkan.hpp"
#include "renderer/mesh/inc/triangle_mesh_vulkan.hpp"
#include "renderer/render_target/inc/render_target_window_vulkan.hpp"
#include "renderer/rendering_api/inc/rendering_api_vulkan.hpp"
#include "renderer/texture/inc/texture_vulkan.hpp"
#include "renderer/uniform/inc/uniform_collection_vulkan.hpp"
#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace rendering_device {
RenderingDeviceVulkan::RenderingDeviceVulkan(
    rendering_api::RenderingApiVulkan* f_parentApi
)
    : RenderingDevice(),
      m_parentApi(f_parentApi)
{}

std::shared_ptr<render_target::RenderTargetWindow>
    RenderingDeviceVulkan::getRenderTargetWindow()
{
    if (!isValid()) {
        throw std::runtime_error(
            "RenderingDeviceVulkan::getRenderTargetWindow() device needs to be valid"
        );
    }

    if (!m_window) {
        throw std::
            runtime_error(
                "RenderingDeviceVulkan::getRenderTargetWindow() needs a window "
                "attached to the device"
            );
    }

    if (!m_renderTargetWindow) {
        throw std::
            runtime_error(
                "RenderingDeviceVulkan::getRenderTargetWindow() no render target window "
                "is " "available"
            );
    }

    return m_renderTargetWindow;
}

std::shared_ptr<image::Image> RenderingDeviceVulkan::createImage()
{
    return std::make_shared<image::ImageVulkan>(this);
}

std::shared_ptr<material::Material> RenderingDeviceVulkan::createMaterial()
{
    return std::make_shared<material::MaterialVulkan>(this);
}

std::shared_ptr<command_buffer::CommandBuffer> RenderingDeviceVulkan::createCommandBuffer()
{
    auto l_commandBuffer =
        std::make_shared<command_buffer::CommandBufferVulkan>(this, m_commandPool);
    l_commandBuffer->create();
    return l_commandBuffer;
}

std::shared_ptr<command_buffer::CommandBuffer>
    RenderingDeviceVulkan::createSingleUseCommandBuffer()
{
    auto l_commandBuffer =
        std::make_shared<command_buffer::CommandBufferVulkan>(this, m_commandPool);
    l_commandBuffer->setUsage(true).create();
    return l_commandBuffer;
}

std::shared_ptr<command_buffer::CommandBuffer>
    RenderingDeviceVulkan::getRenderingCommandBuffer()
{
    if (!m_renderingCommandBuffer) {
        m_renderingCommandBuffer =
            std::make_shared<command_buffer::CommandBufferVulkan>(this, m_commandPool);
        m_renderingCommandBuffer->setBufferCount(m_maxFramesInFlight)
            .setRendering(true)
            .create();
    }
    return m_renderingCommandBuffer;
}

std::shared_ptr<mesh::TriangleMesh> RenderingDeviceVulkan::createTriangleMesh()
{
    return std::make_shared<mesh::TriangleMeshVulkan>(this);
}

std::shared_ptr<uniform::UniformCollection>
    RenderingDeviceVulkan::createUniformCollection()
{
    return std::make_shared<uniform::UniformCollectionVulkan>(this);
}

std::shared_ptr<texture::Texture> RenderingDeviceVulkan::createTexture()
{
    return std::make_shared<texture::TextureVulkan>(this);
}

bool RenderingDeviceVulkan::preFrame()
{
    while (
        vk::Result::eTimeout
        == m_device.waitForFences(*m_inFlightFences[m_currentFrame], vk::True, UINT64_MAX)
    )
        ;

    auto [l_result, l_imageIndex] =
        m_renderTargetWindow->acquireSwapchain().acquireNextImage(
            UINT64_MAX, *m_presentCompleteSemaphore[m_semaphoreIndex], nullptr
        );
    m_currentImageIndex = l_imageIndex;

    if (l_result == vk::Result::eErrorOutOfDateKHR) {
        m_renderTargetWindow->recreateSwapChain();
        return false;
    }
    if (l_result != vk::Result::eSuccess && l_result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error(
            "RenderingDeviceVulkan::preFrame() failed to acquire swap chain image!"
        );
    }
    m_device.resetFences(*m_inFlightFences[m_currentFrame]);

    return true;
}

void RenderingDeviceVulkan::postFrame()
{
    const vk::PresentInfoKHR l_presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &*m_renderFinishedSemaphore[m_currentImageIndex],
        .swapchainCount     = 1,
        .pSwapchains        = &*m_renderTargetWindow->acquireSwapchain(),
        .pImageIndices      = &m_currentImageIndex
    };
    vk::Result l_result;

    try {
        l_result = m_queue.presentKHR(l_presentInfoKHR);
        if (l_result == vk::Result::eErrorOutOfDateKHR
            || l_result == vk::Result::eSuboptimalKHR || m_window->isResized())
        {
            m_window->resizeHandled();
            m_renderTargetWindow->recreateSwapChain();
        }
        else if (l_result != vk::Result::eSuccess) {
            throw std::runtime_error(
                "RenderingDeviceVulkan::postFrame() failed to present swap chain image!"
            );
        }
    } catch (const vk::OutOfDateKHRError&) {
        m_window->resizeHandled();
        m_renderTargetWindow->recreateSwapChain();
    }

    m_semaphoreIndex = (m_semaphoreIndex + 1) % m_presentCompleteSemaphore.size();
    m_currentFrame   = (m_currentFrame + 1) % m_maxFramesInFlight;
}

void RenderingDeviceVulkan::finishRendering()
{
    m_device.waitIdle();
}

RenderingDeviceVulkan& RenderingDeviceVulkan::addExtension(const char* f_extensionName)
{
    if (m_valid) {
        throw std::
            runtime_error(
                "RenderingDeviceVulkan::addExtension(const char* f_extensionName) you "
                "can " "only add extensions prior device creation"
            );
    }
    m_requiredExtension.push_back(f_extensionName);
    return *this;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::addExtensions(
    std::vector<const char*>& f_extensionNames
)
{
    if (m_valid) {
        throw std::runtime_error(
            "RenderingDeviceVulkan::addExtension(std::vector<const char*>& "
            "f_extensionNames) you can " "only add extensions prior device creation"
        );
    }
    m_requiredExtension.insert(
        m_requiredExtension.end(), f_extensionNames.begin(), f_extensionNames.end()
    );
    return *this;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::addQueue(vk::QueueFlagBits f_queueType)
{
    if (m_valid) {
        throw std::
            runtime_error(
                "RenderingDeviceVulkan::addQueue(vk::QueueFlagBits f_queueType) you can "
                "only add queues before device creation"
            );
    }
    m_requiredQueues.push_back(f_queueType);
    return *this;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::setFeatures(FeatureChain f_featureChain)
{
    if (m_valid) {
        throw std::
            runtime_error(
                "RenderingDeviceVulkan::setFeatures(FeatureChain f_featureChain) you can "
                "only set features before device creation"
            );
    }
    m_requiredFeatures = f_featureChain;
    return *this;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::setWindow(window::Window* f_window)
{
    m_window = f_window;
    return *this;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::create()
{
    pickPhysicalDevice();
    createLogicalDevice();
    m_valid = true;
    createVmaAllocator();
    createRenderTargetWindow();
    createDescriptorPool();
    createCommandPool();
    createSyncObjects();
    return *this;
}

void RenderingDeviceVulkan::submitCommandBuffer(
    command_buffer::CommandBufferVulkan* f_buffer
)
{
    auto& l_commandBuffers = f_buffer->getNativeHandle();

    std::vector<vk::CommandBuffer> l_rawCmdBuffers;
    l_rawCmdBuffers.reserve(l_commandBuffers.size());

    for (const auto& cb : l_commandBuffers) {
        l_rawCmdBuffers.push_back(*cb);
    }
    vk::SubmitInfo l_submitInfo{ .commandBufferCount =
                                     static_cast<uint32_t>(l_rawCmdBuffers.size()),
                                 .pCommandBuffers = l_rawCmdBuffers.data() };

    m_queue.submit(l_submitInfo, nullptr);
    m_queue.waitIdle();
}

void RenderingDeviceVulkan::submitRenderCommandBuffer(
    command_buffer::CommandBufferVulkan* f_buffer
)
{
    auto&                  l_buffers = f_buffer->getNativeHandle();
    vk::PipelineStageFlags l_waitDestinationStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    );
    const vk::SubmitInfo l_submitInfo{
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &*m_presentCompleteSemaphore[m_semaphoreIndex],
        .pWaitDstStageMask    = &l_waitDestinationStageMask,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &*l_buffers[m_currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &*m_renderFinishedSemaphore[m_currentImageIndex]
    };
    m_queue.submit(l_submitInfo, *m_inFlightFences[m_currentFrame]);
}

vk::raii::PhysicalDevice& RenderingDeviceVulkan::getPhysicalDevice()
{
    if (!isValid()) {
        throw std::runtime_error(
            "RenderingDeviceVulkan::getPhysicalDevice() device isn't valid"
        );
    }
    return m_physicalDevice;
}

vk::raii::Device& RenderingDeviceVulkan::getLogicalDevice()
{
    if (!isValid()) {
        throw std::runtime_error(
            "RenderingDeviceVulkan::getLogicalDevice() device isn't valid"
        );
    }
    return m_device;
}

vk::raii::DescriptorPool& RenderingDeviceVulkan::getDescriptorPool()
{
    if (!isValid()) {
        throw std::runtime_error(
            "RenderingDeviceVulkan::getDescriptorPool() device isn't valid"
        );
    }
    return m_descriptorPool;
}

VmaAllocator& RenderingDeviceVulkan::getVmaAllocator()
{
    if (!isValid()) {
        throw std::runtime_error(
            "RenderingDeviceVulkan::getVmaAllocator() device isn't valid"
        );
    }

    return m_allocator.get();
}

vk::Format RenderingDeviceVulkan::getSwapchainSurfaceFormat() const
{
    if (!m_renderTargetWindow) {
        return vk::Format::eUndefined;
    }
    return image::ImageVulkan::convertToVkFormat(m_renderTargetWindow->getFormat());
}

uint32_t RenderingDeviceVulkan::getCurrentImageIndex() const
{
    return m_currentImageIndex;
}

void RenderingDeviceVulkan::createRenderTargetWindow()
{
    if (!m_window) {
        return;
    }
    m_renderTargetWindow = std::make_shared<render_target::RenderTargetWindowVulkan>(this);
    m_renderTargetWindow
        ->setSurface(
            m_window->createVulkanSurface(m_parentApi->getNativeHandle()),
            m_parentApi->getNativeHandle()
        )
        .setWindow(m_window)
        .setFormat(image::ImageFormat::IMAGE_FORMAT_BGRA8_SRGB)
        .setColorSpace(image::ColorSpace::COLOR_SPACE_SRGB_NON_LINEAR)
        .setDepthBuffer(true)
        .create();
}

namespace {

template <typename T>
bool satisfies(const T& f_requested, const T& f_supported)
{
    constexpr size_t l_headerSize = offsetof(T, pNext) + sizeof(void*);
    static_assert(std::is_standard_layout_v<T>, "Features must be POD");

    const char* l_reqBytes = reinterpret_cast<const char*>(&f_requested);
    const char* l_supBytes = reinterpret_cast<const char*>(&f_supported);

    const VkBool32* l_reqFlags =
        reinterpret_cast<const VkBool32*>(l_reqBytes + l_headerSize);
    const VkBool32* l_supFlags =
        reinterpret_cast<const VkBool32*>(l_supBytes + l_headerSize);

    size_t l_count = (sizeof(T) - l_headerSize) / sizeof(VkBool32) - 1;
    for (size_t i = 0; i < l_count; i++) {
        if (l_reqFlags[i] && !l_supFlags[i]) {
            return false;
        }
    }
    return true;
}

template <typename... Ts>
bool satisfiesFeatures(
    const vk::StructureChain<Ts...>& f_requested,
    const vk::StructureChain<Ts...>& f_supported
)
{
    return (
        satisfies(f_requested.template get<Ts>(), f_supported.template get<Ts>()) && ...
    );
}
}   // namespace

void RenderingDeviceVulkan::pickPhysicalDevice()
{
    std::vector<vk::raii::PhysicalDevice> l_devices =
        m_parentApi->getNativeHandle().enumeratePhysicalDevices();
    const auto l_devIter = std::ranges::find_if(l_devices, [&](const auto& f_device) {
        // Check if the device supports the Vulkan 1.4 API version
        bool l_supportsVulkan1_4 = f_device.getProperties().apiVersion
                                >= VK_API_VERSION_1_4;

        // Check if any of the queue families support graphics operations
        auto l_queueFamilies = f_device.getQueueFamilyProperties();
        bool l_supportsRequestedQueues =
            std::ranges::all_of(m_requiredQueues, [&](vk::QueueFlagBits f_flag) {
                return std::ranges::any_of(l_queueFamilies, [&](const auto& f_qfp) {
                    return !!(f_qfp.queueFlags & f_flag);
                });
            });
        // Check if all required device extensions are available
        auto l_availableDeviceExtensions = f_device.enumerateDeviceExtensionProperties();
        bool l_supportsAllRequiredExtensions = std::ranges::all_of(
            m_requiredExtension,
            [&l_availableDeviceExtensions](const auto& f_requiredDeviceExtension) {
                return std::ranges::any_of(
                    l_availableDeviceExtensions,
                    [f_requiredDeviceExtension](const auto& f_availableDeviceExtension) {
                        return strcmp(
                                   f_availableDeviceExtension.extensionName,
                                   f_requiredDeviceExtension
                               )
                            == 0;
                    }
                );
            }
        );

        auto l_features = f_device.template getFeatures2<VULKAN_FEATURE_CHAIN>();
        bool l_supportsRequiredFeatures =
            satisfiesFeatures(m_requiredFeatures, l_features);

        return l_supportsVulkan1_4 && l_supportsRequestedQueues
            && l_supportsAllRequiredExtensions && l_supportsRequiredFeatures;
    });
    if (l_devIter != l_devices.end()) {
        m_physicalDevice = *l_devIter;
    }
    else {
        throw std::runtime_error(
            "RenderingDeviceVulkan::pickPhysicalDevice() failed to find a suitable GPU!"
        );
    }

    auto l_msaaSamples = getMaxUsableSampleCount();
    if (m_MaxMSAASamples > static_cast<uint32_t>(l_msaaSamples)) {
        m_MaxMSAASamples = static_cast<uint32_t>(l_msaaSamples);
        m_msaaSamples    = l_msaaSamples;
    }
    else {
        m_msaaSamples = l_msaaSamples;
    }
}

void RenderingDeviceVulkan::createLogicalDevice()
{
    std::vector<vk::QueueFamilyProperties> l_queueFamilyProperties =
        m_physicalDevice.getQueueFamilyProperties();

    vk::QueueFlags l_requiredMask{};
    for (auto l_flag : m_requiredQueues) {
        l_requiredMask |= l_flag;
    }

    for (uint32_t l_qfpIndex = 0; l_qfpIndex < l_queueFamilyProperties.size();
         l_qfpIndex++)
    {
        if ((l_queueFamilyProperties[l_qfpIndex].queueFlags & l_requiredMask)) {
            if (m_renderTargetWindow) {
                if (m_physicalDevice.getSurfaceSupportKHR(
                        l_qfpIndex, m_renderTargetWindow->getSurface()
                    ))
                {
                    m_queueIndex = l_qfpIndex;
                    break;
                }
            }
            else {
                m_queueIndex = l_qfpIndex;
                break;
            }
        }
    }
    if (m_queueIndex == ~0) {
        throw std::
            runtime_error(
                "RenderingDeviceVulkan::createLogicalDevice() Could not find a queue "
                "withe " "the required features"
            );
    }

    // create a Device
    float                     l_queuePriority = 0.0f;
    vk::DeviceQueueCreateInfo l_deviceQueueCreateInfo{ .queueFamilyIndex = m_queueIndex,
                                                       .queueCount       = 1,
                                                       .pQueuePriorities =
                                                           &l_queuePriority };
    vk::DeviceCreateInfo      l_deviceCreateInfo{
             .pNext = &m_requiredFeatures.get<vk::PhysicalDeviceFeatures2>(),
             .queueCreateInfoCount    = 1,
             .pQueueCreateInfos       = &l_deviceQueueCreateInfo,
             .enabledExtensionCount   = static_cast<uint32_t>(m_requiredExtension.size()),
             .ppEnabledExtensionNames = m_requiredExtension.data()
    };

    m_device = vk::raii::Device(m_physicalDevice, l_deviceCreateInfo);
    m_queue  = vk::raii::Queue(m_device, m_queueIndex, 0);
}

void RenderingDeviceVulkan::createCommandPool()
{
    vk::CommandPoolCreateInfo l_poolInfo{
        .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_queueIndex
    };
    m_commandPool = vk::raii::CommandPool(m_device, l_poolInfo);
}

void RenderingDeviceVulkan::createSyncObjects()
{
    m_presentCompleteSemaphore.clear();
    m_renderFinishedSemaphore.clear();
    m_inFlightFences.clear();

    if (m_renderTargetWindow) {
        for (size_t i = 0; i < m_renderTargetWindow->getSwapchainImageCount(); i++) {
            m_presentCompleteSemaphore.emplace_back(m_device, vk::SemaphoreCreateInfo());
            m_renderFinishedSemaphore.emplace_back(m_device, vk::SemaphoreCreateInfo());
        }
    }


    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        m_inFlightFences.emplace_back(
            m_device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled }
        );
    }
}

void RenderingDeviceVulkan::createVmaAllocator()
{
    VmaAllocatorCreateInfo l_allocatorInfo{ .physicalDevice = *m_physicalDevice,
                                            .device         = *m_device,
                                            .instance = *m_parentApi->getNativeHandle() };

    m_allocator = utils::RaiiVmaAllocator(l_allocatorInfo);
}

void RenderingDeviceVulkan::createDescriptorPool()
{
    std::array l_poolSize = {
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, m_maxFramesInFlight * 2),
        vk::DescriptorPoolSize(
            vk::DescriptorType::eCombinedImageSampler, m_maxFramesInFlight * 2
        )
    };
    vk::DescriptorPoolCreateInfo l_poolInfo{
        .flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets       = m_maxDescriptorSets * m_maxFramesInFlight,
        .poolSizeCount = l_poolSize.size(),
        .pPoolSizes    = l_poolSize.data()
    };
    m_descriptorPool = vk::raii::DescriptorPool(m_device, l_poolInfo);
}

vk::SampleCountFlagBits RenderingDeviceVulkan::getMaxUsableSampleCount()
{
    vk::PhysicalDeviceProperties l_physicalDeviceProperties =
        m_physicalDevice.getProperties();

    vk::SampleCountFlags l_counts =
        l_physicalDeviceProperties.limits.framebufferColorSampleCounts
        & l_physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (l_counts & vk::SampleCountFlagBits::e64) {
        return vk::SampleCountFlagBits::e64;
    }
    if (l_counts & vk::SampleCountFlagBits::e32) {
        return vk::SampleCountFlagBits::e32;
    }
    if (l_counts & vk::SampleCountFlagBits::e16) {
        return vk::SampleCountFlagBits::e16;
    }
    if (l_counts & vk::SampleCountFlagBits::e8) {
        return vk::SampleCountFlagBits::e8;
    }
    if (l_counts & vk::SampleCountFlagBits::e4) {
        return vk::SampleCountFlagBits::e4;
    }
    if (l_counts & vk::SampleCountFlagBits::e2) {
        return vk::SampleCountFlagBits::e2;
    }

    return vk::SampleCountFlagBits::e1;
}

RenderingDeviceVulkan::~RenderingDeviceVulkan() {}

}   // namespace rendering_device
}   // namespace renderer
