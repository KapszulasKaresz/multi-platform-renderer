#include "renderer/scene/inc/node_3d.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace renderer {
namespace scene {
Node3D::Node3D() : Node()
{
    m_name = "Node3D " + std::to_string(m_id);
    updateTransform();
}

Node3D& Node3D::create()
{
    m_valid = true;
    return *this;
}

void Node3D::applyVisitor(NodeVisitor* f_visitor)
{
    if ((f_visitor->getMask() & m_visitorMask) != 0) {
        f_visitor->visit(*this);
    }
}

glm::mat4 Node3D::getTransform() const
{
    if (m_parent && dynamic_cast<Node3D*>(m_parent)) {
        auto parentTransform = static_cast<Node3D*>(m_parent)->getTransform();
        return parentTransform * m_transform;
    }
    return m_transform;
}

Node3D& Node3D::setTransform(const glm::mat4& f_transform)
{
    m_transform = f_transform;

    glm::vec3 l_skew;
    glm::vec4 l_perspective;
    glm::decompose(m_transform, m_scale, m_rotation, m_translation, l_skew, l_perspective);

    return *this;
}

Node3D& Node3D::setTransform(
    const glm::vec3& f_translation,
    const glm::quat& f_rotation,
    const glm::vec3& f_scale
)
{
    m_translation = f_translation;
    m_rotation    = f_rotation;
    m_scale       = f_scale;
    updateTransform();

    return *this;
}

Node3D& Node3D::setRotation(const glm::quat& f_rotation)
{
    m_rotation = f_rotation;
    updateTransform();
    return *this;
}

Node3D& Node3D::setTranslation(const glm::vec3& f_translation)
{
    m_translation = f_translation;
    updateTransform();
    return *this;
}

Node3D& Node3D::setScale(const glm::vec3& f_scale)
{
    m_scale = f_scale;
    updateTransform();
    return *this;
}

void Node3D::updateTransform()
{
    m_transform        = glm::mat4{ 1.0f };
    m_transform        = glm::translate(m_transform, m_translation);
    m_transform        = m_transform * glm::toMat4(m_rotation);
    m_transform        = glm::scale(m_transform, m_scale);
    m_inverseTransform = glm::inverse(m_transform);
}

}   // namespace scene
}   // namespace renderer
