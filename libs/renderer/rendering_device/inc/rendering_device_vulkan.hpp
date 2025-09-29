#ifndef RENDERING_DEVICE_VULKAN_HPP_INCLUDED
#define RENDERING_DEVICE_VULKAN_HPP_INCLUDED

#include <cstdint>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/rendering_device/inc/rendering_device.hpp"
#include "renderer/utils/inc/raii_vma_allocator.hpp"

namespace renderer {
namespace command_buffer {
class CommandBufferVulkan;
}   // namespace command_buffer

namespace image {
class ImageVulkan;
}   // namespace image

namespace rendering_api {
class RenderingApiVulkan;
}   // namespace rendering_api

namespace render_target {
class RenderTarget;
class RenderTargetWindowVulkan;
}   // namespace render_target

namespace rendering_device {
#define VULKAN_FEATURE_CHAIN                                                    \
    vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,            \
        vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features, \
        vk::PhysicalDeviceVulkan14Features,                                     \
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT

class RenderingDeviceVulkan : public RenderingDevice {
public:
    RenderingDeviceVulkan(rendering_api::RenderingApiVulkan* f_parentApi);

    std::shared_ptr<render_target::RenderTargetWindow>
                                                   getRenderTargetWindow() override final;
    std::shared_ptr<image::Image>                  createImage() override final;
    std::shared_ptr<material::Material>            createMaterial() override final;
    std::shared_ptr<command_buffer::CommandBuffer> createCommandBuffer() override final;
    std::shared_ptr<command_buffer::CommandBuffer> createSingleUseCommandBuffer();
    std::shared_ptr<command_buffer::CommandBuffer>
                                        getRenderingCommandBuffer() override final;
    std::shared_ptr<mesh::TriangleMesh> createTriangleMesh() override final;
    std::shared_ptr<uniform::UniformCollection> createUniformCollection() override final;
    std::shared_ptr<texture::Texture>           createTexture() override final;

    bool preFrame() override final;
    void postFrame() override final;
    void finishRendering() override final;

    using FeatureChain = vk::StructureChain<VULKAN_FEATURE_CHAIN>;

    RenderingDeviceVulkan& addExtension(const char* f_extensionName);
    RenderingDeviceVulkan& addExtensions(std::vector<const char*>& f_extensionNames);
    RenderingDeviceVulkan& addQueue(vk::QueueFlagBits f_queueType);
    RenderingDeviceVulkan& setFeatures(FeatureChain f_featureChain);
    RenderingDeviceVulkan& setWindow(window::Window* f_window) override final;
    RenderingDeviceVulkan& create();

    void submitCommandBuffer(command_buffer::CommandBufferVulkan* f_buffer);
    void submitRenderCommandBuffer(command_buffer::CommandBufferVulkan* f_buffer);

    vk::raii::PhysicalDevice& getPhysicalDevice();
    vk::raii::Device&         getLogicalDevice();
    vk::raii::DescriptorPool& getDescriptorPool();
    VmaAllocator&             getVmaAllocator();

    vk::Format getSwapchainSurfaceFormat() const;
    uint32_t   getCurrentImageIndex() const;

    ~RenderingDeviceVulkan();

private:
    void createRenderTargetWindow();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();
    void createSyncObjects();
    void createVmaAllocator();
    void createDescriptorPool();

    rendering_api::RenderingApiVulkan* m_parentApi{ nullptr };
    vk::raii::PhysicalDevice           m_physicalDevice{ nullptr };
    vk::raii::Device                   m_device{ nullptr };
    vk::raii::Queue                    m_queue{ nullptr };
    utils::RaiiVmaAllocator            m_allocator{};
    vk::raii::DescriptorPool           m_descriptorPool{ nullptr };

    std::vector<const char*>       m_requiredExtension{};
    std::vector<vk::QueueFlagBits> m_requiredQueues{};
    FeatureChain                   m_requiredFeatures{};

    window::Window*                                          m_window{ nullptr };
    std::shared_ptr<render_target::RenderTargetWindowVulkan> m_renderTargetWindow{
        nullptr
    };

    vk::raii::CommandPool                                m_commandPool{ nullptr };
    std::shared_ptr<command_buffer::CommandBufferVulkan> m_renderingCommandBuffer{};

    std::vector<vk::raii::Semaphore> m_presentCompleteSemaphore{};
    std::vector<vk::raii::Semaphore> m_renderFinishedSemaphore{};
    std::vector<vk::raii::Fence>     m_inFlightFences{};

    uint32_t m_queueIndex{ UINT32_MAX };
    uint32_t m_semaphoreIndex{ 0 };
    uint32_t m_currentImageIndex{ 0 };

    uint32_t m_maxDescriptorSets{ 1'000 };
};

}   // namespace rendering_device
}   // namespace renderer

#endif
