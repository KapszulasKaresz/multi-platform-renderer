#include "renderer/scene/camera/inc/perspective_camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "renderer/rendering_api/inc/rendering_api.hpp"
#include "renderer/rendering_server/inc/rendering_server.hpp"

namespace renderer {
namespace scene {
PerspectiveCamera::PerspectiveCamera() : Camera() {}

glm::mat4 PerspectiveCamera::V() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 PerspectiveCamera::P() const
{
    auto l_projection = glm::perspective(m_fovY, m_aspectRatio, m_nearPlane, m_farPlane);

    renderer::rendering_api::RenderingApi* l_renderingApi =
        renderer::rendering_server::RenderingServer::getInstance().getRenderingApi();

    if (l_renderingApi->getRenderingAPIType()
        == renderer::rendering_api::RENDERING_API_TYPE_VULKAN)
    {
        l_projection[1][1] *= -1;
    }

    return l_projection;
}

PerspectiveCamera& PerspectiveCamera::setPosition(const glm::vec3& f_position)
{
    m_position = f_position;
    return *this;
}

PerspectiveCamera& PerspectiveCamera::setFront(const glm::vec3& f_front)
{
    m_front = glm::normalize(f_front);
    return *this;
}

PerspectiveCamera& PerspectiveCamera::setUp(const glm::vec3& f_up)
{
    m_up = glm::normalize(f_up);
    return *this;
}

PerspectiveCamera& PerspectiveCamera::setFovY(float f_fovY)
{
    m_fovY = f_fovY;
    return *this;
}

PerspectiveCamera& PerspectiveCamera::setAspectRatio(float f_aspectRatio)
{
    m_aspectRatio = f_aspectRatio;
    return *this;
}

PerspectiveCamera& PerspectiveCamera::setNearPlane(float f_nearPlane)
{
    m_nearPlane = f_nearPlane;
    return *this;
}

PerspectiveCamera& PerspectiveCamera::setFarPlane(float f_farPlane)
{
    m_farPlane = f_farPlane;
    return *this;
}

}   // namespace scene
}   // namespace renderer
