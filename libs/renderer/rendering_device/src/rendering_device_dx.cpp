#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

#include <stdexcept>

#include "renderer/rendering_api/inc/rendering_api_dx.hpp"

namespace renderer {
namespace rendering_device {

RenderingDeviceDX::RenderingDeviceDX(rendering_api::RenderingApiDX* f_parentApi)
    : RenderingDevice(),
      m_parentApi(f_parentApi)
{}

std::shared_ptr<render_target::RenderTargetWindow>
    RenderingDeviceDX::getRenderTargetWindow()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<image::Image> RenderingDeviceDX::createImage()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<material::Material> RenderingDeviceDX::createMaterial()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<command_buffer::CommandBuffer> RenderingDeviceDX::createCommandBuffer()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<command_buffer::CommandBuffer>
    RenderingDeviceDX::getRenderingCommandBuffer()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<mesh::TriangleMesh> RenderingDeviceDX::createTriangleMesh()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<uniform::UniformCollection> RenderingDeviceDX::createUniformCollection()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<texture::Texture> RenderingDeviceDX::createTexture()
{
    throw std::logic_error("Function not yet implemented");
}

std::shared_ptr<render_target::RenderTarget> RenderingDeviceDX::createRenderTarget()
{
    throw std::logic_error("Function not yet implemented");
}

bool RenderingDeviceDX::preFrame()
{
    throw std::logic_error("Function not yet implemented");
}

void RenderingDeviceDX::postFrame()
{
    throw std::logic_error("Function not yet implemented");
}

void RenderingDeviceDX::finishRendering()
{
    throw std::logic_error("Function not yet implemented");
}

RenderingDeviceDX& RenderingDeviceDX::create()
{
    m_valid = true;
    return *this;
}

RenderingDeviceDX::~RenderingDeviceDX() {}
}   // namespace rendering_device
}   // namespace renderer
